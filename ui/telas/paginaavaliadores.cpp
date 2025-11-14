#include "paginaavaliadores.h"
#include "ui_paginaavaliadores.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

PaginaAvaliadores::PaginaAvaliadores(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PaginaAvaliadores)
    , m_table(new QTableView(this))
    , m_model(new QStandardItemModel(0, 5, this)) // ID, Nome, Email, CPF, Categoria
    , m_btnNovo(new QPushButton("Novo", this))
    , m_btnEditar(new QPushButton("Editar", this))
    , m_btnRemover(new QPushButton("Remover", this))
    , m_btnRecarregar(new QPushButton("Recarregar", this))
{
    ui->setupUi(this);

    auto hl = new QHBoxLayout();
    hl->setContentsMargins(0,0,0,0);
    hl->addWidget(m_btnNovo);
    hl->addWidget(m_btnEditar);
    hl->addWidget(m_btnRemover);
    hl->addStretch();
    hl->addWidget(m_btnRecarregar);
    ui->verticalLayout->addLayout(hl);

    ui->verticalLayout->addWidget(m_table);
    m_model->setHorizontalHeaderLabels({"ID","Nome","Email","CPF","Categoria"});
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(0, Qt::AscendingOrder);

    connect(m_btnNovo,       &QPushButton::clicked, this, &PaginaAvaliadores::onNovo);
    connect(m_btnEditar,     &QPushButton::clicked, this, &PaginaAvaliadores::onEditar);
    connect(m_btnRemover,    &QPushButton::clicked, this, &PaginaAvaliadores::onRemover);
    connect(m_btnRecarregar, &QPushButton::clicked, this, &PaginaAvaliadores::onRecarregar);

    carregarDoArquivo();
}

PaginaAvaliadores::~PaginaAvaliadores() { delete ui; }

int PaginaAvaliadores::selectedRow() const {
    const auto idx = m_table->currentIndex();
    return idx.isValid() ? idx.row() : -1;
}

void PaginaAvaliadores::addAvaliador(const QString& nome, const QString& email,
                                     const QString& cpf,  const QString& categoria)
{
    QList<QStandardItem*> row;
    auto id = new QStandardItem(QString::number(m_nextId++));
    id->setEditable(false);
    row << id
        << new QStandardItem(nome)
        << new QStandardItem(email)
        << new QStandardItem(cpf)
        << new QStandardItem(categoria);
    m_model->appendRow(row);
}

void PaginaAvaliadores::onNovo() {
    bool ok;
    const auto nome = QInputDialog::getText(this, "Novo Avaliador", "Nome:", QLineEdit::Normal, {}, &ok);
    if (!ok || nome.trimmed().isEmpty()) return;
    const auto email = QInputDialog::getText(this, "Novo Avaliador", "Email:", QLineEdit::Normal, {}, &ok);
    if (!ok) return;
    const auto cpf   = QInputDialog::getText(this, "Novo Avaliador", "CPF:", QLineEdit::Normal, {}, &ok);
    if (!ok) return;
    const auto cat   = QInputDialog::getText(this, "Novo Avaliador", "Categoria (ex.: Técnico/Graduação):", QLineEdit::Normal, {}, &ok);
    if (!ok) return;

    addAvaliador(nome.trimmed(), email.trimmed(), cpf.trimmed(), cat.trimmed());
    salvarNoArquivo();
}

void PaginaAvaliadores::onEditar() {
    const int r = selectedRow();
    if (r < 0) { QMessageBox::information(this, "Editar", "Selecione um avaliador."); return; }

    bool ok;
    auto nome = QInputDialog::getText(this, "Editar Avaliador", "Nome:", QLineEdit::Normal, m_model->item(r,1)->text(), &ok);
    if (!ok) return;
    auto email = QInputDialog::getText(this, "Editar Avaliador", "Email:", QLineEdit::Normal, m_model->item(r,2)->text(), &ok);
    if (!ok) return;
    auto cpf   = QInputDialog::getText(this, "Editar Avaliador", "CPF:", QLineEdit::Normal, m_model->item(r,3)->text(), &ok);
    if (!ok) return;
    auto cat   = QInputDialog::getText(this, "Editar Avaliador", "Categoria:", QLineEdit::Normal, m_model->item(r,4)->text(), &ok);
    if (!ok) return;

    m_model->item(r,1)->setText(nome);
    m_model->item(r,2)->setText(email);
    m_model->item(r,3)->setText(cpf);
    m_model->item(r,4)->setText(cat);

    salvarNoArquivo();
}

void PaginaAvaliadores::onRemover() {
    const int r = selectedRow();
    if (r < 0) { QMessageBox::information(this, "Remover", "Selecione um avaliador."); return; }
    if (QMessageBox::question(this, "Remover", "Remover avaliador selecionado?") == QMessageBox::Yes) {
        m_model->removeRow(r);
        salvarNoArquivo();
    }
}

void PaginaAvaliadores::onRecarregar() { carregarDoArquivo(); }

bool PaginaAvaliadores::salvarNoArquivo() const {
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
            s.replace(';', ',');
            cols << s;
        }
        out << cols.join(';') << '\n';
    }
    return true;
}

bool PaginaAvaliadores::carregarDoArquivo() {
    QFile f(m_arquivo);
    if (!f.exists()) return true;
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
        const QStringList p = line.split(';');
        if (p.size() < 5) continue;

        QList<QStandardItem*> row;
        auto idItem = new QStandardItem(p[0]); idItem->setEditable(false);
        row << idItem << new QStandardItem(p[1]) << new QStandardItem(p[2])
            << new QStandardItem(p[3]) << new QStandardItem(p[4]);
        m_model->appendRow(row);
    }
    recomputarNextId();
    return true;
}

void PaginaAvaliadores::recomputarNextId() {
    int maxId = 0;
    for (int r = 0; r < m_model->rowCount(); ++r) {
        bool ok = false;
        int id = m_model->item(r,0)->text().toInt(&ok);
        if (ok && id > maxId) maxId = id;
    }
    m_nextId = maxId + 1;
}
