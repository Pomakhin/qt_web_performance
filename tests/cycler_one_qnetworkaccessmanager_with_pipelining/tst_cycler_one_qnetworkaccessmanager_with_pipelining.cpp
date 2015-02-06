/*
 * Copyright (C) 2010 Nokia
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

#include "benchmark.h"
#include "common_init.h"
#include "databasenetworkaccessmanager.h"
#include "databasetests.h"
#include "webpage.h"

#include <QIODevice>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QWebView>

class tst_cycler_one_qnetworkaccessmanager_with_pipelining : public QObject
{
    Q_OBJECT

public:
    ~tst_cycler_one_qnetworkaccessmanager_with_pipelining();

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void load_data();
    void load();

private:
    QWebView* m_view;
};

class PipeliningNetworkAccessManager : public QNetworkAccessManager {
    Q_OBJECT

public:
    PipeliningNetworkAccessManager() {}
    QNetworkReply* createRequest(Operation op, const QNetworkRequest & req, QIODevice* outgoingData);
};

QNetworkReply* PipeliningNetworkAccessManager::createRequest(Operation op, const QNetworkRequest & req, QIODevice* outgoingData)
{
    QNetworkRequest request = req;
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    QNetworkReply* reply =  QNetworkAccessManager::createRequest(op, request, outgoingData);
    return reply;
}

tst_cycler_one_qnetworkaccessmanager_with_pipelining::~tst_cycler_one_qnetworkaccessmanager_with_pipelining()
{
    benchmarkOutput();
}

void tst_cycler_one_qnetworkaccessmanager_with_pipelining::initTestCase()
{
    QWebSettings::globalSettings()->setMaximumPagesInCache(0);
    QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);

    m_view = new QWebView;
    WebPage* page = new WebPage(m_view);
    m_view->setPage(page);

    if (QSqlDatabase::database().isValid())
        page->setNetworkAccessManager(new DatabaseNetworkAccessManager);
    else
        page->setNetworkAccessManager(new PipeliningNetworkAccessManager);

#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || defined(Q_WS_QWS)
    const QSize viewportSize(1024, 768);
    page->setPreferredContentsSize(viewportSize);
    m_view->showFullScreen();
    m_view->window()->raise();
#else
    m_view->showMaximized();
#endif

    QTest::qWaitForWindowExposed(m_view);
}

void tst_cycler_one_qnetworkaccessmanager_with_pipelining::cleanupTestCase()
{
    delete m_view;
}

void tst_cycler_one_qnetworkaccessmanager_with_pipelining::load_data()
{
    add_test_urls();
}

void tst_cycler_one_qnetworkaccessmanager_with_pipelining::load()
{
    QFETCH(QUrl, url);

    WEB_BENCHMARK("cycler_one_qnetworkaccessmanager_with_pipelining::load", url.toString()) {
        m_view->load(url);
        // really wait for loading..
        ::waitForSignal(m_view, SIGNAL(loadFinished(bool)));
        TIME_NOW;
        m_view->load(QUrl("about:blank"));
        QWebSettings::clearMemoryCaches();
    }
}

DBWEBTEST_MAIN(tst_cycler_one_qnetworkaccessmanager_with_pipelining)
#include "tst_cycler_one_qnetworkaccessmanager_with_pipelining.moc"
