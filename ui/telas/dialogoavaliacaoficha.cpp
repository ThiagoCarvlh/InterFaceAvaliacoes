#include "dialogoavaliacaoficha.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

DialogoAvaliacaoFicha::DialogoAvaliacaoFicha(int idProjeto,
                                             int idFicha,
                                             const QString& nomeProjeto,
                                             const QString& cpfAvaliador,
                                             const QString& nomeAvaliador,
                                             int idNota,
                                             QWidget* parent)
    : QDialog(parent)
    , m_idProjeto(idProjeto)
    , m_idFicha(idFicha)
    , m_nomeProjeto(nomeProjeto)
    , m_cpfAvaliador(cpfAvaliador)
    , m_nomeAvaliador(nomeAvaliador)
    , m_idNota(idNota)
{
    setWindowTitle("Avaliação do Projeto");
    resize(900, 600);
    setModal(true);

    // Estilo básico escuro (sem exagero, mas compatível)
    setStyleSheet(R"(
        QDialog {
            background-color: #0a0e1a;
        }
        QLabel {
            color: #E0E0E0;
        }
        QGroupBox {
            border: 1px solid #2a3f5f;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 10px;
            color: #00D4FF;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 4px 10px;
            background: #1a2332;
            border-radius: 4px;
        }
        QDoubleSpinBox {
            background-color: #1a2332;
            border: 1px solid #2a3f5f;
            border-radius: 4px;
            padding: 4px 8px;
            color: #FFFFFF;
        }
        QDoubleSpinBox:focus {
            border: 1px solid #00D4FF;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00A8CC, stop:1 #0088FF);
            border: none;
            border-radius: 4px;
            padding: 8px 18px;
            color: #FFFFFF;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00D4FF, stop:1 #00A8FF);
        }
        QPushButton#btnCancel {
            background: #2a3f5f;
        }
        QPushButton#btnCancel:hover {
            background: #3a4f6f;
        }
    )");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto* lblTitulo = new QLabel(
        QString("Avaliação do Projeto: <b>%1</b>").arg(m_nomeProjeto),
        this
        );
    root->addWidget(lblTitulo);

    // Área rolável com a ficha
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    auto* scrollWidget = new QWidget(scroll);
    m_mainLayout = new QVBoxLayout(scrollWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(12);

    scroll->setWidget(scrollWidget);
    root->addWidget(scroll, 1);

    // Botões
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto* btnCancelar = new QPushButton("Cancelar", this);
    btnCancelar->setObjectName("btnCancel");
    auto* btnSalvar   = new QPushButton("Salvar Avaliação", this);
    btnLayout->addWidget(btnCancelar);
    btnLayout->addWidget(btnSalvar);
    root->addLayout(btnLayout);

    connect(btnCancelar, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnSalvar,   &QPushButton::clicked, this, &DialogoAvaliacaoFicha::onSalvar);

    // Carrega ficha e monta campos
    FichaSimples ficha;
    if (!carregarFicha(ficha)) {
        QMessageBox::warning(this, "Ficha",
                             "Não foi possível carregar a ficha associada ao projeto.");
        reject();
        return;
    }

    montarUI(ficha);
    carregarAvaliacoesQuesitos();
}

// ================== Carregar ficha ==================

bool DialogoAvaliacaoFicha::carregarFicha(FichaSimples& ficha)
{
    QFile f(m_arquivoFichas);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList p = line.split(';');
        if (p.size() < 14) continue;

        int idx = 0;
        int id = p[idx++].toInt();
        if (id != m_idFicha)
            continue;

        ficha.id        = id;
        ficha.tipoFicha = p[idx++];
        QString resolucaoNum = p[idx++];
        QString resolucaoAno = p[idx++];
        ficha.curso    = p[idx++];
        QString categoriaCurso = p[idx++];
        ficha.notaMin  = p[idx++].toDouble();
        ficha.notaMax  = p[idx++].toDouble();
        bool incluirData       = (p[idx++] == "1");
        bool incluirProfAval   = (p[idx++] == "1");
        bool incluirProfOrient = (p[idx++] == "1");
        bool incluirObs        = (p[idx++] == "1");
        QString textoAprov     = p[idx++];

        int numSecoes = p[idx++].toInt();

        ficha.secoes.clear();
        for (int i = 0; i < numSecoes && idx < p.size(); ++i) {
            SecaoSimples sec;
            sec.identificador = p[idx++];
            if (idx >= p.size()) break;
            sec.titulo = p[idx++];
            if (idx >= p.size()) break;
            int numQuesitos = p[idx++].toInt();

            for (int j = 0; j < numQuesitos && idx < p.size(); ++j) {
                QuesitoCampo qc;
                qc.idSecao      = sec.identificador;
                qc.nomeQuesito  = p[idx++];
                if (idx >= p.size()) break;
                bool autoCalc   = (p[idx++] == "1");
                if (idx >= p.size()) break;
                qc.temPeso      = (p[idx++] == "1");
                if (idx >= p.size()) break;
                qc.peso         = p[idx++].toDouble();

                if (!autoCalc) {
                    sec.quesitos.append(qc);
                }
                // se auto-calculado, ignoramos aqui (sem input)
            }

            ficha.secoes.append(sec);
        }

        return true;
    }

    return false;
}

// ================== Montar UI ==================

void DialogoAvaliacaoFicha::montarUI(const FichaSimples& ficha)
{
    // Info simples da ficha
    auto* info = new QLabel(
        QString("Ficha: <b>%1</b> &nbsp;&nbsp; Curso: <b>%2</b> &nbsp;&nbsp; Escala: %3 a %4")
            .arg(ficha.tipoFicha)
            .arg(ficha.curso)
            .arg(ficha.notaMin)
            .arg(ficha.notaMax),
        this
        );
    m_mainLayout->addWidget(info);

    for (const auto& sec : ficha.secoes) {
        auto* box = new QGroupBox(
            QString("%1 - %2").arg(sec.identificador, sec.titulo),
            this
            );
        auto* form = new QFormLayout(box);
        form->setSpacing(8);

        for (const auto& q : sec.quesitos) {
            auto* lbl = new QLabel(q.nomeQuesito, box);
            auto* spin = new QDoubleSpinBox(box);
            spin->setRange(ficha.notaMin, ficha.notaMax);
            spin->setDecimals(1);
            spin->setSingleStep(0.1);
            spin->setValue(ficha.notaMax); // por padrão nota máxima

            QuesitoCampo campo = q;
            campo.spin = spin;
            m_campos.append(campo);

            form->addRow(lbl, spin);
        }

        m_mainLayout->addWidget(box);
    }

    m_mainLayout->addStretch();
}

// ================== Carregar / Salvar quesitos ==================

void DialogoAvaliacaoFicha::carregarAvaliacoesQuesitos()
{
    QFile f(m_arquivoAvaliacoes);
    if (!f.exists()) return;
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList p = line.split(';');
        if (p.size() < 6) continue;

        int     idNotaLinha  = p[0].toInt();
        int     idProjLinha  = p[1].toInt();
        QString cpfLinha     = p[2].trimmed();
        QString idSecao      = p[3].trimmed();
        QString nomeQues     = p[4].trimmed();
        double  notaQ        = p[5].toDouble();

        if (idNotaLinha != m_idNota ||
            idProjLinha != m_idProjeto ||
            cpfLinha    != m_cpfAvaliador)
            continue;

        for (QuesitoCampo& campo : m_campos) {
            if (campo.idSecao == idSecao &&
                campo.nomeQuesito == nomeQues &&
                campo.spin) {
                campo.spin->setValue(notaQ);
            }
        }
    }
}

void DialogoAvaliacaoFicha::salvarAvaliacoesQuesitos()
{
    QFile f(m_arquivoAvaliacoes);
    QVector<QString> linhas;

    if (f.exists()) {
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Avaliações",
                                 "Não foi possível abrir avaliacoes.csv para leitura.");
            return;
        }

        QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        in.setCodec("UTF-8");
#endif

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
            QString cpfLinha    = p[2].trimmed();

            // remove entradas antigas deste avaliador/projeto/idNota
            if (idNotaLinha == m_idNota &&
                idProjLinha == m_idProjeto &&
                cpfLinha    == m_cpfAvaliador) {
                continue;
            }

            linhas.append(line);
        }

        f.close();
    }

    // adiciona as linhas novas
    for (const QuesitoCampo& campo : m_campos) {
        if (!campo.spin) continue;
        double nota = campo.spin->value();

        QString linha = QString("%1;%2;%3;%4;%5;%6")
                            .arg(m_idNota)
                            .arg(m_idProjeto)
                            .arg(m_cpfAvaliador)
                            .arg(campo.idSecao)
                            .arg(campo.nomeQuesito)
                            .arg(QString::number(nota, 'f', 2));
        linhas.append(linha);
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Avaliações",
                             "Não foi possível abrir avaliacoes.csv para escrita.");
        return;
    }

    QTextStream out(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif
    for (const QString& l : linhas)
        out << l << '\n';
}

double DialogoAvaliacaoFicha::calcularNotaFinal() const
{
    double somaPonderada = 0.0;
    double somaPesos     = 0.0;

    for (const QuesitoCampo& campo : m_campos) {
        if (!campo.spin) continue;
        double nota = campo.spin->value();
        double peso = campo.temPeso ? campo.peso : 1.0;
        somaPonderada += nota * peso;
        somaPesos     += peso;
    }

    if (somaPesos <= 0.0) return 0.0;
    return somaPonderada / somaPesos;
}

// ================== Slot Salvar ==================

void DialogoAvaliacaoFicha::onSalvar()
{
    m_notaFinal = calcularNotaFinal();
    salvarAvaliacoesQuesitos();
    accept();
}
