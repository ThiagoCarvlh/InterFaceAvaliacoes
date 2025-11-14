#include "paginaprojetos.h"
#include "ui_paginaprojetos.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

PaginaProjetos::PaginaProjetos(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PaginaProjetos)
    , m_table(new QTableView(this))
    , m_model(new QStandardItemModel(0, 4, this))
    , m_btnNovo(new QPushButton("Novo", this))
    , m_btnEditar(new QPushButton("Editar", this))
    , m_btnRemover(new QPushButton("Remover", this))
    , m_btnRecarregar(new QPushButton("Recarregar", this))
{
    ui->setupUi(this);

    // Botões
    auto hl = new QHBoxLayout();
    hl->setContentsMargins(0,0,0,0);
    hl->addWidget(m_btnNovo);
    hl->addWidget(m_btnEditar);
    hl->addWidget(m_btnRemover);
    hl->addStretch();
    hl->addWidget(m_btnRecarregar);
    ui->verticalLayout->addLayout(hl);

    // Tabela
    ui->verticalLayout->addWidget(m_table);
    m_model->setHorizontalHeaderLabels({"ID","Nome","Descricao","Responsavel"});
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(0, Qt::AscendingOrder);

    connect(m_btnNovo,       &QPushButton::clicked, this, &PaginaProjetos::onNovo);
    connect(m_btnEditar,     &QPushButton::clicked, this, &PaginaProjetos::onEditar);
    connect(m_btnRemover,    &QPushButton::clicked, this, &PaginaProjetos::onRemover);
    connect(m_btnRecarregar, &QPushButton::clicked, this, &PaginaProjetos::onRecarregar);

    // Carrega do arquivo (se existir)
    carregarDoArquivo();
}

void PaginaProjetos::addProjeto(const QString& nome, const QString& desc, const QString& resp) {
    QList<QStandardItem*> row;
    auto id = new QStandardItem(QString::number(m_nextId++));
    id->setEditable(false);
    row << id
        << new QStandardItem(nome)
        << new QStandardItem(desc)
        << new QStandardItem(resp);
    m_model->appendRow(row);
}

int PaginaProjetos::selectedRow() const {
    const auto idx = m_table->currentIndex();
    return idx.isValid() ? idx.row() : -1;
}

void PaginaProjetos::onNovo() {
    bool ok;
    const auto nome = QInputDialog::getText(this, "Novo Projeto", "Nome:", QLineEdit::Normal, {}, &ok);
    if (!ok || nome.trimmed().isEmpty()) return;
    const auto desc = QInputDialog::getText(this, "Novo Projeto", "Descrição:", QLineEdit::Normal, {}, &ok);
    if (!ok) return;
    const auto resp = QInputDialog::getText(this, "Novo Projeto", "Responsável:", QLineEdit::Normal, {}, &ok);
    if (!ok) return;

    addProjeto(nome.trimmed(), desc.trimmed(), resp.trimmed());
    salvarNoArquivo();
}

void PaginaProjetos::onEditar() {
    const int r = selectedRow();
    if (r < 0) { QMessageBox::information(this, "Editar", "Selecione um projeto."); return; }

    bool ok;
    auto nome = QInputDialog::getText(this, "Editar Projeto", "Nome:",
                                      QLineEdit::Normal, m_model->item(r,1)->text(), &ok);
    if (!ok) return;
    auto desc = QInputDialog::getText(this, "Editar Projeto", "Descrição:",
                                      QLineEdit::Normal, m_model->item(r,2)->text(), &ok);
    if (!ok) return;
    auto resp = QInputDialog::getText(this, "Editar Projeto", "Responsável:",
                                      QLineEdit::Normal, m_model->item(r,3)->text(), &ok);
    if (!ok) return;

    m_model->item(r,1)->setText(nome);
    m_model->item(r,2)->setText(desc);
    m_model->item(r,3)->setText(resp);

    salvarNoArquivo();
}

void PaginaProjetos::onRemover() {
    const int r = selectedRow();
    if (r < 0) { QMessageBox::information(this, "Remover", "Selecione um projeto."); return; }
    if (QMessageBox::question(this, "Remover", "Remover projeto selecionado?") == QMessageBox::Yes) {
        m_model->removeRow(r);
        salvarNoArquivo();
    }
}

bool PaginaProjetos::salvarNoArquivo() const {
    QFile f(m_arquivo);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Salvar", "Não foi possível abrir '"+m_arquivo+"' para escrita.");
        return false;
    }
    QTextStream out(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif
    for (int r = 0; r < m_model->rowCount(); ++r) {
        QStringList cols;
        for (int c = 0; c < m_model->columnCount(); ++c) {
            QString s = m_model->item(r, c)->text();
            s.replace(';', ','); // evita quebrar o CSV
            cols << s;
        }
        out << cols.join(';') << '\n';
    }
    return true;
}

bool PaginaProjetos::carregarDoArquivo() {
    QFile f(m_arquivo);
    if (!f.exists()) return true; // nada a carregar
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Carregar", "Não foi possível abrir '"+m_arquivo+"' para leitura.");
        return false;
    }
    m_model->removeRows(0, m_model->rowCount());

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        const QStringList parts = line.split(';');
        if (parts.size() < 4) continue;

        QList<QStandardItem*> row;
        auto idItem = new QStandardItem(parts[0]);
        idItem->setEditable(false);
        row << idItem
            << new QStandardItem(parts[1])
            << new QStandardItem(parts[2])
            << new QStandardItem(parts[3]);
        m_model->appendRow(row);
    }
    recomputarNextId();
    return true;
}

void PaginaProjetos::onRecarregar() {
    carregarDoArquivo();   // já limpa, carrega e recalcula m_nextId
}

void PaginaProjetos::recomputarNextId() {
    int maxId = 0;
    for (int r = 0; r < m_model->rowCount(); ++r) {
        bool ok = false;
        const int id = m_model->item(r, 0)->text().toInt(&ok);
        if (ok && id > maxId) maxId = id;
    }
    m_nextId = maxId + 1;
}

PaginaProjetos::~PaginaProjetos() { delete ui; }
