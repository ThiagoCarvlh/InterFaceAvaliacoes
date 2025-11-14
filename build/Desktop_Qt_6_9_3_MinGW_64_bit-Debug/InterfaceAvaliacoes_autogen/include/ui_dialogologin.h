/********************************************************************************
** Form generated from reading UI file 'dialogologin.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGOLOGIN_H
#define UI_DIALOGOLOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogoLogin
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QDialog *DialogoLogin)
    {
        if (DialogoLogin->objectName().isEmpty())
            DialogoLogin->setObjectName("DialogoLogin");
        verticalLayout = new QVBoxLayout(DialogoLogin);
        verticalLayout->setObjectName("verticalLayout");

        retranslateUi(DialogoLogin);

        QMetaObject::connectSlotsByName(DialogoLogin);
    } // setupUi

    void retranslateUi(QDialog *DialogoLogin)
    {
        (void)DialogoLogin;
    } // retranslateUi

};

namespace Ui {
    class DialogoLogin: public Ui_DialogoLogin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGOLOGIN_H
