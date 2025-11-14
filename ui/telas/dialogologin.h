// ui/telas/dialogologin.h  (opcional por enquanto)
#pragma once
#include <QDialog>
namespace Ui { class DialogoLogin; }
class DialogoLogin : public QDialog {
    Q_OBJECT
public:
    explicit DialogoLogin(QWidget* parent = nullptr);
    ~DialogoLogin();
private:
    Ui::DialogoLogin* ui;
};
