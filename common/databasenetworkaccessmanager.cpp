/*
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

#include "databasenetworkaccessmanager.h"

#include "benchmark.h"
#include "common_init.h"
#include "databasenetworkreply.h"

#include <QApplication>

bool connectToDatabase(const QString& name)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(name);
    const bool isValid = db.isValid() && db.open() && db.tables().size() > 0;
    if (!isValid)
        QSqlDatabase::removeDatabase(db.connectionName());
    return isValid;
}


QNetworkReply* DatabaseNetworkAccessManager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData);

    if (req.url().scheme() == QLatin1String("data"))
        return QNetworkAccessManager::createRequest(op, req, outgoingData);

    DatabaseNetworkReply* reply = 0;
    if (op != GetOperation)
        reply = new DatabaseNetworkReply(QUrl());
    else
        reply = new DatabaseNetworkReply(req.url());

    reply->setOperation(op);
    reply->setRequest(req);
    return reply;
}
