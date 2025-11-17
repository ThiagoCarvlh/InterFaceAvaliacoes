// vinculos.cpp
#include "vinculos.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QHash>

static QString normalizarCpf(const QString& cpf) {
    QString s = cpf;
    s.remove(QRegularExpression("\\D")); // remove tudo que não é dígito
    return s;
}

QVector<VinculoProjeto> carregarVinculos(const QString& arquivo) {
    QVector<VinculoProjeto> res;

    QFile f(arquivo);
    if (!f.exists())
        return res; // sem vínculos ainda

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return res;

    QTextStream in(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    in.setCodec("UTF-8");
#endif

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        const QStringList p = line.split(';');
        if (p.size() < 2) continue;

        bool ok = false;
        int idProj = p[0].toInt(&ok);
        if (!ok) continue;

        VinculoProjeto v;
        v.idProjeto    = idProj;
        v.cpfAvaliador = p[1].trimmed();
        res.push_back(v);
    }
    return res;
}

bool salvarVinculos(const QString& arquivo, const QVector<VinculoProjeto>& lista) {
    QFile f(arquivo);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&f);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif

    for (const auto& v : lista) {
        out << v.idProjeto << ';' << v.cpfAvaliador << '\n';
    }
    return true;
}

int contarProjetosDoAvaliador(const QVector<VinculoProjeto>& lista, const QString& cpf) {
    const QString alvo = normalizarCpf(cpf);
    if (alvo.isEmpty()) return 0;

    int count = 0;
    for (const auto& v : lista) {
        if (normalizarCpf(v.cpfAvaliador) == alvo)
            ++count;
    }
    return count;
}

void removerVinculosPorProjeto(QVector<VinculoProjeto>& lista, int idProjeto) {
    for (int i = lista.size() - 1; i >= 0; --i) {
        if (lista[i].idProjeto == idProjeto) {
            lista.removeAt(i);
        }
    }
}

void removerVinculosPorAvaliador(QVector<VinculoProjeto>& lista, const QString& cpf) {
    const QString alvo = normalizarCpf(cpf);
    if (alvo.isEmpty()) return;

    for (int i = lista.size() - 1; i >= 0; --i) {
        if (normalizarCpf(lista[i].cpfAvaliador) == alvo) {
            lista.removeAt(i);
        }
    }
}
