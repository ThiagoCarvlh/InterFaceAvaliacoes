#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

namespace Ui {
class DialogoLogin;
}

class DialogoLogin : public QDialog
{
    Q_OBJECT

public:
    explicit DialogoLogin(QWidget* parent = nullptr);
    ~DialogoLogin();

    // true = admin, false = avaliador
    bool isAdmin() const { return m_isAdmin; }

    // Dados do avaliador logado (vazios se for admin)
    QString cpfLogado()   const { return m_cpfLogado; }
    QString nomeLogado()  const { return m_nomeLogado; }
    QString cursoLogado() const { return m_cursoLogado; }

private slots:
    void tentarLogin();

private:
    Ui::DialogoLogin* ui;

    QLineEdit*   m_editLogin{};
    QLineEdit*   m_editSenha{};
    QLabel*      m_labelStatus{};
    QPushButton* m_btnLogin{};

    bool    m_isAdmin{false};

    // contexto do avaliador
    QString m_cpfLogado;
    QString m_nomeLogado;
    QString m_cursoLogado;
};
