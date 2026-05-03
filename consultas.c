/* ============================================================
 * consultas.c — Consultas avançadas, atualizações e métricas
 * ============================================================ */

#include "evento.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Utilitários de impressão
 * ============================================================ */
const char* nome_tipo(TipoEvento t) {
    switch (t) {
        case ACIDENTE_TRANSITO:    return "Acidente de Transito";
        case FALHA_SEMAFORO:       return "Falha em Semaforo";
        case INTERRUPCAO_ENERGIA:  return "Interrupcao de Energia";
        case ALAGAMENTO:           return "Alagamento";
        case INCENDIO:             return "Incendio";
        default:                    return "Tipo Desconhecido";
    }
}

const char* nome_status(StatusEvento s) {
    return (s == ATIVO) ? "ATIVO" : "RESOLVIDO";
}

TipoEvento parse_tipo(int opcao) {
    if (opcao >= 0 && opcao <= 4) return (TipoEvento) opcao;
    return TIPO_DESCONHECIDO;
}

DataHora data_hora_atual(void) {
    DataHora dh;
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    dh.dia    = t->tm_mday;
    dh.mes    = t->tm_mon + 1;
    dh.ano    = t->tm_year + 1900;
    dh.hora   = t->tm_hour;
    dh.minuto = t->tm_min;
    return dh;
}

void imprime_evento(Evento e) {
    printf("---------------------------------------------\n");
    printf("ID         : %d\n", e.id);
    printf("Tipo       : %s\n", nome_tipo(e.tipo));
    printf("Severidade : %d\n", e.severidade);
    printf("Registro   : %02d/%02d/%04d %02d:%02d\n",
           e.registro.dia, e.registro.mes, e.registro.ano,
           e.registro.hora, e.registro.minuto);
    printf("Regiao     : %s\n", e.regiao);
    printf("Status     : %s\n", nome_status(e.status));
    printf("---------------------------------------------\n");
}

void imprime_evento_inline(Evento e) {
    printf("[ID %4d] sev=%d %-22s | regiao=%-15s | %s\n",
           e.id, e.severidade, nome_tipo(e.tipo),
           e.regiao, nome_status(e.status));
}

/* ============================================================
 * Consultas avançadas
 * ============================================================ */

/* 1) Listagem de eventos ATIVOS por intervalo de severidade.
 * Percurso in-ordem completo, mas com filtragem por severidade e status.
 * (Ordenação por ID natural sai do percurso.)
 */
static void rec_listar_severidade(struct NO *no, int smin, int smax, int *cont) {
    if (no == NULL) return;
    rec_listar_severidade(no->esq, smin, smax, cont);
    if (no->info.status == ATIVO &&
        no->info.severidade >= smin && no->info.severidade <= smax) {
        imprime_evento_inline(no->info);
        (*cont)++;
    }
    rec_listar_severidade(no->dir, smin, smax, cont);
}

void listar_por_severidade(AVLTree *raiz, int sev_min, int sev_max) {
    int cont = 0;
    if (raiz == NULL || *raiz == NULL) {
        printf("Arvore vazia.\n");
        return;
    }
    printf("\n=== Eventos ATIVOS com severidade entre %d e %d ===\n",
           sev_min, sev_max);
    rec_listar_severidade(*raiz, sev_min, sev_max, &cont);
    printf("Total: %d evento(s)\n", cont);
}

/* 2) Relatório por região: ativos da região X em ordem de ID. */
static void rec_relatorio_regiao(struct NO *no, const char *regiao, int *cont) {
    if (no == NULL) return;
    rec_relatorio_regiao(no->esq, regiao, cont);
    if (no->info.status == ATIVO && strcmp(no->info.regiao, regiao) == 0) {
        imprime_evento_inline(no->info);
        (*cont)++;
    }
    rec_relatorio_regiao(no->dir, regiao, cont);
}

void relatorio_por_regiao(AVLTree *raiz, const char *regiao) {
    int cont = 0;
    if (raiz == NULL || *raiz == NULL) {
        printf("Arvore vazia.\n");
        return;
    }
    printf("\n=== Eventos ATIVOS na regiao '%s' ===\n", regiao);
    rec_relatorio_regiao(*raiz, regiao, &cont);
    printf("Total: %d evento(s)\n", cont);
}

/* 3) Busca por intervalo de ID — usa propriedade da BST pra podar.
 * Esta é a consulta MAIS eficiente porque evita visitar subárvores
 * inteiras que estão fora do intervalo [id_min, id_max].
 */
static void rec_intervalo_id(struct NO *no, int id_min, int id_max, int *cont) {
    if (no == NULL) return;

    /* só desce à esquerda se houver chance de existir id >= id_min */
    if (no->info.id > id_min)
        rec_intervalo_id(no->esq, id_min, id_max, cont);

    if (no->info.id >= id_min && no->info.id <= id_max) {
        imprime_evento_inline(no->info);
        (*cont)++;
    }

    /* só desce à direita se houver chance de existir id <= id_max */
    if (no->info.id < id_max)
        rec_intervalo_id(no->dir, id_min, id_max, cont);
}

void buscar_intervalo_id(AVLTree *raiz, int id_min, int id_max) {
    int cont = 0;
    if (raiz == NULL || *raiz == NULL) {
        printf("Arvore vazia.\n");
        return;
    }
    printf("\n=== Eventos com ID entre %d e %d ===\n", id_min, id_max);
    rec_intervalo_id(*raiz, id_min, id_max, &cont);
    printf("Total: %d evento(s)\n", cont);
}

/* Listagem geral em-ordem (todos os eventos por ID crescente) */
static void rec_em_ordem(struct NO *no) {
    if (no == NULL) return;
    rec_em_ordem(no->esq);
    imprime_evento_inline(no->info);
    rec_em_ordem(no->dir);
}

void listar_em_ordem(AVLTree *raiz) {
    if (raiz == NULL || *raiz == NULL) {
        printf("Arvore vazia.\n");
        return;
    }
    printf("\n=== Todos os eventos (ordenados por ID) ===\n");
    rec_em_ordem(*raiz);
}

/* ============================================================
 * Atualizações
 * Não alteram o ID (chave da AVL), portanto não quebram a estrutura.
 * ============================================================ */
int resolver_evento(AVLTree *raiz, int id) {
    struct NO *no = buscar_AVL(raiz, id);
    if (no == NULL) return 0;
    if (no->info.status == RESOLVIDO) return -1;  /* já estava resolvido */
    no->info.status = RESOLVIDO;
    return 1;
}

int atualizar_severidade(AVLTree *raiz, int id, int nova_sev) {
    if (nova_sev < 1 || nova_sev > 5) return -2;
    struct NO *no = buscar_AVL(raiz, id);
    if (no == NULL) return 0;
    if (no->info.status != ATIVO) return -1;  /* só atualiza ativos */
    no->info.severidade = nova_sev;
    return 1;
}

/* ============================================================
 * Métricas
 * ============================================================ */
int altura_arvore(AVLTree *raiz) {
    if (raiz == NULL || *raiz == NULL) return -1;
    return (*raiz)->alt;
}

static int rec_total_nos(struct NO *no) {
    if (no == NULL) return 0;
    return 1 + rec_total_nos(no->esq) + rec_total_nos(no->dir);
}

int total_nos(AVLTree *raiz) {
    if (raiz == NULL) return 0;
    return rec_total_nos(*raiz);
}

static int rec_total_ativos(struct NO *no) {
    if (no == NULL) return 0;
    int c = (no->info.status == ATIVO) ? 1 : 0;
    return c + rec_total_ativos(no->esq) + rec_total_ativos(no->dir);
}

int total_ativos(AVLTree *raiz) {
    if (raiz == NULL) return 0;
    return rec_total_ativos(*raiz);
}

/* Soma do |FB| de todos os nós, depois divide pelo total. */
static void rec_soma_fb(struct NO *no, long *soma, int *qtd) {
    if (no == NULL) return;
    int fb = fatorBalanceamento_NO(no);
    *soma += (fb < 0) ? -fb : fb;
    (*qtd)++;
    rec_soma_fb(no->esq, soma, qtd);
    rec_soma_fb(no->dir, soma, qtd);
}

double fb_medio(AVLTree *raiz) {
    if (raiz == NULL || *raiz == NULL) return 0.0;
    long soma = 0;
    int qtd = 0;
    rec_soma_fb(*raiz, &soma, &qtd);
    return (qtd == 0) ? 0.0 : (double) soma / qtd;
}

long get_total_rotacoes(void) {
    return total_rotacoes;
}
