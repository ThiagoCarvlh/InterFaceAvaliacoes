/********************************************************************************
** Form generated from reading UI file 'paginanotas.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGINANOTAS_H
#define UI_PAGINANOTAS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaginaNotas
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *paginanotas)
    {
        if (paginanotas->objectName().isEmpty())
            paginanotas->setObjectName("paginanotas");
        verticalLayout = new QVBoxLayout(paginanotas);
        verticalLayout->setObjectName("verticalLayout");

        retranslateUi(paginanotas);

        QMetaObject::connectSlotsByName(paginanotas);
    } // setupUi

    void retranslateUi(QWidget *paginanotas)
    {
        (void)paginanotas;
    } // retranslateUi

};

namespace Ui {
    class PaginaNotas: public Ui_PaginaNotas {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGINANOTAS_H
