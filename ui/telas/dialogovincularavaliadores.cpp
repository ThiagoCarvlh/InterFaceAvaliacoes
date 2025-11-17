#include "dialogovincularavaliadores.h"

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
#include <QRegularExpression>
#include <QSet>

DialogoVincularAvaliadores::DialogoVincularAvaliadores(int idProjeto,
                                                       const QString& nomeProjeto,
                                                       const QString& categoriaProjeto,
                                                       QWidget* parent)
    : QDialog(parent)
    , m_idProjeto(idProjeto)
    , m_nomeProjeto(nomeProjeto)
    , m_categoriaProjeto(categoriaProjeto)
    , m_tabDisponiveis(new QTableView(this))
    , m_tabSelecionados(new QTableView(this))
    , m_modelDisponiveis(new QStandardItemModel(0, 4, this))
    , m_modelSelecionados(new QStandardItemModel(0, 4, this))
    , m_btnAdd(new QPushButton(" >> Adicionar", this))
    , m_btnRemove(new QPushButton(" << Remover", this))
    , m_btnSalvar(new QPushButton(" Salvar", this))
    , m_btnCancelar(new QPushButton("Cancelar", this))
    , m_lblResumo(new QLabel(this))
{
    setModal(true);
    setMinimumSize(800, 500);
    if (parent)
        setStyleSheet(parent->styleSheet());

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // título
    auto* titulo = new QLabel("Vincular Avaliadores ao Projeto", this);
    QFont ft = titulo->font();
    ft.setPointSize(ft.pointSize() + 3);
    ft.setBold(true);
    titulo->setFont(ft);
    titulo->setStyleSheet("color: #00D4FF; padding-bottom: 8px;");
    mainLayout->addWidget(titulo);

    // informações do projeto
    auto* lblInfo = new QLabel(this);
    lblInfo->setText(
        QString("Projeto: <b>%1</b> (ID: %2)<br>"
                "Categoria: <b>%3</b><br>"
                "Selecione até <b>3 avaliadores</b> com a mesma área/especialidade.")
            .arg(m_nomeProjeto)
            .arg(m_idProjeto)
            .arg(m_categoriaProjeto));
    mainLayout->addWidget(lblInfo);

    // modelos
    m_modelDisponiveis->setHorizontalHeaderLabels({"Nome","Email","CPF","Categoria"});
    m_modelSelecionados->setHorizontalHeaderLabels({"Nome","Email","CPF","Categoria"});

    configurarTabela(m_tabDisponiveis,  m_modelDisponiveis);
    configurarTabela(m_tabSelecionados, m_modelSelecionados);

    // layout central (duas tabelas + botões)
    auto* centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(12);

    auto* colEsq = new QVBoxLayout();
    colEsq->addWidget(new QLabel("Avaliadores disponíveis", this));
    colEsq->addWidget(m_tabDisponiveis);

    auto* colDir = new QVBoxLayout();
    colDir->addWidget(new QLabel("Avaliadores selecionados", this));
    colDir->addWidget(m_tabSelecionados);

    auto* midBtns = new QVBoxLayout();
    midBtns->addStretch();
    midBtns->addWidget(m_btnAdd);
    midBtns->addWidget(m_btnRemove);
    midBtns->addStretch();

    centerLayout->addLayout(colEsq, 1);
    centerLayout->addLayout(midBtns);
    centerLayout->addLayout(colDir, 1);

    mainLayout->addLayout(centerLayout);

    // resumo + botões inferiores
    m_lblResumo->setStyleSheet("color: #E0E0E0; padding-top: 4px;");
    mainLayout->addWidget(m_lblResumo);

    auto* footerLayout = new QHBoxLayout();
    footerLayout->addStretch();
    footerLayout->addWidget(m_btnCancelar);
    footerLayout->addWidget(m_btnSalvar);
    mainLayout->addLayout(footerLayout);

    // conexões
    connect(m_btnAdd,     &QPushButton::clicked, this, &DialogoVincularAvaliadores::onAdicionar);
    connect(m_btnRemove,  &QPushButton::clicked, this, &DialogoVincularAvaliadores::onRemover);
    connect(m_btnSalvar,  &QPushButton::clicked, this, &DialogoVincularAvaliadores::onSalvar);
    connect(m_btnCancelar,&QPushButton::clicked, this, &DialogoVincularAvaliadores::reject);

    carregarDados();
    atualizarResumo();
}

void DialogoVincularAvaliadores::configurarTabela(QTableView* table,
                                                  QStandardItemModel* model)
{
    table->setModel(model);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSortingEnabled(true);
    table->sortByColumn(0, Qt::AscendingOrder);
}

void DialogoVincularAvaliadores::carregarDados()
{
    // quais CPFs já estão vinculados a este projeto
    QVector<VinculoProjeto> vincs = carregarVinculos(m_arquivoVinculos);
    QSet<QString> cpfsProjeto;

    for (const auto& v : vincs) {
        if (v.idProjeto == m_idProjeto) {
            QString cpf = v.cpfAvaliador;
            cpf.remove(QRegularExpression("\\D"));
            cpfsProjeto.insert(cpf);
        }
    }

    QFile f(m_arquivoAvaliadores);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        const QStringList p = line.split(';');
        if (p.size() < 7) continue; // ID, Nome, Email, CPF, Categoria, Senha, Status,...

        const QString nome      = p.value(1);
        const QString email     = p.value(2);
        QString       cpf       = p.value(3);
        const QString categoria = p.value(4);
        const QString status    = p.value(6);

        // só avaliadores ativos e da mesma categoria/especialidade do projeto
        if (!status.compare("Ativo", Qt::CaseInsensitive) == 0 &&
            status.trimmed() != "Ativo")
            continue;

        if (categoria.trimmed() != m_categoriaProjeto.trimmed())
            continue;

        QString cpfNorm = cpf;
        cpfNorm.remove(QRegularExpression("\\D"));

        QList<QStandardItem*> row;
        row << new QStandardItem(nome)
            << new QStandardItem(email)
            << new QStandardItem(cpf)
            << new QStandardItem(categoria);

        for (auto* it : row)
            it->setEditable(false);

        if (cpfsProjeto.contains(cpfNorm))
            m_modelSelecionados->appendRow(row);
        else
            m_modelDisponiveis->appendRow(row);
    }

    atualizarResumo();
}

void DialogoVincularAvaliadores::atualizarResumo()
{
    const int qtd = m_modelSelecionados->rowCount();
    m_lblResumo->setText(
        QString("Avaliadores selecionados: <b>%1</b> de 3").arg(qtd));
}

int DialogoVincularAvaliadores::totalSelecionados() const
{
    return m_modelSelecionados ? m_modelSelecionados->rowCount() : 0;
}

void DialogoVincularAvaliadores::onAdicionar()
{
    const QModelIndex idx = m_tabDisponiveis->currentIndex();
    if (!idx.isValid()) return;

    if (m_modelSelecionados->rowCount() >= 3) {
        QMessageBox::warning(this, "Limite atingido",
                             "Cada projeto pode ter no máximo 3 avaliadores.");
        return;
    }

    const int row = idx.row();
    QList<QStandardItem*> novaLinha;
    for (int c = 0; c < m_modelDisponiveis->columnCount(); ++c) {
        auto* src = m_modelDisponiveis->item(row, c);
        auto* copy = new QStandardItem(src ? src->text() : QString());
        copy->setEditable(false);
        novaLinha << copy;
    }
    m_modelSelecionados->appendRow(novaLinha);
    m_modelDisponiveis->removeRow(row);

    atualizarResumo();
}

void DialogoVincularAvaliadores::onRemover()
{
    const QModelIndex idx = m_tabSelecionados->currentIndex();
    if (!idx.isValid()) return;

    const int row = idx.row();
    QList<QStandardItem*> novaLinha;
    for (int c = 0; c < m_modelSelecionados->columnCount(); ++c) {
        auto* src = m_modelSelecionados->item(row, c);
        auto* copy = new QStandardItem(src ? src->text() : QString());
        copy->setEditable(false);
        novaLinha << copy;
    }
    m_modelDisponiveis->appendRow(novaLinha);
    m_modelSelecionados->removeRow(row);

    atualizarResumo();
}

void DialogoVincularAvaliadores::onSalvar()
{
    const int qtd = m_modelSelecionados->rowCount();
    if (qtd > 3) {
        QMessageBox::warning(this, "Erro",
                             "O projeto não pode ter mais de 3 avaliadores.");
        return;
    }

    auto lista = carregarVinculos(m_arquivoVinculos);
    removerVinculosPorProjeto(lista, m_idProjeto);

    for (int r = 0; r < m_modelSelecionados->rowCount(); ++r) {
        QString cpf = m_modelSelecionados->item(r, 2)->text(); // coluna CPF
        VinculoProjeto v;
        v.idProjeto    = m_idProjeto;
        v.cpfAvaliador = cpf;
        lista.push_back(v);
    }

    if (!salvarVinculos(m_arquivoVinculos, lista)) {
        QMessageBox::warning(this, "Erro",
                             "Não foi possível salvar os vínculos.");
        return;
    }

    accept(); // fecha diálogo com QDialog::Accepted
}
