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

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qgraphicsview.h>
#include <qgraphicswebview.h>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qpaintengine.h>
#include <qpainter.h>

class tst_GraphicsPainting : public QObject
{
    Q_OBJECT

public:
    ~tst_GraphicsPainting();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void paint_data();
    void paint();

private:
    QGraphicsWebView* m_view;
    WebPage* m_page;
    QGraphicsView* m_gView;
    QGraphicsScene* m_scene;
};

tst_GraphicsPainting::~tst_GraphicsPainting()
{
    benchmarkOutput();
}

void tst_GraphicsPainting::init()
{
    m_scene = new QGraphicsScene();
    m_gView = new QGraphicsView(m_scene);
    m_view = new QGraphicsWebView();
    m_page = new WebPage(m_view);
    m_view->setPage(m_page);
    m_scene->addItem(m_view);

    QSize viewportSize(1024, 768);
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    // screensize is a more typical rendering unit for embedded
    m_page->setPreferredContentsSize(viewportSize);
    const QSize screenSize = QApplication::desktop()->geometry().size();
    m_gView->setFixedSize(screenSize);
    m_page->setViewportSize(screenSize);
    m_gView->window()->raise();
#else
    m_gView->setFixedSize(viewportSize);
    m_page->setViewportSize(viewportSize);
#endif
    m_view->setGeometry(m_gView->viewport()->geometry());

    if (QSqlDatabase::database().isValid())
        m_page->setNetworkAccessManager(new DatabaseNetworkAccessManager);
}

void tst_GraphicsPainting::cleanup()
{
    delete m_view;
}

void tst_GraphicsPainting::paint_data()
{
    add_test_urls();
}

void tst_GraphicsPainting::paint()
{
    QFETCH(QUrl, url);

    m_view->load(url);
    ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

    /* force a layout */
    QWebFrame* mainFrame = m_page->mainFrame();
    mainFrame->toPlainText();

    QPixmap pixmap(m_page->viewportSize());
#if defined(Q_WS_X11)
    const bool needToSync = pixmap.paintEngine()->type() != QPaintEngine::Raster;
#endif
    WEB_BENCHMARK("graphicsview::paint", url.toString()) {
        QPainter painter(&pixmap);
        m_gView->render(&painter);

        painter.end();

#if defined(Q_WS_X11)
        // force badness... to have some reliable result on non raster..
        // this will make the result "unreal" in some ways but might be
        // better than just using the raster engine.
        if (needToSync)
            QApplication::syncX();
#endif
    }
}

DBWEBTEST_MAIN(tst_GraphicsPainting)
#include "tst_graphicspainting.moc"
