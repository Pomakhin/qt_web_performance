/*
 * Copyright (C) 2010 Viatcheslav Ostapenko, Nokia
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

#include <qwebframe.h>
#include <qgraphicsview.h>
#include <qgraphicswebview.h>
#include <qpainter.h>

#include <QTestEventList>

class JavaInterface : public QObject
{
    Q_OBJECT
};

class CJavaInterfaceRoot: public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(JavaInterface* browser MEMBER m_pObj)

    JavaInterface* m_pObj;
};

struct PaintingGraphicsWebViewBench : public QGraphicsWebView
{
    Q_OBJECT
public:
    PaintingGraphicsWebViewBench() : m_controller(0) {}

    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w)
    {
        if(m_controller) {
            m_controller->startSubMeasure();
            QGraphicsWebView::paint(p, o, w);
            m_controller->stopSubMeasure();
        } else {
            QGraphicsWebView::paint(p, o, w);
        }
        emit painted();
    }

    void setController(SubSectionBenchmarkController* controller)
    {
        m_controller = controller;
    }
signals:
    void painted();

private:
    SubSectionBenchmarkController* m_controller;
};

class tst_GraphicsScrolling : public QObject
{
    Q_OBJECT

public:
    ~tst_GraphicsScrolling();

public Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void onJavaInited();

private Q_SLOTS:
    void scroll_data();
    void scroll();
    void paintingSpeed_data();
    void paintingSpeed();

private:
    PaintingGraphicsWebViewBench* m_view;
    WebPage* m_page;
    QNetworkAccessManager* m_networkAccessManager;
    QGraphicsView* m_gView;
    QGraphicsScene* m_scene;
    CJavaInterfaceRoot  m_scriptRoot;
};

tst_GraphicsScrolling::~tst_GraphicsScrolling()
{
    benchmarkOutput();
}

void tst_GraphicsScrolling::initTestCase()
{
    if (QSqlDatabase::database().isValid())
        m_networkAccessManager = new DatabaseNetworkAccessManager;
    else
        m_networkAccessManager = 0;
}

void tst_GraphicsScrolling::onJavaInited()
{
    m_scriptRoot.m_pObj = new JavaInterface();
    m_page->mainFrame()->addToJavaScriptWindowObject("gilauncher", &m_scriptRoot);
}

void tst_GraphicsScrolling::init()
{
    m_scene = new QGraphicsScene();
    m_gView = new QGraphicsView(m_scene);
    m_view = new PaintingGraphicsWebViewBench();
    m_page = new WebPage(m_view);
    m_view->setPage(m_page);
    m_scene->addItem(m_view);

    connect(m_page->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(onJavaInited()));

    QWebSettings::globalSettings()->setMaximumPagesInCache(10);
    QWebSettings::globalSettings()->setObjectCacheCapacities(8*1024*1024, 16*1024*1024, 24*1024*1024);

    //QWebSettings::globalSettings()->setAttribute(QWebSettings::TiledBackingStoreEnabled, true);
//    QWebSettings::globalSettings()->setAttribute(QWebSettings::FrameFlatteningEnabled, true);
//    QWebSettings::globalSettings()->setAttribute(QWebSettings::ScrollAnimatorEnabled, true);

//    m_gView->setOptimizationFlag(QGraphicsView::DontSavePainterState , true);
//    m_gView->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
//    m_gView->setOptimizationFlag(QGraphicsView::IndirectPainting, true);

//    m_gView->setRenderHint(QPainter::TextAntialiasing, true);
//    m_gView->setRenderHint(QPainter::Antialiasing, true);
//    m_gView->setRenderHint(QPainter::SmoothPixmapTransform, true);
//    m_gView->setRenderHint(QPainter::HighQualityAntialiasing, true);
//    m_gView->setRenderHint(QPainter::NonCosmeticDefaultPen, true);

//    m_gView->setCacheMode(QGraphicsView::CacheBackground);

    const QSize viewportSize(1024, 768);
    m_page->setPreferredContentsSize(viewportSize);
    if (m_networkAccessManager)
        m_page->setNetworkAccessManager(m_networkAccessManager);

#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    m_gView->showFullScreen();
    m_gView->window()->raise();
#else
    m_page->setViewportSize(viewportSize);
    m_gView->setFixedSize(viewportSize);
    m_gView->show();
#endif
    m_view->setGeometry(m_gView->viewport()->geometry());

    QTest::qWaitForWindowExposed(m_gView);
}

void tst_GraphicsScrolling::cleanup()
{
    delete m_gView;
}

void tst_GraphicsScrolling::scroll_data()
{
    add_test_urls();
}

/*
 * We want to test how fast we can scroll. The scrolling
 * process starts with the QWebPage::scrollBy call, this
 * will generate DOM Events which will result in a
 * QWidget::update() call. QCoreApplication::processEvents
 * is used to test the scrolling.
 */
void tst_GraphicsScrolling::scroll()
{
    QFETCH(QUrl, url);

    m_view->load(url);
    ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

    // wait for Javascript's lazy loading of ressources
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN)
    QTest::qWait(1500);
#else
    QTest::qWait(3000);
#endif

    /* force a layout */
    QWebFrame* mainFrame = m_page->mainFrame();

    const int scrollIncrement = 10;
    // first rendering outside of the benchmark
   // mainFrame->setScrollPosition(QPoint(0, 0));
    m_view->update();
    qApp->processEvents();

    int l_height = mainFrame->evaluateJavaScript("$('.wrapper').prop('scrollHeight');").toInt();
    int l_screenHeight = mainFrame->evaluateJavaScript("$('.wrapper').height();").toInt();
    int l_scrollArea = l_height - l_screenHeight;
    int l_curScroll = 0;

    WEB_BENCHMARK_TIME_PER_FRAME("graphicsscrolling::scroll", url.toString()) {
        do {
            web__controller.newFrame();

//            QString l_javaStr = "$('.wrapper').animate({scrollTop: '%1'}, 500); null";
//            l_javaStr = l_javaStr.arg(l_curScroll);
//            mainFrame->evaluateJavaScript(l_javaStr);
            //mainFrame->evaluateJavaScript("$('.wrapper').animate({scrollTop: $('.wrapper').scrollTop() + 30}); null");
            QString l_javaStr = QString("$('.wrapper').scrollTop($('.wrapper').scrollTop() + %1); null").arg(scrollIncrement);
            mainFrame->evaluateJavaScript(l_javaStr);
            l_curScroll += scrollIncrement;

            qApp->processEvents();
        } while(l_curScroll < l_scrollArea);

        do {
            web__controller.newFrame();
            //mainFrame->evaluateJavaScript("var p = $('.wrapper'); p.animate({scrollTop: p.scrollTop() - 30})");
            QString l_javaStr = QString("$('.wrapper').scrollTop($('.wrapper').scrollTop() - %1); null").arg(scrollIncrement);
            mainFrame->evaluateJavaScript(l_javaStr);
            l_curScroll -= scrollIncrement;
            qApp->processEvents();
        } while(l_curScroll > 0);
    }
}

void tst_GraphicsScrolling::paintingSpeed_data()
{
    add_test_urls();
}

/*
 * This benchmark measure the painting time of scrolling.
 * The page is scrolled to the bottom, and then to the top. The
 * total painting time is the value of interest.
 */
void tst_GraphicsScrolling::paintingSpeed()
{
    QFETCH(QUrl, url);

    m_view->load(url);
    ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

    // wait for Javascript's lazy loading of ressources
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    QTest::qWait(1500);
#else
    QTest::qWait(3000);
#endif

    QWebFrame* mainFrame = m_page->mainFrame();
//    if (mainFrame->scrollBarValue(Qt::Vertical) == 300/*mainFrame->scrollBarMaximum(Qt::Vertical)*/) {
//        QSKIP("No scrolling for this page", SkipSingle);
//    }

    int l_height = mainFrame->evaluateJavaScript("$('.wrapper').prop('scrollHeight');").toInt();
    int l_screenHeight = mainFrame->evaluateJavaScript("$('.wrapper').height();").toInt();
    int l_scrollArea = l_height - l_screenHeight;
    int l_curScroll = 0;
    const int scrollIncrement = 10;

    WEB_BENCHMARK_SUBSECTION("graphicsscrolling::paintingSpeed", url.toString()) {
        m_view->setController(&web__controller);
        while (l_curScroll < l_scrollArea)
        { // scroll forward
            //mainFrame->scroll(0, scrollIncrement);
            QString l_javaStr = QString("$('.wrapper').scrollTop($('.wrapper').scrollTop() + %1); null").arg(scrollIncrement);
            mainFrame->evaluateJavaScript(l_javaStr);
            l_curScroll += scrollIncrement;
            waitForSignal(m_view, SIGNAL(painted()));
        }
        QCoreApplication::processEvents();
        while (l_curScroll > 0)
        { // then backward
            //mainFrame->scroll(0, -scrollIncrement);
            QString l_javaStr = QString("$('.wrapper').scrollTop($('.wrapper').scrollTop() - %1); null").arg(scrollIncrement);
            mainFrame->evaluateJavaScript(l_javaStr);
            l_curScroll -= scrollIncrement;
            waitForSignal(m_view, SIGNAL(painted()));
        }
    }

    // Give all paintings time to finish
    QCoreApplication::processEvents();
    QTest::qWait(50);
    m_view->setController(0);
}

DBWEBTEST_MAIN(tst_GraphicsScrolling)
#include "tst_graphicsscrolling.moc"
