#include <QtTest/QtTest>

class tst_font_usage : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testQFont();
};

void tst_font_usage::testQFont()
{
    QBENCHMARK_ONCE {
        QFont font0;
        font0.setFamily(QString::fromUtf8("Arial"));
        font0.setPixelSize(11);
        font0.setItalic(0);
        font0.setWeight(50);
        font0.setWordSpacing(0);
        font0.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font0.setCapitalization(QFont::MixedCase);
        font0.bold();
        font0.pointSizeF();
        { QFontMetrics fm(font0); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font0.fixedPitch();
        { QFontMetrics fm(font0); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font1;
        font1.setFamily(QString::fromUtf8("Sans Serif"));
        font1.setPixelSize(11);
        font1.setItalic(0);
        font1.setWeight(75);
        font1.setWordSpacing(0);
        font1.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font1.setCapitalization(QFont::MixedCase);
        font1.bold();
        font1.pointSizeF();
        { QFontMetrics fm(font1); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font1.fixedPitch();
        { QFontMetrics fm(font1); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font2;
        font2.setFamily(QString::fromUtf8("Sans Serif"));
        font2.setPixelSize(11);
        font2.setItalic(0);
        font2.setWeight(50);
        font2.setWordSpacing(0);
        font2.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font2.setCapitalization(QFont::MixedCase);
        font2.bold();
        font2.pointSizeF();
        { QFontMetrics fm(font2); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font2.fixedPitch();
        { QFontMetrics fm(font2); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font3;
        font3.setFamily(QString::fromUtf8("Arial"));
        font3.setPixelSize(24);
        font3.setItalic(0);
        font3.setWeight(50);
        font3.setWordSpacing(0);
        font3.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font3.setCapitalization(QFont::MixedCase);
        font3.bold();
        font3.pointSizeF();
        { QFontMetrics fm(font3); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font3.fixedPitch();
        { QFontMetrics fm(font3); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font4;
        font4.setFamily(QString::fromUtf8("Arial"));
        font4.setPixelSize(12);
        font4.setItalic(0);
        font4.setWeight(50);
        font4.setWordSpacing(0);
        font4.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font4.setCapitalization(QFont::MixedCase);
        font4.bold();
        font4.pointSizeF();
        { QFontMetrics fm(font4); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font4.fixedPitch();
        { QFontMetrics fm(font4); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font5;
        font5.setFamily(QString::fromUtf8("Arial"));
        font5.setPixelSize(13);
        font5.setItalic(0);
        font5.setWeight(50);
        font5.setWordSpacing(0);
        font5.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font5.setCapitalization(QFont::MixedCase);
        font5.bold();
        font5.pointSizeF();
        { QFontMetrics fm(font5); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font5.fixedPitch();
        { QFontMetrics fm(font5); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font6;
        font6.setFamily(QString::fromUtf8("Arial"));
        font6.setPixelSize(13);
        font6.setItalic(1);
        font6.setWeight(50);
        font6.setWordSpacing(0);
        font6.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font6.setCapitalization(QFont::MixedCase);
        font6.bold();
        font6.pointSizeF();
        { QFontMetrics fm(font6); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font6.fixedPitch();
        { QFontMetrics fm(font6); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font7;
        font7.setFamily(QString::fromUtf8("Arial"));
        font7.setPixelSize(11);
        font7.setItalic(0);
        font7.setWeight(75);
        font7.setWordSpacing(0);
        font7.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font7.setCapitalization(QFont::MixedCase);
        font7.bold();
        font7.pointSizeF();
        { QFontMetrics fm(font7); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font7.fixedPitch();
        { QFontMetrics fm(font7); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font8;
        font8.setFamily(QString::fromUtf8("Arial"));
        font8.setPixelSize(9);
        font8.setItalic(0);
        font8.setWeight(50);
        font8.setWordSpacing(0);
        font8.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font8.setCapitalization(QFont::MixedCase);
        font8.bold();
        font8.pointSizeF();
        { QFontMetrics fm(font8); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font8.fixedPitch();
        { QFontMetrics fm(font8); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font9;
        font9.setFamily(QString::fromUtf8("Arial"));
        font9.setPixelSize(7);
        font9.setItalic(0);
        font9.setWeight(50);
        font9.setWordSpacing(0);
        font9.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font9.setCapitalization(QFont::MixedCase);
        font9.bold();
        font9.pointSizeF();
        { QFontMetrics fm(font9); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font9.fixedPitch();
        { QFontMetrics fm(font9); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font10;
        font10.setFamily(QString::fromUtf8("Arial"));
        font10.setPixelSize(13);
        font10.setItalic(0);
        font10.setWeight(75);
        font10.setWordSpacing(0);
        font10.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font10.setCapitalization(QFont::MixedCase);
        font10.bold();
        font10.pointSizeF();
        { QFontMetrics fm(font10); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font10.fixedPitch();
        { QFontMetrics fm(font10); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font11;
        font11.setFamily(QString::fromUtf8("Arial"));
        font11.setPixelSize(12);
        font11.setItalic(0);
        font11.setWeight(75);
        font11.setWordSpacing(0);
        font11.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font11.setCapitalization(QFont::MixedCase);
        font11.bold();
        font11.pointSizeF();
        { QFontMetrics fm(font11); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font11.fixedPitch();
        { QFontMetrics fm(font11); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font12;
        font12.setFamily(QString::fromUtf8("Arial"));
        font12.setPixelSize(19);
        font12.setItalic(0);
        font12.setWeight(50);
        font12.setWordSpacing(0);
        font12.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font12.setCapitalization(QFont::MixedCase);
        font12.bold();
        font12.pointSizeF();
        { QFontMetrics fm(font12); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font12.fixedPitch();
        { QFontMetrics fm(font12); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font13;
        font13.setFamily(QString::fromUtf8("Arial"));
        font13.setPixelSize(10);
        font13.setItalic(0);
        font13.setWeight(50);
        font13.setWordSpacing(0);
        font13.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font13.setCapitalization(QFont::MixedCase);
        font13.bold();
        font13.pointSizeF();
        { QFontMetrics fm(font13); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font13.fixedPitch();
        { QFontMetrics fm(font13); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font14;
        font14.setFamily(QString::fromUtf8("Arial"));
        font14.setPixelSize(13);
        font14.setItalic(1);
        font14.setWeight(75);
        font14.setWordSpacing(0);
        font14.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font14.setCapitalization(QFont::MixedCase);
        font14.bold();
        font14.pointSizeF();
        { QFontMetrics fm(font14); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font14.fixedPitch();
        { QFontMetrics fm(font14); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font15;
        font15.setFamily(QString::fromUtf8("Times New Roman"));
        font15.setPixelSize(16);
        font15.setItalic(1);
        font15.setWeight(50);
        font15.setWordSpacing(0);
        font15.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font15.setCapitalization(QFont::MixedCase);
        font15.bold();
        font15.pointSizeF();
        { QFontMetrics fm(font15); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font15.fixedPitch();
        { QFontMetrics fm(font15); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font16;
        font16.setFamily(QString::fromUtf8("Times New Roman"));
        font16.setPixelSize(13);
        font16.setItalic(1);
        font16.setWeight(50);
        font16.setWordSpacing(0);
        font16.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font16.setCapitalization(QFont::MixedCase);
        font16.bold();
        font16.pointSizeF();
        { QFontMetrics fm(font16); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font16.fixedPitch();
        { QFontMetrics fm(font16); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font17;
        font17.setFamily(QString::fromUtf8("Times New Roman"));
        font17.setPixelSize(16);
        font17.setItalic(0);
        font17.setWeight(50);
        font17.setWordSpacing(0);
        font17.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font17.setCapitalization(QFont::MixedCase);
        font17.bold();
        font17.pointSizeF();
        { QFontMetrics fm(font17); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font17.fixedPitch();
        { QFontMetrics fm(font17); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font18;
        font18.setFamily(QString::fromUtf8("Times New Roman"));
        font18.setPixelSize(13);
        font18.setItalic(0);
        font18.setWeight(50);
        font18.setWordSpacing(0);
        font18.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font18.setCapitalization(QFont::MixedCase);
        font18.bold();
        font18.pointSizeF();
        { QFontMetrics fm(font18); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font18.fixedPitch();
        { QFontMetrics fm(font18); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font19;
        font19.setFamily(QString::fromUtf8("Arial"));
        font19.setPixelSize(17);
        font19.setItalic(0);
        font19.setWeight(75);
        font19.setWordSpacing(0);
        font19.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font19.setCapitalization(QFont::MixedCase);
        font19.bold();
        font19.pointSizeF();
        { QFontMetrics fm(font19); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font19.fixedPitch();
        { QFontMetrics fm(font19); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font20;
        font20.setFamily(QString::fromUtf8("Arial"));
        font20.setPixelSize(17);
        font20.setItalic(1);
        font20.setWeight(75);
        font20.setWordSpacing(0);
        font20.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font20.setCapitalization(QFont::MixedCase);
        font20.bold();
        font20.pointSizeF();
        { QFontMetrics fm(font20); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font20.fixedPitch();
        { QFontMetrics fm(font20); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font21;
        font21.setFamily(QString::fromUtf8("Arial"));
        font21.setPixelSize(15);
        font21.setItalic(0);
        font21.setWeight(75);
        font21.setWordSpacing(0);
        font21.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font21.setCapitalization(QFont::MixedCase);
        font21.bold();
        font21.pointSizeF();
        { QFontMetrics fm(font21); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font21.fixedPitch();
        { QFontMetrics fm(font21); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font22;
        font22.setFamily(QString::fromUtf8("Arial"));
        font22.setPixelSize(11);
        font22.setItalic(1);
        font22.setWeight(50);
        font22.setWordSpacing(0);
        font22.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font22.setCapitalization(QFont::MixedCase);
        font22.bold();
        font22.pointSizeF();
        { QFontMetrics fm(font22); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font22.fixedPitch();
        { QFontMetrics fm(font22); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font23;
        font23.setFamily(QString::fromUtf8("Arial"));
        font23.setPixelSize(10);
        font23.setItalic(1);
        font23.setWeight(75);
        font23.setWordSpacing(0);
        font23.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font23.setCapitalization(QFont::MixedCase);
        font23.bold();
        font23.pointSizeF();
        { QFontMetrics fm(font23); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font23.fixedPitch();
        { QFontMetrics fm(font23); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font24;
        font24.setFamily(QString::fromUtf8("Arial"));
        font24.setPixelSize(10);
        font24.setItalic(1);
        font24.setWeight(50);
        font24.setWordSpacing(0);
        font24.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font24.setCapitalization(QFont::MixedCase);
        font24.bold();
        font24.pointSizeF();
        { QFontMetrics fm(font24); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font24.fixedPitch();
        { QFontMetrics fm(font24); fm.averageCharWidth(); fm.maxWidth(); }
        QFont font25;
        font25.setFamily(QString::fromUtf8("Arial"));
        font25.setPixelSize(11);
        font25.setItalic(1);
        font25.setWeight(75);
        font25.setWordSpacing(0);
        font25.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font25.setCapitalization(QFont::MixedCase);
        font25.bold();
        font25.pointSizeF();
        { QFontMetrics fm(font25); fm.ascent(); fm.descent(); fm.lineSpacing(); fm.xHeight(); fm.width(QLatin1Char(' ')); fm.leading(); }
        font25.fixedPitch();
        { QFontMetrics fm(font25); fm.averageCharWidth(); fm.maxWidth(); }
    }
}

QTEST_MAIN(tst_font_usage)
#include "font_usage.moc"
