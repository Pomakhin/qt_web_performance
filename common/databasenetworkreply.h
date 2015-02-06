#ifndef DATABASENETWORKREPLY_H
#define DATABASENETWORKREPLY_H

#include <QNetworkReply>
#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

class DatabaseNetworkAccessManager;

class DatabaseNetworkReply : public QNetworkReply
{
public:
    DatabaseNetworkReply(const QUrl &url)
        : m_buffer(0)
    {
        setOpenMode(QIODevice::ReadOnly);

        QSqlQuery query;
        query.prepare("SELECT response, header, data from responses where url = ?");
        query.addBindValue(url.toEncoded(QUrl::RemoveFragment));

        if (!query.exec()) {
            qWarning() << "Query failed: " << query.lastError();
            return;
        }
        if (!query.next()) {
            m_content = "HTTP/1.0 404 Page not available in the database \r\nContent-Type: text/html\r\n\r\nNot found.\r\n";
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 404);
        } else {
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, query.value(0));

            QByteArray headers = query.value(1).toByteArray();
            int startPos = 0;
            int delimiterPos = headers.indexOf("\r\n");
            while (delimiterPos > startPos) {
                QByteArray row = headers.mid(startPos, delimiterPos - startPos);
                int columnDelimiterPos = row.indexOf(":");

                QByteArray headerName = row.left(columnDelimiterPos).trimmed();
                QByteArray headerValue = row.mid(columnDelimiterPos+1).trimmed();
                setRawHeader(headerName, headerValue);

                if (headerName.toLower() == "location") {
                    setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl(headerValue));
                }

                startPos = delimiterPos + 2;
                delimiterPos = headers.indexOf("\r\n", startPos);
            }

            QByteArray data = query.value(2).toByteArray();
            m_content += data;
        }
        QTimer::singleShot(0, this, SIGNAL(readyRead()));
        QTimer::singleShot(0, this, SIGNAL(finished()));
    }

    void abort()
    {
        if (m_buffer) {
            delete m_buffer;
            m_buffer = 0;
        }
    }

    qint64 bytesAvailable() const
    {
        openBuffer();
        return m_buffer->bytesAvailable();
    }

protected:
    qint64 readData(char* data, qint64 maxSize)
    {
        openBuffer();
        return m_buffer->read(data, maxSize);
    }

private:
    void openBuffer() const
    {
        if (m_buffer)
            return;
        m_buffer = new QBuffer(const_cast<QByteArray*>(&m_content), const_cast<DatabaseNetworkReply*>(this));
        m_buffer->open(QIODevice::ReadOnly);
    }

    QByteArray m_content;
    mutable QBuffer* m_buffer;

    friend class DatabaseNetworkAccessManager;
};

#endif // DATABASENETWORKREPLY_H
