#include "dialogologin.h"
#include "ui_dialogologin.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QTextStream>

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
    const QString login = m_editLogin->text().trimmed();
    const QString senha = m_editSenha->text();

    if (login.isEmpty() || senha.isEmpty()) {
        m_labelStatus->setText("Preencha login e senha.");
        return;
    }

    // 1) ADMIN: login "admin" / senha "admin123"
    if (login == "admin" && senha == "admin123") {
        m_isAdmin = true;
        accept();
        return;
    }

    // 2) AVALIADOR: login = CPF que está em avaliadores.txt, senha fixa "123456"
    QFile f("avaliadores.txt");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_labelStatus->setText("Não foi possível abrir 'avaliadores.txt'. Tente como admin.");
        return;
    }

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList cols = line.split(';');
        if (cols.size() < 4) continue; // id;nome;email;cpf;...

        const QString cpf = cols[3].trimmed();
        if (cpf == login) {
            if (senha != "123456") {
                m_labelStatus->setText("Senha de avaliador inválida (use 123456).");
                return;
            }
            m_isAdmin = false;
            accept();
            return;
        }
    }

    m_labelStatus->setText("Usuário não encontrado.");
}
