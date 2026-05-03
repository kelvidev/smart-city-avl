/* ============================================================
 * main.c — Interface textual do Sistema de Eventos Criticos
 * Menu hierárquico: Cadastros / Consultas / Atualizacoes / Metricas
 * ============================================================
 *
 * Compilação:  gcc main.c avl.c consultas.c -o eventos
 * Execução  :  ./eventos
 * ============================================================ */

#include "evento.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------- Leitura segura --------
 * Lê uma linha inteira e tenta converter para int.
 * Retorna 1 em sucesso e escreve em *out, 0 em erro.
 */
static int ler_int(const char *prompt, int *out) {
    char buf[64];
    printf("%s", prompt);
    if (fgets(buf, sizeof(buf), stdin) == NULL) return 0;
    char *fim;
    long v = strtol(buf, &fim, 10);
    if (fim == buf) return 0;
    *out = (int) v;
    return 1;
}

static int ler_string(const char *prompt, char *out, int tam) {
    printf("%s", prompt);
    if (fgets(out, tam, stdin) == NULL) return 0;
    /* remover \n final */
    int n = strlen(out);
    if (n > 0 && out[n-1] == '\n') out[n-1] = '\0';
    return 1;
}

/* ============================================================
 * Cadastros
 * ============================================================ */
static void menu_cadastrar(AVLTree *raiz) {
    Evento e;
    int op;

    if (!ler_int("ID do evento: ", &e.id) || e.id < 0) {
        printf("ID invalido.\n");
        return;
    }

    printf("Tipo do evento:\n");
    printf("  0) Acidente de Transito\n");
    printf("  1) Falha em Semaforo\n");
    printf("  2) Interrupcao de Energia\n");
    printf("  3) Alagamento\n");
    printf("  4) Incendio\n");
    if (!ler_int("Escolha: ", &op) || op < 0 || op > 4) {
        printf("Tipo invalido.\n");
        return;
    }
    e.tipo = parse_tipo(op);

    if (!ler_int("Severidade (1 a 5): ", &e.severidade) ||
        e.severidade < 1 || e.severidade > 5) {
        printf("Severidade invalida.\n");
        return;
    }

    if (!ler_string("Regiao: ", e.regiao, sizeof(e.regiao)) ||
        strlen(e.regiao) == 0) {
        printf("Regiao invalida.\n");
        return;
    }

    e.registro = data_hora_atual();
    e.status   = ATIVO;

    int r = inserir_AVL(raiz, e);
    if (r == 1)
        printf(">>> Evento %d cadastrado com sucesso.\n", e.id);
    else
        printf(">>> Falha: ID %d ja existe ou erro de alocacao.\n", e.id);
}

static void menu_remover(AVLTree *raiz) {
    int id;
    if (!ler_int("ID a remover: ", &id)) {
        printf("ID invalido.\n");
        return;
    }
    int r = remover_AVL(raiz, id);
    if (r == 1)       printf(">>> Evento %d removido.\n", id);
    else if (r == -1) printf(">>> Erro: evento ainda esta ATIVO. Resolva antes de remover.\n");
    else              printf(">>> Erro: ID %d nao encontrado.\n", id);
}

static void menu_buscar(AVLTree *raiz) {
    int id;
    if (!ler_int("ID a buscar: ", &id)) {
        printf("ID invalido.\n");
        return;
    }
    struct NO *no = buscar_AVL(raiz, id);
    if (no == NULL) printf(">>> ID %d nao encontrado.\n", id);
    else            imprime_evento(no->info);
}

static void submenu_cadastros(AVLTree *raiz) {
    int op;
    do {
        printf("\n--- Cadastros ---\n");
        printf("1) Cadastrar novo evento\n");
        printf("2) Remover evento (somente RESOLVIDOS)\n");
        printf("3) Buscar evento por ID\n");
        printf("0) Voltar\n");
        if (!ler_int("Opcao: ", &op)) op = -1;
        switch (op) {
            case 1: menu_cadastrar(raiz); break;
            case 2: menu_remover(raiz);   break;
            case 3: menu_buscar(raiz);    break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

/* ============================================================
 * Consultas / Relatórios
 * ============================================================ */
static void menu_severidade(AVLTree *raiz) {
    int smin, smax;
    if (!ler_int("Severidade minima (1-5): ", &smin) ||
        !ler_int("Severidade maxima (1-5): ", &smax) ||
        smin < 1 || smax > 5 || smin > smax) {
        printf("Intervalo invalido.\n");
        return;
    }
    listar_por_severidade(raiz, smin, smax);
}

static void menu_regiao(AVLTree *raiz) {
    char regiao[50];
    if (!ler_string("Regiao a consultar: ", regiao, sizeof(regiao)) ||
        strlen(regiao) == 0) {
        printf("Regiao invalida.\n");
        return;
    }
    relatorio_por_regiao(raiz, regiao);
}

static void menu_intervalo(AVLTree *raiz) {
    int a, b;
    if (!ler_int("ID minimo: ", &a) || !ler_int("ID maximo: ", &b) || a > b) {
        printf("Intervalo invalido.\n");
        return;
    }
    buscar_intervalo_id(raiz, a, b);
}

static void submenu_consultas(AVLTree *raiz) {
    int op;
    do {
        printf("\n--- Consultas e Relatorios ---\n");
        printf("1) Listar ATIVOS por intervalo de severidade\n");
        printf("2) Relatorio por regiao (ATIVOS)\n");
        printf("3) Buscar por intervalo de ID\n");
        printf("4) Listar todos (ordem crescente de ID)\n");
        printf("0) Voltar\n");
        if (!ler_int("Opcao: ", &op)) op = -1;
        switch (op) {
            case 1: menu_severidade(raiz);  break;
            case 2: menu_regiao(raiz);       break;
            case 3: menu_intervalo(raiz);    break;
            case 4: listar_em_ordem(raiz);   break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

/* ============================================================
 * Atualizações
 * ============================================================ */
static void menu_resolver(AVLTree *raiz) {
    int id;
    if (!ler_int("ID a resolver: ", &id)) { printf("Invalido.\n"); return; }
    int r = resolver_evento(raiz, id);
    if (r == 1)       printf(">>> Evento %d marcado como RESOLVIDO.\n", id);
    else if (r == -1) printf(">>> Evento %d ja estava resolvido.\n", id);
    else              printf(">>> ID nao encontrado.\n");
}

static void menu_atualizar_sev(AVLTree *raiz) {
    int id, sev;
    if (!ler_int("ID: ", &id) || !ler_int("Nova severidade (1-5): ", &sev)) {
        printf("Invalido.\n");
        return;
    }
    int r = atualizar_severidade(raiz, id, sev);
    if (r == 1)       printf(">>> Severidade atualizada.\n");
    else if (r == 0)  printf(">>> ID nao encontrado.\n");
    else if (r == -1) printf(">>> So eh possivel atualizar eventos ATIVOS.\n");
    else              printf(">>> Severidade fora do intervalo 1-5.\n");
}

static void submenu_atualizacoes(AVLTree *raiz) {
    int op;
    do {
        printf("\n--- Atualizacoes ---\n");
        printf("1) Marcar evento como RESOLVIDO\n");
        printf("2) Atualizar severidade de evento ATIVO\n");
        printf("0) Voltar\n");
        if (!ler_int("Opcao: ", &op)) op = -1;
        switch (op) {
            case 1: menu_resolver(raiz);       break;
            case 2: menu_atualizar_sev(raiz);  break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

/* ============================================================
 * Métricas
 * ============================================================ */
static void mostrar_metricas(AVLTree *raiz) {
    printf("\n--- Metricas da Arvore ---\n");
    printf("Altura total          : %d\n", altura_arvore(raiz));
    printf("Numero total de nos   : %d\n", total_nos(raiz));
    printf("Eventos ativos        : %d\n", total_ativos(raiz));
    printf("Fator de balanc. medio: %.4f\n", fb_medio(raiz));
    printf("Total de rotacoes     : %ld\n", get_total_rotacoes());
}

/* ============================================================
 * Carga de demonstração (ajuda no teste rápido)
 * ============================================================ */
static void carga_demo(AVLTree *raiz) {
    Evento dados[] = {
        {50, ACIDENTE_TRANSITO,    4, {0}, "Centro",     ATIVO},
        {30, FALHA_SEMAFORO,       2, {0}, "Centro",     ATIVO},
        {70, INCENDIO,             5, {0}, "Zona Sul",   ATIVO},
        {20, INTERRUPCAO_ENERGIA,  3, {0}, "Zona Norte", ATIVO},
        {40, ALAGAMENTO,           4, {0}, "Zona Leste", ATIVO},
        {60, ACIDENTE_TRANSITO,    3, {0}, "Zona Sul",   ATIVO},
        {80, FALHA_SEMAFORO,       1, {0}, "Zona Sul",   ATIVO},
        {10, INCENDIO,             5, {0}, "Zona Norte", ATIVO},
        {90, ALAGAMENTO,           2, {0}, "Zona Oeste", ATIVO},
        {25, INTERRUPCAO_ENERGIA,  4, {0}, "Zona Norte", ATIVO},
    };
    int n = sizeof(dados) / sizeof(dados[0]);
    for (int i = 0; i < n; i++) {
        dados[i].registro = data_hora_atual();
        inserir_AVL(raiz, dados[i]);
    }
    printf(">>> Carga de demonstracao: %d eventos inseridos.\n", n);
}

/* ============================================================
 * Loop principal
 * ============================================================ */
int main(void) {
    AVLTree *raiz = cria_AVL();
    if (raiz == NULL) {
        fprintf(stderr, "Erro ao criar arvore.\n");
        return 1;
    }

    int op;
    printf("===========================================\n");
    printf(" Sistema de Gerenciamento de Eventos\n");
    printf(" Cidade Inteligente — Estrutura: AVL\n");
    printf("===========================================\n");

    do {
        printf("\n=== MENU PRINCIPAL ===\n");
        printf("1) Cadastros\n");
        printf("2) Consultas e Relatorios\n");
        printf("3) Atualizacoes\n");
        printf("4) Metricas da Arvore\n");
        printf("5) Carregar dados de demonstracao\n");
        printf("0) Sair\n");
        if (!ler_int("Opcao: ", &op)) op = -1;

        switch (op) {
            case 1: submenu_cadastros(raiz);    break;
            case 2: submenu_consultas(raiz);    break;
            case 3: submenu_atualizacoes(raiz); break;
            case 4: mostrar_metricas(raiz);     break;
            case 5: carga_demo(raiz);           break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);

    libera_AVL(raiz);
    return 0;
}
