/********************************************************************************
** Form generated from reading ui file 'SoundsTab.ui'
**
** Created: Sat 17. Sep 14:50:28 2011
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SOUNDSTAB_H
#define UI_SOUNDSTAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SoundsTabForm
{
public:
    QHBoxLayout *horizontalLayout_8;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout_4;
    QRadioButton *radioButton_Voice;
    QRadioButton *radioButton_Beeps;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLabel *label_Frequency;
    QSlider *horizontalSlider_Frequency;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_2;
    QLabel *label_Loudness;
    QSlider *horizontalSlider_Loudness;

    void setupUi(QWidget *SoundsTabForm)
    {
    if (SoundsTabForm->objectName().isEmpty())
        SoundsTabForm->setObjectName(QString::fromUtf8("SoundsTabForm"));
    SoundsTabForm->resize(631, 373);
    horizontalLayout_8 = new QHBoxLayout(SoundsTabForm);
    horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
    verticalLayout_3 = new QVBoxLayout();
    verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
    groupBox = new QGroupBox(SoundsTabForm);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    groupBox->setMaximumSize(QSize(800, 120));
    verticalLayout_5 = new QVBoxLayout(groupBox);
    verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
    verticalLayout_4 = new QVBoxLayout();
    verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
    radioButton_Voice = new QRadioButton(groupBox);
    radioButton_Voice->setObjectName(QString::fromUtf8("radioButton_Voice"));
    radioButton_Voice->setChecked(true);

    verticalLayout_4->addWidget(radioButton_Voice);

    radioButton_Beeps = new QRadioButton(groupBox);
    radioButton_Beeps->setObjectName(QString::fromUtf8("radioButton_Beeps"));

    verticalLayout_4->addWidget(radioButton_Beeps);


    verticalLayout_5->addLayout(verticalLayout_4);


    verticalLayout_3->addWidget(groupBox);

    groupBox_3 = new QGroupBox(SoundsTabForm);
    groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
    verticalLayout = new QVBoxLayout(groupBox_3);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    label = new QLabel(groupBox_3);
    label->setObjectName(QString::fromUtf8("label"));

    horizontalLayout_2->addWidget(label);

    label_Frequency = new QLabel(groupBox_3);
    label_Frequency->setObjectName(QString::fromUtf8("label_Frequency"));

    horizontalLayout_2->addWidget(label_Frequency);


    verticalLayout->addLayout(horizontalLayout_2);

    horizontalSlider_Frequency = new QSlider(groupBox_3);
    horizontalSlider_Frequency->setObjectName(QString::fromUtf8("horizontalSlider_Frequency"));
    horizontalSlider_Frequency->setEnabled(false);
    horizontalSlider_Frequency->setOrientation(Qt::Horizontal);

    verticalLayout->addWidget(horizontalSlider_Frequency);

    horizontalLayout_7 = new QHBoxLayout();
    horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
    label_2 = new QLabel(groupBox_3);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    horizontalLayout_7->addWidget(label_2);

    label_Loudness = new QLabel(groupBox_3);
    label_Loudness->setObjectName(QString::fromUtf8("label_Loudness"));

    horizontalLayout_7->addWidget(label_Loudness);


    verticalLayout->addLayout(horizontalLayout_7);

    horizontalSlider_Loudness = new QSlider(groupBox_3);
    horizontalSlider_Loudness->setObjectName(QString::fromUtf8("horizontalSlider_Loudness"));
    horizontalSlider_Loudness->setEnabled(false);
    horizontalSlider_Loudness->setValue(99);
    horizontalSlider_Loudness->setOrientation(Qt::Horizontal);

    verticalLayout->addWidget(horizontalSlider_Loudness);


    verticalLayout_3->addWidget(groupBox_3);


    horizontalLayout_8->addLayout(verticalLayout_3);


    retranslateUi(SoundsTabForm);

    QMetaObject::connectSlotsByName(SoundsTabForm);
    } // setupUi

    void retranslateUi(QWidget *SoundsTabForm)
    {
    SoundsTabForm->setWindowTitle(QApplication::translate("SoundsTabForm", "Form", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("SoundsTabForm", "Notification", 0, QApplication::UnicodeUTF8));
    radioButton_Voice->setText(QApplication::translate("SoundsTabForm", "Voice", 0, QApplication::UnicodeUTF8));
    radioButton_Beeps->setText(QApplication::translate("SoundsTabForm", "Beeps", 0, QApplication::UnicodeUTF8));
    groupBox_3->setTitle(QApplication::translate("SoundsTabForm", "Buzzer", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("SoundsTabForm", "Frequency", 0, QApplication::UnicodeUTF8));
    label_Frequency->setText(QApplication::translate("SoundsTabForm", "N/A", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("SoundsTabForm", "Loudness", 0, QApplication::UnicodeUTF8));
    label_Loudness->setText(QApplication::translate("SoundsTabForm", "N/A", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(SoundsTabForm);
    } // retranslateUi

};

namespace Ui {
    class SoundsTabForm: public Ui_SoundsTabForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOUNDSTAB_H
