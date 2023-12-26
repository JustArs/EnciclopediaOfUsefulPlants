#include "photowindow.h"
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

PhotoWindow::PhotoWindow(const QString &imagePath, QWidget *parent) :
    QDialog(parent)
{
    // Создайте новую метку для изображения
    QLabel *imageLabel = new QLabel(this);

    // Загрузите изображение
    QPixmap image(imagePath);

    // Увеличьте изображение в два раза
    image = image.scaled(image.width() * 1.2, image.height() * 1.2, Qt::KeepAspectRatio);

    // Установите изображение для метки
    imageLabel->setPixmap(image);

    // Создайте новый слой
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Добавьте метку на слой
    layout->addWidget(imageLabel);

    // Установите слой для этого окна
    setLayout(layout);

}

