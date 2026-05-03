/* ============================================================
 * test_harness.c — testes automatizados que demonstram os
 * cenários documentados no Relatório Técnico.
 * Compila junto com avl.c e consultas.c.
 * ============================================================ */

#include "evento.h"
#include <stdio.h>

static Evento mk(int id, TipoEvento t, int sev, const char *reg, StatusEvento s) {
    Evento e;
    e.id = id; e.tipo = t; e.severidade = sev;
    e.registro = data_hora_atual();
    snprintf(e.regiao, sizeof(e.regiao), "%s", reg);
    e.status = s;
    return e;
}

static void linha(const char *t) {
    printf("\n========== %s ==========\n", t);
}

int main(void) {
    AVLTree *raiz = cria_AVL();

    /* TESTE 1: insercoes que forcam rotacoes */
    linha("TESTE 1 — Insercoes em ordem crescente (forca rotacoes)");
    inserir_AVL(raiz, mk(10, INCENDIO, 5, "ZN", ATIVO));
    inserir_AVL(raiz, mk(20, ALAGAMENTO, 4, "ZN", ATIVO));
    inserir_AVL(raiz, mk(30, ACIDENTE_TRANSITO, 3, "Centro", ATIVO));  /* gera RR */
    inserir_AVL(raiz, mk(40, FALHA_SEMAFORO, 2, "Centro", ATIVO));
    inserir_AVL(raiz, mk(50, INTERRUPCAO_ENERGIA, 1, "ZS", ATIVO));   /* gera RR */
    inserir_AVL(raiz, mk(25, INCENDIO, 5, "ZN", ATIVO));               /* gera LR/RL */
    printf("Apos insercoes 10,20,30,40,50,25:\n");
    listar_em_ordem(raiz);
    printf("Altura: %d | Nos: %d | Rotacoes: %ld | FB medio: %.4f\n",
           altura_arvore(raiz), total_nos(raiz),
           get_total_rotacoes(), fb_medio(raiz));

    /* TESTE 2: rejeicao de remocao de evento ATIVO */
    linha("TESTE 2 — Tentativa de remover evento ATIVO");
    int r = remover_AVL(raiz, 30);
    printf("remover_AVL(30) com status ATIVO -> retorno = %d (esperado -1)\n", r);

    /* TESTE 3: resolver e remover */
    linha("TESTE 3 — Resolver e remover");
    resolver_evento(raiz, 30);
    r = remover_AVL(raiz, 30);
    printf("Apos resolver(30) e remover(30) -> retorno = %d (esperado 1)\n", r);
    listar_em_ordem(raiz);

    /* TESTE 4: atualizar severidade */
    linha("TESTE 4 — Atualizar severidade");
    int rs = atualizar_severidade(raiz, 25, 2);
    printf("atualizar_severidade(25, 2) = %d\n", rs);
    struct NO *no = buscar_AVL(raiz, 25);
    if (no) printf("Severidade do ID 25 agora: %d\n", no->info.severidade);

    rs = atualizar_severidade(raiz, 99, 3);
    printf("atualizar_severidade(99) [inexistente] = %d (esperado 0)\n", rs);
    rs = atualizar_severidade(raiz, 25, 9);
    printf("atualizar_severidade(25, 9) [fora 1-5] = %d (esperado -2)\n", rs);

    /* TESTE 5: consultas avancadas */
    linha("TESTE 5 — Consultas avancadas");
    /* mais alguns dados pra dar volume */
    inserir_AVL(raiz, mk(15, INCENDIO, 5, "ZN", ATIVO));
    inserir_AVL(raiz, mk(35, ALAGAMENTO, 4, "ZS", ATIVO));
    inserir_AVL(raiz, mk(45, FALHA_SEMAFORO, 1, "Centro", ATIVO));
    inserir_AVL(raiz, mk(55, ACIDENTE_TRANSITO, 5, "ZS", ATIVO));

    listar_por_severidade(raiz, 4, 5);
    relatorio_por_regiao(raiz, "ZN");
    buscar_intervalo_id(raiz, 20, 45);

    /* TESTE 6: metricas finais */
    linha("TESTE 6 — Metricas finais");
    printf("Altura total          : %d\n", altura_arvore(raiz));
    printf("Numero total de nos   : %d\n", total_nos(raiz));
    printf("Eventos ativos        : %d\n", total_ativos(raiz));
    printf("FB medio              : %.4f\n", fb_medio(raiz));
    printf("Total de rotacoes     : %ld\n", get_total_rotacoes());

    libera_AVL(raiz);
    return 0;
}
