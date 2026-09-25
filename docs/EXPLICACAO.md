# Como o Safedrive funciona (explicação para a apresentação)

## A ideia geral em uma frase

Cada **linha** das matrizes é um **instante de tempo** (uma amostra). Cada **coluna** é um tipo de dado. A linha 7 de `velocidades`, de `sensores_frontais`, de `processamento` e de `status` fala sempre do **mesmo instante**. É esse "alinhamento por linha" que substitui a `struct` que não podemos usar.

```
            velocidades        sensores_frontais         sensores_laterais   processamento        status
linha i ->  [atual, frente]    [radar, lidar, camera]    [esq, dir]          [validada, segura]   [frontal, esq, dir]
            ^---------- entrada (main lê / aleatório) ----------^            ^--- calculado pelas funções ---^
```

## O caminho de um dado

```
main (lê teclado)  ->  matrizes de entrada
                              |
     opção 3: fundir_sensores -> calcular_distancia_segura -> analisar_risco_frontal -> analisar_faixas
                              |
                       exibir_relatorio (única que imprime o relatório)
```

1. **Entrada.** A `main` é a única que usa `scanf`. Ela pergunta o atrito e a sensibilidade e depois mostra o menu.
2. **Processamento.** As funções de cálculo **não leem nem imprimem nada**. Recebem as matrizes por parâmetro e escrevem o resultado em `processamento` e `status`.
3. **Saída.** `exibir_relatorio` só **lê** as matrizes e traduz os números em texto.

### E como as funções "devolvem" matrizes sem ponteiro?

Em C, quando você passa uma matriz para uma função (`float processamento[][2]`), a função mexe **na mesma matriz** da `main`, não numa cópia. Por isso `fundir_sensores` consegue gravar em `processamento` sem `return` e sem declarar ponteiro nenhum.

O contador `n` (quantas amostras existem) é um `int` simples. Esse **é copiado** quando vai para uma função. Por isso quem altera `n` é sempre a `main`:
- na opção 1, `n = inicializar_matrizes(...)`: a função **retorna** 50;
- na opção 2, a `main` faz `n++` depois de ler a amostra.

## Cada regra, passo a passo

### A) Fusão de sensores: mediana

Três sensores medem a mesma distância, mas cada um tem ruído. Se um deles "enlouquecer" (ex.: a câmera diz 80 m e os outros dizem 30 e 31), a **média** seria puxada para cima (47 m). A **mediana** ignora o valor estranho e fica com 31 m. Por isso o enunciado pede a mediana.

`mediana3(a, b, c)` testa: "a está entre b e c?". Se sim, é ele. Senão testa o b. Se nenhum dos dois, é o c.

### B) Distância segura

```
v  = velocidade_atual / 3.6            (km/h -> m/s)
d  = v * tempo_reacao  +  v² / (2 * atrito * 9.81)
     \_ anda antes de frear _/   \______ distância freando ______/
```

- `tempo_reacao(sens)` devolve 1.0, 1.5 ou 2.0 s.
- Atrito menor (pista molhada) significa denominador menor, então a distância fica maior.
- Exemplo: 100 km/h, atrito 0,7, sensibilidade 2 dá 27,78·1,5 + 771,6/13,73 = **97,85 m**.

### C) Risco frontal (AEB)

```
se (vel_atual - vel_frente) <= 0  -> 0 SEGURO   (não estou me aproximando)
senão se validada >= segura       -> 0 SEGURO
senão se validada >= segura / 2   -> 1 ATENÇÃO
senão                             -> 2 RISCO DE COLISÃO (AEB)
```

A primeira pergunta vem antes de tudo: mesmo com o carro da frente perto, se ele está mais rápido a distância só aumenta.

### D) Assistente de faixa dinâmico

```
margem = 0.50
se velocidade > 80:  margem += (velocidade - 80) * 0.01     (100 km/h -> 0.70 m)

leitura < margem          -> 2 PERIGO DE INVASÃO
leitura < margem + 0.20   -> 1 ATENÇÃO
senão                     -> 0 NORMAL
```

A mesma função `classificar_faixa` é usada para a esquerda (coluna 1) e para a direita (coluna 2). A regra é uma só, então o código também é um só.

### E) Relatório e status geral

`status_geral` pega o **maior** código da linha (0, 1 ou 2):
- se algum é 2, o maior é 2: **INTERVENÇÃO CRÍTICA EXIGIDA**;
- se não tem 2 mas tem 1, o maior é 1: **ATENÇÃO**;
- tudo 0 dá **NORMAL**.

"Pegar o maior" já resolve as três regras do enunciado de uma vez.

## Trade-offs (as escolhas que fizemos e por quê)

| Escolha | Por que fizemos assim | O que "perdemos" |
|---------|----------------------|------------------|
| **Um único arquivo `.c`** | Entrega simples (o zip leva só `.c`), fácil de ler de cima a baixo | Em projeto grande, separaríamos em `.h` / `.c` |
| **`float` em vez de `double`** | `scanf("%f")` é mais simples e evita o erro clássico de esquecer `%lf` | Menos casas de precisão (irrelevante para metros e km/h) |
| **`#define` para constantes** (`MAX_AMOSTRAS`, `GRAVIDADE`) | Constante de pré-processador não é variável global, então respeita a restrição | Nenhum |
| **Matriz fixa de 100 linhas** | Sem `malloc` é o único jeito; o enunciado fixa 100 | Memória reservada mesmo com poucas amostras (e um limite rígido) |
| **Opção 1 sobrescreve tudo** (volta para 50 amostras) | O enunciado diz "inicializar"; recomeçar do zero é o comportamento mais previsível | Amostras digitadas antes são perdidas |
| **Opção 3 recalcula todas as amostras** | Simples e sempre consistente, porque uma amostra nova já entra no próximo relatório | Refaz contas já feitas (custo desprezível para 100 linhas) |
| **Mediana por comparações** em vez de ordenar | 3 valores cabem em 2 `if`s; não precisa de algoritmo de ordenação | Não serve para N sensores (aí ordenaríamos) |
| **Status geral = maior código** | Uma regra cobre os 3 casos do enunciado | Nenhum; é equivalente às 3 regras |
| **Validação de entrada** (atrito > 0, sensibilidade 1-3, letras no menu, EOF) | Projeto que trava leva **nota zero**; atrito 0 causaria divisão por zero | Um pouco mais de código na `main` |
| **Amostra só é "confirmada" (`n++`) se as 7 leituras deram certo** | Evita uma linha pela metade nas matrizes | Se errar um campo, tem que digitar a amostra inteira de novo |
| **Não validamos valores negativos nas amostras** | Mantém a `main` curta; o enunciado não pede | Dá para digitar velocidade negativa (o cálculo roda, só não faz sentido físico) |
| **Dados aleatórios "realistas"** (3 sensores = distância real + ruído) | Mostra a mediana funcionando de verdade na demonstração | Nenhum |
| **`printf` também na `main`** (menu e perguntas) | Sem imprimir o menu não dá para usar o programa; os cálculos continuam sem nenhum `printf` | Leitura muito literal da regra "só uma função imprime". Vale confirmar com o professor |
| **Acentos nas mensagens** (ATENÇÃO, INTERVENÇÃO) | O enunciado usa os textos com acento | No terminal do Windows (Dev-C++/CMD) os acentos podem sair "quebrados"; no Linux/VS Code saem certos |

## Perguntas prováveis na apresentação

- **"Por que a mediana e não a média?"** Porque a mediana ignora um sensor com leitura absurda (ver regra A).
- **"Onde vocês usam ponteiro?"** Em lugar nenhum. Matrizes passadas como parâmetro já são compartilhadas com a `main`, e o `&` do `scanf` é o uso padrão da biblioteca, não um ponteiro declarado.
- **"Como a função sabe quantas linhas processar?"** Recebe `n` por parâmetro e faz `for (i = 0; i < n; i++)`.
- **"O que acontece com 101 amostras?"** A `main` checa `n >= MAX_AMOSTRAS` antes de ler e avisa que o limite foi atingido.
- **"Por que o carro na mesma velocidade é SEGURO mesmo a 5 m?"** Velocidade relativa 0 quer dizer que a distância não diminui, e é o que a regra C manda.
