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

#ifndef WebCore_classes_h
#define WebCore_classes_h

#include <QUrl>

class QNetworkAccessManager;
extern QNetworkAccessManager* globalManager;

namespace WebCore {
    class ResourceHandleClient;

    class KURL : public QUrl {
    public:
        KURL(const QUrl& url)
            : QUrl(url)
        {}
    };

    class ResourceRequest {
    public:
        ResourceRequest(const KURL& url)
            : m_url(url)
        {}

        QByteArray httpMethod() const { return "GET"; }
        void setURL(const KURL& url) { m_url = url; }
        void setHTTPMethod(const QByteArray& data) { if (data != "GET") qFatal("BAD BAD"); }

        KURL url() const { return m_url; }

        QNetworkRequest toNetworkRequest() const
        {
            QNetworkRequest req;
            // known slicing...
            req.setUrl(m_url);

            #ifdef __GNUC__
            #warning "TODO: also set raw headers... this might make a difference"
            #endif
            return req;
        }

    private:
        KURL m_url;
    };

    class ResourceResponse {
    public:
        ResourceResponse(const KURL& url, const QString& mimeType,
                         qlonglong length, const QString& encoding, const QString)
            : m_url(url)
            , m_mimeType(mimeType)
            , m_length(length)
            , m_encoding(encoding)
        {}

        void setHTTPStatusCode(int code) { m_code = code; }
        void setHTTPStatusText(const QByteArray& text) { m_text = text; }
        void setHTTPHeaderField(const QString& header, const QString& val) {}

    private:
        KURL m_url;
        QString m_mimeType;
        qlonglong m_length;
        QString m_encoding;
        int m_code;
        QByteArray m_text;
    };

    class ResourceHandle {
    public:
        ResourceHandle(ResourceHandleClient* client, const ResourceRequest& request)
            : m_client(client)
            , m_request(request)
        {
        }

        ResourceHandleClient* client() const { return m_client; }
        ResourceRequest request() const { return m_request; }

    private:
        ResourceHandleClient* m_client;
        ResourceRequest m_request;
    };

    class ResourceError {
    public:
        ResourceError(const QString&, int, const QString&, const QString&)
        { qFatal("Errors are not allowed here."); }
    };

    class ResourceHandleClient {
    public:
        virtual void didFail(ResourceHandle*, const ResourceError& error) { qFatal("Should not fail!!!"); }
        virtual void didReceiveResponse(ResourceHandle*, const ResourceResponse&) = 0;
        virtual void didReceiveData(ResourceHandle*, const char*, int, int) = 0;
        virtual void didFinishLoading(ResourceHandle*) = 0;
        virtual void willSendRequest(ResourceHandle*, const ResourceRequest& req, const ResourceResponse&) = 0;
    };
}

#endif
