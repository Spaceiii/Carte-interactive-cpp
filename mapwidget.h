#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QMap>
#include <QNetworkAccessManager>

class MapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);

    void setCenter(double lon, double lat);
    void setZoom(int zoom);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPointF latLonToTilePos(double lon, double lat, int z);
    QPointF screenPosToLatLon(const QPoint &pos);
    void loadTile(int x, int y, int z);
    void updateVisibleTiles();

    QNetworkAccessManager *networkManager;
    QMap<QString, QPixmap> tileCache;
    double lon;
    double lat;
    int zoom;
};

#endif // MAPWIDGET_H
