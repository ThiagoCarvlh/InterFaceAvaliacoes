#pragma once

#include <QDialog>
#include <QString>

class QTableView;
class QStandardItemModel;
class QPushButton;

class DialogoSelecionarFicha : public QDialog
{
    Q_OBJECT
public:
    explicit DialogoSelecionarFicha(const QString& cursoProjeto,
                                    QWidget* parent = nullptr);

    int fichaIdSelecionada() const { return m_fichaIdSelecionada; }
    QString fichaLabelSelecionada() const { return m_fichaLabelSelecionada; }

private slots:
    void onConfirmar();
    void onDuploClique(const QModelIndex& idx);

private:
    void carregarFichas();
    void configurarTabela();

    QString m_cursoProjeto;
    QTableView*         m_table{};
    QStandardItemModel* m_model{};
    QPushButton*        m_btnOk{};
    QPushButton*        m_btnCancel{};

    int     m_fichaIdSelecionada{-1};
    QString m_fichaLabelSelecionada;

    const QString m_arquivoFichas{"fichas.txt"};
};
