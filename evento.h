/* ============================================================
 * evento.h — Sistema de Gerenciamento de Eventos Críticos
 * Cidade Inteligente — Estruturas e protótipos
 * ============================================================ */

#ifndef EVENTO_H
#define EVENTO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -------- Tipos de evento -------- */
typedef enum {
    ACIDENTE_TRANSITO = 0,
    FALHA_SEMAFORO    = 1,
    INTERRUPCAO_ENERGIA = 2,
    ALAGAMENTO        = 3,
    INCENDIO          = 4,
    TIPO_DESCONHECIDO = 5
} TipoEvento;

/* -------- Status do evento -------- */
typedef enum {
    ATIVO    = 0,
    RESOLVIDO = 1
} StatusEvento;

/* -------- Data e hora do registro -------- */
typedef struct {
    int dia;
    int mes;
    int ano;
    int hora;
    int minuto;
} DataHora;

/* -------- Informações do evento (campo info do nó) -------- */
typedef struct {
    int id;                    /* chave primária da AVL */
    TipoEvento tipo;
    int severidade;            /* 1 a 5 */
    DataHora registro;
    char regiao[50];
    StatusEvento status;
} Evento;

/* -------- Nó da árvore AVL --------
 * Estilo idêntico ao usado em sala (struct NO com info, alt, esq, dir).
 */
struct NO {
    Evento info;
    int alt;
    struct NO *esq;
    struct NO *dir;
};
typedef struct NO* AVLTree;

/* -------- Contador global de rotações --------
 * Usado pelas métricas. Incrementado dentro das funções de rotação.
 */
extern long total_rotacoes;

/* ============================================================
 * Protótipos — AVL básica
 * ============================================================ */
AVLTree* cria_AVL(void);
void libera_AVL(AVLTree *raiz);
int  altura_NO(struct NO *no);
int  maior(int a, int b);
int  fatorBalanceamento_NO(struct NO *no);

/* Rotações */
void rotacaoLL(AVLTree *raiz);
void rotacaoRR(AVLTree *raiz);
void rotacaoLR(AVLTree *raiz);
void rotacaoRL(AVLTree *raiz);

/* Operações */
int  inserir_AVL(AVLTree *raiz, Evento e);
int  remover_AVL(AVLTree *raiz, int id);
struct NO* buscar_AVL(AVLTree *raiz, int id);

/* ============================================================
 * Protótipos — Consultas avançadas
 * ============================================================ */
void listar_por_severidade(AVLTree *raiz, int sev_min, int sev_max);
void relatorio_por_regiao(AVLTree *raiz, const char *regiao);
void buscar_intervalo_id(AVLTree *raiz, int id_min, int id_max);
void listar_em_ordem(AVLTree *raiz);

/* ============================================================
 * Protótipos — Atualizações
 * ============================================================ */
int  resolver_evento(AVLTree *raiz, int id);
int  atualizar_severidade(AVLTree *raiz, int id, int nova_sev);

/* ============================================================
 * Protótipos — Métricas
 * ============================================================ */
int    altura_arvore(AVLTree *raiz);
int    total_nos(AVLTree *raiz);
int    total_ativos(AVLTree *raiz);
double fb_medio(AVLTree *raiz);
long   get_total_rotacoes(void);

/* ============================================================
 * Protótipos — Utilitários e impressão
 * ============================================================ */
const char* nome_tipo(TipoEvento t);
const char* nome_status(StatusEvento s);
TipoEvento  parse_tipo(int opcao);
void imprime_evento(Evento e);
void imprime_evento_inline(Evento e);
DataHora data_hora_atual(void);

#endif /* EVENTO_H */
