// ============================================================================
//         PROJETO WAR ESTRUTURADO - DESAFIO DE CÓDIGO
// ============================================================================
//        
// ============================================================================
//
// OBJETIVOS:
// - Modularizar completamente o código em funções especializadas.
// - Implementar um sistema de missões para um jogador.
// - Criar uma função para verificar se a missão foi cumprida.
// - Utilizar passagem por referência (ponteiros) para modificar dados e
//   passagem por valor/referência constante (const) para apenas ler.
// - Foco em: Design de software, modularização, const correctness, lógica de jogo.
//
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_JOGADORES 6
#define MAX_COR 10
#define MAX_NOME 20
#define MAX_MISSAO 150

// Estrutura do território
typedef struct {
    char nome[30];
    char cor[MAX_COR];
    int tropas;
} Territorio;

// Estrutura do jogador
typedef struct {
    char nome[MAX_NOME];
    char cor[MAX_COR];
    char missao[MAX_MISSAO];
} Jogador;

// Cores disponíveis
const char *CORES_DISPONIVEIS[] = {"vermelha", "azul", "verde", "amarela", "preta", "branca"};
const int TOTAL_CORES = 6;

// Funções
void limparBuffer();
void cadastrarTerritorios(Territorio *mapa, int n, Jogador *jogadores, int numJogadores);
void exibirMapa(const Territorio *mapa, int n, const Jogador *jogadores, int numJogadores);
void atacar(Territorio *atacante, Territorio *defensor);
int escolherTerritorio(const Territorio *mapa, int n, const char *tipo, const char *cor_atacante);
void liberarMemoria(Territorio *mapa, Jogador *jogadores);
void atribuirMissao(char *destino, char *missoes[], int totalMissoes);
int verificarMissao(const char *missao, const Territorio *mapa, int tamanho, const char *corJogador, int numJogadores);
void exibirMissao(const char *missao);
int configurarJogadores(Jogador *jogadores, int *numJogadores);

// ---------------- MAIN ----------------
int main(void)
{
    srand((unsigned)time(NULL));
    int n, numJogadores;
    int idx_atacante, idx_defensor;
    char continuar;

    printf("==== JOGO WAR - MULTIPLAYER ====\n");
    printf("Quantos territorios deseja cadastrar? ");
    if (scanf("%d", &n) != 1 || n < 2) {
        printf("Erro: necessário pelo menos 2 territórios.\n");
        return EXIT_FAILURE;
    }
    limparBuffer();

    // Configurar jogadores
    if (!configurarJogadores(NULL, &numJogadores)) {
        return EXIT_FAILURE;
    }

    Territorio *mapa = malloc(n * sizeof(Territorio));
    Jogador *jogadores = malloc(numJogadores * sizeof(Jogador));
    if (!mapa || !jogadores) {
        printf("Erro de alocação de memória.\n");
        free(mapa); free(jogadores);
        return EXIT_FAILURE;
    }

    if (!configurarJogadores(jogadores, &numJogadores)) {
        liberarMemoria(mapa, jogadores);
        return EXIT_FAILURE;
    }

    cadastrarTerritorios(mapa, n, jogadores, numJogadores);
    exibirMapa(mapa, n, jogadores, numJogadores);

    // Missões disponíveis
    char *missoes[] = {
        "Conquistar 3 territorios seguidos.",
        "Eliminar todas as tropas da cor vermelha.",
        "Controlar pelo menos 2 territorios com mais de 5 tropas.",
        "Conquistar todos os territorios da cor azul.",
        "Manter todos os seus territorios com pelo menos 2 tropas.",
        "Conquistar 4 territorios em qualquer posicao.",
        "Eliminar o jogador com a cor preta."
    };
    int totalMissoes = sizeof(missoes) / sizeof(missoes[0]);

    // Atribuir missões
    for (int i = 0; i < numJogadores; i++) {
        atribuirMissao(jogadores[i].missao, missoes, totalMissoes);
        printf("\nMissão de %s (%s): ", jogadores[i].nome, jogadores[i].cor);
        exibirMissao(jogadores[i].missao);
    }

    printf("\nPressione Enter para começar as batalhas...");
    getchar();

    do {
        printf("\n======= SIMULACAO DE ATAQUE =======\n");
        idx_atacante = escolherTerritorio(mapa, n, "atacante", NULL);
        if (idx_atacante == -1) continue;

        if (mapa[idx_atacante].tropas < 2) {
            printf("Aviso: o atacante precisa de pelo menos 2 tropas.\n");
            continue;
        }

        idx_defensor = escolherTerritorio(mapa, n, "defensor", mapa[idx_atacante].cor);
        if (idx_defensor == -1) continue;

        atacar(&mapa[idx_atacante], &mapa[idx_defensor]);
        exibirMapa(mapa, n, jogadores, numJogadores);

        // Verificar missões
        for (int j = 0; j < numJogadores; j++) {
            if (verificarMissao(jogadores[j].missao, mapa, n, jogadores[j].cor, numJogadores)) {
                printf("\n");
                printf("****************************************************\n");
                printf("*** %s (%s) CUMPRIU SUA MISSAO! ***\n", jogadores[j].nome, jogadores[j].cor);
                printf("*** Missão: %s\n", jogadores[j].missao);
                printf("****************************************************\n");
                liberarMemoria(mapa, jogadores);
                return EXIT_SUCCESS;
            }
        }

        printf("\nDeseja realizar outro ataque? (s/n): ");
        if (scanf(" %c", &continuar) != 1) continuar = 'n';
        continuar = tolower(continuar);
        limparBuffer();
    } while (continuar == 's');

    liberarMemoria(mapa, jogadores);
    printf("\nJogo encerrado. Memória liberada.\n");
    return EXIT_SUCCESS;
}

// -------- FUNÇÕES AUXILIARES --------
void limparBuffer() {
    while (getchar() != '\n');
}

int configurarJogadores(Jogador *jogadores, int *numJogadores) {
    if (jogadores == NULL) {
        // Apenas validação
        printf("Quantos jogadores (2 a 6)? ");
        if (scanf("%d", numJogadores) != 1 || *numJogadores < 2 || *numJogadores > 6) {
            printf("Erro: número de jogadores inválido (2 a 6).\n");
            return 0;
        }
        limparBuffer();
        return 1;
    }

    // Configuração real
    printf("\n--- CADASTRO DOS JOGADORES ---\n");
    char cores_usadas[MAX_JOGADORES][MAX_COR] = {0};

    for (int i = 0; i < *numJogadores; i++) {
        printf("\nJogador %d:\n", i + 1);
        printf(" Nome: ");
        fgets(jogadores[i].nome, MAX_NOME, stdin);
        jogadores[i].nome[strcspn(jogadores[i].nome, "\n")] = '\0';

        // Escolher cor
        while (1) {
            printf(" Escolha a cor (");
            for (int c = 0; c < TOTAL_CORES; c++) {
                int usada = 0;
                for (int u = 0; u < i; u++) {
                    if (strcmp(CORES_DISPONIVEIS[c], cores_usadas[u]) == 0) {
                        usada = 1;
                        break;
                    }
                }
                if (!usada) printf("%s%s", c > 0 ? ", " : "", CORES_DISPONIVEIS[c]);
            }
            printf("): ");
            char entrada[20];
            fgets(entrada, sizeof(entrada), stdin);
            entrada[strcspn(entrada, "\n")] = '\0';

            // Normaliza
            for (int k = 0; entrada[k]; k++) entrada[k] = tolower(entrada[k]);

            int valida = 0;
            for (int c = 0; c < TOTAL_CORES; c++) {
                if (strcmp(entrada, CORES_DISPONIVEIS[c]) == 0) {
                    int ja_usada = 0;
                    for (int u = 0; u < i; u++) {
                        if (strcmp(entrada, cores_usadas[u]) == 0) {
                            ja_usada = 1;
                            break;
                        }
                    }
                    if (!ja_usada) {
                        strcpy(jogadores[i].cor, entrada);
                        strcpy(cores_usadas[i], entrada);
                        valida = 1;
                        break;
                    } else {
                        printf("Erro: cor já usada!\n");
                    }
                }
            }
            if (valida) break;
            printf("Cor inválida. Tente novamente.\n");
        }
    }
    return 1;
}

void cadastrarTerritorios(Territorio *mapa, int n, Jogador *jogadores, int numJogadores)
{
    printf("\n--- CADASTRO DOS TERRITÓRIOS ---\n");
    for (int i = 0; i < n; ++i) {
        printf("\nTerritório %d:\n", i + 1);
        printf(" Nome: ");
        fgets(mapa[i].nome, sizeof(mapa[i].nome), stdin);
        mapa[i].nome[strcspn(mapa[i].nome, "\n")] = '\0';

        // Escolher dono (cor)
        printf(" Dono (cor): ");
        char entrada[20];
        fgets(entrada, sizeof(entrada), stdin);
        entrada[strcspn(entrada, "\n")] = '\0';
        for (int k = 0; entrada[k]; k++) entrada[k] = tolower(entrada[k]);

        int valida = 0;
        for (int j = 0; j < numJogadores; j++) {
            if (strcmp(entrada, jogadores[j].cor) == 0) {
                strcpy(mapa[i].cor, entrada);
                valida = 1;
                break;
            }
        }
        if (!valida) {
            printf("Cor não reconhecida. Usando 'neutro'.\n");
            strcpy(mapa[i].cor, "neutro");
        }

        do {
            printf(" Tropas (mínimo 1): ");
            if (scanf("%d", &mapa[i].tropas) != 1 || mapa[i].tropas < 1) {
                printf("Entrada inválida. Digite um número >= 1.\n");
                limparBuffer();
                mapa[i].tropas = 0;
            } else {
                break;
            }
        } while (1);
        limparBuffer();
    }
}

void exibirMapa(const Territorio *mapa, int n, const Jogador *jogadores, int numJogadores)
{
    printf("\n====== MAPA MUNDIAL ATUALIZADO ======\n");
    for (int i = 0; i < n; ++i) {
        printf("\n[Território %d] %s\n", i + 1, mapa[i].nome);
        printf("  Cor   : %s", mapa[i].cor);
        // Mostra nome do jogador se for dele
        for (int j = 0; j < numJogadores; j++) {
            if (strcmp(mapa[i].cor, jogadores[j].cor) == 0) {
                printf(" (%s)", jogadores[j].nome);
                break;
            }
        }
        printf("\n  Tropas: %d\n", mapa[i].tropas);
    }
    printf("======================================\n");
}

void atacar(Territorio *atacante, Territorio *defensor)
{
    int d_atac = (rand() % 6) + 1;
    int d_def = (rand() % 6) + 1;
    printf("\n--- ROLAGEM DE DADOS ---\n");
    printf("Atacante (%s): %d\n", atacante->nome, d_atac);
    printf("Defensor (%s): %d\n", defensor->nome, d_def);

    if (d_atac > d_def) {
        printf("Resultado: ATAQUE BEM-SUCEDIDO!\n");
        strcpy(defensor->cor, atacante->cor);
        int transferidas = atacante->tropas / 2;
        if (transferidas < 1) transferidas = 1;
        defensor->tropas = transferidas;
        atacante->tropas -= transferidas;
        printf("%s agora pertence ao exército %s (%d tropas).\n",
               defensor->nome, defensor->cor, defensor->tropas);
    } else {
        printf("Resultado: ATAQUE FALHOU!\n");
        atacante->tropas -= 1;
        if (atacante->tropas < 0) atacante->tropas = 0;
        printf("%s perdeu 1 tropa. Restam: %d\n",
               atacante->nome, atacante->tropas);
    }
}

int escolherTerritorio(const Territorio *mapa, int n, const char *tipo, const char *cor_atacante)
{
    int idx;
    while (1) {
        printf("Escolha o território %s (1 a %d): ", tipo, n);
        if (scanf("%d", &idx) != 1 || idx < 1 || idx > n) {
            printf("Erro: número inválido.\n");
            limparBuffer();
            continue;
        }
        idx--;
        limparBuffer();

        if (strcmp(tipo, "defensor") == 0 && cor_atacante &&
            strcmp(mapa[idx].cor, cor_atacante) == 0) {
            printf("Erro: não pode atacar território da mesma cor (%s).\n", cor_atacante);
            continue;
        }
        return idx;
    }
}

void atribuirMissao(char *destino, char *missoes[], int totalMissoes)
{
    int sorteio = rand() % totalMissoes;
    strncpy(destino, missoes[sorteio], MAX_MISSAO - 1);
    destino[MAX_MISSAO - 1] = '\0';
}

void exibirMissao(const char *missao)
{
    printf("%s\n", missao);
}

int verificarMissao(const char *missao, const Territorio *mapa, int tamanho, const char *corJogador, int numJogadores)
{
    // 1. Eliminar cor vermelha
    if (strstr(missao, "vermelha")) {
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, "vermelha") == 0 && mapa[i].tropas > 0)
                return 0;
        return 1;
    }

    // 2. 3 territórios seguidos
    if (strstr(missao, "3 territorios")) {
        int atual = 0, max = 0;
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, corJogador) == 0) {
                atual++;
                if (atual > max) max = atual;
            } else {
                atual = 0;
            }
        }
        return max >= 3;
    }

    // 3. 2 territórios com >5 tropas
    if (strstr(missao, "2 territorios") && strstr(missao, "5 tropas")) {
        int cont = 0;
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, corJogador) == 0 && mapa[i].tropas > 5)
                cont++;
        return cont >= 2;
    }

    // 4. Eliminar cor azul
    if (strstr(missao, "cor azul")) {
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, "azul") == 0 && mapa[i].tropas > 0)
                return 0;
        return 1;
    }

    // 5. Todos os seus territórios com >=2 tropas
    if (strstr(missao, "todos") && strstr(missao, "2 tropas")) {
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, corJogador) == 0 && mapa[i].tropas < 2)
                return 0;
        return 1;
    }

    // 6. Conquistar 4 territórios
    if (strstr(missao, "4 territorios")) {
        int cont = 0;
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, corJogador) == 0)
                cont++;
        return cont >= 4;
    }

    // 7. Eliminar jogador com cor preta
    if (strstr(missao, "cor preta")) {
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, "preta") == 0 && mapa[i].tropas > 0)
                return 0;
        return 1;
    }

    return 0;
}

void liberarMemoria(Territorio *mapa, Jogador *jogadores)
{
    free(jogadores);
    free(mapa);
}