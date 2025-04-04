#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QMessageBox>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "mainwindow.h"
#include "mapwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _network_manager(new QNetworkAccessManager(this)) {
    setWindowTitle(QString{"Vive le BUT"});

    // Create menus
    _file_menu = menuBar()->addMenu(QString{tr("&File")});
    _help_menu = menuBar()->addMenu(QString{tr("&Help")});

    // Populate menus (menu items)
    QAction *quitAction = _file_menu->addAction(QString{"Quit"});
    QAction *aboutAction = _help_menu->addAction(QString{"About"});

    // Container for window
    _main_widget.reset(new QGroupBox{this});
    setCentralWidget(_main_widget.get());

    // Main horizontal layout
    QHBoxLayout *mainLayout = new QHBoxLayout{};

    // Search layout (search bar and suggestions)
    QVBoxLayout *searchLayout = new QVBoxLayout{};

    // Search bar
    _search_bar.reset(new QLineEdit{_main_widget.get()});
    _search_bar->setPlaceholderText("Rechercher...");
    searchLayout->addWidget(_search_bar.get());

    // Search button
    _search_button.reset(new QPushButton{QString{"Search"}, _main_widget.get()});
    searchLayout->addWidget(_search_button.get());

    // Suggestions list (static suggestions)
    _suggestions_list.reset(new QListWidget{_main_widget.get()});
    searchLayout->addWidget(_suggestions_list.get());

    // Add search layout to main layout
    mainLayout->addLayout(searchLayout);

    // Put main layout into central widget
    _main_widget->setLayout(mainLayout);

    // Map widget on the right side
    MapWidget *mapWidget = new MapWidget(_main_widget.get());
    mapWidget->setZoomLevel(10);
    mapWidget->setCenterCoordinates(6.839349, 47.64263);

    mainLayout->addWidget(mapWidget);

    // Connect the search button to the slot
    connect(_search_button.get(), &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);

    // Connect menu actions to slots
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuitActionTriggered);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutActionTriggered);
}

MainWindow::~MainWindow() {}

void MainWindow::onSearchButtonClicked() {
    QString searchText = _search_bar->text();
    if (!searchText.isEmpty()) {
        performSearch(searchText);
        _search_bar->clear(); // Vider le champ de saisie
    }
}

void MainWindow::performSearch(const QString &query) {
    QString url = QString("https://nominatim.openstreetmap.org/search?q=%1&format=json&polygon=0&addressdetails=0")
    .arg(query);
    QNetworkRequest request(url);
    QNetworkReply *reply = _network_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::onSearchReplyReceived);
}

void MainWindow::onSearchReplyReceived() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            QJsonArray jsonArray = jsonDoc.array();

            _suggestions_list->clear();
            for (const QJsonValue &value : jsonArray) {
                QJsonObject obj = value.toObject();
                QString displayName = obj["display_name"].toString();
                double lon = obj["lon"].toDouble();
                double lat = obj["lat"].toDouble();
                _suggestions_list->addItem(displayName);
                // Store coordinates with the item (e.g., using QListWidgetItem::setData)
            }
        } else {
            QMessageBox::warning(this, "Error", "Network error occurred.");
        }
        reply->deleteLater();
    }
}

void MainWindow::onQuitActionTriggered() {
    QApplication::quit();
}

void MainWindow::onAboutActionTriggered() {
    QString aboutText = "Application développée par:\nTimothée MEYER et Hugues ESTRADE\n\n"
                        "Cette application est un projet BUT.\n"
                        "Date: " + QDate::currentDate().toString("yyyy-MM-dd");
    QMessageBox::about(this, tr("À propos"), aboutText);
}
