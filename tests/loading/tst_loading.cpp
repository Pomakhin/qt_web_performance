/*
 * Copyright (C) 2009 Holger Hans Peter Freyther
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

#include <QDesktopWidget>
#include <qwebframe.h>
#include <qwebview.h>
#include <qpainter.h>

class tst_Loading : public QObject
{
    Q_OBJECT

public:
    ~tst_Loading();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void load_data();
    void load();

private:
    QWebView* m_view;
    WebPage* m_page;
};

tst_Loading::~tst_Loading()
{
    benchmarkOutput();
}

void tst_Loading::init()
{
    QWebSettings::globalSettings()->setMaximumPagesInCache(0);
    QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);
    m_view = new QWebView;
    m_page = new WebPage(m_view);
    m_view->setPage(m_page);

    QSize viewportSize(1024, 768);
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    m_page->setPreferredContentsSize(viewportSize);
    const QSize screenSize = QApplication::desktop()->geometry().size();
    m_view->setFixedSize(screenSize);
    m_page->setViewportSize(screenSize);
    m_view->window()->raise();
#else
    m_view->setFixedSize(viewportSize);
    m_page->setViewportSize(viewportSize);
#endif

    if (QSqlDatabase::database().isValid())
        m_page->setNetworkAccessManager(new DatabaseNetworkAccessManager);
}

void tst_Loading::cleanup()
{
    delete m_view;
}

void tst_Loading::load_data()
{
    add_test_urls();
}

void tst_Loading::load()
{
    QFETCH(QUrl, url);

    WEB_BENCHMARK("loading::load", url.toString()) {
        m_view->load(url);
        // really wait for loading..
        ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));

        TIME_NOW
        m_view->load(QUrl("about:blank"));
        QWebSettings::clearMemoryCaches();
    }
}

DBWEBTEST_MAIN(tst_Loading)
#include "tst_loading.moc"
