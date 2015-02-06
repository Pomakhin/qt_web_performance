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
#include "webpage.h"

#include <QGLWidget>
#include <qgraphicswebview.h>
#include <qgraphicsview.h>
#include <qwebframe.h>
#include <qwebview.h>


/*
 For those benchmarks, we assume page loaded contains an animation
 trying to update at full frame rate.

 We measure the time between two paints, and assume is it longer
 than the refresh time of the animation (otherwhise the test case is
 saturated and the result is mostly useless).

 We render maxFrameCount frames, and measure the time take for each frame.
 */

// half a minute of testing per page should be fine :)
const int maxFrameCount = 60 /*FPS*/ * 15 /*seconds*/;

static void loadUrl(QWebPage *const page, const QUrl& url) {
    page->mainFrame()->load(url);
    ::waitForSignal(page, SIGNAL(loadFinished(bool)));

    // wait for Javascript's lazy loading of ressources
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN)
    QTest::qWait(1500);
#else
    QTest::qWait(500);
#endif
}

class WebWidget;

class tst_continuous_animation : public QObject
{
    Q_OBJECT

public:
    ~tst_continuous_animation();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void webViewrendering_data();
    void webViewrendering();

    void graphicsWebViewrendering_data();
    void graphicsWebViewrendering();

    void mobileGraphicsWebViewrendering_data();
    void mobileGraphicsWebViewrendering();

private:
    void runGraphicsViewBasedTest(WebWidget*, const char*);

    QWebPage* m_page;
};

tst_continuous_animation::~tst_continuous_animation()
{
    benchmarkOutput();
}

void tst_continuous_animation::init()
{
    m_page = new WebPage;
    if (QSqlDatabase::database().isValid())
        m_page->setNetworkAccessManager(new DatabaseNetworkAccessManager);
}

void tst_continuous_animation::cleanup()
{
    delete m_page;
}

void tst_continuous_animation::webViewrendering_data()
{
    add_test_urls();
}


class BenchmarkingWebView : public QWebView
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent* event)
    {
        QWebView::paintEvent(event);
        emit painted();
    }

signals:
    void painted();
};

void tst_continuous_animation::webViewrendering()
{
    QFETCH(QUrl, url);

    BenchmarkingWebView view;
    view.setPage(m_page);

#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    const QSize viewportSize(1024, 768);
    m_page->setPreferredContentsSize(viewportSize);
    view.showFullScreen();
    view.window()->raise();
#else
    view.showMaximized();
#endif
    QTest::qWaitForWindowExposed(&view);
    loadUrl(m_page, url);

    WEB_BENCHMARK_TIME_PER_FRAME("continuous_animation::webViewrendering", url.toString()) {
        m_page->triggerAction(QWebPage::Reload, true);
        if (!::waitForSignal(m_page, SIGNAL(loadFinished(bool)), 3000))
        {
            ABORT_BENCHMARK("Unabled to reload the page in 3 seconds");
        }


        m_page->triggerAction(QWebPage::Reload, true);
        for (int i = 0; i < maxFrameCount; ++i) {
            if (!::waitForSignal(&view, SIGNAL(painted()), 3000))
                ABORT_BENCHMARK("Stalled: no new frames after 3 seconds");
            web__controller.newFrame();
        }
    }
}


class BenchmarkingGraphicsWebView : public QGraphicsWebView
{
    Q_OBJECT

public:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w)
    {
        QGraphicsWebView::paint(p, o, w);
        emit painted();
    }

signals:
    void painted();
};

class WebWidget : public QGraphicsView
{
    Q_OBJECT

public:
    WebWidget()
    {
        setViewport(new QGLWidget);
        setFrameShape(NoFrame);
        setScene(new QGraphicsScene(this));
        scene()->addItem(&m_webview);
        connect(&m_webview, SIGNAL(painted()), this, SIGNAL(painted()));
    }

    void setPage(QWebPage* page) { m_webview.setPage(page); }

protected:
    BenchmarkingGraphicsWebView m_webview;

signals:
    void painted();
};

class DesktopWebWidget : public WebWidget
{
protected:
    void resizeEvent(QResizeEvent *event)
    {
        m_webview.resize(event->size());
        scene()->setSceneRect(QRect(QPoint(), event->size()));
        WebWidget::resizeEvent(event);
    }
};

void tst_continuous_animation::graphicsWebViewrendering_data()
{
    add_test_urls();
}

void tst_continuous_animation::graphicsWebViewrendering()
{
    DesktopWebWidget desktopWidget;
    runGraphicsViewBasedTest(&desktopWidget, "continuous_animation::graphicsWebViewrendering");
}

class MobileWebWidget : public WebWidget
{
public:
    MobileWebWidget()
    {
        m_webview.setResizesToContents(true);
        m_webview.settings()->setAttribute(QWebSettings::FrameFlatteningEnabled, true);
    }
};

void tst_continuous_animation::mobileGraphicsWebViewrendering_data()
{
    add_test_urls();
}

void tst_continuous_animation::mobileGraphicsWebViewrendering()
{
    MobileWebWidget mobileWebWidget;
    runGraphicsViewBasedTest(&mobileWebWidget, "continuous_animation::mobileGraphicsWebViewrendering");
}

void tst_continuous_animation::runGraphicsViewBasedTest(WebWidget* view, const char* testName)
{
    QFETCH(QUrl, url);

    view->setPage(m_page);


#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    const QSize viewportSize(1024, 768);
    m_page->setPreferredContentsSize(viewportSize);
    view->showFullScreen();
    view->window()->raise();
#else
    view->showMaximized();
#endif
    QTest::qWaitForWindowExposed(view);
    loadUrl(m_page, url);

    WEB_BENCHMARK_TIME_PER_FRAME(testName, url.toString()) {
        m_page->triggerAction(QWebPage::Reload, true);
        if (!::waitForSignal(m_page, SIGNAL(loadFinished(bool)), 3000))
            ABORT_BENCHMARK("Unabled to reload the page in 3 seconds");

        for (int i = 0; i < maxFrameCount; ++i) {
            if (!::waitForSignal(view, SIGNAL(painted()), 3000))
                ABORT_BENCHMARK("Stalled: no new frames after 3 seconds");
            web__controller.newFrame();
        }
    }
}

DBWEBTEST_MAIN(tst_continuous_animation)
#include "tst_continuous_animation.moc"
