#include "janelaprincipal.h"
#include "ui_janelaprincipal.h"

#include <QStackedWidget>
#include <QToolBar>
#include <QAction>

#include "paginaprojetos.h"
#include "paginaavaliadores.h"
#include "paginafichas.h"
#include "paginanotas.h"

JanelaPrincipal::JanelaPrincipal(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JanelaPrincipal)
{
    ui->setupUi(this);
    setWindowTitle("Sistema de Avaliações");

    // QStackedWidget veio do .ui com objectName "stack"
    m_stack = ui->stack;
    if (!m_stack) {
        m_stack = new QStackedWidget(this);
        setCentralWidget(m_stack);
    }

    // Páginas
    m_pagProjetos    = new PaginaProjetos(this);
    m_pagAvaliadores = new PaginaAvaliadores(this);
    m_pagFichas      = new PaginaFichas(this);
    m_pagNotas       = new PaginaNotas(this);

    m_stack->addWidget(m_pagProjetos);
    m_stack->addWidget(m_pagAvaliadores);
    m_stack->addWidget(m_pagFichas);
    m_stack->addWidget(m_pagNotas);

    criarToolbar();
    irProjetos(); // padrão para admin; depois o login ajusta
}

JanelaPrincipal::~JanelaPrincipal()
{
    delete ui;
}

void JanelaPrincipal::criarToolbar()
{
    m_toolbar = addToolBar("Navegação");
    m_toolbar->setMovable(false);

    // Se quiser, depois a gente coloca ícones aqui
    m_actProjetos    = m_toolbar->addAction("Projetos");
    m_actAvaliadores = m_toolbar->addAction("Avaliadores");
    m_actFichas      = m_toolbar->addAction("Fichas");
    m_actNotas       = m_toolbar->addAction("Notas");

    connect(m_actProjetos,    &QAction::triggered, this, &JanelaPrincipal::irProjetos);
    connect(m_actAvaliadores, &QAction::triggered, this, &JanelaPrincipal::irAvaliadores);
    connect(m_actFichas,      &QAction::triggered, this, &JanelaPrincipal::irFichas);
    connect(m_actNotas,       &QAction::triggered, this, &JanelaPrincipal::irNotas);
}

void JanelaPrincipal::trocarPagina(QWidget *pagina)
{
    if (!m_stack || !pagina) return;
    m_stack->setCurrentWidget(pagina);
}

void JanelaPrincipal::irProjetos()
{
    trocarPagina(m_pagProjetos);
}

void JanelaPrincipal::irAvaliadores()
{
    trocarPagina(m_pagAvaliadores);
}

void JanelaPrincipal::irFichas()
{
    trocarPagina(m_pagFichas);
}

void JanelaPrincipal::irNotas()
{
    trocarPagina(m_pagNotas);
}

void JanelaPrincipal::configurarPorLogin(bool admin,
                                         const QString& cpf,
                                         const QString& nome,
                                         const QString& curso)
{
    m_admin = admin;

    // Admin vê tudo; avaliador não vê Projetos/Avaliadores
    if (m_actProjetos)    m_actProjetos->setVisible(admin);
    if (m_actAvaliadores) m_actAvaliadores->setVisible(admin);
    if (m_actFichas)      m_actFichas->setVisible(true);
    if (m_actNotas)       m_actNotas->setVisible(true);

    if (!admin) {
        // MODO AVALIADOR: restringir visão e informar contexto à PaginaNotas
        if (m_pagNotas) {
            m_pagNotas->setAvaliador(cpf, nome, curso);
        }
        irNotas();
    } else {
        // MODO ADMIN: PaginaNotas em modo administrativo (sem avaliador)
        if (m_pagNotas) {
            m_pagNotas->setAvaliador(QString(), QString(), QString());
        }
        irProjetos();
    }
}
