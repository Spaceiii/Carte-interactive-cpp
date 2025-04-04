#include "mapwidget.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent), networkManager(new QNetworkAccessManager(this)) {
    setMinimumSize(520, 520);
}

void MapWidget::setCenter(double lon, double lat) {
    this->lon = lon;
    this->lat = lat;
    updateVisibleTiles();
    update();
}

void MapWidget::setZoom(int zoom) {
    this->zoom = zoom;
    updateVisibleTiles();
    update();
}

void MapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPointF centerTilePos = latLonToTilePos(lon, lat, zoom);
    int tileX = static_cast<int>(centerTilePos.x());
    int tileY = static_cast<int>(centerTilePos.y());

    bool isLoading = false;

    for (int x = tileX - 1; x <= tileX + 1; ++x) {
        for (int y = tileY - 1; y <= tileY + 1; ++y) {
            QString tileKey = QString("%1-%2-%3").arg(zoom).arg(x).arg(y);
            if (!tileCache.contains(tileKey)) {
                isLoading = true;
                loadTile(x, y, zoom);
            }
            if (tileCache.contains(tileKey)) {
                QPixmap tile = tileCache[tileKey];
                QPointF tilePos = QPointF((x - tileX) * 256, (y - tileY) * 256);
                painter.drawPixmap(tilePos, tile);
            }
        }
    }

    if (isLoading) {
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 40));
        painter.drawText(event->rect(), Qt::AlignCenter, "EN ATTENTE");
    }
}

QPointF MapWidget::latLonToTilePos(double lon, double lat, int z) {
    double x = (lon + 180.0) / 360.0 * (1 << z);
    double y = (1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * (1 << z);
    return QPointF(x, y);
}

QPointF MapWidget::screenPosToLatLon(const QPoint &pos) {
    QPointF centerTilePos = latLonToTilePos(lon, lat, zoom);
    int tileX = static_cast<int>(centerTilePos.x());
    int tileY = static_cast<int>(centerTilePos.y());

    double x = (pos.x() / 256.0) + tileX - 1;
    double y = (pos.y() / 256.0) + tileY - 1;

    double lon = x / (1 << zoom) * 360.0 - 180.0;
    double lat = atan(sinh(M_PI * (1 - 2 * y / (1 << zoom)))) * 180.0 / M_PI;

    return QPointF(lon, lat);
}

void MapWidget::loadTile(int x, int y, int z) {
    QString tileKey = QString("%1-%2-%3").arg(z).arg(x).arg(y);
    QString tilePath = QString("tiles/%1/%2/%3.png").arg(z).arg(x).arg(y);

    if (QFile::exists(tilePath)) {
        tileCache[tileKey] = QPixmap(tilePath);
    } else {
        QUrl url(QString("https://tile.openstreetmap.org/%1/%2/%3.png").arg(z).arg(x).arg(y));
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent", "laCarteuuu/1.0");

        QNetworkReply *reply = networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply, tileKey, tilePath, x, y, z]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QPixmap pixmap;
                pixmap.loadFromData(data);
                if (!pixmap.isNull()) {
                    tileCache[tileKey] = pixmap;
                    QDir().mkpath(QFileInfo(tilePath).absolutePath());
                    pixmap.save(tilePath);
                    update();
                }
            } else {
                QTimer::singleShot(1000, this, [this, x, y, z](){
                    loadTile(x, y, z);
                });
            }
            reply->deleteLater();
        });
    }
}

void MapWidget::updateVisibleTiles() {
    tileCache.clear();
    QPointF centerTilePos = latLonToTilePos(lon, lat, zoom);
    int tileX = static_cast<int>(centerTilePos.x());
    int tileY = static_cast<int>(centerTilePos.y());

    for (int x = tileX - 1; x <= tileX + 1; ++x) {
        for (int y = tileY - 1; y <= tileY + 1; ++y) {
            loadTile(x, y, zoom);
        }
    }
}
