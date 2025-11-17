#pragma once

#include <QDialog>

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

private slots:
    void tentarLogin();

private:
    Ui::DialogoLogin* ui;

    QLineEdit*  m_editLogin{};
    QLineEdit*  m_editSenha{};
    QLabel*     m_labelStatus{};
    QPushButton* m_btnLogin{};

    bool m_isAdmin{false};
};
