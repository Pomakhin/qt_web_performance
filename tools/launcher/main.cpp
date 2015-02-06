#include <QApplication>
#include <qwebpage.h>
#include <qwebview.h>

#include <QDebug>

class Monitor : public QObject {
    Q_OBJECT
public:
    Monitor(QWebView* view)
        : m_view(view)
    {
        connect(view, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    }

public Q_SLOTS:
    void loadFinished(bool result) {
        qWarning("Load finished with result: %d on '%s'", result, qPrintable(m_view->url().toString()));
    }

private:
    QWebView* m_view;
};


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWebSettings::globalSettings()->setMaximumPagesInCache(0);
    QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);
    QWebView* view = new QWebView;
    Monitor mon(view);

    view->show();
    view->showFullScreen();

    if (app.arguments().size() >= 2)
        view->load(QUrl(app.arguments().at(1)));
    else
        view->load(QUrl("http://doc.qtsoftware.com"));

    return app.exec();
}

#include "main.moc"
