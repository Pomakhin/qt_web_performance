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

#include "font.h"

#include <limits.h>

#include <QFontMetrics>
#include <QTextLine>

// See WebCore/platform/graphics/qt/FontQt.cpp for the origin of the code
static bool treatAsSpace(QChar c) { return c == ' ' || c == '\t' || c == '\n' || c == 0x00A0; }
static QTextLine setupLayout(QTextLayout* layout, bool isRtl, int padding)
{
    int flags = isRtl ? Qt::TextForceRightToLeft : Qt::TextForceLeftToRight;
    if (padding)
        flags |= Qt::TextJustificationForced;
    layout->setFlags(flags);
    layout->beginLayout();
    QTextLine line = layout->createLine();
    line.setLineWidth(INT_MAX/256);
    if (padding)
        line.setLineWidth(line.naturalTextWidth() + padding);
    layout->endLayout();
    return line;
}

float floatWidth(const QFont& font, const QString& text, bool isRtl, int padding, int wordSpacing)
{
    if (!text.length())
        return 0;

    QTextLayout layout(text, font);
    QTextLine line = setupLayout(&layout, isRtl, padding);
    int w = int(line.naturalTextWidth());
    // WebKit expects us to ignore word spacing on the first character (as opposed to what Qt does)
    if (treatAsSpace(text[0]))
        w -= wordSpacing;

    return w + padding;
}

float floatWidth_fasta(const QFont& font, const QString& text, bool isRtl, int padding, int wordSpacing)
{
    if (!text.length())
        return 0;

    if (text.length() == 1 && treatAsSpace(text[0]))
        return QFontMetrics(font).width(text[0]) - wordSpacing + padding;

    QTextLayout layout(text, font);
    QTextLine line = setupLayout(&layout, isRtl, padding);
    int w = int(line.naturalTextWidth());
    // WebKit expects us to ignore word spacing on the first character (as opposed to what Qt does)
    if (treatAsSpace(text[0]))
        w -= wordSpacing;

    return w + padding;
}

