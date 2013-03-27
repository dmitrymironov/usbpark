/********************************************************************************
** Form generated from reading ui file 'WebCamTab.ui'
**
** Created: Sat 17. Sep 14:50:28 2011
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_WEBCAMTAB_H
#define UI_WEBCAMTAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WebCamTabForm
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QComboBox *comboBox_cam;
    QCheckBox *checkBox_enableVideo;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *WebCamTabForm)
    {
    if (WebCamTabForm->objectName().isEmpty())
        WebCamTabForm->setObjectName(QString::fromUtf8("WebCamTabForm"));
    WebCamTabForm->resize(397, 300);
    verticalLayout = new QVBoxLayout(WebCamTabForm);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    groupBox = new QGroupBox(WebCamTabForm);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    gridLayout = new QGridLayout(groupBox);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    comboBox_cam = new QComboBox(groupBox);
    comboBox_cam->setObjectName(QString::fromUtf8("comboBox_cam"));
    comboBox_cam->setEditable(true);

    gridLayout->addWidget(comboBox_cam, 0, 0, 1, 1);


    verticalLayout->addWidget(groupBox);

    checkBox_enableVideo = new QCheckBox(WebCamTabForm);
    checkBox_enableVideo->setObjectName(QString::fromUtf8("checkBox_enableVideo"));
    checkBox_enableVideo->setChecked(true);

    verticalLayout->addWidget(checkBox_enableVideo);

    verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer);


    retranslateUi(WebCamTabForm);

    QMetaObject::connectSlotsByName(WebCamTabForm);
    } // setupUi

    void retranslateUi(QWidget *WebCamTabForm)
    {
    WebCamTabForm->setWindowTitle(QApplication::translate("WebCamTabForm", "Form", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("WebCamTabForm", "Select Video Cam", 0, QApplication::UnicodeUTF8));
    comboBox_cam->clear();
    comboBox_cam->insertItems(0, QStringList()
     << QApplication::translate("WebCamTabForm", "dshow://", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("WebCamTabForm", "v4l2://", 0, QApplication::UnicodeUTF8)
    );
    checkBox_enableVideo->setText(QApplication::translate("WebCamTabForm", "Enable video", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(WebCamTabForm);
    } // retranslateUi

};

namespace Ui {
    class WebCamTabForm: public Ui_WebCamTabForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WEBCAMTAB_H
