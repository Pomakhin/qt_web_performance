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

#include "webpage.h"

WebPage::WebPage(QObject* parent)
    : QWebPage(parent)
{
}

bool WebPage::shouldInterruptJavaScript()
{
    return false;
}

void WebPage::javaScriptAlert(QWebFrame*, const QString&)
{
    // just swallow the alert
}

bool WebPage::javaScriptConfirm(QWebFrame*, const QString&)
{
    return true;
}

bool WebPage::javaScriptPrompt(QWebFrame*, const QString&, const QString&, QString*)
{
    return false;
}
