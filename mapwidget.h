#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    MapWidget(QWidget *parent = nullptr);
    void setZoomLevel(int zoomLevel);
    void setCenterCoordinates(double longitude, double latitude);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int zoomLevel;
    double centerLongitude;
    double centerLatitude;
    QNetworkAccessManager *networkManager;
    QPixmap loadTile(int x, int y, int zoom);

private slots:
    void onTileDownloaded(QNetworkReply *reply);
};

#endif // MAPWIDGET_H
