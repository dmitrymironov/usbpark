/********************************************************************************
** Form generated from reading ui file 'GUITab.ui'
**
** Created: Sat 17. Sep 14:50:28 2011
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_GUITAB_H
#define UI_GUITAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GUITabForm
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QSpacerItem *horizontalSpacer;
    QSpinBox *spinBox_AutoHideDelay;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer_2;
    QSpinBox *spinBox_ShowDelay;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QSpacerItem *horizontalSpacer_3;
    QComboBox *comboBox_SelectLanguage;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_4;
    QLineEdit *lineEdit_backgroundImage;
    QPushButton *pushButton_imageFile;
    QPushButton *pushButton_clearSettings;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *GUITabForm)
    {
    if (GUITabForm->objectName().isEmpty())
        GUITabForm->setObjectName(QString::fromUtf8("GUITabForm"));
    GUITabForm->resize(400, 300);
    gridLayout = new QGridLayout(GUITabForm);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    label = new QLabel(GUITabForm);
    label->setObjectName(QString::fromUtf8("label"));

    horizontalLayout_3->addWidget(label);

    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_3->addItem(horizontalSpacer);

    spinBox_AutoHideDelay = new QSpinBox(GUITabForm);
    spinBox_AutoHideDelay->setObjectName(QString::fromUtf8("spinBox_AutoHideDelay"));
    spinBox_AutoHideDelay->setValue(5);

    horizontalLayout_3->addWidget(spinBox_AutoHideDelay);


    gridLayout->addLayout(horizontalLayout_3, 0, 0, 1, 1);

    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    label_2 = new QLabel(GUITabForm);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    horizontalLayout_2->addWidget(label_2);

    horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_2->addItem(horizontalSpacer_2);

    spinBox_ShowDelay = new QSpinBox(GUITabForm);
    spinBox_ShowDelay->setObjectName(QString::fromUtf8("spinBox_ShowDelay"));
    spinBox_ShowDelay->setValue(5);

    horizontalLayout_2->addWidget(spinBox_ShowDelay);


    gridLayout->addLayout(horizontalLayout_2, 1, 0, 1, 1);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    label_3 = new QLabel(GUITabForm);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    horizontalLayout->addWidget(label_3);

    horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer_3);

    comboBox_SelectLanguage = new QComboBox(GUITabForm);
    comboBox_SelectLanguage->setObjectName(QString::fromUtf8("comboBox_SelectLanguage"));

    horizontalLayout->addWidget(comboBox_SelectLanguage);


    gridLayout->addLayout(horizontalLayout, 2, 0, 1, 2);

    horizontalLayout_4 = new QHBoxLayout();
    horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
    label_4 = new QLabel(GUITabForm);
    label_4->setObjectName(QString::fromUtf8("label_4"));

    horizontalLayout_4->addWidget(label_4);

    lineEdit_backgroundImage = new QLineEdit(GUITabForm);
    lineEdit_backgroundImage->setObjectName(QString::fromUtf8("lineEdit_backgroundImage"));

    horizontalLayout_4->addWidget(lineEdit_backgroundImage);

    pushButton_imageFile = new QPushButton(GUITabForm);
    pushButton_imageFile->setObjectName(QString::fromUtf8("pushButton_imageFile"));
    pushButton_imageFile->setMaximumSize(QSize(20, 16777215));

    horizontalLayout_4->addWidget(pushButton_imageFile);


    gridLayout->addLayout(horizontalLayout_4, 3, 0, 1, 1);

    pushButton_clearSettings = new QPushButton(GUITabForm);
    pushButton_clearSettings->setObjectName(QString::fromUtf8("pushButton_clearSettings"));

    gridLayout->addWidget(pushButton_clearSettings, 4, 0, 1, 1);

    horizontalSpacer_4 = new QSpacerItem(133, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_4, 4, 1, 1, 1);

    verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(verticalSpacer, 5, 0, 1, 1);


    retranslateUi(GUITabForm);

    QMetaObject::connectSlotsByName(GUITabForm);
    } // setupUi

    void retranslateUi(QWidget *GUITabForm)
    {
    GUITabForm->setWindowTitle(QApplication::translate("GUITabForm", "Form", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("GUITabForm", "Auto hide delay", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("GUITabForm", "Show delay", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("GUITabForm", "Select language", 0, QApplication::UnicodeUTF8));
    comboBox_SelectLanguage->clear();
    comboBox_SelectLanguage->insertItems(0, QStringList()
     << QApplication::translate("GUITabForm", "Russian", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("GUITabForm", "English", 0, QApplication::UnicodeUTF8)
    );
    label_4->setText(QApplication::translate("GUITabForm", "Image", 0, QApplication::UnicodeUTF8));
    pushButton_imageFile->setText(QApplication::translate("GUITabForm", "...", 0, QApplication::UnicodeUTF8));
    pushButton_clearSettings->setText(QApplication::translate("GUITabForm", "Clear all settings and restart", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(GUITabForm);
    } // retranslateUi

};

namespace Ui {
    class GUITabForm: public Ui_GUITabForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GUITAB_H
