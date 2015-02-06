#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent* e);
    void showEvent(QShowEvent* event);

private slots:
    void loadPage();
    void updateLoadProgress(int value);
    void loadPageFromList(QModelIndex index);

private:
    void populateFromDatabase();
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
