/*
 * Simplistic threaded http server with a one thread per request
 * threading model. Someone should write a proper solution with
 * worker threads and giving work to them.
 *
 *
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

#include <QCoreApplication>
#include <QStringList>
#include <QThreadPool>
#include <QUrl>
#include <QVariant>

#include <QTcpSocket>
#include <QTcpServer>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <unistd.h>

class HttpRequestThread;
class HttpRequest {
    friend class HttpRequestThread;
public:
    HttpRequest();

    enum Request {
        Request_Unknown,
        Request_GET,
        Request_POST,
    };

    QByteArray uri;
    Request request;

private:
    QByteArray host, path;
};

HttpRequest::HttpRequest()
    : request(Request_Unknown)
{}

class HttpRequestThread : public QRunnable {
public:
    HttpRequestThread(int socket);
    ~HttpRequestThread();

    static bool lookup(const QByteArray&, QByteArray&, QByteArray&, QByteArray&);
    static bool search(const QByteArray&, QByteArray& response,
                       QByteArray& header, QByteArray& data);

private:
    void run();

    HttpRequest parseHeader();
    bool sendFile(const HttpRequest&);
    bool send404(const QByteArray& reason);

private:
    int m_fd;
    QTcpSocket* m_socket;
};

class HttpServer : public QTcpServer {
    Q_OBJECT
public:
    HttpServer(QObject*);
    void incomingConnection(int socketDescriptor);

private:
    QThreadPool m_pool;
};

HttpServer::HttpServer(QObject* parent)
    : QTcpServer(parent)
{
    m_pool.setMaxThreadCount(8);
}

void HttpServer::incomingConnection(int fd)
{
    QRunnable* thread = new HttpRequestThread(fd);
    m_pool.start(thread);
}


HttpRequestThread::HttpRequestThread(int fd)
    : m_fd(fd)
    , m_socket(0)
{
    setAutoDelete(true);
}

HttpRequestThread::~HttpRequestThread()
{
}

bool HttpRequestThread::lookup(const QByteArray& req, QByteArray& response,
                               QByteArray& headers, QByteArray& data)
{
    if (search(req, response, headers, data))
        return true;

    QUrl url = QUrl::fromEncoded(req);
    return search(url.toEncoded(QUrl::RemoveFragment | QUrl::RemoveQuery), response, headers, data); 
}

bool HttpRequestThread::search(const QByteArray& req, QByteArray& response,
                               QByteArray& headers, QByteArray& data)
{
    QSqlQuery query;
    query.prepare("SELECT response, header, data from responses where url = ?");
    query.addBindValue(req);

    if (!query.exec()) {
        qWarning() << "Query failed: " << query.lastError();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    response = query.value(0).toByteArray();
    headers = query.value(1).toByteArray();
    data = query.value(2).toByteArray();
    return true;
}

HttpRequest HttpRequestThread::parseHeader()
{
    HttpRequest req;
    bool firstLine = true;

    // blocking reads...
    while (m_socket->state() == QTcpSocket::ConnectedState) {
        while (m_socket->canReadLine()) {
            QByteArray line = m_socket->readLine();

            if (firstLine) {
                firstLine = false;

                if (line.toLower().startsWith("get ") && line.contains("HTTP/1.")) {
                    static const unsigned get_length = sizeof "get " - 1;

                    req.path = line.mid(get_length, line.lastIndexOf("HTTP/1.") - get_length - 1);
                    req.request = HttpRequest::Request_GET;
                } else {
                    qWarning("Unhandled operation: %p '%s'", this, line.data());
                }
            } else if (line == "\r\n") {
                return req;
            } else if (line.toLower().startsWith("host: ")) {
                req.host = line.mid(sizeof "host: " - 1).trimmed();
                req.uri = "http://" + req.host + req.path;
            }
        }

        m_socket->waitForReadyRead(-1);
    }

    return req;
}

bool HttpRequestThread::send404(const QByteArray& reason)
{
    m_socket->write("HTTP/1.0 404 " + reason + "\r\nContent-Type: text/html\r\n\r\nNot found.\r\n");
    return false;
}

bool HttpRequestThread::sendFile(const HttpRequest& req)
{
    if (req.uri.isEmpty() || req.request != HttpRequest::Request_GET)
        return send404("Bad Request");


    QByteArray response, headers, data;

    // try the normal uri first, then strip fragment and query
    if (!lookup(req.uri, response, headers, data)) {
        qWarning("Searching failed: %p '%s'\n", this, req.uri.data());
        return send404("Not found");
    }

    m_socket->write("HTTP/1.1 " + response + " OK\r\n");
    m_socket->write("Connection: close\r\n");
    m_socket->write(headers);
    m_socket->write(data);

    return true;
}

void HttpRequestThread::run()
{
    m_socket = new QTcpSocket();

    if (!m_socket->setSocketDescriptor(m_fd)) {
        qWarning() << "Adopting the socket failed";
        close(m_fd);
        m_fd = -1;
        delete m_socket;
        return;
    }

    sendFile(parseHeader());
    m_socket->flush();
    m_socket->disconnectFromHost();

    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->waitForDisconnected();
    delete m_socket;
}


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QString fileName = "crawl_db.db";
    if (app.arguments().size() >= 2)
        fileName = app.arguments().at(1);

    qWarning("Seving crawl db: '%s'", qPrintable(fileName));
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);
    if (!db.open())
        qCritical() << "Failed to open the db";

    QTcpServer* server = new HttpServer(&app);
    if (!server->listen(QHostAddress::Any, 80))
        qFatal("Failed to listen");

    return app.exec();
}

#include "http_server.moc"
