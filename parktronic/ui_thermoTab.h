/********************************************************************************
** Form generated from reading ui file 'thermoTab.ui'
**
** Created: Mon 19. Sep 18:05:39 2011
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_THERMOTAB_H
#define UI_THERMOTAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_thermoForm
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_2;
    QTableWidget *thermoWidget;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_thermo;

    void setupUi(QWidget *thermoForm)
    {
    if (thermoForm->objectName().isEmpty())
        thermoForm->setObjectName(QString::fromUtf8("thermoForm"));
    thermoForm->resize(590, 287);
    verticalLayout = new QVBoxLayout(thermoForm);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    groupBox = new QGroupBox(thermoForm);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    horizontalLayout_2 = new QHBoxLayout(groupBox);
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    thermoWidget = new QTableWidget(groupBox);
    if (thermoWidget->columnCount() < 2)
        thermoWidget->setColumnCount(2);
    QTableWidgetItem *__colItem = new QTableWidgetItem();
    thermoWidget->setHorizontalHeaderItem(0, __colItem);
    QTableWidgetItem *__colItem1 = new QTableWidgetItem();
    thermoWidget->setHorizontalHeaderItem(1, __colItem1);
    if (thermoWidget->rowCount() < 5)
        thermoWidget->setRowCount(5);
    QTableWidgetItem *__rowItem = new QTableWidgetItem();
    thermoWidget->setVerticalHeaderItem(0, __rowItem);
    QTableWidgetItem *__rowItem1 = new QTableWidgetItem();
    thermoWidget->setVerticalHeaderItem(1, __rowItem1);
    QTableWidgetItem *__rowItem2 = new QTableWidgetItem();
    thermoWidget->setVerticalHeaderItem(2, __rowItem2);
    QTableWidgetItem *__rowItem3 = new QTableWidgetItem();
    thermoWidget->setVerticalHeaderItem(3, __rowItem3);
    QTableWidgetItem *__rowItem4 = new QTableWidgetItem();
    thermoWidget->setVerticalHeaderItem(4, __rowItem4);
    QTableWidgetItem *__tableItem = new QTableWidgetItem();
    thermoWidget->setItem(0, 0, __tableItem);
    QTableWidgetItem *__tableItem1 = new QTableWidgetItem();
    thermoWidget->setItem(0, 1, __tableItem1);
    QTableWidgetItem *__tableItem2 = new QTableWidgetItem();
    thermoWidget->setItem(1, 0, __tableItem2);
    QTableWidgetItem *__tableItem3 = new QTableWidgetItem();
    thermoWidget->setItem(1, 1, __tableItem3);
    QTableWidgetItem *__tableItem4 = new QTableWidgetItem();
    thermoWidget->setItem(2, 0, __tableItem4);
    QTableWidgetItem *__tableItem5 = new QTableWidgetItem();
    thermoWidget->setItem(2, 1, __tableItem5);
    QTableWidgetItem *__tableItem6 = new QTableWidgetItem();
    thermoWidget->setItem(3, 0, __tableItem6);
    QTableWidgetItem *__tableItem7 = new QTableWidgetItem();
    thermoWidget->setItem(3, 1, __tableItem7);
    QTableWidgetItem *__tableItem8 = new QTableWidgetItem();
    thermoWidget->setItem(4, 0, __tableItem8);
    QTableWidgetItem *__tableItem9 = new QTableWidgetItem();
    thermoWidget->setItem(4, 1, __tableItem9);
    thermoWidget->setObjectName(QString::fromUtf8("thermoWidget"));

    horizontalLayout_2->addWidget(thermoWidget);


    verticalLayout->addWidget(groupBox);

    verticalSpacer = new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);

    pushButton_thermo = new QPushButton(thermoForm);
    pushButton_thermo->setObjectName(QString::fromUtf8("pushButton_thermo"));

    horizontalLayout->addWidget(pushButton_thermo);


    verticalLayout->addLayout(horizontalLayout);


    retranslateUi(thermoForm);

    QMetaObject::connectSlotsByName(thermoForm);
    } // setupUi

    void retranslateUi(QWidget *thermoForm)
    {
    thermoForm->setWindowTitle(QApplication::translate("thermoForm", "Thermal Readings", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("thermoForm", "Sensors", 0, QApplication::UnicodeUTF8));
    thermoWidget->horizontalHeaderItem(0)->setText(QApplication::translate("thermoForm", "Zone", 0, QApplication::UnicodeUTF8));
    thermoWidget->horizontalHeaderItem(1)->setText(QApplication::translate("thermoForm", "Temperature,C", 0, QApplication::UnicodeUTF8));
    thermoWidget->verticalHeaderItem(0)->setText(QApplication::translate("thermoForm", "401740071", 0, QApplication::UnicodeUTF8));
    thermoWidget->verticalHeaderItem(1)->setText(QApplication::translate("thermoForm", "401740072", 0, QApplication::UnicodeUTF8));
    thermoWidget->verticalHeaderItem(2)->setText(QApplication::translate("thermoForm", "401740073", 0, QApplication::UnicodeUTF8));
    thermoWidget->verticalHeaderItem(3)->setText(QApplication::translate("thermoForm", "401740074", 0, QApplication::UnicodeUTF8));
    thermoWidget->verticalHeaderItem(4)->setText(QApplication::translate("thermoForm", "401740075", 0, QApplication::UnicodeUTF8));

    const bool __sortingEnabled = thermoWidget->isSortingEnabled();
    thermoWidget->setSortingEnabled(false);
    thermoWidget->item(0, 0)->setText(QApplication::translate("thermoForm", "Interior", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(0, 1)->setText(QApplication::translate("thermoForm", "21.2", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(1, 0)->setText(QApplication::translate("thermoForm", "Trunk", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(1, 1)->setText(QApplication::translate("thermoForm", "18.1", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(2, 0)->setText(QApplication::translate("thermoForm", "Cooling liquid", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(2, 1)->setText(QApplication::translate("thermoForm", "27.0", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(3, 0)->setText(QApplication::translate("thermoForm", "Oil", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(3, 1)->setText(QApplication::translate("thermoForm", "86.3", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(4, 0)->setText(QApplication::translate("thermoForm", "Outdoors", 0, QApplication::UnicodeUTF8));
    thermoWidget->item(4, 1)->setText(QApplication::translate("thermoForm", "-40.5", 0, QApplication::UnicodeUTF8));

    thermoWidget->setSortingEnabled(__sortingEnabled);
    pushButton_thermo->setText(QApplication::translate("thermoForm", "Update", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(thermoForm);
    } // retranslateUi

};

namespace Ui {
    class thermoForm: public Ui_thermoForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_THERMOTAB_H
