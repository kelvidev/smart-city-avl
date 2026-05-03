/* ============================================================
 * avl.c — Implementação da Árvore AVL
 * Estilo do professor: struct NO, info, alt, esq, dir.
 * altura_NO(NULL) == -1.  Rotações LL, RR, LR, RL.
 * ============================================================ */

#include "evento.h"
#include <stdlib.h>

long total_rotacoes = 0;   /* contador global de rotações */

/* ============================================================
 * Criação e liberação
 * ============================================================ */
AVLTree* cria_AVL(void) {
    AVLTree *raiz = (AVLTree*) malloc(sizeof(AVLTree));
    if (raiz != NULL) *raiz = NULL;
    return raiz;
}

static void libera_no(struct NO *no) {
    if (no == NULL) return;
    libera_no(no->esq);
    libera_no(no->dir);
    free(no);
}

void libera_AVL(AVLTree *raiz) {
    if (raiz == NULL) return;
    libera_no(*raiz);
    free(raiz);
}

/* ============================================================
 * Funções auxiliares
 * ============================================================ */

/* altura de um nó. NULL tem altura -1 (convenção do professor) */
int altura_NO(struct NO *no) {
    if (no == NULL) return -1;
    return no->alt;
}

int maior(int a, int b) {
    return (a > b) ? a : b;
}

/* fator de balanceamento = altura(esq) - altura(dir)  */
int fatorBalanceamento_NO(struct NO *no) {
    if (no == NULL) return 0;
    return altura_NO(no->esq) - altura_NO(no->dir);
}

/* ============================================================
 * Rotações
 * Recebem ponteiro pra ponteiro porque alteram a raiz da subárvore.
 * ============================================================ */

/* LL — rotação simples à direita.
 * Aplicada quando o desbalanceamento veio da esquerda da esquerda.
 */
void rotacaoLL(AVLTree *raiz) {
    struct NO *no = (*raiz)->esq;
    (*raiz)->esq = no->dir;
    no->dir = *raiz;

    (*raiz)->alt = maior(altura_NO((*raiz)->esq), altura_NO((*raiz)->dir)) + 1;
    no->alt      = maior(altura_NO(no->esq),       (*raiz)->alt) + 1;

    *raiz = no;
    total_rotacoes++;
}

/* RR — rotação simples à esquerda.
 * Aplicada quando o desbalanceamento veio da direita da direita.
 */
void rotacaoRR(AVLTree *raiz) {
    struct NO *no = (*raiz)->dir;
    (*raiz)->dir = no->esq;
    no->esq = *raiz;

    (*raiz)->alt = maior(altura_NO((*raiz)->esq), altura_NO((*raiz)->dir)) + 1;
    no->alt      = maior(altura_NO(no->dir),       (*raiz)->alt) + 1;

    *raiz = no;
    total_rotacoes++;
}

/* LR — rotação dupla esquerda-direita.
 * Faz RR no filho esquerdo, depois LL na raiz.
 * Conta como 2 rotações (cada simples incrementa).
 */
void rotacaoLR(AVLTree *raiz) {
    rotacaoRR(&(*raiz)->esq);
    rotacaoLL(raiz);
}

/* RL — rotação dupla direita-esquerda.
 * Faz LL no filho direito, depois RR na raiz.
 */
void rotacaoRL(AVLTree *raiz) {
    rotacaoLL(&(*raiz)->dir);
    rotacaoRR(raiz);
}

/* ============================================================
 * Inserção
 * Retorna 1 em sucesso, 0 se ID já existe ou erro de alocação.
 * ============================================================ */
int inserir_AVL(AVLTree *raiz, Evento e) {
    int res;
    if (*raiz == NULL) {
        struct NO *novo = (struct NO*) malloc(sizeof(struct NO));
        if (novo == NULL) return 0;
        novo->info = e;
        novo->alt  = 0;
        novo->esq  = NULL;
        novo->dir  = NULL;
        *raiz = novo;
        return 1;
    }

    struct NO *atual = *raiz;
    if (e.id < atual->info.id) {
        if ((res = inserir_AVL(&atual->esq, e)) == 1) {
            /* corrigir balanceamento */
            if (fatorBalanceamento_NO(atual) >= 2) {
                if (e.id < atual->esq->info.id)
                    rotacaoLL(raiz);
                else
                    rotacaoLR(raiz);
            }
        }
    } else if (e.id > atual->info.id) {
        if ((res = inserir_AVL(&atual->dir, e)) == 1) {
            /* desbalanceou para a direita -> FB <= -2 */
            if (fatorBalanceamento_NO(atual) <= -2) {
                if (e.id > atual->dir->info.id)
                    rotacaoRR(raiz);
                else
                    rotacaoRL(raiz);
            }
        }
    } else {
        /* ID já existe — não insere duplicata */
        return 0;
    }

    /* atualizar altura ao retornar da recursão */
    atual->alt = maior(altura_NO(atual->esq), altura_NO(atual->dir)) + 1;
    return res;
}

/* ============================================================
 * Busca
 * ============================================================ */
struct NO* buscar_AVL(AVLTree *raiz, int id) {
    if (raiz == NULL || *raiz == NULL) return NULL;
    struct NO *atual = *raiz;
    while (atual != NULL) {
        if (id == atual->info.id) return atual;
        if (id < atual->info.id)  atual = atual->esq;
        else                       atual = atual->dir;
    }
    return NULL;
}

/* ============================================================
 * Remoção (apenas eventos com status RESOLVIDO podem ser removidos)
 *
 * Retorno:
 *   1  sucesso
 *   0  ID não encontrado
 *  -1  evento ainda ATIVO (não pode remover)
 * ============================================================ */
static int rebalancea_apos_remocao(AVLTree *raiz) {
    struct NO *atual = *raiz;
    atual->alt = maior(altura_NO(atual->esq), altura_NO(atual->dir)) + 1;

    int fb = fatorBalanceamento_NO(atual);

    /* desbalanceado para a esquerda */
    if (fb >= 2) {
        if (fatorBalanceamento_NO(atual->esq) >= 0)
            rotacaoLL(raiz);
        else
            rotacaoLR(raiz);
    }
    /* desbalanceado para a direita */
    else if (fb <= -2) {
        if (fatorBalanceamento_NO(atual->dir) <= 0)
            rotacaoRR(raiz);
        else
            rotacaoRL(raiz);
    }
    return 1;
}

int remover_AVL(AVLTree *raiz, int id) {
    if (raiz == NULL || *raiz == NULL) return 0;
    struct NO *atual = *raiz;
    int res;

    if (id < atual->info.id) {
        if ((res = remover_AVL(&atual->esq, id)) == 1)
            rebalancea_apos_remocao(raiz);
        return res;
    }
    if (id > atual->info.id) {
        if ((res = remover_AVL(&atual->dir, id)) == 1)
            rebalancea_apos_remocao(raiz);
        return res;
    }

    /* achou o nó alvo */
    if (atual->info.status == ATIVO) {
        /* regra do enunciado: só remove resolvido */
        return -1;
    }

    /* caso 1: folha ou um único filho */
    if (atual->esq == NULL || atual->dir == NULL) {
        struct NO *velho = atual;
        *raiz = (atual->esq != NULL) ? atual->esq : atual->dir;
        free(velho);
        return 1;
    }

    /* caso 2: dois filhos — substituir pelo sucessor (menor da subárvore direita) */
    struct NO *suc = atual->dir;
    while (suc->esq != NULL) suc = suc->esq;

    /* copia info do sucessor para o nó atual.
     * Importante: precisamos remover o sucessor mesmo que ele seja ATIVO,
     * porque a "remoção lógica" é do nó alvo (já validado RESOLVIDO).
     * Truque: marcamos temporariamente o sucessor como RESOLVIDO antes
     * de chamar remover_AVL na subárvore direita.
     */
    atual->info = suc->info;
    /* força remoção física do sucessor pela função recursiva */
    suc->info.status = RESOLVIDO;
    remover_AVL(&atual->dir, suc->info.id);
    rebalancea_apos_remocao(raiz);
    return 1;
}
