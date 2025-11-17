#include <QApplication>
#include <QIcon> // <-- 1. INCLUA ISSO
#include "dialogologin.h"
#include "janelaprincipal.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 2. PEGUE O CAMINHO DO SEU RESOURCE (baseado na sua imagem)
    QString caminhoIconeApp = ":/img/img/Logo.jpg";

    // 3. DEFINA O ÍCONE NA APLICAÇÃO (ISSO AFETA TODAS AS JANELAS)
    a.setWindowIcon(QIcon(caminhoIconeApp));

    DialogoLogin dlg;
    // NÃo precisa mais de dlg.setWindowIcon(),
    // pois ela já vai "herdar" o ícone da aplicação.

    if (dlg.exec() != QDialog::Accepted)
        return 0;

    JanelaPrincipal w;
    // NÃo precisa mais de w.setWindowIcon(),
    // pois ela também vai "herdar".

    w.configurarPorLogin(dlg.isAdmin());
    w.show();

    return a.exec();
}
