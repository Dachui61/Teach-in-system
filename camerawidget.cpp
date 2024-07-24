#include "CameraWidget.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QWindow>
#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QMenu>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <cstring> // 添加此头文件以使用 strcpy

CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent), lUserID(-1), lRealPlayHandle(-1), isRecording(false) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    videoLabel = new QLabel(this);
    layout->addWidget(videoLabel);

    // 设置窗口默认大小为 320x240
    resize(320, 240);

    // 初始化SDK
    NET_DVR_Init();

    // 设置设备信息
    NET_DVR_USER_LOGIN_INFO loginInfo = {0};
    strcpy(loginInfo.sDeviceAddress, "10.16.26.167"); // 设备IP
    loginInfo.wPort = 8000; // 设备端口
    strcpy(loginInfo.sUserName, "admin"); // 用户名
    strcpy(loginInfo.sPassword, "abcd1234"); // 密码

    NET_DVR_DEVICEINFO_V40 deviceInfo = {0};
    lUserID = NET_DVR_Login_V40(&loginInfo, &deviceInfo);
    if (lUserID < 0) {
        qDebug() << "Login error, error code:" << NET_DVR_GetLastError();
        return;
    }

    // 设置预览参数
    NET_DVR_PREVIEWINFO previewInfo = {0};
    previewInfo.hPlayWnd = reinterpret_cast<HWND>(videoLabel->winId()); // 传递窗口句柄
    previewInfo.lChannel = 1; // 通道号
    previewInfo.dwStreamType = 0; // 主码流
    previewInfo.dwLinkMode = 0; // TCP模式
    previewInfo.bBlocked = 1; // 阻塞方式

    // 启动预览
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &previewInfo, nullptr, nullptr);
    if (lRealPlayHandle < 0) {
        qDebug() << "RealPlay error, error code:" << NET_DVR_GetLastError();
        return;
    }
}

CameraWidget::~CameraWidget() {
    // 停止预览
    if (lRealPlayHandle >= 0) {
        NET_DVR_StopRealPlay(lRealPlayHandle);
    }

    // 注销设备
    if (lUserID >= 0) {
        NET_DVR_Logout(lUserID);
    }

    // 释放SDK资源
    NET_DVR_Cleanup();
}

void CameraWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->pos());
    }
    QWidget::mousePressEvent(event);
}

void CameraWidget::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(tr("Context menu"), this);

    QAction actionSwitchToMain(tr("切换到主监控画面"), this);
    connect(&actionSwitchToMain, &QAction::triggered, this, [this]() { emit switchToMain(this); });
    contextMenu.addAction(&actionSwitchToMain);

    QAction actionStartRecording(tr("开始录屏"), this);
    connect(&actionStartRecording, &QAction::triggered, this, &CameraWidget::startRecordingSlot);
    contextMenu.addAction(&actionStartRecording);

    QAction actionStopRecording(tr("结束录屏"), this);
    connect(&actionStopRecording, &QAction::triggered, this, &CameraWidget::stopRecordingSlot);
    contextMenu.addAction(&actionStopRecording);

    QAction actionViewHistory(tr("查看历史录屏文件"), this);
    connect(&actionViewHistory, &QAction::triggered, this, &CameraWidget::viewHistorySlot);
    contextMenu.addAction(&actionViewHistory);

    contextMenu.exec(mapToGlobal(pos));
}

void CameraWidget::startRecordingSlot() {
    if (!isRecording && lRealPlayHandle >= 0) {
        recordingPath = QFileDialog::getSaveFileName(this, tr("选择录像保存位置"), "", tr("录像文件 (*.mp4 *.avi)"));
        if (!recordingPath.isEmpty()) {
            // 将 recordingPath 转换为非 const char* 类型
            QByteArray recordingPathArray = recordingPath.toLocal8Bit();
            char *recordingPathCStr = recordingPathArray.data();

            if (NET_DVR_SaveRealData(lRealPlayHandle, recordingPathCStr)) {
                isRecording = true;
                qDebug() << "Recording started, saving to:" << recordingPath;
            } else {
                qDebug() << "Failed to start recording, error code:" << NET_DVR_GetLastError();
            }
        }
    }
}

void CameraWidget::stopRecordingSlot() {
    if (isRecording && lRealPlayHandle >= 0) {
        if (NET_DVR_StopSaveRealData(lRealPlayHandle)) {
            isRecording = false;
            qDebug() << "Recording stopped.";
        } else {
            qDebug() << "Failed to stop recording, error code:" << NET_DVR_GetLastError();
        }
    }
}

void CameraWidget::viewHistorySlot() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开录像文件"), "", tr("录像文件 (*.mp4 *.avi)"));
    if (fileName.isEmpty()) return;

    QDialog *playbackDialog = new QDialog(this);
    playbackDialog->setWindowTitle(tr("历史录像"));

    QVBoxLayout *dialogLayout = new QVBoxLayout(playbackDialog);
    QVideoWidget *videoWidget = new QVideoWidget(playbackDialog);
    dialogLayout->addWidget(videoWidget);

    QMediaPlayer *player = new QMediaPlayer(playbackDialog);
    player->setVideoOutput(videoWidget);
    player->setMedia(QUrl::fromLocalFile(fileName));

    connect(player, &QMediaPlayer::stateChanged, playbackDialog, [player, playbackDialog](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            player->deleteLater();
            playbackDialog->deleteLater();
        }
    });

    playbackDialog->resize(640, 480);
    playbackDialog->show();
    player->play();
}
