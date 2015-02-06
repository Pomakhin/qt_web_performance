/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.  <info@staikos.net>
    Copyright (C) 2008 Holger Hans Peter Freyther

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "QNetworkReplyHandler.h"

#include "WebCore_classes.h"
#include <QDateTime>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <qwebframe.h>
#include <qwebpage.h>

#include <QDebug>
#include <QCoreApplication>

namespace WebCore {

QNetworkReplyHandler::QNetworkReplyHandler(ResourceHandle* handle, LoadMode loadMode)
    : QObject(0)
    , m_reply(0)
    , m_resourceHandle(handle)
    , m_redirected(false)
    , m_responseSent(false)
    , m_responseDataSent(false)
    , m_loadMode(loadMode)
    , m_shouldStart(true)
    , m_shouldFinish(false)
    , m_shouldSendResponse(false)
    , m_shouldForwardData(false)
{
    const ResourceRequest &r = m_resourceHandle->request();

    if (r.httpMethod() == "GET")
        m_method = QNetworkAccessManager::GetOperation;
    else if (r.httpMethod() == "HEAD")
        m_method = QNetworkAccessManager::HeadOperation;
    else if (r.httpMethod() == "POST")
        m_method = QNetworkAccessManager::PostOperation;
    else if (r.httpMethod() == "PUT")
        m_method = QNetworkAccessManager::PutOperation;
    else
        m_method = QNetworkAccessManager::UnknownOperation;

    m_request = r.toNetworkRequest();

    if (m_loadMode == LoadNormal)
        start();
}

void QNetworkReplyHandler::setLoadMode(LoadMode mode)
{
    // https://bugs.webkit.org/show_bug.cgi?id=26556
    // We cannot call sendQueuedItems() from here, because the signal that 
    // caused us to get into deferred mode, might not be processed yet.
    switch (mode) {
    case LoadNormal:
        m_loadMode = LoadResuming;
        emit processQueuedItems();
        break;
    case LoadDeferred:
        m_loadMode = LoadDeferred;
        break;
    case LoadResuming:
        Q_ASSERT(0); // should never happen
        break;
    };
}

void QNetworkReplyHandler::abort()
{
    m_resourceHandle = 0;
    if (m_reply) {
        QNetworkReply* reply = release();
        reply->abort();
        reply->deleteLater();
        deleteLater();
    }
}

QNetworkReply* QNetworkReplyHandler::release()
{
    QNetworkReply* reply = m_reply;
    if (m_reply) {
        disconnect(m_reply, 0, this, 0);
        // We have queued connections to the QNetworkReply. Make sure any
        // posted meta call events that were the result of a signal emission
        // don't reach the slots in our instance.
        QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
        m_reply = 0;
    }
    return reply;
}

void QNetworkReplyHandler::finish()
{
    m_shouldFinish = (m_loadMode != LoadNormal);
    if (m_shouldFinish)
        return;

    sendResponseIfNeeded();

    if (!m_resourceHandle)
        return;
    ResourceHandleClient* client = m_resourceHandle->client();
    if (!client) {
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }
    QNetworkReply* oldReply = m_reply;
    if (m_redirected) {
        resetState();
        start();
    } else if (m_reply->error() != QNetworkReply::NoError
               // a web page that returns 401/403/404 can still have content
               && ((m_reply->error() != QNetworkReply::ContentOperationNotPermittedError
               && m_reply->error() != QNetworkReply::ContentNotFoundError
               && m_reply->error() != QNetworkReply::ProtocolUnknownError
               && m_reply->error() != QNetworkReply::UnknownContentError)
               // If the web page sent content, let's give it to the user.
               || !m_responseDataSent)
               && m_reply->error() != QNetworkReply::AuthenticationRequiredError
               && m_reply->error() != QNetworkReply::ProxyAuthenticationRequiredError) {
        QUrl url = m_reply->url();
        ResourceError error(url.host(), m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                            url.toString(), m_reply->errorString());
        client->didFail(m_resourceHandle, error);
    } else {
        client->didFinishLoading(m_resourceHandle);
    }
    oldReply->deleteLater();
    if (oldReply == m_reply)
        m_reply = 0;
}

void QNetworkReplyHandler::sendResponseIfNeeded()
{
    m_shouldSendResponse = (m_loadMode != LoadNormal);
    if (m_shouldSendResponse)
        return;

    if (m_responseSent || !m_resourceHandle)
        return;
    m_responseSent = true;

    ResourceHandleClient* client = m_resourceHandle->client();
    if (!client)
        return;

    QString contentType = m_reply->header(QNetworkRequest::ContentTypeHeader).toString();
#if 1
    QString encoding;
    QString mimeType;
#else
    QString encoding = extractCharsetFromMediaType(contentType);
    QString mimeType = extractMIMETypeFromMediaType(contentType);

    if (mimeType.isEmpty()) {
        // let's try to guess from the extension
        QString extension = m_reply->url().path();
        int index = extension.lastIndexOf(QLatin1Char('.'));
        if (index > 0) {
            extension = extension.mid(index + 1);
            mimeType = MIMETypeRegistry::getMIMETypeForExtension(extension);
        }
    }
#endif

    KURL url(m_reply->url());
#if 0
    QString suggestedFilename = filenameFromHTTPContentDisposition(QString::fromAscii(m_reply->rawHeader("Content-Disposition")));

    if (suggestedFilename.isEmpty())
        suggestedFilename = url.lastPathComponent();
#endif

    ResourceResponse response(url, mimeType,
                              m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong(),
                              encoding,
                              QString());

    const bool isLocalFileReply = (m_reply->url().scheme() == QLatin1String("file"));
    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (!isLocalFileReply) {
        response.setHTTPStatusCode(statusCode);
        response.setHTTPStatusText(m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray().constData());
    }
    else if (m_reply->error() == QNetworkReply::ContentNotFoundError)
        response.setHTTPStatusCode(404);


    /* Fill in the other fields
     * For local file requests remove the content length and the last-modified
     * headers as required by fast/dom/xmlhttprequest-get.xhtml
     */
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
   foreach (QNetworkReply::RawHeaderPair rawHeader, m_reply->rawHeaderPairs()) {
        const QByteArray headerName = rawHeader.first;
       if (isLocalFileReply
           && (headerName == "Content-Length" || headerName == "Last-Modified"))
           continue;

       response.setHTTPHeaderField(QString::fromAscii(headerName), QString::fromAscii(rawHeader.second));
   }
#else
    foreach (QByteArray headerName, m_reply->rawHeaderList()) {

        if (isLocalFileReply
            && (headerName == "Content-Length" || headerName == "Last-Modified"))
            continue;

        response.setHTTPHeaderField(QString::fromAscii(headerName), QString::fromAscii(m_reply->rawHeader(headerName)));
    }
#endif

    if (isLocalFileReply)
        response.setHTTPHeaderField(QString::fromAscii("Cache-Control"), QString::fromAscii("no-cache"));

    QUrl redirection = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirection.isValid()) {
        QUrl newUrl = m_reply->url().resolved(redirection);
        ResourceRequest newRequest = m_resourceHandle->request();
        newRequest.setURL(newUrl);

        if (((statusCode >= 301 && statusCode <= 303) || statusCode == 307) && m_method == QNetworkAccessManager::PostOperation) {
            m_method = QNetworkAccessManager::GetOperation;
            newRequest.setHTTPMethod("GET");
        }

        client->willSendRequest(m_resourceHandle, newRequest, response);
        m_redirected = true;
        m_request = newRequest.toNetworkRequest();
    } else {
        client->didReceiveResponse(m_resourceHandle, response);
    }
}

void QNetworkReplyHandler::forwardData()
{
    m_shouldForwardData = (m_loadMode != LoadNormal);
    if (m_shouldForwardData)
        return;

    sendResponseIfNeeded();

    // don't emit the "Document has moved here" type of HTML
    if (m_redirected)
        return;

    if (!m_resourceHandle)
        return;

    QByteArray data = m_reply->read(m_reply->bytesAvailable());

    ResourceHandleClient* client = m_resourceHandle->client();
    if (!client)
        return;

    if (!data.isEmpty()) {
        m_responseDataSent = true;
        client->didReceiveData(m_resourceHandle, data.constData(), data.length(), data.length() /*FixMe*/);
    }
}

void QNetworkReplyHandler::start()
{
    m_shouldStart = false;

    QNetworkAccessManager* manager = globalManager;
#if 0
    ResourceHandleInternal* d = m_resourceHandle->getInternal();

    QNetworkAccessManager* manager = d->m_frame->page()->networkAccessManager();
#endif

    const QUrl url = m_request.url();
    const QString scheme = url.scheme();
    // Post requests on files and data don't really make sense, but for
    // fast/forms/form-post-urlencoded.html and for fast/forms/button-state-restore.html
    // we still need to retrieve the file/data, which means we map it to a Get instead.
    if (m_method == QNetworkAccessManager::PostOperation
        && (!url.toLocalFile().isEmpty() || url.scheme() == QLatin1String("data")))
        m_method = QNetworkAccessManager::GetOperation;

    switch (m_method) {
        case QNetworkAccessManager::GetOperation:
            m_reply = manager->get(m_request);
            break;
        case QNetworkAccessManager::PostOperation: {
            qFatal("POST OPERATION is not allowed");
            break;
        }
        case QNetworkAccessManager::HeadOperation:
            m_reply = manager->head(m_request);
            break;
        case QNetworkAccessManager::PutOperation: {
            qFatal("PUT OPERATION is not allowed");
            break;
        }
        case QNetworkAccessManager::UnknownOperation: {
            m_reply = 0;
            ResourceHandleClient* client = m_resourceHandle->client();
            if (client) {
                ResourceError error(url.host(), 400 /*bad request*/,
                                    url.toString(),
                                    QCoreApplication::translate("QWebPage", "Bad HTTP request"));
                client->didFail(m_resourceHandle, error);
            }
            return;
        }
    }

    m_reply->setParent(this);

    connect(m_reply, SIGNAL(finished()),
            this, SLOT(finish()), Qt::QueuedConnection);

    // For http(s) we know that the headers are complete upon metaDataChanged() emission, so we
    // can send the response as early as possible
    if (scheme == QLatin1String("http") || scheme == QLatin1String("https"))
        connect(m_reply, SIGNAL(metaDataChanged()),
                this, SLOT(sendResponseIfNeeded()), Qt::DirectConnection);

    connect(m_reply, SIGNAL(readyRead()),
            this, SLOT(forwardData()), Qt::DirectConnection);
    connect(this, SIGNAL(processQueuedItems()),
            this, SLOT(sendQueuedItems()), Qt::DirectConnection);
}

void QNetworkReplyHandler::resetState()
{
    m_redirected = false;
    m_responseSent = false;
    m_responseDataSent = false;
    m_shouldStart = true;
    m_shouldFinish = false;
    m_shouldSendResponse = false;
    m_shouldForwardData = false;
}

void QNetworkReplyHandler::sendQueuedItems()
{
    if (m_loadMode != LoadResuming)
        return;
    m_loadMode = LoadNormal;

    if (m_shouldStart)
        start();

    if (m_shouldSendResponse)
        sendResponseIfNeeded();

    if (m_shouldForwardData)
        forwardData();

    if (m_shouldFinish)
        finish(); 
}

}

#include "moc_QNetworkReplyHandler.cpp"

