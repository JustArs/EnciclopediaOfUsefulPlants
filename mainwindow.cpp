#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QSqlError>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QFileInfo>
#include <QLabel>
#include <QPixmap>
#include <QFileDialog>
#include <QSettings>
#include <QMap>
#include <QWidgetItem>
#include <QTimer>
#include <QDateTime>

void MainWindow::onListItemSelected()
{
    QListWidgetItem *selectedItem = listWidget->currentItem();
    if (!selectedItem) {
        return;
    }

    // Сохранение заметки для предыдущего элемента
    if (lastSelectedItem != nullptr && textEdit->toPlainText() != "")
        notes[lastSelectedItem->text()] = textEdit->toPlainText();

    // Отображение заметки для текущего элемента
    if (notes.contains(selectedItem->text()))
        textEdit->setPlainText(notes[selectedItem->text()]);
    else
        textEdit->clear();

    lastSelectedItem = selectedItem;  // Обновление последнего выбранного элемента

    QSqlTableModel *model = new QSqlTableModel(this, db);

    model->setTable("plants");
    model->select();

    for (int i = 0; i < model->rowCount(); i++) {
        QSqlRecord record = model->record(i);
        if (record.value("name").toString() == selectedItem->text()) {
            shortDescription->setText(record.value("short_description").toString());
            textBrowser->setText(record.value("long_description").toString());
            QString imagePath = record.value("image_path").toString();
            if (QFile::exists(imagePath)) {
                QPixmap image(imagePath);
                imageLabel->setPixmap(image.scaled(200, 200, Qt::KeepAspectRatio));
                currentImagePath = imagePath;
            } else {
                qDebug() << "Image file does not exist: " << imagePath;
            }

            break;
        }
    }
    QSettings settings("YourOrganization", "YourApplication");
    settings.setValue("lastSelectedElement", selectedItem->text());
    QList<QListWidgetItem *> items = listWidget->findItems(lastSelectedElement, Qt::MatchExactly);
    if (!items.isEmpty()) {
        listWidget->setCurrentItem(items.first());
        onListItemSelected();  // Вызовите этот слот, чтобы отобразить информацию о выбранном элементе
    }
    saveNotesAndTimes();

    ignoreChanges = true;  // Игнорировать следующие изменения текста

    // Отображение заметки для текущего элемента
    if (notes.contains(selectedItem->text()))
        textEdit->setPlainText(notes[selectedItem->text()]);
    else
        textEdit->clear();

    ignoreChanges = false;  // Прекратить игнорирование изменений текста
}

void MainWindow::onShowFullSizeButtonClicked()
{
    if (!currentImagePath.isEmpty() && QFile::exists(currentImagePath)) {
        QDialog *imageDialog = new QDialog(this);
        imageDialog->setWindowTitle("Полный размер");

        QLabel *imageLabel = new QLabel(imageDialog);
        QPixmap image(currentImagePath);
        imageLabel->setPixmap(image);

        QVBoxLayout *layout = new QVBoxLayout(imageDialog);
        layout->addWidget(imageLabel);

        imageDialog->setLayout(layout);
        imageDialog->exec();
    }
}

void MainWindow::onSaveButtonClicked()
{
    if (!currentImagePath.isEmpty() && QFile::exists(currentImagePath)) {
        QString fileName = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "Изображения (*.png *.xpm *.jpg)");
        if (!fileName.isEmpty()) {
            QFile::copy(currentImagePath, fileName);
        }
    }
}

void MainWindow::updateLastNoteLabel()
{
    QListWidgetItem *currentItem = listWidget->currentItem();
    if (currentItem != nullptr && lastChangeTimes.contains(currentItem->text())) {
        int secondsAgo = lastChangeTimes[currentItem->text()].secsTo(QDateTime::currentDateTime());
        lastNoteLabel->setText("Последнее изменение заметки было " + QString::number(secondsAgo) + " секунд назад");
    } else {
        lastNoteLabel->clear();
    }
}

void MainWindow::onOpenWindowButtonClicked() {
    PhotoWindow *PhotoWindowInstance = new PhotoWindow(currentImagePath, this);

    PhotoWindowInstance->show();
}

void MainWindow::onSearchFieldTextChanged(const QString &text)
{
    for (int i = 0; i < listWidget->count(); i++) {
        QListWidgetItem *item = listWidget->item(i);
        if (item->text().contains(text, Qt::CaseInsensitive)) {
            item->setHidden(false);
        } else {
            item->setHidden(true);
        }
    }
    searchField->setFocus();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    // Найдите элемент с этим текстом и выберите его
    QList<QListWidgetItem *> items = listWidget->findItems(lastSelectedElement, Qt::MatchExactly);
    if (!items.isEmpty()) {
        listWidget->setCurrentItem(items.first());
        onListItemSelected();  // Вызовите этот слот, чтобы отобразить информацию о выбранном элементе
    }
}

void MainWindow::saveNotesAndTimes()
{
    QSettings settings("YourOrganization", "YourApplication");

    settings.beginGroup("Notes");
    for (auto i = notes.begin(); i != notes.end(); ++i) {
        settings.setValue(i.key(), i.value());
    }
    settings.endGroup();

    settings.beginGroup("LastChangeTimes");
    for (auto i = lastChangeTimes.begin(); i != lastChangeTimes.end(); ++i) {
        settings.setValue(i.key(), i.value().toString(Qt::ISODate));
    }
    settings.endGroup();
}

void MainWindow::onNoteChanged()
{
    if (ignoreChanges)
        return;  // Игнорировать это изменение текста

    // Обновление времени последнего изменения для текущей заметки
    QListWidgetItem *currentItem = listWidget->currentItem();
    if (currentItem != nullptr && notes[currentItem->text()] != textEdit->toPlainText()) {
        lastChangeTimes[currentItem->text()] = QDateTime::currentDateTime();
        lastNoteTime = QDateTime::currentDateTime();
    }
}

void MainWindow::loadNotesAndTimes()
{
    QSettings settings("YourOrganization", "YourApplication");

    settings.beginGroup("Notes");
    QStringList keys = settings.childKeys();
    for (const QString &key : keys) {
        notes[key] = settings.value(key).toString();
    }
    settings.endGroup();

    settings.beginGroup("LastChangeTimes");
    keys = settings.childKeys();
    for (const QString &key : keys) {
        lastChangeTimes[key] = QDateTime::fromString(settings.value(key).toString(), Qt::ISODate);
    }
    settings.endGroup();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    db(QSqlDatabase::addDatabase("QSQLITE")),
    searchField(new QLineEdit(this)),
    imageLabel(new QLabel(this)),
    shortDescription(new QLabel(this)),  // Новый QLabel для краткого описания
    saveButton(new QPushButton("Сохранить изображение", this)),
    mainLayout(new QHBoxLayout)
{

    listWidget = new QListWidget(this);
    textBrowser = new QTextBrowser(this);
    textEdit = new QTextEdit(this);
    textEdit->setGeometry(QRect(QPoint(100, 100), QSize(200, 50)));

    QVBoxLayout *leftLayout = new QVBoxLayout;

    QMap<QString, QString> notes; //создание заметки
    connect(listWidget, &QListWidget::currentItemChanged, this, &MainWindow::onListItemSelected);

    leftLayout->addWidget(listWidget);
    leftLayout->insertWidget(0, searchField);
    connect(searchField, &QLineEdit::textChanged, this, &MainWindow::onSearchFieldTextChanged);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    QHBoxLayout *imageLayout = new QHBoxLayout;
    imageLayout->addWidget(imageLabel);
    imageLayout->addWidget(shortDescription);
    imageLayout->setStretchFactor(imageLabel, 1);
    imageLayout->setStretchFactor(shortDescription, 3);
    shortDescription->setWordWrap(true);
    rightLayout->addLayout(imageLayout);
    rightLayout->addWidget(textBrowser);
    rightLayout->addWidget(textEdit);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    listWidget->setFixedWidth(300);
    listWidget->setFixedHeight(600);

    searchField->setFixedWidth(300);
    searchField->setPlaceholderText("Поиск");

    QPushButton *openWindowButton = new QPushButton("Показать в отдельном окне", this);
    connect(openWindowButton, &QPushButton::clicked, this, &MainWindow::onOpenWindowButtonClicked);

    QVBoxLayout *buttonsLayout = new QVBoxLayout;

    // Добавьте openWindowButton под saveButton
    imageLayout->addWidget(openWindowButton);
    imageLayout->addWidget(imageLabel);  // Добавьте imageLabel сверху
    imageLayout->addWidget(shortDescription);  // Добавьте shortDescription под imageLabel
    imageLayout->addWidget(saveButton);  // Добавьте saveButton под shortDescription
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveButtonClicked);  // Подключите сигнал clicked к слоту onSaveButtonClicked

    // Заполнение QListWidget
    QStringList elements;
    elements << "Алоэ Вера" << "Алтей" << "Анютины Глазки" << "Бальзамин" << "Горчица" << "Дягиль" << "Ежевика" << "Женьшень" << "Зверобой" << "Иван-чай" << "Календула" << "Лаванда" << "Мелисса" << "Настурция" << "Облепиха" << "Петрушка" << "Ромашка" << "Солодка" << "Тысячелистник" << "Шалфей";

    for (int i = 0; i < elements.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon("icon.png"));
        item->setText(elements[i]);
        listWidget->addItem(item);
    }

    QTimer *timer = new QTimer(this);
    timer->start(1000);  // Обновление каждую секунду

    // Создание QLabel
    lastNoteLabel = new QLabel(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateLastNoteLabel);
    rightLayout->addWidget(lastNoteLabel);
    connect(textEdit, &QTextEdit::textChanged, this, &MainWindow::onNoteChanged);

    QSettings settings("YourOrganization", "YourApplication");
    QString lastSelectedElement = settings.value("lastSelectedElement").toString();

    // Найдите элемент с этим текстом и выберите его
    QList<QListWidgetItem *> items = listWidget->findItems(lastSelectedElement, Qt::MatchExactly);
    if (!items.isEmpty()) {
        listWidget->setCurrentItem(items.first());
    }

    // Подключение сигнала и слота
    connect(listWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onListItemSelected);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    db.setDatabaseName("mydatabase.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка базы данных", "Не удалось открыть базу данных");
        return;
    }
    QFileInfo fileInfo(db.databaseName());
    QString absolutePath = fileInfo.absoluteFilePath();
    qDebug() << "Абсолютный путь к базе данных:" << absolutePath;

    resize(1280, 720);  // Измените это на желаемый размер окна
    setFixedSize(size());  // Запретить изменение размера окна

    loadNotesAndTimes();
}
