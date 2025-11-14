#include "paginanotas.h"
#include "ui_paginanotas.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

PaginaNotas::PaginaNotas(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PaginaNotas)
    , m_table(new QTableView(this))
    , m_model(new QStandardItemModel(0, 4, this)) // ID, Projeto, Avaliador, Media
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
    m_model->setHorizontalHeaderLabels({"ID","Projeto","Avaliador","Media"});
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(0, Qt::AscendingOrder);

    connect(m_btnNovo,       &QPushButton::clicked, this, &PaginaNotas::onNovo);
    connect(m_btnEditar,     &QPushButton::clicked, this, &PaginaNotas::onEditar);
    connect(m_btnRemover,    &QPushButton::clicked, this, &PaginaNotas::onRemover);
    connect(m_btnRecarregar, &QPushButton::clicked, this, &PaginaNotas::onRecarregar);

    carregarDoArquivo();
}

PaginaNotas::~PaginaNotas() { delete ui; }

int PaginaNotas::selectedRow() const {
    const auto idx = m_table->currentIndex();
    return idx.isValid() ? idx.row() : -1;
}

void PaginaNotas::addNota(const QString& projeto, const QString& avaliador, double media) {
    QList<QStandardItem*> row;
    auto id = new QStandardItem(QString::number(m_nextId++));
    id->setEditable(false);
    row << id
        << new QStandardItem(projeto)
        << new QStandardItem(avaliador)
        << new QStandardItem(QString::number(media, 'f', 2));
    m_model->appendRow(row);
}

void PaginaNotas::onNovo() {
    bool ok;
    const auto projeto   = QInputDialog::getText(this, "Nova Nota", "Projeto:", QLineEdit::Normal, {}, &ok);
    if (!ok || projeto.trimmed().isEmpty()) return;
    const auto avaliador = QInputDialog::getText(this, "Nova Nota", "Avaliador:", QLineEdit::Normal, {}, &ok);
    if (!ok) return;
    double media = QInputDialog::getDouble(this, "Nova Nota", "Média (0–10):", 7.0, 0.0, 10.0, 2, &ok);
    if (!ok) return;

    addNota(projeto.trimmed(), avaliador.trimmed(), media);
    salvarNoArquivo();
}

void PaginaNotas::onEditar() {
    const int r = selectedRow();
    if (r < 0) { QMessageBox::information(this, "Editar", "Selecione um item."); return; }

    bool ok;
    auto projeto = QInputDialog::getText(this, "Editar Nota", "Projeto:",
                                         QLineEdit::Normal, m_model->item(r,1)->text(), &ok);
    if (!ok) return;
    auto avaliador = QInputDialog::getText(this, "Editar Nota", "Avaliador:",
                                           QLineEdit::Normal, m_model->item(r,2)->text(), &ok);
    if (!ok) return;
    double media = QInputDialog::getDouble(this, "Editar Nota", "Média (0–10):",
                                           m_model->item(r,3)->text().toDouble(), 0.0, 10.0, 2, &ok);
    if (!ok) return;

    m_model->item(r,1)->setText(projeto);
    m_model->item(r,2)->setText(avaliador);
    m_model->item(r,3)->setText(QString::number(media,'f',2));

    salvarNoArquivo();
}

void PaginaNotas::onRemover() {
    const int r = selectedRow();
    if (r < 0) { QMessageBox::information(this, "Remover", "Selecione um item."); return; }
    if (QMessageBox::question(this, "Remover", "Remover registro selecionado?") == QMessageBox::Yes) {
        m_model->removeRow(r);
        salvarNoArquivo();
    }
}

void PaginaNotas::onRecarregar() { carregarDoArquivo(); }

bool PaginaNotas::salvarNoArquivo() const {
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

bool PaginaNotas::carregarDoArquivo() {
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
        if (p.size() < 4) continue;

        QList<QStandardItem*> row;
        auto idItem = new QStandardItem(p[0]); idItem->setEditable(false);
        row << idItem << new QStandardItem(p[1])
            << new QStandardItem(p[2]) << new QStandardItem(p[3]);
        m_model->appendRow(row);
    }
    recomputarNextId();
    return true;
}

void PaginaNotas::recomputarNextId() {
    int maxId = 0;
    for (int r = 0; r < m_model->rowCount(); ++r) {
        bool ok = false;
        int id = m_model->item(r,0)->text().toInt(&ok);
        if (ok && id > maxId) maxId = id;
    }
    m_nextId = maxId + 1;
}
