#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class QTableView;
class QStandardItemModel;
class QPushButton;
class QLabel;

#include "vinculos.h"

class DialogoVincularAvaliadores : public QDialog
{
    Q_OBJECT
public:
    DialogoVincularAvaliadores(int idProjeto,
                               const QString& nomeProjeto,
                               const QString& categoriaProjeto,
                               QWidget* parent = nullptr);

    // usado pela tela de Projetos para saber quantos ficaram vinculados
    int totalSelecionados() const;

private slots:
    void onAdicionar();
    void onRemover();
    void onSalvar();

private:
    // dados do projeto
    int     m_idProjeto{0};
    QString m_nomeProjeto;
    QString m_categoriaProjeto;

    // UI
    QTableView*         m_tabDisponiveis{};
    QTableView*         m_tabSelecionados{};
    QStandardItemModel* m_modelDisponiveis{};
    QStandardItemModel* m_modelSelecionados{};
    QPushButton*        m_btnAdd{};
    QPushButton*        m_btnRemove{};
    QPushButton*        m_btnSalvar{};
    QPushButton*        m_btnCancelar{};
    QLabel*             m_lblResumo{};

    // arquivos
    const QString m_arquivoAvaliadores{"avaliadores.csv"};
    const QString m_arquivoVinculos{"vinculos_projetos.csv"};

    // métodos internos
    void configurarTabela(QTableView* table, QStandardItemModel* model);
    void carregarDados();
    void atualizarResumo();
};
