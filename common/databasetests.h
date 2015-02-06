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

#ifndef DATABASETESTS_H
#define DATABASETESTS_H

#include "urlfilereader.h"
#include "benchmarkoutputwriter.h"
#include "common_init.h"

#include <QCoreApplication>
#include <QSqlQuery>

static bool initUrlFromFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QList<UrlInfo> urlList = readUrlList(&file);
    foreach (const UrlInfo &urlInfo, urlList) {
        add_url_to_test_urls(urlInfo.first);
    }
    return true;
}

static void initUrlFromDatabase()
{
    QSqlQuery knownUrlSelect;
    knownUrlSelect.prepare("SELECT url FROM mirrored_urls");
    if (knownUrlSelect.exec()) {
        while (knownUrlSelect.next())
            add_url_to_test_urls(knownUrlSelect.value(0).toUrl());
    }
}

#if defined (Q_OS_SYMBIAN)
#include "sym_iap_util.h"
#endif

QStringList mainImpl()
{
#if defined (Q_OS_SYMBIAN) && defined (USE_DEFAULT_IAP)
    qt_SetDefaultIap();
#endif

    enum OutputType {
        Human,
        Csv
    };

    const QStringList arguments = QCoreApplication::arguments();
    QStringList ouputArguments;
    ouputArguments.append(arguments[0]);

    const int argumentsCount = arguments.size();
    bool loadedUrlFromFile = false;
    bool loadUrlFromDatabase = false;
    QString outputFilePath;
    OutputType outputType = Human;
    for (int i = 1; i < argumentsCount; ++i) {
        const QString argument = arguments.at(i);
        if (argument == QLatin1String("-help")) {
            printf("Optionnal arguments for the WebKit benchmarks:\n"
                   " -out filename                 Save the output in filename instead of showing it in the terminal\n"
                   " -database path_to_database    Use the database as source of data for the test."
                   " If the database contains a list of url, those url are the one tested\n"
                   " -urlfile filename             Use the urls of filename for the test. Those urls have precedence over those defined with -database\n"
                   " -outputtype csv|human         Change the way the benchmark results are reported. \"human\" is the default\n"
                   "\n\n");
        } else if (argument == QLatin1String("-out")) {
            ++i;
            if (i < argumentsCount) {
                outputFilePath = arguments.at(i);
                continue;
            }
        } else if (argument == QLatin1String("-urlfile")) {
            ++i;
            if (!initUrlFromFile(arguments.at(i)))
                qWarning() << "Error while including urlfile " << arguments.at(i);
            else
                loadedUrlFromFile = true;
            continue;
        } else if (argument == QLatin1String("-database")) {
            ++i;
            if (i < argumentsCount) {
                if (!connectToDatabase(arguments.at(i))) {
                    qWarning() << "the database " << arguments.at(i) << " cannot be opened.";
                } else
                    loadUrlFromDatabase = true;
            } else {
                qFatal("-database is defined but no database file specified");
            }
            continue;
        } else if (arguments.at(i) == QLatin1String("-iterations")) {
            ++i;
            if (i < argumentsCount) {
                bool success;
                int iterations = arguments.at(i).toInt(&success);
                if (success)
                    BenchmarkController::setDefaultIterations(iterations + 1);  // + 1 for the warmup
                else
                    qWarning() << "-iterations expect a number argument. Default iterations will be used.";
            }
            continue;
        } else if (arguments.at(i) == QLatin1String("-outputtype")) {
            ++i;
            if (i < argumentsCount) {
                const QString outputTypeString = arguments.at(i);
                if (outputTypeString == QLatin1String("csv")) {
                    outputType = Csv;
                }
                continue;
            }
        }
        ouputArguments.append(arguments.at(i));
    }

    // configure the output writer
    if (outputType == Csv) {
        outWriter = QSharedPointer<BenchmarkOutputWriter>(new BenchmarkOutputCsv(outputFilePath));
    } else {
        outWriter = QSharedPointer<BenchmarkOutputWriter>(new BenchmarkOutputHuman(outputFilePath));
    }

    // url file has precedence over the database
    // so a url file can be used to reduce the scope of a database
    if (!loadedUrlFromFile && loadUrlFromDatabase)
        initUrlFromDatabase();

    return ouputArguments;
}


#endif // DATABASETESTS_H
