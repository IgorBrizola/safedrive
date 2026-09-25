# Safedrive: processamento de telemetria ADAS

Projeto 1 de Algoritmos e Programação II (FCI Mackenzie, 2026.2).
Enunciado original: [`docs/enunciado-projeto1.pdf`](docs/enunciado-projeto1.pdf).
Explicação da lógica e dos trade-offs: [`docs/EXPLICACAO.md`](docs/EXPLICACAO.md).

## Como compilar e rodar

```bash
make          # ou: gcc -Wall -Wextra -o safedrive safedrive.c
./safedrive
make zip      # gera entrega.zip com a versão comentada
```

Este repositório tem o código **sem comentários**. A versão comentada, com o cabeçalho de identificação do grupo, fica só na máquina local em `comentado/safedrive_comentado.c` (ignorada pelo git) e é ela que vai no zip do Moodle.

## Antes de entregar

- [ ] Preencher **nome completo e matrícula** de cada integrante no comentário do topo de `comentado/safedrive_comentado.c` (sem isso a nota perde 1,0 ponto).
- [ ] Enviar **somente o `.zip` gerado por `make zip`**: nada de executável, `.o` ou pasta de build (também perde 1,0 ponto).
- [ ] Só uma pessoa do grupo envia pelo Moodle.
- Apresentação: 24/09/2026 (02P11) ou 25/09/2026 (02N11 e 02N12). Entrega: **27/09/2026**.

## Levantamento de requisitos

### Restrições técnicas (seção 2)

| # | Requisito | Onde está no código |
|---|-----------|---------------------|
| R1 | Proibido `struct`, `malloc`/`realloc`, ponteiros explícitos e variáveis globais | Todas as matrizes são locais da `main` e vão para as funções como parâmetro |
| R2 | Funções de cálculo sem `scanf`/`printf`, recebendo tudo por parâmetro | `fundir_sensores`, `calcular_distancia_segura`, `analisar_risco_frontal`, `analisar_faixas` e as auxiliares |
| R3 | Só uma função dedicada imprime o relatório; a leitura acontece só na `main` | `exibir_relatorio` (relatório) e `main` (todos os `scanf`) |

### Dados (seção 3)

| # | Requisito | Onde está no código |
|---|-----------|---------------------|
| D1 | Constante `MAX_AMOSTRAS = 100` | `#define MAX_AMOSTRAS 100` |
| D2 | `velocidades[][2]`: 0 = atual, 1 = veículo à frente | `main` |
| D3 | `sensores_frontais[][3]`: 0 = radar, 1 = lidar, 2 = câmera | `main` |
| D4 | `sensores_laterais[][2]`: 0 = faixa esquerda, 1 = faixa direita | `main` |
| D5 | `processamento[][2]`: 0 = distância validada, 1 = distância segura | `main` |
| D6 | `status[][3]` (int): 0 = frontal, 1 = faixa esq., 2 = faixa dir. | `main` |
| D7 | Função que inicializa as matrizes com 50 registros aleatórios | `inicializar_matrizes` |

### Menu (seção 4)

| # | Requisito | Onde está no código |
|---|-----------|---------------------|
| M1 | Ao iniciar, pedir o atrito da via e a sensibilidade (1-Esportivo, 2-Normal, 3-Seguro) | começo da `main`, com validação |
| M2 | Opção 1: carregar os 50 registros iniciais | `inicializar_matrizes` |
| M3 | Opção 2: ler 2 velocidades + 5 sensores e gravar na próxima linha vazia | `main`, grava na linha `n` |
| M4 | Opção 3: rodar todos os cálculos em sequência e depois o relatório | `main`, opção 3 |
| M5 | Opção 4: sair | `main` |

### Regras de processamento (seção 5)

| # | Regra | Onde está no código |
|---|-------|---------------------|
| A | Mediana das 3 leituras frontais vai para `processamento[i][0]` | `fundir_sensores` + `mediana3` |
| B | `d = v·TR + v² / (2·atrito·9.81)`, com v em m/s (km/h ÷ 3.6) e TR = 1.0/1.5/2.0 s | `calcular_distancia_segura` + `tempo_reacao` |
| C | Velocidade relativa ≤ 0 dá 0; senão validada ≥ segura dá 0, ≥ 50% dá 1, < 50% dá 2 | `analisar_risco_frontal` |
| D | Margem de 0,50 m + 0,01 m por km/h acima de 80; abaixo da margem dá 2, abaixo de margem + 0,20 dá 1, senão 0 | `analisar_faixas` + `classificar_faixa` |
| E | Relatório por amostra: entradas, processados, status traduzido, status geral (2 → INTERVENÇÃO CRÍTICA, 1 → ATENÇÃO, 0 → NORMAL) | `exibir_relatorio` + `status_geral` |

## Testes feitos

| Cenário | Resultado esperado | Resultado |
|---------|--------------------|-----------|
| 100 km/h vs 60 km/h, sensores 30/31/80 m, atrito 0,7, sens. 2 | mediana 31 m, segura 97,85 m, AEB acionado | ok |
| Faixas 0,40 m e 1,50 m a 100 km/h (margem 0,70 m) | esquerda PERIGO, direita NORMAL | ok |
| Velocidades iguais (60/60) com obstáculo a 5 m | frontal SEGURO (não há aproximação) | ok |
| 120 km/h, faixa 0,95 m (margem 0,90 m) | ATENÇÃO | ok |
| Letras ou valores fora da faixa no atrito, na sensibilidade e no menu | pede de novo / "Opção inválida" | ok |
| Letra no meio da inserção de amostra | amostra descartada, contador não muda | ok |
| Inserir a 101ª amostra | "Limite de 100 amostras atingido" | ok |
| Fim da entrada (Ctrl+D / EOF) | programa encerra sem travar | ok |
| Relatório sem amostras | mensagem "Nenhuma amostra registrada" | ok |
