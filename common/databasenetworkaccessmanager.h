/*
 * Copyright (C) 2009 Benjamin Poulain, Nokia
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

#ifndef DATABASENETWORKACCESSMANAGER_H
#define DATABASENETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QStringList>
#include <QSqlDatabase>

class QIODevice;
class QString;
class QNetworkReply;
class QNetworkRequest;

bool connectToDatabase(const QString& name);

class DatabaseNetworkAccessManager : public QNetworkAccessManager
{
protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData = 0);
};

QStringList mainImpl();

#define DBWEBTEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    benchmark_parent = new Benchmark("total"); \
    QApplication app(argc, argv); \
    TestObject tc; \
    return QTest::qExec(&tc, mainImpl()); \
}

#endif // DATABASENETWORKACCESSMANAGER_H
