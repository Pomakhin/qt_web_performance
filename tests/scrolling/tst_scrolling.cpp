/*
 * Copyright (C) 2009 Holger Hans Peter Freyther
 * Copyright (C) 2010 Benjamin Poulain, Nokia
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
#include "paintingwebviewbench.h"
#include "webpage.h"

#include <qwebframe.h>
#include <qwebview.h>

static void loadUrl(QWebView* view, const QUrl& url) {
    view->load(url);
    ::waitForSignal(view, SIGNAL(loadFinished(bool)));

    // wait for Javascript's lazy loading of ressources
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN)
    QTest::qWait(1500);
#else
    QTest::qWait(500);
#endif
}

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

static bool canScroll(QWebFrame* mainFrame) {
    if (mainFrame->scrollBarValue(Qt::Vertical) == mainFrame->scrollBarMaximum(Qt::Vertical))
        return false;
    return true;
}

class tst_Scrolling : public QObject
{
    Q_OBJECT

public:
    ~tst_Scrolling();

public Q_SLOTS:
    void init();
    void cleanup();

    void onJavaInited();

private Q_SLOTS:
    void scroll_data();
    void scroll();

    void paintingSpeed_data();
    void paintingSpeed();

private:
    PaintingWebViewBench* m_view;
    CJavaInterfaceRoot  m_scriptRoot;
};

void tst_Scrolling::onJavaInited()
{
    m_scriptRoot.m_pObj = new JavaInterface();
    m_view->page()->mainFrame()->addToJavaScriptWindowObject("gilauncher", &m_scriptRoot);
}

tst_Scrolling::~tst_Scrolling()
{
    benchmarkOutput();
}

void tst_Scrolling::init()
{
    m_view = new PaintingWebViewBench();

    QWebPage* page = new WebPage(m_view);
    if (QSqlDatabase::database().isValid())
        page->setNetworkAccessManager(new DatabaseNetworkAccessManager);

    m_view->setPage(page);

    const QSize viewportSize(1024, 768);
    m_view->setGeometry(QRect(0,0,1024,768));

#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    const QSize viewportSize(1024, 768);
    page->setPreferredContentsSize(viewportSize);


    m_view->showFullScreen();
    m_view->window()->raise();
#else
    //m_view->showMaximized();
    m_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_view->show();
#endif
    connect(m_view->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(onJavaInited()));
    QTest::qWaitForWindowExposed(m_view);
}

void tst_Scrolling::cleanup()
{
    delete m_view;
}

void tst_Scrolling::scroll_data()
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
void tst_Scrolling::scroll()
{
    QFETCH(QUrl, url);

    loadUrl(m_view, url);;


    /* force a layout */
    QWebFrame* mainFrame = m_view->page()->mainFrame();

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

void tst_Scrolling::paintingSpeed_data()
{
    add_test_urls();
}

/*
 * This benchmark measure the painting time of scrolling.
 * The page is scrolled to the bottom, and then to the top. The
 * total painting time is the value of interest.
 */
void tst_Scrolling::paintingSpeed()
{
    QFETCH(QUrl, url);

    loadUrl(m_view, url);

    QWebFrame* mainFrame = m_view->page()->mainFrame();
//    if (mainFrame->scrollBarValue(Qt::Vertical) == 300/*mainFrame->scrollBarMaximum(Qt::Vertical)*/) {
//        QSKIP("No scrolling for this page", SkipSingle);
//    }

    int l_height = mainFrame->evaluateJavaScript("$('.wrapper').prop('scrollHeight');").toInt();
    int l_screenHeight = mainFrame->evaluateJavaScript("$('.wrapper').height();").toInt();
    int l_scrollArea = l_height - l_screenHeight;
    int l_curScroll = 0;
    const int scrollIncrement = 10;

    WEB_BENCHMARK_SUBSECTION("graphicsscrolling::paintingSpeed", url.toString()) {
        m_view->controller = &web__controller;
        m_view->testing = true;
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
    m_view->controller = 0;


}

DBWEBTEST_MAIN(tst_Scrolling)
#include "tst_scrolling.moc"
