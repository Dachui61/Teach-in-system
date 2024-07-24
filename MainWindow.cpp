#include "MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建顶部的QSplitter来分隔主监控画面和控制面板
    topSplitter = new QSplitter(Qt::Horizontal, this);
    topSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 主监控画面
    mainCameraWidget = new CameraWidget(this);
    mainCameraWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topSplitter->addWidget(mainCameraWidget);

    // 控制面板
    controlPanel = new QWidget(this);
    setupControlPanel();
    controlPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topSplitter->addWidget(controlPanel);

    // 设置控制面板宽度占比30%
    QList<int> sizes;
    sizes << 70 << 30; // 70% 主监控画面, 30% 控制面板
    topSplitter->setSizes(sizes);

    mainLayout->addWidget(topSplitter, 7); // 70% 主监控画面和控制面板

    // 缩略图滚动区域
    thumbnailArea = new QScrollArea(this);
    thumbnailArea->setWidgetResizable(true);
    thumbnailArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // 设置横向滚动条
    thumbnailArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 关闭纵向滚动条
    thumbnailContainer = new QWidget(thumbnailArea);

    setupThumbnails();

    thumbnailArea->setWidget(thumbnailContainer);

    mainLayout->addWidget(thumbnailArea, 3); // 30% 缩略图滚动区域

    setCentralWidget(centralWidget);

    setWindowTitle("示教系统");
    setFixedSize(1200, 800); // 设置主窗口固定大小

    // 将窗口移动到屏幕中心
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // 延迟调用调整布局比例，确保设置生效
    QTimer::singleShot(0, this, [=]() {
        QList<int> sizes;
        sizes << width() * 0.7 << width() * 0.3; // 70% 主监控画面, 30% 控制面板
        topSplitter->setSizes(sizes);
    });
}


MainWindow::~MainWindow() {
}

void MainWindow::setupControlPanel() {
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);

    // 示例控件
    QLabel *label = new QLabel("控制面板", controlPanel);
    controlLayout->addWidget(label);

    // 添加更多控制面板的控件
}

void MainWindow::setupThumbnails() {
    QHBoxLayout *thumbnailLayout = new QHBoxLayout(thumbnailContainer); // 使用QHBoxLayout布局
    thumbnailLayout->setContentsMargins(0, 0, 0, 0); // 去掉布局的边距
    thumbnailLayout->setSpacing(0); // 去掉缩略图之间的间距

    // 示例缩略图
    for (int i = 0; i < 4; ++i) {
        CameraWidget *thumbnailWidget  = new CameraWidget(this);
        thumbnailWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        thumbnailWidget->setFixedSize(thumbnailArea->height() * 16 / 9, thumbnailArea->height()); // 设置固定大小以保持16:9比例
        thumbnailLayout->addWidget(thumbnailWidget);
        // 连接右键菜单的信号和主窗口的槽函数
        connect(thumbnailWidget, &CameraWidget::switchToMain, this, &MainWindow::onSwitchToMain);
    }

    thumbnailContainer->setLayout(thumbnailLayout);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
//    if (Qt::WindowMaximized) {
//        // Set the window to be always on top
//        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
//    } else {
//        // Remove the always on top flag when not maximized
//        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
//    }


    QMainWindow::resizeEvent(event);

    int newHeight = height() * 0.3; // 30% 高度
    thumbnailArea->setFixedHeight(newHeight);

    // 调整缩略图大小
    for (int i = 0; i < thumbnailContainer->layout()->count(); ++i) {
        QWidget *widget = thumbnailContainer->layout()->itemAt(i)->widget();
        if (widget) {
            widget->setFixedHeight(newHeight);
            widget->setFixedWidth(newHeight * 16 / 9); // 保持16:9比例
        }
    }

    // 调整主监控画面大小
    if (mainCameraWidget) {
        int mainHeight = height() * 0.7; // 70% 高度
        int mainWidth = width() * 0.7; // 70% 宽度

        // 保持16:9比例
        int adjustedWidth = mainHeight * 16 / 9;
        if (adjustedWidth > mainWidth) {
            adjustedWidth = mainWidth;
            mainHeight = adjustedWidth * 9 / 16;
        }

        mainCameraWidget->setFixedHeight(mainHeight);
        mainCameraWidget->setFixedWidth(adjustedWidth);
    }

    // 更新topSplitter的尺寸
    QTimer::singleShot(0, this, [this]() {
        QList<int> sizes;
        sizes << width() * 0.7 << width() * 0.3; // 70% 主监控画面, 30% 控制面板
        topSplitter->setSizes(sizes);
    });
}


void MainWindow::onSwitchToMain(CameraWidget* widget) {
    // 停止主监控画面和缩略图画面的预览
    mainCameraWidget->setVisible(false);
    widget->setVisible(false);

    // 保存原来的主监控画面
    CameraWidget* oldMainCameraWidget = mainCameraWidget;

    // 将右键点击的缩略图设置为新的主监控画面
    mainCameraWidget = widget;
    topSplitter->replaceWidget(0, mainCameraWidget);

    // 将新的主监控画面大小策略设置为Expanding，并移除固定大小设置
    mainCameraWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    mainCameraWidget->setFixedSize(topSplitter->height() * 16 / 9, topSplitter->height()); // 设置固定大小以保持16:9比例
    int mainHeight = height() * 0.7; // 70% 高度
    int mainWidth = width() * 0.7; // 70% 宽度

    // 保持16:9比例
    int adjustedWidth = mainHeight * 16 / 9;
    if (adjustedWidth > mainWidth) {
        adjustedWidth = mainWidth;
        mainHeight = adjustedWidth * 9 / 16;
    }

    mainCameraWidget->setFixedHeight(mainHeight);
    mainCameraWidget->setFixedWidth(adjustedWidth);

    // 将原来的主监控画面设置为缩略图，并添加到缩略图布局中
    oldMainCameraWidget->setParent(thumbnailContainer);
    oldMainCameraWidget->setSizePolicy(QSizePolicy::Preferred , QSizePolicy::Preferred);
    oldMainCameraWidget->setFixedSize(thumbnailArea->height() * 16 / 9, thumbnailArea->height()); // 设置固定大小以保持16:9比例
    QHBoxLayout *thumbnailLayout = qobject_cast<QHBoxLayout*>(thumbnailContainer->layout());
    if (thumbnailLayout) {
        thumbnailLayout->addWidget(oldMainCameraWidget);
    }

    // 显示新的主监控画面和缩略图
    mainCameraWidget->setVisible(true);
    oldMainCameraWidget->setVisible(true);
}


