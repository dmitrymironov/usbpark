/********************************************************************************
** Form generated from reading ui file 'DiagnosticsTab.ui'
**
** Created: Sat 17. Sep 14:50:28 2011
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_DIAGNOSTICSTAB_H
#define UI_DIAGNOSTICSTAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DiagnosticsTabForm
{
public:
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox_oneClick;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_driveButtons;
    QPushButton *pushButton_diskC;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_sendToNovorado;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_clearLog;
    QLabel *label_logFileSize;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *checkBox_detailedUSBLog;
    QCheckBox *checkBox_applicationLog;
    QCheckBox *checkBox_turnOff;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBox_logTo;
    QLabel *label_logFileName;
    QPushButton *pushButton_selectLogFile;

    void setupUi(QWidget *DiagnosticsTabForm)
    {
    if (DiagnosticsTabForm->objectName().isEmpty())
        DiagnosticsTabForm->setObjectName(QString::fromUtf8("DiagnosticsTabForm"));
    DiagnosticsTabForm->resize(400, 300);
    gridLayout_2 = new QGridLayout(DiagnosticsTabForm);
    gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
    groupBox_oneClick = new QGroupBox(DiagnosticsTabForm);
    groupBox_oneClick->setObjectName(QString::fromUtf8("groupBox_oneClick"));
    verticalLayout_3 = new QVBoxLayout(groupBox_oneClick);
    verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
    horizontalLayout_driveButtons = new QHBoxLayout();
    horizontalLayout_driveButtons->setObjectName(QString::fromUtf8("horizontalLayout_driveButtons"));
    pushButton_diskC = new QPushButton(groupBox_oneClick);
    pushButton_diskC->setObjectName(QString::fromUtf8("pushButton_diskC"));

    horizontalLayout_driveButtons->addWidget(pushButton_diskC);

    horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_driveButtons->addItem(horizontalSpacer);

    pushButton_sendToNovorado = new QPushButton(groupBox_oneClick);
    pushButton_sendToNovorado->setObjectName(QString::fromUtf8("pushButton_sendToNovorado"));

    horizontalLayout_driveButtons->addWidget(pushButton_sendToNovorado);


    verticalLayout_3->addLayout(horizontalLayout_driveButtons);


    gridLayout_2->addWidget(groupBox_oneClick, 0, 0, 1, 1);

    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    pushButton_clearLog = new QPushButton(DiagnosticsTabForm);
    pushButton_clearLog->setObjectName(QString::fromUtf8("pushButton_clearLog"));

    horizontalLayout_3->addWidget(pushButton_clearLog);

    label_logFileSize = new QLabel(DiagnosticsTabForm);
    label_logFileSize->setObjectName(QString::fromUtf8("label_logFileSize"));

    horizontalLayout_3->addWidget(label_logFileSize);


    gridLayout_2->addLayout(horizontalLayout_3, 1, 0, 1, 1);

    verticalSpacer = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout_2->addItem(verticalSpacer, 2, 0, 1, 1);

    groupBox_2 = new QGroupBox(DiagnosticsTabForm);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    gridLayout = new QGridLayout(groupBox_2);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    checkBox_detailedUSBLog = new QCheckBox(groupBox_2);
    checkBox_detailedUSBLog->setObjectName(QString::fromUtf8("checkBox_detailedUSBLog"));

    horizontalLayout_2->addWidget(checkBox_detailedUSBLog);

    checkBox_applicationLog = new QCheckBox(groupBox_2);
    checkBox_applicationLog->setObjectName(QString::fromUtf8("checkBox_applicationLog"));

    horizontalLayout_2->addWidget(checkBox_applicationLog);

    checkBox_turnOff = new QCheckBox(groupBox_2);
    checkBox_turnOff->setObjectName(QString::fromUtf8("checkBox_turnOff"));

    horizontalLayout_2->addWidget(checkBox_turnOff);


    gridLayout->addLayout(horizontalLayout_2, 0, 0, 1, 1);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    checkBox_logTo = new QCheckBox(groupBox_2);
    checkBox_logTo->setObjectName(QString::fromUtf8("checkBox_logTo"));

    horizontalLayout->addWidget(checkBox_logTo);

    label_logFileName = new QLabel(groupBox_2);
    label_logFileName->setObjectName(QString::fromUtf8("label_logFileName"));

    horizontalLayout->addWidget(label_logFileName);

    pushButton_selectLogFile = new QPushButton(groupBox_2);
    pushButton_selectLogFile->setObjectName(QString::fromUtf8("pushButton_selectLogFile"));
    pushButton_selectLogFile->setMaximumSize(QSize(30, 16777215));

    horizontalLayout->addWidget(pushButton_selectLogFile);


    gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);


    gridLayout_2->addWidget(groupBox_2, 3, 0, 1, 1);


    retranslateUi(DiagnosticsTabForm);

    QMetaObject::connectSlotsByName(DiagnosticsTabForm);
    } // setupUi

    void retranslateUi(QWidget *DiagnosticsTabForm)
    {
    DiagnosticsTabForm->setWindowTitle(QApplication::translate("DiagnosticsTabForm", "Diagnostics", 0, QApplication::UnicodeUTF8));
    groupBox_oneClick->setTitle(QApplication::translate("DiagnosticsTabForm", "One-click log save to device", 0, QApplication::UnicodeUTF8));
    pushButton_diskC->setText(QApplication::translate("DiagnosticsTabForm", "C:", 0, QApplication::UnicodeUTF8));
    pushButton_sendToNovorado->setText(QApplication::translate("DiagnosticsTabForm", "Send to Novorado", 0, QApplication::UnicodeUTF8));
    pushButton_clearLog->setText(QApplication::translate("DiagnosticsTabForm", "Clear log", 0, QApplication::UnicodeUTF8));
    label_logFileSize->setText(QApplication::translate("DiagnosticsTabForm", "Log file size: %kb", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("DiagnosticsTabForm", "Settings", 0, QApplication::UnicodeUTF8));
    checkBox_detailedUSBLog->setText(QApplication::translate("DiagnosticsTabForm", " USB log", 0, QApplication::UnicodeUTF8));
    checkBox_applicationLog->setText(QApplication::translate("DiagnosticsTabForm", "Application dbg", 0, QApplication::UnicodeUTF8));
    checkBox_turnOff->setText(QApplication::translate("DiagnosticsTabForm", "Console", 0, QApplication::UnicodeUTF8));
    checkBox_logTo->setText(QApplication::translate("DiagnosticsTabForm", "Log to ", 0, QApplication::UnicodeUTF8));
    label_logFileName->setText(QApplication::translate("DiagnosticsTabForm", "parker.log", 0, QApplication::UnicodeUTF8));
    pushButton_selectLogFile->setText(QApplication::translate("DiagnosticsTabForm", "...", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(DiagnosticsTabForm);
    } // retranslateUi

};

namespace Ui {
    class DiagnosticsTabForm: public Ui_DiagnosticsTabForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIAGNOSTICSTAB_H
