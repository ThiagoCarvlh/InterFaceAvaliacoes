// vinculos.h
#pragma once
#include <QString>
#include <QVector>

// Representa um vínculo "projeto X é avaliado pelo CPF Y"
struct VinculoProjeto {
    int idProjeto{0};
    QString cpfAvaliador; // pode vir com ou sem máscara
};

// Carrega todos os vínculos do arquivo (um por linha: idProjeto;cpfAvaliador)
QVector<VinculoProjeto> carregarVinculos(const QString& arquivo);

// Salva a lista completa de vínculos no arquivo (sobrescreve)
bool salvarVinculos(const QString& arquivo, const QVector<VinculoProjeto>& lista);

// Conta quantos projetos um avaliador (CPF) tem vinculados
int contarProjetosDoAvaliador(const QVector<VinculoProjeto>& lista, const QString& cpf);

// Remove todos os vínculos de um projeto específico
void removerVinculosPorProjeto(QVector<VinculoProjeto>& lista, int idProjeto);

// Remove todos os vínculos de um avaliador (por CPF)
void removerVinculosPorAvaliador(QVector<VinculoProjeto>& lista, const QString& cpf);
