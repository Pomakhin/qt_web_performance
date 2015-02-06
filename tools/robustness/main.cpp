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

#include <QApplication>
#include <QBasicTimer>
#include <QDebug>
#include <QSet>

#include <qwebelement.h>
#include <qwebframe.h>
#include <qwebview.h>
#include <qwebpage.h>

class WebPage : public QWebPage
{
    Q_OBJECT

public:
    WebPage(QWidget *parent)
        : QWebPage(parent)
        , m_blockJs(false)
    {}

    void setBlockJSPrompt(bool block)
    {
        m_blockJs = block;
    }

    virtual void javaScriptAlert(QWebFrame *originatingFrame, const QString& msg);
    virtual bool javaScriptConfirm(QWebFrame *originatingFrame, const QString& msg);
    virtual bool javaScriptPrompt(QWebFrame *originatingFrame, const QString& msg, const QString& defaultValue, QString* result);
    virtual void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);

private:
    bool m_blockJs;
};

void WebPage::javaScriptAlert(QWebFrame *originatingFrame, const QString& msg)
{
    if (m_blockJs)
        return;

    return QWebPage::javaScriptAlert(originatingFrame, msg);
}

bool WebPage::javaScriptConfirm(QWebFrame *originatingFrame, const QString& msg)
{
    if (m_blockJs)
        return false;

    return QWebPage::javaScriptConfirm(originatingFrame, msg);
}

bool WebPage::javaScriptPrompt(QWebFrame *originatingFrame, const QString& msg, const QString& defaultValue, QString* result)
{
    if (m_blockJs)
        return false;

    return QWebPage::javaScriptPrompt(originatingFrame, msg, defaultValue, result);
}

void WebPage::javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID)
{
    if (m_blockJs)
        return;

    return QWebPage::javaScriptConsoleMessage(message, lineNumber, sourceID);
}

// Some sort of a fuzzer. Start with a webpage and then navigate
// through the world wide web.
class Crawler : public QObject {
    Q_OBJECT
public:
    Crawler(const QString& startUrl)
        : m_startUrl(startUrl)
        , m_webView(0)
        , m_playGame(false)
    {}

    void setView(QWebView* view)
    {
        qobject_cast<WebPage*>(view->page())->setBlockJSPrompt(true);

        m_webView = view;
        connect(m_webView, SIGNAL(loadFinished(bool)),
                SLOT(loadFinished(bool)), Qt::QueuedConnection);
    }

    void setGameMode(bool playGame)
    {
        m_playGame = playGame;
    }

public Q_SLOTS:
    void start()
    {
        m_openList.insert(QUrl(m_startUrl));
        navigate(m_startUrl);
    }

private Q_SLOTS:
    void loadFinished(bool finished)
    {
        /* a little game mode */
        if (m_playGame) {
            if (!finished) {
                qFatal("Game Over... Loading failed..");
            } else {
                start();
            }

            return;
        }


        if (!finished)
            qWarning() << "Load of " << m_webView->page()->mainFrame()->requestedUrl() << " failed";

        // extend the closed list with what have been loaded
        recursivelyExcludeLoadedUrls(m_webView->page()->mainFrame());

        // find the new urls
        recursivelyFindNewUrls(m_webView->page()->mainFrame());

        // pick a "random" link
        if (m_openList.isEmpty()) {
            qWarning("All the web has been visited :)");
            qApp->exit();
        }

        QSet<QUrl>::iterator openListIterator = m_openList.begin();
        const int rand = random() % m_openList.size();
        openListIterator += rand;

        navigate(*openListIterator);
    }

private:
    void navigate(const QUrl& target)
    {
        // timeout, in case we will get stuck
        m_timeout.start(3 * 60 * 1000, this);

        qDebug() << "Visiting: " << target.toString();
        m_closedList.insert(target);
        m_openList.remove(target);
        m_webView->load(target);
    }

    void timerEvent(QTimerEvent* e)
    {
        if (m_timeout.timerId() == e->timerId()) {
            qWarning("timeout");
            loadFinished(false);
        }

        QObject::timerEvent(e);
    }

    void recursivelyExcludeLoadedUrls(const QWebFrame* const frame)
    {
        m_openList.remove(frame->url());
        m_closedList.insert(frame->url());

        m_openList.remove(frame->requestedUrl());
        m_closedList.insert(frame->requestedUrl());

        foreach (const QWebFrame* const childFrame, frame->childFrames()) {
            recursivelyExcludeLoadedUrls(childFrame);
        }
    }

    void recursivelyFindNewUrls(const QWebFrame* const frame)
    {
        const QList<QWebElement> result = frame->findAllElements("a").toList();
        if (result.isEmpty())
            return;

        foreach (const QWebElement &element, result) {
            QUrl href(element.attribute("href"));
            if (href.isRelative())
                href = frame->url().resolved(href);
            else if (!href.scheme().startsWith("http"))
                continue;

            // some content use fragment to encode the page state, but it is too much
            // overhead to test all urls with fragments just for the few pages that use
            // it for encoding a state
            href.setFragment(QString());

            if (!m_closedList.contains(href))
                m_openList.insert(href);
        }

        foreach (const QWebFrame* const childFrame, frame->childFrames()) {
            recursivelyFindNewUrls(childFrame);
        }
    }


private:
    QString m_startUrl;
    QBasicTimer m_timeout;
    QWebView* m_webView;
    QSet<QUrl> m_openList;
    QSet<QUrl> m_closedList;
    bool m_playGame;
};


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);

    QStringList arguments = app.arguments();

    QWebView *view = new QWebView;
    view->setPage(new WebPage(view));

    qWarning() << arguments;

    Crawler crawler(arguments.size() >= 2 ? arguments.at(1) : QLatin1String("http://www.digg.com"));
    crawler.setGameMode(arguments.size() >= 3);
    view->showMaximized();
    crawler.setView(view);
    crawler.start();

    return app.exec();
}

#include "main.moc"
