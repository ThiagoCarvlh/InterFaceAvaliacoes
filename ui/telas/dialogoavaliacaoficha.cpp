#include "dialogoavaliacaoficha.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QDateTime>
#include <QPageSize>

// ================== CONSTRUTOR SIMPLES (usado pela PaginaProjetos) ==================

DialogoAvaliacaoFicha::DialogoAvaliacaoFicha(int idProjeto,
                                             int idFicha,
                                             const QString& nomeProjeto,
                                             const QString& responsavelProjeto,
                                             const QString& nomeFicha,
                                             QWidget* parent)
    : QDialog(parent)
    , m_idProjeto(idProjeto)
    , m_idFicha(idFicha)
    , m_nomeProjeto(nomeProjeto)
    , m_responsavelProjeto(responsavelProjeto)
    , m_nomeFicha(nomeFicha)
    , m_cpfAvaliador()
    , m_nomeAvaliador()
    , m_idNota(-1)
{
    setWindowTitle(QString("Avaliação - Projeto %1").arg(idProjeto));
    resize(900, 700);
    setModal(true);

    // Estilo básico escuro
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

    FichaSimples ficha;
    if (!carregarFicha(ficha)) {
        QMessageBox::critical(this, "Erro", "Não foi possível carregar a ficha de avaliação.");
        reject();
        return;
    }

    if (m_nomeFicha.isEmpty())
        m_nomeFicha = ficha.tipoFicha;

    montarUI(ficha);
    // Por enquanto não recarrega avaliações antigas
    // carregarAvaliacoesQuesitos();
}

// ================== CONSTRUTOR COMPLETO (pensado para PaginaNotas) ==================

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
    , m_responsavelProjeto()  // pode ser preenchido depois se quiser
    , m_nomeFicha()
    , m_cpfAvaliador(cpfAvaliador)
    , m_nomeAvaliador(nomeAvaliador)
    , m_idNota(idNota)
{
    setWindowTitle(QString("Avaliação - Projeto %1").arg(idProjeto));
    resize(900, 700);
    setModal(true);

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

    FichaSimples ficha;
    if (!carregarFicha(ficha)) {
        QMessageBox::critical(this, "Erro", "Não foi possível carregar a ficha de avaliação.");
        reject();
        return;
    }

    if (m_nomeFicha.isEmpty())
        m_nomeFicha = ficha.tipoFicha;

    montarUI(ficha);
    // Aqui no futuro daria pra recarregar notas antigas dessa combinação
    // carregarAvaliacoesQuesitos();
}

// ================== CARREGAR FICHA (fichas.txt real) ==================

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

                // Se for quesito auto-calculado, não cria campo de input
                if (!autoCalc) {
                    sec.quesitos.append(qc);
                }
            }

            ficha.secoes.append(sec);
        }

        return true;
    }

    return false;
}

// ================== MONTAR UI ==================

void DialogoAvaliacaoFicha::montarUI(const FichaSimples& ficha)
{
    m_mainLayout = new QVBoxLayout(this);

    // --------- 1. DADOS GERAIS ---------
    QGroupBox* boxDados = new QGroupBox("Dados da avaliação", this);
    QFormLayout* formDados = new QFormLayout(boxDados);

    QLabel* lblProjeto = new QLabel(m_nomeProjeto, this);
    QLabel* lblResp    = new QLabel(m_responsavelProjeto.isEmpty() ? "-" : m_responsavelProjeto, this);
    QLabel* lblFicha   = new QLabel(m_nomeFicha.isEmpty() ? ficha.tipoFicha : m_nomeFicha, this);

    m_editCpfAvaliador  = new QLineEdit(m_cpfAvaliador, this);
    m_editNomeAvaliador = new QLineEdit(m_nomeAvaliador, this);

    formDados->addRow("Projeto:", lblProjeto);
    formDados->addRow("Responsável:", lblResp);
    formDados->addRow("Ficha:", lblFicha);
    formDados->addRow("CPF do avaliador:", m_editCpfAvaliador);
    formDados->addRow("Nome do avaliador:", m_editNomeAvaliador);

    m_mainLayout->addWidget(boxDados);

    // --------- 2. INFO FICHA ---------
    auto* info = new QLabel(
        QString("Ficha: <b>%1</b> &nbsp;&nbsp; Curso: <b>%2</b> &nbsp;&nbsp; Escala: %3 a %4")
            .arg(ficha.tipoFicha)
            .arg(ficha.curso)
            .arg(ficha.notaMin)
            .arg(ficha.notaMax),
        this
        );
    info->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(info);

    // --------- 3. SEÇÕES E QUESITOS ---------
    for (const auto& sec : ficha.secoes) {
        auto* box = new QGroupBox(QString("%1 - %2").arg(sec.identificador, sec.titulo), this);
        auto* form = new QFormLayout(box);
        form->setSpacing(8);

        for (const auto& q : sec.quesitos) {
            auto* lbl  = new QLabel(q.nomeQuesito, box);
            auto* spin = new QDoubleSpinBox(box);

            spin->setRange(ficha.notaMin, ficha.notaMax);
            spin->setDecimals(1);
            spin->setSingleStep(0.5);
            spin->setValue(ficha.notaMax);

            QuesitoCampo campo = q;
            campo.spin = spin;
            m_campos.append(campo);

            form->addRow(lbl, spin);
        }

        m_mainLayout->addWidget(box);
    }

    m_mainLayout->addStretch();

    // --------- 4. BOTÕES ---------
    QHBoxLayout* layoutBotoes = new QHBoxLayout();

    m_btnSalvar   = new QPushButton("Salvar", this);
    m_btnPdf      = new QPushButton("Salvar PDF", this);
    m_btnCancelar = new QPushButton("Cancelar", this);
    m_btnCancelar->setObjectName("btnCancel");

    layoutBotoes->addStretch();
    layoutBotoes->addWidget(m_btnSalvar);
    layoutBotoes->addWidget(m_btnPdf);
    layoutBotoes->addWidget(m_btnCancelar);

    m_mainLayout->addLayout(layoutBotoes);

    // Conexões
    connect(m_btnSalvar, &QPushButton::clicked, this, [this]{
        salvarAvaliacoesQuesitos();
        accept();
    });

    connect(m_btnPdf, &QPushButton::clicked, this, &DialogoAvaliacaoFicha::onSalvarPdf);
    connect(m_btnCancelar, &QPushButton::clicked, this, &DialogoAvaliacaoFicha::reject);
}

// ================== (OPCIONAL) CARREGAR AVALIAÇÕES EXISTENTES ==================

void DialogoAvaliacaoFicha::carregarAvaliacoesQuesitos()
{
    // MVP: não recarregamos avaliações antigas.
    // Quando quiser reabrir/editar, aqui é o lugar de ler avaliacoes.csv e
    // preencher os spins a partir da coluna "notasQuesitos".
}

// ================== SALVAR CSV DE RESUMO ==================

void DialogoAvaliacaoFicha::salvarAvaliacoesQuesitos()
{
    QFile file(m_arquivoAvaliacoes);
    const bool arquivoExistia = file.exists();

    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Erro",
                             "Não foi possível abrir " + m_arquivoAvaliacoes);
        return;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif

    if (!arquivoExistia) {
        out << "idProjeto;nomeProjeto;responsavel;"
               "idFicha;nomeFicha;"
               "cpfAvaliador;nomeAvaliador;"
               "notaFinal;notasQuesitos\n";
    }

    const double notaFinal = calcularNotaFinal();
    m_notaFinal = notaFinal;

    QStringList notasQuesitos;
    for (const auto& campo : m_campos) {
        if (campo.spin) {
            notasQuesitos << QString::number(campo.spin->value(), 'f', 2);
        }
    }

    out << m_idProjeto                  << ';'
        << m_nomeProjeto                << ';'
        << (m_responsavelProjeto.isEmpty() ? "-" : m_responsavelProjeto) << ';'
        << m_idFicha                    << ';'
        << m_nomeFicha                  << ';'
        << m_editCpfAvaliador->text()   << ';'
        << m_editNomeAvaliador->text()  << ';'
        << QString::number(notaFinal, 'f', 2) << ';'
        << notasQuesitos.join('|')
        << '\n';

    file.close();
}

// ================== CÁLCULO DA NOTA FINAL (com peso) ==================

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

// ================== PDF ==================

void DialogoAvaliacaoFicha::onSalvarPdf()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Salvar avaliação em PDF",
        QString("avaliacao_%1_%2.pdf").arg(m_idProjeto).arg(m_idFicha),
        "PDF (*.pdf)"
        );

    if (filename.isEmpty())
        return;

    QString html;
    html += "<h1 align='center'>Avaliação de Projeto</h1><hr>";
    html += "<p><b>Projeto:</b> " + m_nomeProjeto + "</p>";
    html += "<p><b>Responsável:</b> "
            + (m_responsavelProjeto.isEmpty() ? "N/A" : m_responsavelProjeto) + "</p>";
    html += "<p><b>Ficha:</b> " + m_nomeFicha + "</p>";
    html += "<p><b>Avaliador (CPF):</b> " + m_editCpfAvaliador->text() + "</p>";
    html += "<p><b>Avaliador (Nome):</b> " + m_editNomeAvaliador->text() + "</p>";
    html += "<p><b>Data:</b> "
            + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm") + "</p>";

    html += "<br><table border='1' cellspacing='0' cellpadding='4' "
            "width='100%' style='border-collapse: collapse;'>";
    html += "<tr style='background-color: #eeeeee;'>"
            "<th>Seção</th><th>Quesito</th><th>Nota</th></tr>";

    for (const auto& campo : m_campos) {
        if (!campo.spin) continue;
        html += "<tr>";
        html += "<td align='center'>" + campo.idSecao + "</td>";
        html += "<td>" + campo.nomeQuesito + "</td>";
        html += "<td align='center'>" +
                QString::number(campo.spin->value(), 'f', 2) + "</td>";
        html += "</tr>";
    }

    html += "</table>";

    html += QString("<h3 align='right'>Nota Final: %1</h3>")
                .arg(QString::number(calcularNotaFinal(), 'f', 2));

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    printer.setPageSize(QPageSize::A4);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "PDF Gerado", "Arquivo salvo com sucesso!");
}
