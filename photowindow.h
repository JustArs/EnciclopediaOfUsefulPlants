#ifndef PHOTOWINDOW_H
#define PHOTOWINDOW_H

#include <QDialog>
#include <QMainWindow>

class PhotoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PhotoWindow(const QString &imagePath, QWidget *parent = nullptr);

private slots:

private:
    // Здесь вы можете добавить свои виджеты, такие как кнопки, метки и т.д.
};

#endif // NEWWINDOW_H
