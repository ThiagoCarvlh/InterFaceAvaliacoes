#include "dialogoselecionarficha.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

DialogoSelecionarFicha::DialogoSelecionarFicha(const QString& cursoProjeto,
                                               QWidget* parent)
    : QDialog(parent)
    , m_cursoProjeto(cursoProjeto.trimmed())
    , m_table(new QTableView(this))
    , m_model(new QStandardItemModel(0, 5, this))
    , m_btnOk(new QPushButton(" Usar esta ficha", this))
    , m_btnCancel(new QPushButton("Cancelar", this))
{
    setModal(true);
    setMinimumSize(700, 450);
    if (parent)
        setStyleSheet(parent->styleSheet());

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    auto* titulo = new QLabel("Selecionar Ficha de Avaliação", this);
    QFont ft = titulo->font();
    ft.setPointSize(ft.pointSize() + 3);
    ft.setBold(true);
    titulo->setFont(ft);
    titulo->setStyleSheet("color: #00D4FF; padding-bottom: 6px;");
    mainLayout->addWidget(titulo);

    auto* info = new QLabel(this);
    info->setText(
        QString("Curso do projeto: <b>%1</b><br>"
                "Serão listadas fichas cadastradas em <b>Fichas</b>. "
                "Dê um duplo clique ou use o botão para selecionar.")
            .arg(m_cursoProjeto.isEmpty() ? "(não informado)" : m_cursoProjeto));
    mainLayout->addWidget(info);

    configurarTabela();
    mainLayout->addWidget(m_table, 1);

    auto* footer = new QHBoxLayout();
    footer->addStretch();
    footer->addWidget(m_btnCancel);
    footer->addWidget(m_btnOk);
    mainLayout->addLayout(footer);

    connect(m_btnOk,     &QPushButton::clicked,
            this,        &DialogoSelecionarFicha::onConfirmar);
    connect(m_btnCancel, &QPushButton::clicked,
            this,        &DialogoSelecionarFicha::reject);
    connect(m_table,     &QTableView::doubleClicked,
            this,        &DialogoSelecionarFicha::onDuploClique);

    carregarFichas();
}

void DialogoSelecionarFicha::configurarTabela()
{
    m_model->setHorizontalHeaderLabels({"ID", "Tipo", "Resolução", "Nº Quesitos", "Curso"});

    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(0, Qt::AscendingOrder);

    auto* header = m_table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
}
void DialogoSelecionarFicha::carregarFichas()
{
    QFile f(m_arquivoFichas);
    if (!f.exists())
        return;

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Fichas",
                             "Não foi possível abrir o arquivo de fichas.");
        return;
    }

    m_model->removeRows(0, m_model->rowCount());

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        const QStringList p = line.split(';');

        // precisamos de pelo menos os campos fixos até numSecoes
        if (p.size() < 14)
            continue;

        bool okId = false;
        const int id = p.at(0).toInt(&okId);
        if (!okId)
            continue;

        const QString tipo       = p.at(1);
        const QString resolucao  = QString("%1/%2").arg(p.at(2), p.at(3));
        const QString cursoFicha = p.at(4);   // <-- agora é o curso certo

        // calcula o total de quesitos percorrendo as seções serializadas
        int totalQuesitos = 0;
        bool okNumSecoes = false;
        const int numSecoes = p.at(13).toInt(&okNumSecoes);

        int idx = 14; // início das seções
        if (okNumSecoes) {
            for (int s = 0; s < numSecoes && idx < p.size(); ++s) {
                if (idx + 2 >= p.size())
                    break;

                // pula identificador e título
                idx += 2;

                // número de quesitos da seção
                if (idx >= p.size())
                    break;
                bool okQtd = false;
                const int qtd = p.at(idx).toInt(&okQtd);
                ++idx;
                if (!okQtd)
                    break;

                totalQuesitos += qtd;

                // pula dados dos quesitos: 4 campos por quesito
                idx += qtd * 4;
            }
        }

        // filtro por curso do projeto
        if (!m_cursoProjeto.isEmpty()) {
            if (cursoFicha.trimmed() != m_cursoProjeto)
                continue;
        }

        QList<QStandardItem*> row;
        auto* idItem = new QStandardItem(QString::number(id));
        idItem->setEditable(false);

        row << idItem
            << new QStandardItem(tipo)
            << new QStandardItem(resolucao)
            << new QStandardItem(QString::number(totalQuesitos))
            << new QStandardItem(cursoFicha);

        for (auto* it : row)
            it->setEditable(false);

        m_model->appendRow(row);
    }
}


void DialogoSelecionarFicha::onConfirmar()
{
    const QModelIndex idx = m_table->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, "Selecionar Ficha",
                                 "Selecione uma ficha na lista.");
        return;
    }

    const int row = idx.row();
    bool okId = false;
    int id = m_model->item(row, 0)->text().toInt(&okId);
    if (!okId) {
        QMessageBox::warning(this, "Selecionar Ficha",
                             "ID de ficha inválido.");
        return;
    }

    const QString tipo      = m_model->item(row, 1)->text();
    const QString resolucao = m_model->item(row, 2)->text();
    const QString curso     = m_model->item(row, 4)->text();

    m_fichaIdSelecionada = id;
    // Label amigável que vai aparecer na coluna "Ficha" da tabela de projetos
    m_fichaLabelSelecionada =
        QString("%1 - Res. %2 - %3").arg(tipo, resolucao, curso);

    accept();
}

void DialogoSelecionarFicha::onDuploClique(const QModelIndex& idx)
{
    if (!idx.isValid()) return;
    onConfirmar();
}
