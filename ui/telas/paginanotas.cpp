#include "paginanotas.h"
#include "ui_paginanotas.h"
#include "dialogoavaliacaoficha.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QAbstractItemView>
#include <QLabel>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QFileDialog>
#include <QRegularExpression>

// ================== HELPER: Normalizar CPF ==================
static QString normalizarCpf(const QString& cpf) {
    QString s = cpf;
    s.remove(QRegularExpression("\\D")); // remove tudo que não é dígito
    return s;
}

// ================== HELPERS INTERNOS (PROJETOS/VÍNCULOS) ==================

namespace {

struct ProjetoResumo {
    int     id{0};
    QString nome;
    QString categoria;
    QString status;
    int     idFicha{0};
};

// Carrega projetos do arquivo projetos.txt
// Formato: ID;Nome;Descricao;Responsavel;Categoria;Status;Ficha;IdFicha
QMap<int, ProjetoResumo> carregarProjetos(const QString& caminho)
{
    QMap<int, ProjetoResumo> mapa;
    QFile f(caminho);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return mapa;
    }

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList cols = line.split(';');
        if (cols.size() < 8) continue;

        ProjetoResumo p;
        p.id        = cols[0].toInt();
        p.nome      = cols[1].trimmed();
        p.categoria = cols[4].trimmed();
        p.status    = cols[5].trimmed();
        p.idFicha   = cols[7].toInt();

        mapa.insert(p.id, p);
    }

    return mapa;
}

// Carrega os projetos vinculados a um avaliador específico
// Formato do vinculos_projetos.csv: idProjeto;cpfAvaliador
QList<int> carregarProjetosDoAvaliador(const QString& caminhoVinculos,
                                       const QString& cpf)
{
    QList<int> lista;
    QFile f(caminhoVinculos);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return lista;
    }

    // Normaliza o CPF do avaliador logado
    QString cpfNormalizado = normalizarCpf(cpf);

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList cols = line.split(';');
        if (cols.size() < 2) continue;

        const int idProj = cols[0].toInt();
        const QString cpfAval = normalizarCpf(cols[1].trimmed());

        // Compara CPFs normalizados
        if (cpfAval == cpfNormalizado && idProj > 0) {
            if (!lista.contains(idProj))
                lista.append(idProj);
        }
    }

    return lista;
}

} // namespace

// ================== CONSTRUTOR / DESTRUTOR ==================

PaginaNotas::PaginaNotas(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PaginaNotas)
    , m_table(new QTableView(this))
    , m_model(new QStandardItemModel(this))
    , m_btnNovo(new QPushButton("📝 Nova Nota", this))
    , m_btnEditar(new QPushButton("✏️ Editar", this))
    , m_btnRemover(new QPushButton("🗑️ Remover", this))
    , m_btnRecarregar(new QPushButton("🔄 Recarregar", this))
    , m_btnExportCsv(new QPushButton("📊 Exportar CSV", this))
    , m_labelTotal(new QLabel(this))
{
    ui->setupUi(this);
    configurarUi();

    // Modo padrão: admin (sem avaliador logado)
    m_modoAvaliador = false;
    configurarTabelaAdmin();

    carregarNotasDoArquivo();
    recarregarDados();
}

PaginaNotas::~PaginaNotas()
{
    delete ui;
}

// ================== CONFIGURAÇÃO DE UI ==================

void PaginaNotas::configurarUi()
{
    this->setObjectName("paginanotas");

    this->setStyleSheet(R"(
        QWidget#paginanotas {
           background-color: #05070d;
           background-image: url(:/img/img/Logo.jpg);
           background-position: center bottom;
           background-repeat: no-repeat;
        }

        QLabel#titulo {
            color: #00D4FF;
            font-size: 24px;
            font-weight: bold;
            padding: 8px 0px;
        }

        QTableView {
            background-color: transparent;
            alternate-background-color: rgba(32, 42, 60, 0.85);
            gridline-color: #2a3f5f;
            selection-color: #FFFFFF;
            border: 2px solid #2a3f5f;
            border-radius: 10px;
            color: #E0E0E0;
        }

        QTableView::viewport {
            background-color: rgba(26, 35, 50, 0.85);
            background-image: url(:/img/img/Logo.jpg);
            background-position: center;
            background-repeat: no-repeat;
        }

        QTableView::item {
            padding: 10px 8px;
            border: none;
        }

        QTableView::item:hover {
            background-color: rgba(0, 168, 204, 0.15);
        }

        QTableView::item:selected {
            background-color: rgba(0, 220, 255, 0.35);
            border: none;
            color: #FFFFFF;
        }

        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1a2840, stop:1 #15202f);
            color: #00D4FF;
            padding: 12px 8px;
            border: none;
            border-bottom: 2px solid #00A8CC;
            border-right: 1px solid #2a3f5f;
            font-weight: bold;
            font-size: 13px;
        }

        QHeaderView::section:first {
            border-top-left-radius: 8px;
        }

        QHeaderView::section:last {
            border-top-right-radius: 8px;
            border-right: none;
        }

        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00A8CC, stop:1 #0088FF);
            border: none;
            border-radius: 0px;
            padding: 12px 24px;
            color: #FFFFFF;
            font-weight: bold;
            font-size: 13px;
        }

        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00D4FF, stop:1 #00A8FF);
        }

        QPushButton:pressed {
            background: #006688;
        }

        QPushButton#btnDanger {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #D946A6, stop:1 #FF6B9D);
        }

        QPushButton#btnDanger:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #E956B6, stop:1 #FF7BAD);
        }

        QPushButton#btnSecondary {
            background: rgba(42, 63, 95, 0.8);
            border: 2px solid #2a3f5f;
        }

        QPushButton#btnSecondary:hover {
            background: rgba(52, 73, 105, 1);
            border: 2px solid #3a4f6f;
        }

        QLabel#labelTotalNotas {
            color: #00D4FF;
            font-size: 13px;
            font-weight: bold;
            padding: 8px 0px;
        }
    )");

    m_btnRemover->setObjectName("btnDanger");
    m_btnRecarregar->setObjectName("btnSecondary");
    m_btnExportCsv->setObjectName("btnSecondary");
    m_labelTotal->setObjectName("labelTotalNotas");

    auto* root = ui->verticalLayout;
    root->setContentsMargins(120, 40, 120, 40);
    root->setSpacing(20);

    // Header
    auto* headerLayout = new QHBoxLayout();
    auto* titulo = new QLabel("📊 Notas e Avaliações", this);
    titulo->setObjectName("titulo");

    headerLayout->addWidget(titulo);
    headerLayout->addStretch();
    headerLayout->addWidget(m_btnNovo);
    headerLayout->addWidget(m_btnRecarregar);
    root->addLayout(headerLayout);

    // Tabela
    root->addWidget(m_table, 1);

    // Botões de ação
    auto* btnLayoutBottom = new QHBoxLayout();
    btnLayoutBottom->setSpacing(12);
    btnLayoutBottom->addWidget(m_btnEditar);
    btnLayoutBottom->addWidget(m_btnRemover);
    btnLayoutBottom->addStretch();
    btnLayoutBottom->addWidget(m_btnExportCsv);
    root->addLayout(btnLayoutBottom);

    // Rodapé
    root->addWidget(m_labelTotal);

    // Config tabela
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->setFocusPolicy(Qt::NoFocus);

    auto header = m_table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(QHeaderView::Stretch);

    // Conexões
    connect(m_table, &QTableView::doubleClicked,
            this, [this](const QModelIndex&) { onNovo(); });

    connect(m_btnNovo,       &QPushButton::clicked, this, &PaginaNotas::onNovo);
    connect(m_btnEditar,     &QPushButton::clicked, this, &PaginaNotas::onEditar);
    connect(m_btnRemover,    &QPushButton::clicked, this, &PaginaNotas::onRemover);
    connect(m_btnRecarregar, &QPushButton::clicked, this, &PaginaNotas::onRecarregar);
    connect(m_btnExportCsv,  &QPushButton::clicked, this, &PaginaNotas::onExportCsv);
}

void PaginaNotas::configurarTabelaAdmin()
{
    m_model->clear();
    m_model->setColumnCount(6);
    m_model->setHorizontalHeaderLabels({
        "ID Nota", "ID Projeto", "Projeto",
        "CPF Avaliador", "Avaliador", "Nota Final"
    });

    auto header = m_table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    m_btnNovo->setText("📝 Nova Nota");
    m_btnEditar->setVisible(true);
    m_btnRemover->setText("🗑️ Remover");
}

void PaginaNotas::configurarTabelaAvaliador()
{
    m_model->clear();
    m_model->setColumnCount(5);
    m_model->setHorizontalHeaderLabels({
        "ID Projeto", "Projeto", "Curso/Categoria",
        "Situação", "Nota Final"
    });

    auto header = m_table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    m_btnNovo->setText("📝 Avaliar / Editar");
    m_btnEditar->setVisible(false);
    m_btnRemover->setText("🗑️ Remover Minha Nota");
}

void PaginaNotas::atualizarTotalLabel(int total)
{
    if (m_modoAvaliador) {
        if (total == 1)
            m_labelTotal->setText("📊 1 projeto vinculado");
        else
            m_labelTotal->setText(QString("📊 %1 projetos vinculados").arg(total));
    } else {
        if (total == 1)
            m_labelTotal->setText("📊 1 nota registrada");
        else
            m_labelTotal->setText(QString("📊 %1 notas registradas").arg(total));
    }
}

// ================== CONTEXTO DO AVALIADOR ==================

void PaginaNotas::setAvaliador(const QString& cpf,
                               const QString& nome,
                               const QString& curso)
{
    m_cpfLogado   = normalizarCpf(cpf);  // ✅ NORMALIZA AO SETAR
    m_nomeLogado  = nome;
    m_cursoLogado = curso;

    m_modoAvaliador = !m_cpfLogado.trimmed().isEmpty();

    if (m_modoAvaliador)
        configurarTabelaAvaliador();
    else
        configurarTabelaAdmin();

    recarregarDados();
}

// ================== PERSISTÊNCIA ==================

bool PaginaNotas::carregarNotasDoArquivo()
{
    m_notas.clear();
    m_nextId = 1;

    QFile f(m_arquivoNotas);
    if (!f.exists()) {
        return true;
    }

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Carregar Notas",
                             "Não foi possível abrir '" + m_arquivoNotas + "' para leitura.");
        return false;
    }

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif
    int maxId = 0;

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList p = line.split(';');
        if (p.size() < 5) continue;

        Nota n;
        n.idNota        = p[0].toInt();
        n.idProjeto     = p[1].toInt();
        n.cpfAvaliador  = normalizarCpf(p[2].trimmed());  // ✅ NORMALIZA AO CARREGAR
        n.nomeAvaliador = p[3].trimmed();
        n.notaFinal     = p[4].toDouble();
        n.idFicha       = (p.size() >= 6) ? p[5].toInt() : 0;

        m_notas.append(n);
        if (n.idNota > maxId) maxId = n.idNota;
    }

    m_nextId = maxId + 1;
    return true;
}

bool PaginaNotas::salvarNotasNoArquivo() const
{
    QFile f(m_arquivoNotas);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Salvar Notas",
                             "Não foi possível abrir '" + m_arquivoNotas + "' para escrita.");
        return false;
    }

    QTextStream out(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif

    for (const Nota& n : m_notas) {
        out << n.idNota << ";"
            << n.idProjeto << ";"
            << n.cpfAvaliador << ";"  // já está normalizado
            << n.nomeAvaliador << ";"
            << n.notaFinal << ";"
            << n.idFicha << "\n";
    }

    return true;
}

void PaginaNotas::recomputarNextId()
{
    int maxId = 0;
    for (const Nota& n : m_notas) {
        if (n.idNota > maxId) maxId = n.idNota;
    }
    m_nextId = maxId + 1;
}

// ================== PREENCHIMENTO DE TABELA ==================

void PaginaNotas::recarregarDados()
{
    m_model->removeRows(0, m_model->rowCount());

    if (m_modoAvaliador)
        preencherTabelaAvaliador();
    else
        preencherTabelaAdmin();

    atualizarTotalLabel(m_model->rowCount());
}

void PaginaNotas::preencherTabelaAdmin()
{
    QMap<int, ProjetoResumo> projetos = carregarProjetos(m_arquivoProjetos);

    for (const Nota& n : m_notas) {
        const ProjetoResumo p = projetos.value(n.idProjeto);

        const QString nomeProj =
            p.id > 0 ? p.nome : QString("ID %1 (não encontrado)").arg(n.idProjeto);

        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(n.idNota));
        row << new QStandardItem(QString::number(n.idProjeto));
        row << new QStandardItem(nomeProj);
        row << new QStandardItem(n.cpfAvaliador);
        row << new QStandardItem(n.nomeAvaliador);
        row << new QStandardItem(QString::number(n.notaFinal, 'f', 2));

        row[0]->setEditable(false);

        m_model->appendRow(row);
    }
}

void PaginaNotas::preencherTabelaAvaliador()
{
    m_model->removeRows(0, m_model->rowCount());

    if (m_cpfLogado.isEmpty())
        return;

    // ✅ Carrega projetos vinculados ao avaliador
    QList<int> projetosAvaliador =
        carregarProjetosDoAvaliador(m_arquivoVinculos, m_cpfLogado);

    // Carrega os projetos completos
    QMap<int, ProjetoResumo> projetos =
        carregarProjetos(m_arquivoProjetos);

    // Cria mapa de notas já lançadas
    QMap<int, double> notasPorProjeto;
    for (const Nota& n : m_notas) {
        if (n.cpfAvaliador == m_cpfLogado) {
            notasPorProjeto[n.idProjeto] = n.notaFinal;
        }
    }

    // Monta tabela
    for (int idProj : projetosAvaliador)
    {
        if (!projetos.contains(idProj))
            continue;

        const ProjetoResumo& p = projetos[idProj];

        double nota = notasPorProjeto.value(idProj, -1);

        QString situacao = (nota >= 0) ? "✅ Avaliado" : "⏳ Não avaliado";
        QString notaStr = (nota >= 0)
                              ? QString::number(nota, 'f', 2)
                              : "—";

        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(p.id));
        row << new QStandardItem(p.nome);
        row << new QStandardItem(p.categoria);
        row << new QStandardItem(situacao);
        row << new QStandardItem(notaStr);

        row[0]->setEditable(false);

        m_model->appendRow(row);
    }

    m_table->resizeColumnsToContents();
}

// ================== HELPERS ==================

int PaginaNotas::selectedRow() const
{
    const auto idx = m_table->currentIndex();
    return idx.isValid() ? idx.row() : -1;
}

PaginaNotas::Nota* PaginaNotas::encontrarNotaPorId(int idNota)
{
    for (Nota& n : m_notas) {
        if (n.idNota == idNota)
            return &n;
    }
    return nullptr;
}

PaginaNotas::Nota* PaginaNotas::encontrarNotaDoAvaliador(int idProjeto, const QString& cpf)
{
    QString cpfNorm = normalizarCpf(cpf);
    for (Nota& n : m_notas) {
        if (n.idProjeto == idProjeto && n.cpfAvaliador == cpfNorm)
            return &n;
    }
    return nullptr;
}

void PaginaNotas::removerAvaliacoesDoArquivo(int idNota, int idProjeto, const QString& cpf)
{
    QFile f(m_arquivoAvaliacoes);
    if (!f.exists())
        return;

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    QVector<QString> linhas;
    QString cpfNorm = normalizarCpf(cpf);

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList p = line.split(';');
        if (p.size() < 6) {
            linhas.append(line);
            continue;
        }

        int     idNotaLinha = p[0].toInt();
        int     idProjLinha = p[1].toInt();
        QString cpfLinha    = normalizarCpf(p[2].trimmed());

        if (idNotaLinha == idNota &&
            idProjLinha == idProjeto &&
            cpfLinha     == cpfNorm) {
            continue;
        }

        linhas.append(line);
    }

    f.close();

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;

    QTextStream out(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif

    for (const QString& l : linhas)
        out << l << '\n';
}

// ================== SLOTS (CRUD) ==================

void PaginaNotas::onNovo()
{
    if (m_modoAvaliador) {
        const int r = selectedRow();
        if (r < 0) {
            QMessageBox::information(this, "Avaliar Projeto",
                                     "Selecione um projeto para avaliar.");
            return;
        }

        const int idProj = m_model->item(r, 0)->text().toInt();

        // Carrega projeto
        QMap<int, ProjetoResumo> projetos = carregarProjetos(m_arquivoProjetos);
        if (!projetos.contains(idProj)) {
            QMessageBox::warning(this, "Projetos",
                                 "Projeto não encontrado no arquivo de projetos.");
            return;
        }

        const ProjetoResumo& p = projetos[idProj];

        if (p.idFicha <= 0) {
            QMessageBox::warning(this, "Ficha",
                                 "Este projeto não possui ficha associada.\n\n"
                                 "Peça ao administrador para definir uma ficha para este projeto.");
            return;
        }

        // Verifica se já existe nota
        Nota* notaExistente = encontrarNotaDoAvaliador(idProj, m_cpfLogado);

        int idNota = 0;
        if (notaExistente) {
            idNota = notaExistente->idNota;
        } else {
            idNota = m_nextId;
        }

        // ✅ ABRE O DIÁLOGO DE AVALIAÇÃO
        DialogoAvaliacaoFicha dlg(
            idProj,
            p.idFicha,
            p.nome,
            m_cpfLogado,
            m_nomeLogado,
            idNota,
            this
            );

        if (dlg.exec() != QDialog::Accepted)
            return;

        double notaFinal = dlg.notaFinal();

        if (!notaExistente) {
            // Cria nova nota
            Nota n;
            n.idNota        = idNota;
            n.idProjeto     = idProj;
            n.idFicha       = p.idFicha;
            n.cpfAvaliador  = m_cpfLogado;
            n.nomeAvaliador = m_nomeLogado;
            n.notaFinal     = notaFinal;

            m_notas.append(n);
            if (idNota >= m_nextId)
                m_nextId = idNota + 1;
        } else {
            // Atualiza nota existente
            notaExistente->notaFinal = notaFinal;
            notaExistente->idFicha   = p.idFicha;
        }

        salvarNotasNoArquivo();
        recarregarDados();

        QMessageBox::information(this, "Sucesso",
                                 "Avaliação salva com sucesso!");
    } else {
        // MODO ADMIN: criação manual
        bool ok = false;

        int idProj = QInputDialog::getInt(
            this, "Nova Nota", "ID do Projeto:",
            1, 1, 999999, 1, &ok
            );
        if (!ok) return;

        QString cpf = QInputDialog::getText(
            this, "Nova Nota", "CPF do Avaliador:",
            QLineEdit::Normal, {}, &ok
            );
        if (!ok) return;

        QString nome = QInputDialog::getText(
            this, "Nova Nota", "Nome do Avaliador:",
            QLineEdit::Normal, {}, &ok
            );
        if (!ok) return;

        double valor = QInputDialog::getDouble(
            this, "Nova Nota", "Nota final (0–10):",
            7.0, 0.0, 10.0, 2, &ok
            );
        if (!ok) return;

        QMap<int, ProjetoResumo> projetos = carregarProjetos(m_arquivoProjetos);
        const int idFicha = projetos.value(idProj).idFicha;

        Nota n;
        n.idNota        = m_nextId++;
        n.idProjeto     = idProj;
        n.idFicha       = idFicha;
        n.cpfAvaliador  = normalizarCpf(cpf);
        n.nomeAvaliador = nome.trimmed();
        n.notaFinal     = valor;

        m_notas.append(n);
        salvarNotasNoArquivo();
        recarregarDados();
    }
}

void PaginaNotas::onEditar()
{
    // Modo avaliador: editar = mesma lógica de avaliar
    if (m_modoAvaliador) {
        onNovo();
        return;
    }

    // MODO ADMIN
    const int r = selectedRow();
    if (r < 0) {
        QMessageBox::information(this, "Editar Nota",
                                 "Selecione uma nota para editar.");
        return;
    }

    const int idNota = m_model->item(r, 0)->text().toInt();
    Nota* nota = encontrarNotaPorId(idNota);
    if (!nota)
        return;

    bool ok = false;

    int idProj = QInputDialog::getInt(
        this, "Editar Nota", "ID do Projeto:",
        nota->idProjeto, 1, 999999, 1, &ok
        );
    if (!ok) return;

    QString cpf = QInputDialog::getText(
        this, "Editar Nota", "CPF do Avaliador:",
        QLineEdit::Normal, nota->cpfAvaliador, &ok
        );
    if (!ok) return;

    QString nome = QInputDialog::getText(
        this, "Editar Nota", "Nome do Avaliador:",
        QLineEdit::Normal, nota->nomeAvaliador, &ok
        );
    if (!ok) return;

    double valor = QInputDialog::getDouble(
        this, "Editar Nota", "Nota final (0–10):",
        nota->notaFinal, 0.0, 10.0, 2, &ok
        );
    if (!ok) return;

    // Atualiza a nota em memória
    nota->idProjeto     = idProj;
    nota->cpfAvaliador  = normalizarCpf(cpf);   // mantém CPF normalizado
    nota->nomeAvaliador = nome.trimmed();
    nota->notaFinal     = valor;

    // Mantém idFicha em sincronia com o projeto
    QMap<int, ProjetoResumo> projetos = carregarProjetos(m_arquivoProjetos);
    nota->idFicha = projetos.value(idProj).idFicha;

    salvarNotasNoArquivo();
    recarregarDados();
}

void PaginaNotas::onRemover()
{
    const int r = selectedRow();
    if (r < 0) {
        QMessageBox::information(
            this, "Remover",
            m_modoAvaliador
                ? "Selecione um projeto para remover sua nota."
                : "Selecione uma nota para remover."
            );
        return;
    }

    if (m_modoAvaliador) {
        // MODO AVALIADOR: remove apenas a SUA nota daquele projeto
        const int idProj = m_model->item(r, 0)->text().toInt();

        int idx = -1;
        for (int i = 0; i < m_notas.size(); ++i) {
            const Nota& n = m_notas[i];
            if (n.idProjeto == idProj && n.cpfAvaliador == m_cpfLogado) {
                idx = i;
                break;
            }
        }

        if (idx < 0) {
            QMessageBox::information(this, "Remover Nota",
                                     "Este projeto ainda não possui uma nota sua.");
            return;
        }

        const Nota n = m_notas[idx];

        if (QMessageBox::question(this, "Remover Nota",
                                  "Remover sua nota para este projeto?")
            != QMessageBox::Yes)
            return;

        // Remove da memória
        m_notas.remove(idx);
        salvarNotasNoArquivo();

        // Remove também as avaliações detalhadas (quesitos)
        removerAvaliacoesDoArquivo(n.idNota, n.idProjeto, n.cpfAvaliador);

        recarregarDados();
    } else {
        // MODO ADMIN: remove qualquer nota
        const int idNota = m_model->item(r, 0)->text().toInt();

        int idx = -1;
        for (int i = 0; i < m_notas.size(); ++i) {
            if (m_notas[i].idNota == idNota) {
                idx = i;
                break;
            }
        }

        if (idx < 0) return;

        const Nota n = m_notas[idx];

        if (QMessageBox::question(this, "Remover Nota",
                                  "Remover nota selecionada?")
            != QMessageBox::Yes)
            return;

        m_notas.remove(idx);
        salvarNotasNoArquivo();

        removerAvaliacoesDoArquivo(n.idNota, n.idProjeto, n.cpfAvaliador);

        recarregarDados();
    }
}

void PaginaNotas::onRecarregar()
{
    // Recarrega notas do arquivo e reconstrói a tabela
    if (!carregarNotasDoArquivo())
        return;

    recarregarDados();
}

void PaginaNotas::onExportCsv()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Exportar notas para CSV",
        "notas.csv",
        "Arquivos CSV (*.csv);;Todos os arquivos (*.*)"
        );

    if (filename.isEmpty())
        return;

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Exportar CSV",
                             "Não foi possível abrir o arquivo para escrita.");
        return;
    }

    QTextStream out(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif

    // Carrega resumo dos projetos para enriquecer o CSV
    QMap<int, ProjetoResumo> projetos = carregarProjetos(m_arquivoProjetos);

    // Cabeçalho
    out << "IdNota;IdProjeto;Projeto;CategoriaProjeto;StatusProjeto;"
           "IdFicha;CpfAvaliador;NomeAvaliador;NotaFinal\n";

    for (const Nota& n : m_notas) {
        const ProjetoResumo p = projetos.value(n.idProjeto);

        QString nomeProj = (p.id > 0)
                               ? p.nome
                               : QString("ID %1 (não encontrado)").arg(n.idProjeto);
        QString categ  = p.categoria;
        QString status = p.status;

        // Se a nota ainda estiver sem idFicha (compatível com arquivo antigo),
        // usa o idFicha do projeto
        int idFichaExport = (n.idFicha > 0) ? n.idFicha : p.idFicha;

        // Evita quebrar o CSV com ';' dentro dos textos
        nomeProj.replace(';', ',');
        categ.replace(';', ',');
        status.replace(';', ',');
        QString cpf = n.cpfAvaliador;
        cpf.replace(';', ',');
        QString nomeAval = n.nomeAvaliador;
        nomeAval.replace(';', ',');

        out << n.idNota      << ';'
            << n.idProjeto   << ';'
            << nomeProj      << ';'
            << categ         << ';'
            << status        << ';'
            << idFichaExport << ';'
            << cpf           << ';'
            << nomeAval      << ';'
            << n.notaFinal   << '\n';
    }

    QMessageBox::information(this, "Exportar CSV",
                             "Notas exportadas com sucesso.");
}
