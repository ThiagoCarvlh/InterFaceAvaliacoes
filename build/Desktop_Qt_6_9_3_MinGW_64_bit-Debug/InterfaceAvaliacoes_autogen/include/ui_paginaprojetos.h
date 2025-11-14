/********************************************************************************
** Form generated from reading UI file 'paginaprojetos.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGINAPROJETOS_H
#define UI_PAGINAPROJETOS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaginaProjetos
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *paginaprojetos)
    {
        if (paginaprojetos->objectName().isEmpty())
            paginaprojetos->setObjectName("paginaprojetos");
        verticalLayout = new QVBoxLayout(paginaprojetos);
        verticalLayout->setObjectName("verticalLayout");

        retranslateUi(paginaprojetos);

        QMetaObject::connectSlotsByName(paginaprojetos);
    } // setupUi

    void retranslateUi(QWidget *paginaprojetos)
    {
        (void)paginaprojetos;
    } // retranslateUi

};

namespace Ui {
    class PaginaProjetos: public Ui_PaginaProjetos {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGINAPROJETOS_H
