#pragma once
#include <QWidget>
#include <QString>

class QTableView;
class QStandardItemModel;
class QPushButton;

namespace Ui { class PaginaAvaliadores; }

class PaginaAvaliadores : public QWidget {
    Q_OBJECT
public:
    explicit PaginaAvaliadores(QWidget* parent = nullptr);
    ~PaginaAvaliadores();

private slots:
    void onNovo();
    void onEditar();
    void onRemover();
    void onRecarregar();

private:
    Ui::PaginaAvaliadores* ui;
    QTableView*         m_table{};
    QStandardItemModel* m_model{};
    QPushButton *m_btnNovo{}, *m_btnEditar{}, *m_btnRemover{}, *m_btnRecarregar{};
    int m_nextId{1};

    const QString m_arquivo = "avaliadores.txt";
    bool salvarNoArquivo() const;
    bool carregarDoArquivo();
    void recomputarNextId();

    int  selectedRow() const;
    void addAvaliador(const QString& nome, const QString& email,
                      const QString& cpf, const QString& categoria);
};
