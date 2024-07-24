#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include "HCNetSDK.h"

class CameraWidget : public QWidget {
    Q_OBJECT

public:
    CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();

signals:
    void switchToMain(CameraWidget* widget);
    void startRecording(CameraWidget* widget);
    void stopRecording(CameraWidget* widget);
    void viewHistory(CameraWidget* widget);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void showContextMenu(const QPoint &pos);
    void startRecordingSlot();
    void stopRecordingSlot();
    void viewHistorySlot();

private:
    QLabel *videoLabel;
    QTimer *timer;
    LONG lUserID;
    LONG lRealPlayHandle;
    bool isRecording;
    QString recordingPath;
};

#endif // CAMERAWIDGET_H
