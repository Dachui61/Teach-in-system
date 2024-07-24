#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QSplitter>
#include "CameraWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QLabel>
#include <QTimer>
#include <QResizeEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSwitchToMain(CameraWidget* widget);


private:
    void setupControlPanel();
    void setupThumbnails();

    CameraWidget *mainCameraWidget;
    QWidget *controlPanel;
    QScrollArea *thumbnailArea;
    QWidget *thumbnailContainer;
    QSplitter *topSplitter;
};

#endif // MAINWINDOW_H
