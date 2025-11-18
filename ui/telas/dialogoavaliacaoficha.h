#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class QVBoxLayout;
class QDoubleSpinBox;

class DialogoAvaliacaoFicha : public QDialog
{
    Q_OBJECT
public:
    DialogoAvaliacaoFicha(int idProjeto,
                          int idFicha,
                          const QString& nomeProjeto,
                          const QString& cpfAvaliador,
                          const QString& nomeAvaliador,
                          int idNota,
                          QWidget* parent = nullptr);

    double notaFinal() const { return m_notaFinal; }
    int    idNota()    const { return m_idNota; }

private slots:
    void onSalvar();

private:
    struct QuesitoCampo {
        QString        idSecao;
        QString        nomeQuesito;
        double         peso{1.0};
        bool           temPeso{false};
        QDoubleSpinBox* spin{};
    };

    struct SecaoSimples {
        QString identificador;
        QString titulo;
        QVector<QuesitoCampo> quesitos;
    };

    struct FichaSimples {
        int id{0};
        QString tipoFicha;
        QString curso;
        double notaMin{0.0};
        double notaMax{10.0};
        QVector<SecaoSimples> secoes;
    };

    // Contexto
    int     m_idProjeto{};
    int     m_idFicha{};
    QString m_nomeProjeto;
    QString m_cpfAvaliador;
    QString m_nomeAvaliador;
    int     m_idNota{};       // idNota já decidido pela PaginaNotas
    double  m_notaFinal{0.0};

    // UI
    QVBoxLayout*        m_mainLayout{};
    QVector<QuesitoCampo> m_campos;

    // Arquivos
    const QString m_arquivoFichas     = "fichas.txt";
    const QString m_arquivoAvaliacoes = "avaliacoes.csv";

    bool   carregarFicha(FichaSimples& ficha);
    void   montarUI(const FichaSimples& ficha);
    void   carregarAvaliacoesQuesitos();
    void   salvarAvaliacoesQuesitos();
    double calcularNotaFinal() const;
};
