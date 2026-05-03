# Sistema de Gerenciamento de Eventos Críticos

Sistema em C que monitora eventos críticos urbanos (acidentes, incêndios, alagamentos, falhas de semáforo, interrupções de energia) usando uma **árvore AVL** como estrutura central. Cada evento tem ID, tipo, severidade (1-5), data/hora, região e status (ATIVO/RESOLVIDO). Permite cadastro, busca, remoção (apenas resolvidos), atualizações, consultas avançadas e métricas da árvore.

## Como rodar

```bash
gcc main.c avl.c consultas.c -o eventos
./eventos
```

No menu, digite **5** para carregar dados de demonstração e depois explore as opções.

## Como testar

```bash
gcc test_harness.c avl.c consultas.c -o testes
./testes
```

Roda automaticamente os 6 cenários de teste (rotações, remoção, atualização, consultas e métricas).