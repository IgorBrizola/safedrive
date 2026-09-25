#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_AMOSTRAS 100
#define AMOSTRAS_INICIAIS 50
#define GRAVIDADE 9.81

float aleatorio(float minimo, float maximo)
{
    return minimo + (rand() / (float) RAND_MAX) * (maximo - minimo);
}

float mediana3(float a, float b, float c)
{
    if ((a >= b && a <= c) || (a <= b && a >= c)) {
        return a;
    }
    if ((b >= a && b <= c) || (b <= a && b >= c)) {
        return b;
    }
    return c;
}

float tempo_reacao(int sensibilidade)
{
    if (sensibilidade == 1) {
        return 1.0;
    }
    if (sensibilidade == 2) {
        return 1.5;
    }
    return 2.0;
}

int classificar_faixa(float leitura, float margem)
{
    if (leitura < margem) {
        return 2;
    }
    if (leitura < margem + 0.20) {
        return 1;
    }
    return 0;
}

int status_geral(int status[][3], int i)
{
    int maior = status[i][0];
    int j;

    for (j = 1; j < 3; j++) {
        if (status[i][j] > maior) {
            maior = status[i][j];
        }
    }
    return maior;
}

int inicializar_matrizes(float velocidades[][2], float sensores_frontais[][3],
                         float sensores_laterais[][2])
{
    int i;
    float distancia_real;

    for (i = 0; i < AMOSTRAS_INICIAIS; i++) {
        velocidades[i][0] = aleatorio(30.0, 150.0);
        velocidades[i][1] = aleatorio(20.0, 150.0);

        distancia_real = aleatorio(5.0, 150.0);
        sensores_frontais[i][0] = distancia_real + aleatorio(-2.0, 2.0);
        sensores_frontais[i][1] = distancia_real + aleatorio(-1.0, 1.0);
        sensores_frontais[i][2] = distancia_real + aleatorio(-5.0, 5.0);

        sensores_laterais[i][0] = aleatorio(0.30, 2.00);
        sensores_laterais[i][1] = aleatorio(0.30, 2.00);
    }
    return AMOSTRAS_INICIAIS;
}

void fundir_sensores(float sensores_frontais[][3], float processamento[][2],
                     int n)
{
    int i;

    for (i = 0; i < n; i++) {
        processamento[i][0] = mediana3(sensores_frontais[i][0],
                                       sensores_frontais[i][1],
                                       sensores_frontais[i][2]);
    }
}

void calcular_distancia_segura(float velocidades[][2], float processamento[][2],
                               int n, float atrito, int sensibilidade)
{
    int i;
    float v;
    float tr = tempo_reacao(sensibilidade);

    for (i = 0; i < n; i++) {
        v = velocidades[i][0] / 3.6;
        processamento[i][1] = (v * tr) + (v * v) / (2 * atrito * GRAVIDADE);
    }
}

void analisar_risco_frontal(float velocidades[][2], float processamento[][2],
                            int status[][3], int n)
{
    int i;
    float velocidade_relativa;
    float validada;
    float segura;

    for (i = 0; i < n; i++) {
        velocidade_relativa = velocidades[i][0] - velocidades[i][1];
        validada = processamento[i][0];
        segura = processamento[i][1];

        if (velocidade_relativa <= 0) {
            status[i][0] = 0;
        } else if (validada >= segura) {
            status[i][0] = 0;
        } else if (validada >= 0.5 * segura) {
            status[i][0] = 1;
        } else {
            status[i][0] = 2;
        }
    }
}

void analisar_faixas(float velocidades[][2], float sensores_laterais[][2],
                     int status[][3], int n)
{
    int i;
    float margem;

    for (i = 0; i < n; i++) {
        margem = 0.50;
        if (velocidades[i][0] > 80.0) {
            margem = margem + (velocidades[i][0] - 80.0) * 0.01;
        }
        status[i][1] = classificar_faixa(sensores_laterais[i][0], margem);
        status[i][2] = classificar_faixa(sensores_laterais[i][1], margem);
    }
}

void exibir_relatorio(float velocidades[][2], float sensores_frontais[][3],
                      float sensores_laterais[][2], float processamento[][2],
                      int status[][3], int n, float atrito, int sensibilidade)
{
    int i;
    int j;
    int geral;

    printf("\n==================== RELATÓRIO DE RISCOS ====================\n");
    printf("Atrito da via: %.2f | Sensibilidade: %d | Tempo de reação: %.1f s\n",
           atrito, sensibilidade, tempo_reacao(sensibilidade));

    if (n == 0) {
        printf("\nNenhuma amostra registrada. Use a opção 1 ou 2 do menu.\n");
        return;
    }

    for (i = 0; i < n; i++) {
        printf("\n--------------------- Amostra %3d ---------------------\n", i + 1);

        printf("[Entrada]\n");
        printf("  Velocidade atual ........: %7.2f km/h\n", velocidades[i][0]);
        printf("  Velocidade veículo frente: %7.2f km/h\n", velocidades[i][1]);
        printf("  Radar / Lidar / Câmera ..: %7.2f m | %7.2f m | %7.2f m\n",
               sensores_frontais[i][0], sensores_frontais[i][1],
               sensores_frontais[i][2]);
        printf("  Faixa esquerda / direita : %7.2f m | %7.2f m\n",
               sensores_laterais[i][0], sensores_laterais[i][1]);

        printf("[Processamento]\n");
        printf("  Distância validada ......: %7.2f m\n", processamento[i][0]);
        printf("  Distância segura exigida : %7.2f m\n", processamento[i][1]);

        printf("[Status]\n");
        printf("  Frontal .................: ");
        if (status[i][0] == 0) {
            printf("SEGURO\n");
        } else if (status[i][0] == 1) {
            printf("ATENÇÃO\n");
        } else {
            printf("RISCO DE COLISÃO (AEB ACIONADO)\n");
        }

        for (j = 1; j <= 2; j++) {
            if (j == 1) {
                printf("  Faixa esquerda ..........: ");
            } else {
                printf("  Faixa direita ...........: ");
            }

            if (status[i][j] == 0) {
                printf("NORMAL\n");
            } else if (status[i][j] == 1) {
                printf("ATENÇÃO\n");
            } else {
                printf("PERIGO DE INVASÃO\n");
            }
        }

        geral = status_geral(status, i);
        if (geral == 2) {
            printf(">>> STATUS GERAL: INTERVENÇÃO CRÍTICA EXIGIDA <<<\n");
        } else if (geral == 1) {
            printf("STATUS GERAL: ATENÇÃO\n");
        } else {
            printf("STATUS GERAL: NORMAL\n");
        }
    }
    printf("\n=============================================================\n");
}

int main(void)
{
    float velocidades[MAX_AMOSTRAS][2];
    float sensores_frontais[MAX_AMOSTRAS][3];
    float sensores_laterais[MAX_AMOSTRAS][2];
    float processamento[MAX_AMOSTRAS][2];
    int status[MAX_AMOSTRAS][3];

    int n = 0;
    int opcao = 0;
    int lidos;
    int c;
    float atrito = 0;
    int sensibilidade = 0;

    srand(time(NULL));

    printf("=== SAFEDRIVE - Telemetria ADAS ===\n");

    do {
        printf("Atrito da via (ex.: 0.7 asfalto seco, 0.4 molhado): ");
        lidos = scanf("%f", &atrito);
        if (lidos == EOF) {
            return 0;
        }
        if (lidos != 1) {
            while ((c = getchar()) != '\n' && c != EOF);
            atrito = 0;
        }
        if (atrito <= 0) {
            printf("Valor inválido. O atrito deve ser maior que zero.\n");
        }
    } while (atrito <= 0);

    do {
        printf("Sensibilidade do ADAS (1-Esportivo, 2-Normal, 3-Seguro): ");
        lidos = scanf("%d", &sensibilidade);
        if (lidos == EOF) {
            return 0;
        }
        if (lidos != 1) {
            while ((c = getchar()) != '\n' && c != EOF);
            sensibilidade = 0;
        }
        if (sensibilidade < 1 || sensibilidade > 3) {
            printf("Valor inválido. Escolha 1, 2 ou 3.\n");
        }
    } while (sensibilidade < 1 || sensibilidade > 3);

    do {
        printf("\n----------- MENU -----------\n");
        printf("1. Carregar dados iniciais\n");
        printf("2. Inserir nova amostra\n");
        printf("3. Processar e exibir relatório de riscos\n");
        printf("4. Sair\n");
        printf("Amostras registradas: %d/%d\n", n, MAX_AMOSTRAS);
        printf("Opção: ");

        lidos = scanf("%d", &opcao);
        if (lidos == EOF) {
            opcao = 4;
        } else if (lidos != 1) {
            while ((c = getchar()) != '\n' && c != EOF);
            opcao = 0;
        }

        if (opcao == 1) {
            n = inicializar_matrizes(velocidades, sensores_frontais,
                                     sensores_laterais);
            printf("%d amostras aleatórias carregadas.\n", n);
        } else if (opcao == 2) {
            if (n >= MAX_AMOSTRAS) {
                printf("Limite de %d amostras atingido.\n", MAX_AMOSTRAS);
            } else {
                lidos = 0;
                printf("Velocidade atual (km/h): ");
                lidos = lidos + scanf("%f", &velocidades[n][0]);
                printf("Velocidade do veículo à frente (km/h): ");
                lidos = lidos + scanf("%f", &velocidades[n][1]);
                printf("Radar (m): ");
                lidos = lidos + scanf("%f", &sensores_frontais[n][0]);
                printf("Lidar (m): ");
                lidos = lidos + scanf("%f", &sensores_frontais[n][1]);
                printf("Câmera (m): ");
                lidos = lidos + scanf("%f", &sensores_frontais[n][2]);
                printf("Distância da faixa esquerda (m): ");
                lidos = lidos + scanf("%f", &sensores_laterais[n][0]);
                printf("Distância da faixa direita (m): ");
                lidos = lidos + scanf("%f", &sensores_laterais[n][1]);

                if (lidos == 7) {
                    n++;
                    printf("Amostra %d registrada.\n", n);
                } else {
                    while ((c = getchar()) != '\n' && c != EOF);
                    printf("Entrada inválida. Amostra descartada.\n");
                }
            }
        } else if (opcao == 3) {
            fundir_sensores(sensores_frontais, processamento, n);
            calcular_distancia_segura(velocidades, processamento, n,
                                      atrito, sensibilidade);
            analisar_risco_frontal(velocidades, processamento, status, n);
            analisar_faixas(velocidades, sensores_laterais, status, n);
            exibir_relatorio(velocidades, sensores_frontais, sensores_laterais,
                             processamento, status, n, atrito, sensibilidade);
        } else if (opcao == 4) {
            printf("Encerrando o simulador.\n");
        } else {
            printf("Opção inválida.\n");
        }
    } while (opcao != 4);

    return 0;
}
