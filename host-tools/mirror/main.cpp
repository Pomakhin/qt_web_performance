/*
 * Copyright (C) 2009, 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2009, 2010 Holger Hans Peter Freyther
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QApplication>

#include <qwebframe.h>
#include <qwebpage.h>
#include <qwebview.h>
#include <qwebsettings.h>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "benchmark.h"
#include "cookiejar.h"
#include "urlfilereader.h"

#include <QNetworkCookie>

/**
 * A proxy for the QNetworkReply...
 */
class NetworkReplyProxy : public QNetworkReply {
    Q_OBJECT
public:
    NetworkReplyProxy(QObject* parent, QNetworkReply* reply)
        : QNetworkReply(parent)
        , m_reply(reply)
    {
        qWarning("Starting network job: %p %s", this, qPrintable(m_reply->url().toString()));
        // apply attributes...
        setOperation(m_reply->operation());
        setRequest(m_reply->request());
        setUrl(m_reply->url());

        // handle these to forward
        connect(m_reply, SIGNAL(metaDataChanged()), SLOT(applyMetaData()));
        connect(m_reply, SIGNAL(readyRead()), SLOT(readInternal()));
        connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorInternal(QNetworkReply::NetworkError)));

        // forward signals
        connect(m_reply, SIGNAL(finished()), SIGNAL(finished()));
        connect(m_reply, SIGNAL(uploadProgress(qint64,qint64)), SIGNAL(uploadProgress(qint64,qint64)));
        connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), SIGNAL(downloadProgress(qint64,qint64)));

        // for the data proxy...
        setOpenMode(ReadOnly);
    }

    ~NetworkReplyProxy()
    {
        if (m_reply->url().scheme() != "data")
            writeData();
        delete m_reply;
    }

    // virtual  methids
    void abort() { m_reply->abort(); }
    void close() { m_reply->close(); }
    bool isSequential() const { return m_reply->isSequential(); }

    // not possible...
    void setReadBufferSize(qint64 size) { QNetworkReply::setReadBufferSize(size); m_reply->setReadBufferSize(size); }

    // ssl magic is not done....
    // isFinished()/isRunning can not be done *sigh*


    // QIODevice proxy...
    virtual qint64 bytesAvailable() const
    {
        return m_buffer.size() + QIODevice::bytesAvailable();
    }

    virtual qint64 bytesToWrite() const { return -1; }
    virtual bool canReadLine() const { qFatal("not implemented"); return false; }

    virtual bool waitForReadyRead(int) { qFatal("not implemented"); return false; }
    virtual bool waitForBytesWritten(int) { qFatal("not implemented"); return false; }

    virtual qint64 readData(char* data, qint64 maxlen)
    {
        qint64 size = qMin(maxlen, qint64(m_buffer.size()));
        memcpy(data, m_buffer.constData(), size);
        m_buffer.remove(0, size);
        return size;
    }
public Q_SLOTS:
    void ignoreSslErrors() { m_reply->ignoreSslErrors(); }
    void applyMetaData() {
        QList<QByteArray> headers = m_reply->rawHeaderList();
        foreach(QByteArray header, headers)
            setRawHeader(header, m_reply->rawHeader(header));

        setHeader(QNetworkRequest::ContentTypeHeader, m_reply->header(QNetworkRequest::ContentTypeHeader));
        setHeader(QNetworkRequest::ContentLengthHeader, m_reply->header(QNetworkRequest::ContentLengthHeader));
        setHeader(QNetworkRequest::LocationHeader, m_reply->header(QNetworkRequest::LocationHeader));
        setHeader(QNetworkRequest::LastModifiedHeader, m_reply->header(QNetworkRequest::LastModifiedHeader));
        setHeader(QNetworkRequest::SetCookieHeader, m_reply->header(QNetworkRequest::SetCookieHeader));

        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute));
        setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute));
        setAttribute(QNetworkRequest::RedirectionTargetAttribute, m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute));
        setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, m_reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute));
        setAttribute(QNetworkRequest::CacheLoadControlAttribute, m_reply->attribute(QNetworkRequest::CacheLoadControlAttribute));
        setAttribute(QNetworkRequest::CacheSaveControlAttribute, m_reply->attribute(QNetworkRequest::CacheSaveControlAttribute));
        setAttribute(QNetworkRequest::SourceIsFromCacheAttribute, m_reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute));
        setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, m_reply->attribute(QNetworkRequest::DoNotBufferUploadDataAttribute));
        emit metaDataChanged();
    }
    void errorInternal(QNetworkReply::NetworkError _error)
    {
        setError(_error, errorString());
        emit error(_error);
    }
    void readInternal()
    {
        QByteArray data = m_reply->readAll();
        m_data += data;
        m_buffer += data;
        emit readyRead();
    }

private:
    void writeData();
    static void writeData(const QByteArray& url, const QByteArray& data, const QByteArray& header, int operation, int response);

    QNetworkReply* m_reply;
    QByteArray m_data;
    QByteArray m_buffer;
};

void NetworkReplyProxy::writeData()
{
    QByteArray httpHeader;
    QList<QByteArray> headers = rawHeaderList();
    foreach(QByteArray header, headers) {
        if (header.toLower() == "content-encoding"
            || header.toLower() == "transfer-encoding"
            || header.toLower() == "content-length"
            || header.toLower() == "connection")
            continue;

        // special case for cookies.... we need to generate separate lines
        // QNetworkCookie::toRawForm is a bit broken and we have to do this
        // ourselves... some simple heuristic here..
        if (header.toLower() == "set-cookie") {
            QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(rawHeader(header));
            foreach (QNetworkCookie cookie, cookies) {
                httpHeader += "set-cookie: " + cookie.toRawForm() + "\r\n";
            }
        } else {
            httpHeader += header + ": " + rawHeader(header) + "\r\n";
        }
    }
    httpHeader += "content-length: " + QByteArray::number(m_data.size()) + "\r\n";
    httpHeader += "\r\n";

    if(m_reply->error() != QNetworkReply::NoError) {
        qWarning() << "\tError with: " << this << url() << error();
        return;
    }

    const QByteArray origUrl = m_reply->url().toEncoded();
    const QByteArray strippedUrl = m_reply->url().toEncoded(QUrl::RemoveFragment | QUrl::RemoveQuery);
    writeData(origUrl, m_data, httpHeader, operation(), attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    if (origUrl != strippedUrl)
        writeData(strippedUrl, m_data, httpHeader, operation(), attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
}

void NetworkReplyProxy::writeData(const QByteArray& url, const QByteArray& data, const QByteArray& header, int operation, int response)
{
    static int dumpId = -1;
    qWarning("Writing result for: %s data size: %u dumpId: %u", url.constData(), data.size(), ++dumpId);

    QSqlQuery query;
    query.prepare("INSERT INTO responses(operation, response, url, data, header) VALUES(:op, :response, :url, :data, :header)");
    query.bindValue(":op", operation);
    query.bindValue(":response", response);
    query.bindValue(":url", url);
    query.bindValue(":data", data);
    query.bindValue(":header", header);

    // verify that the old and new data re the same...
    if (!query.exec()) {
        qWarning() << "\tInsert into failed with:" << query.lastError();
        QSqlQuery select;
        select.prepare("SELECT data, header FROM responses WHERE url = :url");
        select.bindValue(":url", url);
        if (!select.exec()) {
            qWarning() << "\tUnknown issue for storing: " << url << select.lastError();
            return;
        }

        select.next();
        if (header != select.value(1))
            qWarning() << "\tHeaders are different, not storing them for: " << url;
        if (data != select.value(0))
            qWarning() << "\tData is different, not storing it for: " << url;
    } else {
        QFile file(QString("dump.%1").arg(dumpId));
        file.open(QFile::WriteOnly | QFile::Truncate);
        file.write(data);
        file.close();
    }
}

class NetworkAccessManagerProxy : public QNetworkAccessManager {
    Q_OBJECT
public:
    NetworkAccessManagerProxy(QNetworkCookieJar* jar)
        : QNetworkAccessManager()
    {
        setCookieJar(jar);
    }

    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                                         QIODevice *outgoingData)
    {
        QNetworkReply* reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
        return new NetworkReplyProxy(this, reply);
    }
};

class MirroringWebPage : public QWebPage {
    Q_OBJECT
public:
    MirroringWebPage(QObject* parent)
        : QWebPage(parent)
    {
        connect(this, SIGNAL(loadFinished(bool)), this, SLOT(pageLoaded(bool)));
    }

    void load(const QUrl &url, const QString &description)
    {
        m_description = description;
        mainFrame()->load(url);
    }

private Q_SLOTS:
    void pageLoaded(bool ok)
    {
        if (ok) {
            if (mainFrame()->requestedUrl() == QUrl(QLatin1String("about:blank")))
                return;

            QSqlQuery query;
            query.prepare("INSERT INTO mirrored_urls(url, description, date) VALUES(:url, :description, :date)");
            query.bindValue(":url", mainFrame()->requestedUrl());
            query.bindValue(":description", m_description);
            query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
            // verify that the old and new data re the same...
            if (!query.exec())
                qWarning() << "\tInsert into mirrored_urls failed with:" << query.lastError();
        } else {
            qWarning() << "Loading of " << mainFrame()->requestedUrl() << " failed!";
        }
    }

private:
    QString m_description;
};


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("crawl_db.db");
    if (!db.open())
        qCritical() << "Failed to open the crawl db";

    /*
     * initialize tables... for the poor
     */
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS responses(operation int,"
                                                          "response int, "
                                                          "url blob NOT NULL UNIQUE,"
                                                          "data blob, header blob)"))
        qFatal("Creating the table failed...");
    if (!query.exec("CREATE TABLE IF NOT EXISTS mirrored_urls(url blob NOT NULL UNIQUE,"
                                                              "description text NULL,"
                                                              "date text NULL)"))
        qFatal("Creating the table failed...");

    CookieJar jar;
    QStringList args = app.arguments();
    QUrl url("http://www.google.com/news");
    bool keepRunning = false;
    bool isVisible = false;

    QList<UrlInfo> urlList;
    for (int i = 1; i < args.size(); ++i) {
        const QString& arg = args[i];

        if (arg == QLatin1String("-c") && i + 1< args.size()) {
            bool loaded = jar.load(args.at(i + 1));
            qWarning("Loading jar from: %s %d", qPrintable(args.at(i + 1)), loaded);
            ++i;
        } else if (arg == QLatin1String("-k")) {
            keepRunning = true;
        } else if (arg == QLatin1String("-v")) {
            isVisible = true;
        } else if (arg == QLatin1String("-urlfile") || arg == QLatin1String("-u")) {
            ++i;
            if (!(i < args.size()))
                qFatal("-urlfile but no file specified");
            QFile urlFile(args[i]);
            if (!urlFile.open(QIODevice::ReadOnly)) {
                qCritical() << "-urlfile but file " << args[i] << " cannot be read: " << urlFile.errorString();
                return -1;
            }
            QList<UrlInfo> aUrlList = readUrlList(&urlFile);
            urlList.append(aUrlList);
        } else if (arg == QLatin1String("-h") || arg == QLatin1String("--help")) {
            fprintf(stderr, "%s options [-urlfile path_to_file] [url] [url2] [...]\n", argv[0]);
            fprintf(stderr, "\t-c cookies.ini\tUse the cookies from this file.\n"
                            "\t\t\tThe cookie file is compatible with Arora.\n");
            fprintf(stderr, "\t-v\t\tShow the WebView when running\n");
            fprintf(stderr, "\t-k\t\tKeep the application running.\n");
            fprintf(stderr, "\t-urlfile\t\tMirror the url of the file.\n");
            return -1;
        } else {
            urlList.append(UrlInfo(arg, QString()));
        }
    }

    QWebView* view = new QWebView;
    MirroringWebPage* page = new MirroringWebPage(view);
    page->setNetworkAccessManager(new NetworkAccessManagerProxy(&jar));
    view->setPage(page);

    if (isVisible) {
        view->show();
        QCoreApplication::processEvents();
    }

    QSet<QUrl> knownUrl;
    QSqlQuery knownUrlSelect;
    knownUrlSelect.prepare("SELECT url FROM mirrored_urls");
    if (knownUrlSelect.exec()) {
        while (knownUrlSelect.next())
            knownUrl.insert(knownUrlSelect.value(0).toUrl());
    }

    foreach (const UrlInfo &urlUnit, urlList) {
        if (knownUrl.contains(urlUnit.first))
            continue;
        qWarning() << "Using url: " << urlUnit.first;
        page->load(urlUnit.first, urlUnit.second);
        ::waitForSignal(page, SIGNAL(loadFinished(bool)), /* ten second timeout */ 10000);

        QEventLoop loop;
        QTimer timer;
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(8000);
        loop.exec();
        // We need to reset the URL in order to load successive pages with fragments
        // new pages
        page->mainFrame()->load(QUrl(QLatin1String("about:blank")));
        QApplication::processEvents();
    }

    /* continue mode for the poor */
    if (keepRunning)
        return app.exec();
    return 0;
}

#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
quint64 qHash(const QUrl &url)
{
    return qHash(url.toString());
}
#endif

#include "main.moc"
