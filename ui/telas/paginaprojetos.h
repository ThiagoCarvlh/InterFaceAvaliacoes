#pragma once
#include <QWidget>
#include <QString>

class QTableView;
class QStandardItemModel;
class QPushButton;

namespace Ui { class PaginaProjetos; }

class PaginaProjetos : public QWidget {
    Q_OBJECT
public:
    explicit PaginaProjetos(QWidget* parent = nullptr);
    ~PaginaProjetos();

private slots:
    void onNovo();
    void onEditar();
    void onRemover();
    void onRecarregar();

private:
    Ui::PaginaProjetos* ui;
    QTableView*         m_table{};
    QStandardItemModel* m_model{};
    QPushButton *m_btnNovo{}, *m_btnEditar{}, *m_btnRemover{},*m_btnRecarregar{};;
    int m_nextId{1};
    // ===== persistência simples =====
    const QString m_arquivo = "projetos.txt";   // ao lado do executável (pasta de build)
    bool salvarNoArquivo() const;
    bool carregarDoArquivo();
    void recomputarNextId();

    void addProjeto(const QString& nome, const QString& desc, const QString& resp);
    int  selectedRow() const;
};
