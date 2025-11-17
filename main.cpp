#include <QApplication>
#include "dialogologin.h"
#include "janelaprincipal.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DialogoLogin dlg;
    if (dlg.exec() != QDialog::Accepted)
        return 0;   // usuário cancelou

    JanelaPrincipal w;
    w.configurarPorLogin(dlg.isAdmin());
    w.show();

    return a.exec();
}
