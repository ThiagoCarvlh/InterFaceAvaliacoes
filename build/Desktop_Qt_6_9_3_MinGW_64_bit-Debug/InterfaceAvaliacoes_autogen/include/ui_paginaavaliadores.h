/********************************************************************************
** Form generated from reading UI file 'paginaavaliadores.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGINAAVALIADORES_H
#define UI_PAGINAAVALIADORES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaginaAvaliadores
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *paginaavaliadores)
    {
        if (paginaavaliadores->objectName().isEmpty())
            paginaavaliadores->setObjectName("paginaavaliadores");
        verticalLayout = new QVBoxLayout(paginaavaliadores);
        verticalLayout->setObjectName("verticalLayout");

        retranslateUi(paginaavaliadores);

        QMetaObject::connectSlotsByName(paginaavaliadores);
    } // setupUi

    void retranslateUi(QWidget *paginaavaliadores)
    {
        (void)paginaavaliadores;
    } // retranslateUi

};

namespace Ui {
    class PaginaAvaliadores: public Ui_PaginaAvaliadores {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGINAAVALIADORES_H
