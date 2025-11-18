#include "dialogologin.h"
#include "ui_dialogologin.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

DialogoLogin::DialogoLogin(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogoLogin)
{
    ui->setupUi(this);
    setWindowTitle("Login");
    setModal(true);
    resize(420, 260);

    // Usa o layout raiz do .ui (verticalLayout)
    auto* root = ui->verticalLayout;
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    // Título
    auto* titulo = new QLabel("Acesso ao Sistema de Avaliações", this);
    QFont f = titulo->font();
    f.setPointSize(f.pointSize() + 2);
    f.setBold(true);
    titulo->setFont(f);
    root->addWidget(titulo);

    // Formulário
    auto* form = new QFormLayout();
    form->setSpacing(10);

    m_editLogin = new QLineEdit(this);
    m_editSenha = new QLineEdit(this);
    m_editSenha->setEchoMode(QLineEdit::Password);

    m_editLogin->setPlaceholderText("admin ou CPF do avaliador");
    m_editSenha->setPlaceholderText("senha");

    form->addRow("Login:", m_editLogin);
    form->addRow("Senha:", m_editSenha);

    root->addLayout(form);

    // Mensagem de status
    m_labelStatus = new QLabel(this);
    m_labelStatus->setStyleSheet("color: #FF6B9D;");
    root->addWidget(m_labelStatus);

    // Botão
    m_btnLogin = new QPushButton("Entrar", this);
    root->addWidget(m_btnLogin);

    connect(m_btnLogin, &QPushButton::clicked,
            this, &DialogoLogin::tentarLogin);
    connect(m_editSenha, &QLineEdit::returnPressed,
            this, &DialogoLogin::tentarLogin);
}

DialogoLogin::~DialogoLogin()
{
    delete ui;
}

void DialogoLogin::tentarLogin()
{
    // Login digitado
    QString login = m_editLogin->text().trimmed();
    const QString senhaDigitada = m_editSenha->text();

    if (login.isEmpty() || senhaDigitada.isEmpty()) {
        m_labelStatus->setText("Preencha login e senha.");
        return;
    }

    // 1) ADMIN
    if (login == "admin" && senhaDigitada == "admin123") {
        m_isAdmin     = true;
        m_cpfLogado   = {};
        m_nomeLogado  = {};
        m_cursoLogado = {};
        accept();
        return;
    }

    // 2) AVALIADOR (login = CPF)
    // se quiser aceitar com/sem máscara, normaliza:
    // login.remove(QRegularExpression("\\D"));

    QFile f("avaliadores.csv");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_labelStatus->setText("Não foi possível abrir 'avaliadores.csv'. Tente como admin.");
        return;
    }

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    bool encontrado = false;

    // Formato real:
    // 0:id ; 1:nome ; 2:email ; 3:cpf ; 4:cat+curso ; 5:senha ; 6:status ; 7:...
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList cols = line.split(';');
        if (cols.size() < 6) continue;

        QString cpfArquivo = cols[3].trimmed();
        // se quiser aceitar CPF com máscara, descomenta:
        // cpfArquivo.remove(QRegularExpression("\\D"));
        // login.remove(QRegularExpression("\\D"));

        if (cpfArquivo == login) {
            encontrado = true;

            const QString senhaArquivo = cols[5].trimmed();   // SENHA = col F
            if (senhaArquivo != senhaDigitada) {
                m_labelStatus->setText("Senha inválida para este avaliador.");
                return;
            }

            m_isAdmin     = false;
            m_cpfLogado   = cpfArquivo;
            m_nomeLogado  = cols[1].trimmed();  // Nome
            m_cursoLogado = cols[4].trimmed();  // "Graduação - Engenharia de Software"

            accept();
            return;
        }
    }

    if (!encontrado) {
        m_labelStatus->setText("Avaliador não encontrado para esse CPF.");
    }
}



