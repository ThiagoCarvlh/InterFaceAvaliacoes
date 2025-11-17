#pragma once

#include <QMainWindow>

class QStackedWidget;
class QToolBar;
class QAction;

class PaginaProjetos;
class PaginaAvaliadores;
class PaginaFichas;
class PaginaNotas;

namespace Ui {
class JanelaPrincipal;
}

class JanelaPrincipal : public QMainWindow
{
    Q_OBJECT

public:
    explicit JanelaPrincipal(QWidget *parent = nullptr);
    ~JanelaPrincipal();

    // true = admin (vê tudo), false = avaliador (vê só Fichas/Notas)
    void configurarPorLogin(bool admin);

private slots:
    void irProjetos();
    void irAvaliadores();
    void irFichas();
    void irNotas();

private:
    Ui::JanelaPrincipal *ui{};

    QStackedWidget* m_stack{};
    QToolBar*       m_toolbar{};
    QAction *m_actProjetos{};
    QAction *m_actAvaliadores{};
    QAction *m_actFichas{};
    QAction *m_actNotas{};

    PaginaProjetos*    m_pagProjetos{};
    PaginaAvaliadores* m_pagAvaliadores{};
    PaginaFichas*      m_pagFichas{};
    PaginaNotas*       m_pagNotas{};

    bool m_admin{false};

    void criarToolbar();
    void trocarPagina(QWidget* pagina);
};
