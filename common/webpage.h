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

#ifndef WEBPAGE_H
#define WEBPAGE_H

 #include <QtWebKitWidgets/QWebPage>

class WebPage : public QWebPage
{
    Q_OBJECT
public:
    WebPage(QObject* parent = 0);

protected:
    virtual void javaScriptAlert(QWebFrame* frame, const QString& msg);
    virtual bool javaScriptConfirm(QWebFrame* frame, const QString& msg);
    virtual bool javaScriptPrompt(QWebFrame* frame, const QString& msg, const QString& defaultValue, QString* result);

private slots:
    bool shouldInterruptJavaScript();
};

#endif // WEBPAGE_H
