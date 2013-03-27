/********************************************************************************
** Form generated from reading ui file 'FirmwareUpdateTab.ui'
**
** Created: Sat 17. Sep 14:50:28 2011
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_FIRMWAREUPDATETAB_H
#define UI_FIRMWAREUPDATETAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FirmwareUpdateForm
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_CheckConnection;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_FirmwareVersion;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_Ping;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_Ping;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_Reboot;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label;
    QLineEdit *lineEdit_Password;
    QLabel *label_Reboot;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QComboBox *comboBox;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_Upload;
    QProgressBar *progressBar_Update;

    void setupUi(QWidget *FirmwareUpdateForm)
    {
    if (FirmwareUpdateForm->objectName().isEmpty())
        FirmwareUpdateForm->setObjectName(QString::fromUtf8("FirmwareUpdateForm"));
    FirmwareUpdateForm->resize(400, 300);
    verticalLayout_2 = new QVBoxLayout(FirmwareUpdateForm);
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    pushButton_CheckConnection = new QPushButton(FirmwareUpdateForm);
    pushButton_CheckConnection->setObjectName(QString::fromUtf8("pushButton_CheckConnection"));

    horizontalLayout->addWidget(pushButton_CheckConnection);

    horizontalSpacer_4 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer_4);

    label_FirmwareVersion = new QLabel(FirmwareUpdateForm);
    label_FirmwareVersion->setObjectName(QString::fromUtf8("label_FirmwareVersion"));

    horizontalLayout->addWidget(label_FirmwareVersion);


    verticalLayout_2->addLayout(horizontalLayout);

    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    pushButton_Ping = new QPushButton(FirmwareUpdateForm);
    pushButton_Ping->setObjectName(QString::fromUtf8("pushButton_Ping"));
    pushButton_Ping->setEnabled(false);

    horizontalLayout_2->addWidget(pushButton_Ping);

    horizontalSpacer_2 = new QSpacerItem(10, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_2->addItem(horizontalSpacer_2);

    label_Ping = new QLabel(FirmwareUpdateForm);
    label_Ping->setObjectName(QString::fromUtf8("label_Ping"));

    horizontalLayout_2->addWidget(label_Ping);


    verticalLayout_2->addLayout(horizontalLayout_2);

    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    pushButton_Reboot = new QPushButton(FirmwareUpdateForm);
    pushButton_Reboot->setObjectName(QString::fromUtf8("pushButton_Reboot"));
    pushButton_Reboot->setEnabled(false);

    horizontalLayout_3->addWidget(pushButton_Reboot);

    horizontalSpacer_3 = new QSpacerItem(58, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_3->addItem(horizontalSpacer_3);

    label = new QLabel(FirmwareUpdateForm);
    label->setObjectName(QString::fromUtf8("label"));

    horizontalLayout_3->addWidget(label);

    lineEdit_Password = new QLineEdit(FirmwareUpdateForm);
    lineEdit_Password->setObjectName(QString::fromUtf8("lineEdit_Password"));
    lineEdit_Password->setEnabled(false);
    lineEdit_Password->setEchoMode(QLineEdit::Password);

    horizontalLayout_3->addWidget(lineEdit_Password);

    label_Reboot = new QLabel(FirmwareUpdateForm);
    label_Reboot->setObjectName(QString::fromUtf8("label_Reboot"));

    horizontalLayout_3->addWidget(label_Reboot);


    verticalLayout_2->addLayout(horizontalLayout_3);

    verticalSpacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_2->addItem(verticalSpacer);

    groupBox = new QGroupBox(FirmwareUpdateForm);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    verticalLayout = new QVBoxLayout(groupBox);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    horizontalLayout_4 = new QHBoxLayout();
    horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
    comboBox = new QComboBox(groupBox);
    comboBox->setObjectName(QString::fromUtf8("comboBox"));
    comboBox->setEnabled(false);

    horizontalLayout_4->addWidget(comboBox);

    horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_4->addItem(horizontalSpacer);

    pushButton_Upload = new QPushButton(groupBox);
    pushButton_Upload->setObjectName(QString::fromUtf8("pushButton_Upload"));
    pushButton_Upload->setEnabled(false);

    horizontalLayout_4->addWidget(pushButton_Upload);


    verticalLayout->addLayout(horizontalLayout_4);

    progressBar_Update = new QProgressBar(groupBox);
    progressBar_Update->setObjectName(QString::fromUtf8("progressBar_Update"));
    progressBar_Update->setValue(0);

    verticalLayout->addWidget(progressBar_Update);


    verticalLayout_2->addWidget(groupBox);


    retranslateUi(FirmwareUpdateForm);

    QMetaObject::connectSlotsByName(FirmwareUpdateForm);
    } // setupUi

    void retranslateUi(QWidget *FirmwareUpdateForm)
    {
    FirmwareUpdateForm->setWindowTitle(QApplication::translate("FirmwareUpdateForm", "Form", 0, QApplication::UnicodeUTF8));
    pushButton_CheckConnection->setText(QApplication::translate("FirmwareUpdateForm", "Check Connection", 0, QApplication::UnicodeUTF8));
    label_FirmwareVersion->setText(QApplication::translate("FirmwareUpdateForm", "None", 0, QApplication::UnicodeUTF8));
    pushButton_Ping->setText(QApplication::translate("FirmwareUpdateForm", "Ping", 0, QApplication::UnicodeUTF8));
    label_Ping->setText(QApplication::translate("FirmwareUpdateForm", "Nop", 0, QApplication::UnicodeUTF8));
    pushButton_Reboot->setText(QApplication::translate("FirmwareUpdateForm", "Reboot", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("FirmwareUpdateForm", "Password", 0, QApplication::UnicodeUTF8));
    label_Reboot->setText(QApplication::translate("FirmwareUpdateForm", "Nop", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("FirmwareUpdateForm", "Microcontroller Update", 0, QApplication::UnicodeUTF8));
    pushButton_Upload->setText(QApplication::translate("FirmwareUpdateForm", "Upload", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(FirmwareUpdateForm);
    } // retranslateUi

};

namespace Ui {
    class FirmwareUpdateForm: public Ui_FirmwareUpdateForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FIRMWAREUPDATETAB_H
