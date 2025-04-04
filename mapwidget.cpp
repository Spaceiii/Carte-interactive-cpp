#include "mapwidget.h"
#include <QPainter>
#include <cmath>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent), zoomLevel(10), centerLongitude(6.839349), centerLatitude(47.64263), networkManager(new QNetworkAccessManager(this)) {}

void MapWidget::setZoomLevel(int zoomLevel) {
    this->zoomLevel = zoomLevel;
    update();
}

void MapWidget::setCenterCoordinates(double longitude, double latitude) {
    this->centerLongitude = longitude;
    this->centerLatitude = latitude;
    update();
}

void MapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    int tileSize = 256;
    int numTilesX = width() / tileSize + 2;
    int numTilesY = height() / tileSize + 2;

    // Calculate the tile coordinates for the center of the map
    int centerTileX = static_cast<int>((centerLongitude + 180.0) / 360.0 * (1 << zoomLevel));
    int centerTileY = static_cast<int>((1.0 - log(tan(centerLatitude * M_PI / 180.0) + 1.0 / cos(centerLatitude * M_PI / 180.0)) / M_PI) / 2.0 * (1 << zoomLevel));

    for (int x = -numTilesX / 2; x < numTilesX / 2; ++x) {
        for (int y = -numTilesY / 2; y < numTilesY / 2; ++y) {
            int tileX = centerTileX + x;
            int tileY = centerTileY + y;
            QPixmap tile = loadTile(tileX, tileY, zoomLevel);
            if (!tile.isNull()) {
                int screenX = (x + numTilesX / 2) * tileSize;
                int screenY = (y + numTilesY / 2) * tileSize;
                painter.drawPixmap(screenX, screenY, tile);
            } else {
                qDebug() << "Failed to load tile at" << tileX << tileY << zoomLevel;
            }
        }
    }
}

QPixmap MapWidget::loadTile(int x, int y, int zoom) {
    QString url = QString("https://maptiles.p.rapidapi.com/fr/map/v1/%1/%2/%3.png").arg(zoom).arg(x).arg(y);
    qDebug() << "Loading tile from URL:" << url;

    QNetworkRequest request(url);
    QByteArray apiKey = "a7e023fad0msh46f2825da56deb7p1945d1jsn5496bbf3a80d"; // Remplace par ta clé API
    request.setRawHeader("x-rapidapi-key", apiKey);
    request.setRawHeader("x-rapidapi-host", "maptiles.p.rapidapi.com");

    QNetworkReply *reply = networkManager->get(request);

    // Connect the reply's finished signal to the slot
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onTileDownloaded(reply);
    });

    // Return a blank pixmap while waiting for the download
    return QPixmap();
}

void MapWidget::onTileDownloaded(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QPixmap pixmap;
        if (pixmap.loadFromData(data)) {
            qDebug() << "Tile downloaded successfully";
            // Cache the pixmap or directly use it in paintEvent
            // For simplicity, we'll just trigger an update to repaint
            update();
        } else {
            qDebug() << "Failed to load pixmap from data";
        }
    } else {
        qDebug() << "Network error:" << reply->errorString();
    }
    reply->deleteLater();
}
