// ui/telas/janelaprincipal.cpp
#include "janelaprincipal.h"
#include "ui_janelaprincipal.h"

#include "paginaprojetos.h"
#include "paginaavaliadores.h"
#include "paginanotas.h"

#include <QToolBar>
#include <QAction>
#include <QStackedWidget>
#include <QMessageBox>

JanelaPrincipal::JanelaPrincipal(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::JanelaPrincipal)
{
    ui->setupUi(this);
    setWindowTitle("Interface de Avaliacoes");

    // Localiza o QStackedWidget por nome (aceita 'stack' ou 'stackedWidget')
    QStackedWidget* stack = findChild<QStackedWidget*>("stack");
    if (!stack) stack = findChild<QStackedWidget*>("stackedWidget");
    if (!stack) {
        QMessageBox::critical(this, "Erro de UI",
                              "Nao encontrei um QStackedWidget chamado 'stack' ou 'stackedWidget' "
                              "em janelaprincipal.ui.");
        return;
    }

    // Cria as paginas
    m_pagProjetos    = new PaginaProjetos(this);
    m_pagAvaliadores = new PaginaAvaliadores(this);
    m_pagNotas       = new PaginaNotas(this);

    // Adiciona ao stacked
    stack->addWidget(m_pagProjetos);
    stack->addWidget(m_pagAvaliadores);
    stack->addWidget(m_pagNotas);
    stack->setCurrentWidget(m_pagProjetos);

    // Toolbar de navegacao
    auto tb     = addToolBar("Navegar");
    auto acProj = tb->addAction("Projetos");
    auto acAval = tb->addAction("Avaliadores");
    auto acNotas= tb->addAction("Notas");

    connect(acProj,  &QAction::triggered, this, [stack, this]{ stack->setCurrentWidget(m_pagProjetos); });
    connect(acAval,  &QAction::triggered, this, [stack, this]{ stack->setCurrentWidget(m_pagAvaliadores); });
    connect(acNotas, &QAction::triggered, this, [stack, this]{ stack->setCurrentWidget(m_pagNotas); });
}

JanelaPrincipal::~JanelaPrincipal() { delete ui; }
