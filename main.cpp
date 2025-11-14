// main.cpp
#include <QApplication>
#include "ui/telas/janelaprincipal.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    JanelaPrincipal w;
    w.show();
    return app.exec();
}
