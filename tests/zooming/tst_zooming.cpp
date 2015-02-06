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
#include <qwebview.h>
#include <qpainter.h>

struct PaintingWebViewBench : public QWebView
{
    Q_OBJECT
public:
    PaintingWebViewBench(QWidget* parent = 0) : QWebView(parent), m_controller(0) {}

    void paintEvent(QPaintEvent* event)
    {
        if(m_controller) {
            m_controller->startSubMeasure();
            QWebView::paintEvent(event);
            m_controller->stopSubMeasure();
        } else {
            QWebView::paintEvent(event);
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

class tst_Zooming : public QObject
{
    Q_OBJECT

public:
    ~tst_Zooming();

public Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void zooming_data();
    void zooming();

private:
    PaintingWebViewBench* m_view;
    WebPage* m_page;
    QNetworkAccessManager* m_networkAccessManager;
};

tst_Zooming::~tst_Zooming()
{
    benchmarkOutput();
}

void tst_Zooming::initTestCase()
{
    if (QSqlDatabase::database().isValid())
        m_networkAccessManager = new DatabaseNetworkAccessManager;
    else
        m_networkAccessManager = 0;
}

void tst_Zooming::init()
{
    m_view = new PaintingWebViewBench();
    m_page = new WebPage(m_view);
    m_view->setPage(m_page);

    const QSize viewportSize(1024, 768);
    m_page->setPreferredContentsSize(viewportSize);
    if (m_networkAccessManager)
        m_page->setNetworkAccessManager(m_networkAccessManager);

#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    m_view->showFullScreen();
    m_view->window()->raise();
#else
    m_page->setViewportSize(viewportSize);
    m_view->setFixedSize(viewportSize);
    m_view->show();
#endif

    QTest::qWaitForWindowExposed(m_view);
}

void tst_Zooming::cleanup()
{
    delete m_view;
}

void tst_Zooming::zooming_data()
{
    add_test_urls();
}

void tst_Zooming::zooming()
{
    QFETCH(QUrl, url);

    m_view->load(url);
    ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

    // wait for Javascript's lazy loading of ressources
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    QTest::qWait(1500);
#else
    QTest::qWait(500);
#endif

    WEB_BENCHMARK_SUBSECTION("graphicsscrolling::paintingSpeed", url.toString()) {
        m_view->setController(&web__controller);
        qreal f = 1.0;
        qreal zoomStep = 0.25;
        for( ; f > 0.5; f -= zoomStep) {
            m_view->setZoomFactor(f);
            waitForSignal(m_view, SIGNAL(painted()));
        }
        QCoreApplication::processEvents();
        for( ; f < 2; f += zoomStep) {
            m_view->setZoomFactor(f);
            waitForSignal(m_view, SIGNAL(painted()));
        }
        QCoreApplication::processEvents();
        for( ; f > 0.5; f -= zoomStep) {
            m_view->setZoomFactor(f);
            waitForSignal(m_view, SIGNAL(painted()));
        }
    }

    // Give all paintings time to finish
    QCoreApplication::processEvents();
    QTest::qWait(50);
    m_view->setController(0);
}

DBWEBTEST_MAIN(tst_Zooming)
#include "tst_zooming.moc"
