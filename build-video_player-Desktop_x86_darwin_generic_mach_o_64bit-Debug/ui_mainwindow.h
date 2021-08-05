/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.0.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "videoslider.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *openFileBtn;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *playBtn;
    QPushButton *stopBtn;
    QHBoxLayout *horizontalLayout;
    VideoSlider *timeSlider;
    QLabel *timeLabel;
    QLabel *label_3;
    QLabel *durationLabel;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *muteBtn;
    VideoSlider *volumnSlider;
    QLabel *volumnLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        openFileBtn = new QPushButton(centralwidget);
        openFileBtn->setObjectName(QString::fromUtf8("openFileBtn"));
        openFileBtn->setGeometry(QRect(310, 180, 112, 32));
        horizontalLayoutWidget_4 = new QWidget(centralwidget);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(70, 470, 651, 38));
        horizontalLayout_4 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        playBtn = new QPushButton(horizontalLayoutWidget_4);
        playBtn->setObjectName(QString::fromUtf8("playBtn"));

        horizontalLayout_2->addWidget(playBtn);

        stopBtn = new QPushButton(horizontalLayoutWidget_4);
        stopBtn->setObjectName(QString::fromUtf8("stopBtn"));

        horizontalLayout_2->addWidget(stopBtn);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        timeSlider = new VideoSlider(horizontalLayoutWidget_4);
        timeSlider->setObjectName(QString::fromUtf8("timeSlider"));
        timeSlider->setEnabled(false);
        timeSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(timeSlider);

        timeLabel = new QLabel(horizontalLayoutWidget_4);
        timeLabel->setObjectName(QString::fromUtf8("timeLabel"));

        horizontalLayout->addWidget(timeLabel);

        label_3 = new QLabel(horizontalLayoutWidget_4);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout->addWidget(label_3);

        durationLabel = new QLabel(horizontalLayoutWidget_4);
        durationLabel->setObjectName(QString::fromUtf8("durationLabel"));

        horizontalLayout->addWidget(durationLabel);


        horizontalLayout_2->addLayout(horizontalLayout);


        horizontalLayout_4->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        muteBtn = new QPushButton(horizontalLayoutWidget_4);
        muteBtn->setObjectName(QString::fromUtf8("muteBtn"));

        horizontalLayout_3->addWidget(muteBtn);

        volumnSlider = new VideoSlider(horizontalLayoutWidget_4);
        volumnSlider->setObjectName(QString::fromUtf8("volumnSlider"));
        volumnSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_3->addWidget(volumnSlider);

        volumnLabel = new QLabel(horizontalLayoutWidget_4);
        volumnLabel->setObjectName(QString::fromUtf8("volumnLabel"));

        horizontalLayout_3->addWidget(volumnLabel);


        horizontalLayout_4->addLayout(horizontalLayout_3);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        openFileBtn->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\346\226\207\344\273\266", nullptr));
        playBtn->setText(QCoreApplication::translate("MainWindow", "\346\222\255\346\224\276", nullptr));
        stopBtn->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        timeLabel->setText(QCoreApplication::translate("MainWindow", "00:00:00", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "/", nullptr));
        durationLabel->setText(QCoreApplication::translate("MainWindow", "00:00:00", nullptr));
        muteBtn->setText(QCoreApplication::translate("MainWindow", "\351\235\231\351\237\263", nullptr));
        volumnLabel->setText(QCoreApplication::translate("MainWindow", "100", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
