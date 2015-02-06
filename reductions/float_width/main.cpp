/*
* Copyright (C) 2010 Holger Hans Peter Freyther
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "benchmark.h"
#include "font.h"

#include <QtTest/QtTest>
#include <QDebug>
#include <QFile>
#include <QDataStream>


struct Text {
    int font_id;
    short letterSpacing, wordSpacing;
    QString string;
    bool is_rtl;
    int padding;

    int expected_result;
};

struct Font {
    int id;
    QString family;
    int point_size;
    int weight;
    bool italic;

    QFont toQFont() const
    {
        return QFont(family, point_size, weight, italic);
    }
};

QDataStream& operator>>(QDataStream& stream, Text& text)
{
    stream >> text.font_id;
    stream >> text.letterSpacing;
    stream >> text.wordSpacing;
    stream >> text.string;
    stream >> text.is_rtl;
    stream >> text.padding;
    stream >> text.expected_result;

    return stream;
}

QDataStream& operator>>(QDataStream& stream, Font& font)
{
    stream >> font.id;
    stream >> font.family;
    stream >> font.point_size;
    stream >> font.weight;
    stream >> font.italic;

    return stream;
}

class tst_FloatWidth : public QObject {
    Q_OBJECT
public:
    tst_FloatWidth();
    ~tst_FloatWidth();

private Q_SLOTS:
    void floatWidth_western();
    void floatWidth_western_fasta();
    void floatWidth_western_compare();
//    void floatWidth_korean();
//    void floatWidth_japanese();

private:
    QVector<QFont> western_fonts;
    QList<Text> western_data;
};

tst_FloatWidth::~tst_FloatWidth()
{
    benchmarkOutput();
}

static void loadData(const QString& fileName, QVector<QFont>& fonts, QList<Text>& data)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qFatal("Failed to find '%s'", qPrintable(fileName));
    }

    QDataStream stream(&file);
    int type;

    while (!stream.atEnd()) {
        stream >> type;
        if (type == 1) {
            Font font;
            stream >> font;
            fonts.push_back(font.toQFont());
        } else if (type == 2) {
            Text text;
            stream >> text;
            data.append(text);
        } else {
            qFatal("Wrong file format");
        }
    }
}

tst_FloatWidth::tst_FloatWidth()
{
    loadData("text_data_western", western_fonts, western_data);
    qWarning("Loaded: %d fonts and %d text snippets", western_fonts.size(), western_data.size());
}

void tst_FloatWidth::floatWidth_western()
{
    WEB_BENCHMARK("floatWidth_western", "floatWidth_western") {
        foreach(const Text& text, western_data) {
            QFont font = western_fonts.at(text.font_id - 1);
            font.setWordSpacing(text.wordSpacing);
            font.setLetterSpacing(QFont::AbsoluteSpacing, text.letterSpacing);
            floatWidth(font, text.string, text.is_rtl, text.padding, text.wordSpacing);
        }
    }
}

void tst_FloatWidth::floatWidth_western_fasta()
{
    WEB_BENCHMARK("floatWidth_western_fasta", "floatWidth_western_fasta") {
        foreach(const Text& text, western_data) {
            QFont font = western_fonts.at(text.font_id - 1);
            font.setWordSpacing(text.wordSpacing);
            font.setLetterSpacing(QFont::AbsoluteSpacing, text.letterSpacing);
            floatWidth_fasta(font, text.string, text.is_rtl, text.padding, text.wordSpacing);
        }
    }
}

void tst_FloatWidth::floatWidth_western_compare()
{
    foreach(const Text& text, western_data) {
        QFont font = western_fonts.at(text.font_id - 1);
        font.setWordSpacing(text.wordSpacing);
        font.setLetterSpacing(QFont::AbsoluteSpacing, text.letterSpacing);
        QCOMPARE(floatWidth_fasta(font, text.string, text.is_rtl, text.padding, text.wordSpacing),
                 floatWidth(font, text.string, text.is_rtl, text.padding, text.wordSpacing));
    }
}

QTEST_MAIN(tst_FloatWidth)
#include "main.moc"
