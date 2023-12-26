#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDateTime>
#include "photowindow.h"
#include "favouritewindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void showEvent(QShowEvent *event);

public slots:
    void onNoteChanged();

private slots:
    void onListItemSelected();
    void onSearchFieldTextChanged(const QString &text);
    void onSaveButtonClicked();
    void onShowFullSizeButtonClicked();
    void onOpenWindowButtonClicked();
    void saveNotesAndTimes();
    void loadNotesAndTimes();
    void updateLastNoteLabel();


private:
    QSqlDatabase db;
    QLineEdit *searchField;
    QLabel *imageLabel;
    QLabel *shortDescription;
    QPushButton *saveButton;  // Добавьте эту кнопку
    QPushButton *showFullSizeButton;
    QPushButton *openWindowButton;
    QListWidget *listWidget;
    QTextBrowser *textBrowser;
    QHBoxLayout *mainLayout;
    QString currentImagePath;  // Добавьте это
    PhotoWindow *PhotoWindowInstance;
    QStringList elements;
    QString lastSelectedElement;
    QTextEdit *textEdit;
    QListWidgetItem *lastSelectedItem;
    QMap<QString, QString> notes;
    QMap<QString, QDateTime> lastChangeTimes;
    QLabel *lastNoteLabel;
    QDateTime lastNoteTime;
    bool ignoreChanges;
};

#endif // MAINWINDOW_H
