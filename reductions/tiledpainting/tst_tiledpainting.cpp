/*
 * Copyright (C) 2010 Viatcheslav Ostapenko
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

#include <QtTest/QtTest>

#include "common_init.h"
#include "benchmark.h"
#include "databasenetworkaccessmanager.h"
#include "databasetests.h"
#include "webpage.h"

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qwebview.h>
#include <qpaintengine.h>
#include <qpainter.h>

class tst_TiledPainting : public QObject
{
    Q_OBJECT

public:
    ~tst_TiledPainting();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void qPixmapTiledPaint_data();
    void qPixmapTiledPaint();

    void qImageTiledPaint_data();
    void qImageTiledPaint();

    void noOpTiledPaint_data();
    void noOpTiledPaint();
private:
    QWebView* m_view;
    void doTilePaint(QPainter* painter, const QSize& contentSize,
                     qreal scale, const QString& testName,  const QUrl& url,
                     bool needToSync);
};

tst_TiledPainting::~tst_TiledPainting()
{
    benchmarkOutput();
}

void tst_TiledPainting::init()
{
    m_view = new QWebView;
    WebPage* page = new WebPage(m_view);
    m_view->setPage(page);

#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    const QSize viewportSize(1024, 768);
    page->setPreferredContentsSize(viewportSize);
    m_view->showFullScreen();
    m_view->window()->raise();
#else
    m_view->showMaximized();
#endif

    if (QSqlDatabase::database().isValid())
        page->setNetworkAccessManager(new DatabaseNetworkAccessManager);
}

void tst_TiledPainting::cleanup()
{
    delete m_view;
}

static const int minTile = 32;
static const int maxTile = 256;

void tst_TiledPainting::doTilePaint(QPainter* painter, const QSize& contentSize,
                                    qreal scale, const QString& testName, const QUrl& url,
                                    bool needToSync)
{
    QString cTestName;
    painter->save();
    painter->scale(scale, scale);

    for(int tileSize = minTile; tileSize <= maxTile; tileSize *= 2) {
        qreal scaledTileSize = tileSize / scale;
        int xTiles = (contentSize.width() + scaledTileSize - 1) / scaledTileSize;
        int yTiles = (contentSize.height() + scaledTileSize - 1) / scaledTileSize;
        int tileCount = xTiles * yTiles;
        cTestName = testName + ", tile size " + QString::number(tileSize) +
                               ", tile count " + QString::number(tileCount);

        WEB_BENCHMARK(cTestName, url.toString()) {
            for(qreal ypos = 0; ypos < contentSize.height() + scaledTileSize - 1; ypos += scaledTileSize) {
                for(qreal xpos = 0; xpos < contentSize.width() + scaledTileSize - 1; xpos += scaledTileSize) {
                    painter->save();
                    painter->translate(-xpos, -ypos);
                    m_view->page()->mainFrame()->render(painter,
                                        QRegion(xpos, ypos, scaledTileSize, scaledTileSize));
#if defined(Q_WS_X11)
                    // force badness... to have some reliable result on non raster..
                    // this will make the result "unreal" in some ways but might be
                    // better than just using the raster engine.
                    if (needToSync)
                        QApplication::syncX();
#endif
                    painter->restore();
                }
            }
        }
    }

    painter->restore();
}


void tst_TiledPainting::qPixmapTiledPaint_data()
{
    add_test_urls();
}

void tst_TiledPainting::qPixmapTiledPaint()
{
    QFETCH(QUrl, url);

    m_view->load(url);
    ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

    /* force a layout */
    QWebFrame* mainFrame = m_view->page()->mainFrame();
    mainFrame->toPlainText();

    QSize contentSize = m_view->page()->mainFrame()->contentsSize();
    QPixmap pixmap(maxTile, maxTile);
    if (pixmap.isNull())
        QSKIP("Invalid pixmap", SkipSingle);

    QPainter painter(&pixmap);
    if (!painter.paintEngine() || !painter.paintEngine()->paintDevice())
        QSKIP("Invalid engine for pixmap, probably short on memory", SkipSingle);

#if defined(Q_WS_X11)
    const bool needToSync = pixmap.paintEngine()->type() != QPaintEngine::Raster;
#endif

    for(qreal scale = 0.4; scale < 1.7; scale += 0.6 ) {
        QString tst;
        tst.sprintf("TiledPainting::QPixmapTiledPaint scale %f", scale);
        doTilePaint(&painter, contentSize, scale, tst, url, needToSync);
    }
}

void tst_TiledPainting::qImageTiledPaint_data()
{
    add_test_urls();
}

void tst_TiledPainting::qImageTiledPaint()
{
    QFETCH(QUrl, url);

    m_view->load(url);
    ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

    /* force a layout */
    QWebFrame* mainFrame = m_view->page()->mainFrame();
    mainFrame->toPlainText();

    QSize contentSize = m_view->page()->mainFrame()->contentsSize();
    QImage image(maxTile, maxTile, QImage::Format_ARGB32_Premultiplied);
    if (image.isNull())
        QSKIP("Invalid image", SkipSingle);

    QPainter painter(&image);
    if (!painter.paintEngine() || !painter.paintEngine()->paintDevice())
        QSKIP("Invalid engine for image, probably short on memory", SkipSingle);

    for(qreal scale = 0.4; scale < 1.7; scale += 0.6 ) {
        QString tst;
        tst.sprintf("TiledPainting::QImageTiledPaint %f", scale);
        doTilePaint(&painter, contentSize, scale, tst, url, false);
    }
}


class NoOpPaintEngine : public QPaintEngine {
public:
    NoOpPaintEngine( PaintEngineFeatures caps = 0 ) : m_paintDevice(0), m_active(0) {}
    virtual ~NoOpPaintEngine() {}
    virtual bool begin ( QPaintDevice * pdev ) { m_paintDevice = pdev; return true; }
    virtual void drawEllipse ( const QRectF & rect ) {}
    virtual void drawEllipse ( const QRect & rect ) {}
    virtual void drawImage ( const QRectF & rectangle, const QImage & image, const QRectF & sr, Qt::ImageConversionFlags flags = Qt::AutoColor ) {}
    virtual void drawLines ( const QLineF * lines, int lineCount ) {}
    virtual void drawLines ( const QLine * lines, int lineCount ) {}
    virtual void drawPath ( const QPainterPath & path ) {}
    virtual void drawPixmap ( const QRectF & r, const QPixmap & pm, const QRectF & sr ) {}
    virtual void drawPoints ( const QPointF * points, int pointCount ) {}
    virtual void drawPoints ( const QPoint * points, int pointCount ) {}
    virtual void drawPolygon ( const QPointF * points, int pointCount, PolygonDrawMode mode ) {}
    virtual void drawPolygon ( const QPoint * points, int pointCount, PolygonDrawMode mode ) {}
    virtual void drawRects ( const QRectF * rects, int rectCount ) {}
    virtual void drawRects ( const QRect * rects, int rectCount ) {}
    virtual void drawTextItem ( const QPointF & p, const QTextItem & textItem ) {}
    virtual void drawTiledPixmap ( const QRectF & rect, const QPixmap & pixmap, const QPointF & p ) {}
    virtual bool end () { m_paintDevice = 0; }
    bool hasFeature ( PaintEngineFeatures feature ) const { return true; }
    bool isActive () const { return m_active; }
    QPaintDevice*  paintDevice () const { return m_paintDevice; }
    QPainter* painter () const { return 0; }
    void setActive ( bool state ) { m_active = state; }
    virtual Type type () const { return QPaintEngine::User; }
    virtual void updateState ( const QPaintEngineState & state ) { }
    QPaintDevice* m_paintDevice;
    bool m_active;
};

static NoOpPaintEngine sNoOpEngine(QPaintEngine::AllFeatures);

extern int qt_defaultDpiX();
extern int qt_defaultDpiY();

class NoOpPaintDevice : public QPaintDevice
{
public:
    NoOpPaintDevice(int width, int height) : m_width(width), m_height(height) {}
    virtual ~NoOpPaintDevice() {}

    virtual QPaintEngine* paintEngine () const { return &sNoOpEngine; }
    virtual int metric ( PaintDeviceMetric metric ) const
    {
        switch(metric) {
            case PdmWidth :
                return m_width;
            case PdmHeight :
                return m_height;
            case PdmWidthMM :
                return qRound(m_width * 25.4 / qt_defaultDpiX());
            case PdmHeightMM :
                return qRound(m_height * 25.4 / qt_defaultDpiY());
            case PdmNumColors :
                return INT_MAX;
            case PdmDepth :
                return 32;
            case PdmDpiX :
            case PdmPhysicalDpiX :
                return qt_defaultDpiX();
            case PdmDpiY :
            case PdmPhysicalDpiY :
                return qt_defaultDpiY();
            default:
                qWarning("QRasterPixmapData::metric(): Unhandled metric type %d", metric);
                break;
        }
    }
    int m_width;
    int m_height;
};

void tst_TiledPainting::noOpTiledPaint_data()
{
    add_test_urls();
}

void tst_TiledPainting::noOpTiledPaint()
{
    QFETCH(QUrl, url);

    m_view->load(url);
    ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

    /* force a layout */
    QWebFrame* mainFrame = m_view->page()->mainFrame();
    mainFrame->toPlainText();

    QSize contentSize = m_view->page()->mainFrame()->contentsSize();
    NoOpPaintDevice image(maxTile, maxTile);

    QPainter painter(&image);
    if (!painter.paintEngine() || !painter.paintEngine()->paintDevice())
        QSKIP("Invalid engine for image, probably short on memory", SkipSingle);

    doTilePaint(&painter, contentSize, 1, "TiledPainting::NoOpTiledPaint", url, false);
}

DBWEBTEST_MAIN(tst_TiledPainting)
#include "tst_tiledpainting.moc"
