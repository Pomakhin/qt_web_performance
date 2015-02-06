#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "databasenetworkaccessmanager.h"

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QShowEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->addressBar, SIGNAL(returnPressed()), this, SLOT(loadPage()));
    connect(ui->goButton, SIGNAL(clicked()), this, SLOT(loadPage()));
    connect(ui->webView, SIGNAL(loadProgress(int)), this, SLOT(updateLoadProgress(int)));
    connect(ui->urlList, SIGNAL(clicked(QModelIndex)), this, SLOT(loadPageFromList(QModelIndex)));

    // load the database if specified with -database
    const QStringList arguments = QCoreApplication::arguments();
    for (int i = 0; i < arguments.size(); ++i) {
        if (arguments.at(i) == QString::fromLatin1("-database")) {
            ++i;
            if (connectToDatabase(arguments.at(i)))
                break;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent* e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::showEvent(QShowEvent* event)
{
    if (!event->spontaneous() && !QSqlDatabase::database().isValid()) {
        do {
            QString path = QFileDialog::getOpenFileName(this,
                                                        tr("Choose a database file"));
            if (path.isNull()) {
                QTimer::singleShot(0, qApp, SLOT(quit()));
                hide();
                return;
            }
            if (connectToDatabase(path)) {
                break;
            } else {
                QMessageBox::warning(this,
                                     tr("Invalid database file"),
                                     tr("The database file specified is invalid"));
            }
        } while(true);
    }
    ui->webView->page()->setNetworkAccessManager(new DatabaseNetworkAccessManager);
    populateFromDatabase();
    QMainWindow::showEvent(event);
}

void MainWindow::loadPage()
{
    const QString url = ui->addressBar->text();
    ui->webView->load(QUrl::fromUserInput(url));
}

void MainWindow::updateLoadProgress(int value)
{
    ui->statusbar->showMessage(tr("Load progress: %1").arg(value));
}

void MainWindow::loadPageFromList(QModelIndex index)
{
    QModelIndex firstColumnIndex = index.sibling(index.row(), 0);
    ui->addressBar->setText(firstColumnIndex.data().toString());
    loadPage();
}

void MainWindow::populateFromDatabase()
{
    QSqlQuery mirroredUrl;
    mirroredUrl.prepare("SELECT url, description, date FROM mirrored_urls");
    if (mirroredUrl.exec()) {
        QStandardItemModel* model = new QStandardItemModel(this);
        model->setColumnCount(3);
        while (mirroredUrl.next()) {
            QList<QStandardItem*> items;
            items.append(new QStandardItem(mirroredUrl.value(0).toString()));
            items.append(new QStandardItem(mirroredUrl.value(1).toString()));

            const QDateTime date = QDateTime::fromString(mirroredUrl.value(2).toString(), Qt::ISODate);
            items.append(new QStandardItem(date.toString(Qt::DefaultLocaleLongDate)));
            model->appendRow(items);
        }
        ui->urlList->setModel(model);
        ui->urlList->resizeColumnsToContents();
    }
}
