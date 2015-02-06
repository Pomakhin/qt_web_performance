/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2009 Holger Hans Peter Freyther
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

#include "cookiejar.h"

#include <QDebug>
#include <QSettings>
#include <QNetworkCookie>

// serializing functions... this is supposed to match with the
// format arora is using... which is just the raw cookie..
static const unsigned int JAR_VERSION = 23;

QDataStream& operator<<(QDataStream& stream, const QList<QNetworkCookie>&)
{
    qFatal("Not implemented");
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QList<QNetworkCookie>& list)
{
    quint32 tmp;
    list = QList<QNetworkCookie>();


    // check the version first
    stream >> tmp;
    if (tmp != JAR_VERSION) {
        qWarning() << "Unsupported version " << tmp;
        return stream;
    }

    stream >> tmp;
    for (quint32 i = 0; i < tmp; ++i) {
        QByteArray value;
        stream >> value;
        list.append(QNetworkCookie::parseCookies(value));

        if (stream.atEnd()) {
            qWarning() << "Premature end of the cookie file..";
            break;
        }
    }

    qWarning() << "Parsed cookie count: " << list.count();
    return stream;
}

CookieJar::CookieJar()
{}

bool CookieJar::load(const QString& fileName)
{
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
    QSettings cookieData(fileName, QSettings::IniFormat);
    setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieData.value(QLatin1String("cookies"))));
    return true;
}
