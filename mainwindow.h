#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QMenu;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QListWidget;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    // All pointers managed by the main window
    QMenu *_file_menu;
    QMenu *_help_menu;
    QScopedPointer<QGroupBox> _main_widget;
    QScopedPointer<QLineEdit> _search_bar;
    QScopedPointer<QPushButton> _search_button;
    QScopedPointer<QListWidget> _suggestions_list;
    QScopedPointer<QLabel> _map_label;
    QNetworkAccessManager *_network_manager;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSearchButtonClicked();
    void onQuitActionTriggered();
    void onAboutActionTriggered();
    void onSearchReplyReceived();

private:
    void performSearch(const QString &query);
};

#endif // MAINWINDOW_H
