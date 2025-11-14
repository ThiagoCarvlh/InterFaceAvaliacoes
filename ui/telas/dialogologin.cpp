// ui/telas/dialogologin.cpp  (opcional por enquanto)
#include "dialogologin.h"
#include "ui_dialogologin.h"
DialogoLogin::DialogoLogin(QWidget* parent)
    : QDialog(parent), ui(new Ui::DialogoLogin) { ui->setupUi(this); }
DialogoLogin::~DialogoLogin() { delete ui; }
