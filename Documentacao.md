# Relatório Técnico - Avaliação 1 - Assembler e Executor para a arquitetura Neander

## 1. Introdução

Este trabalho apresenta o desenvolvimento de um parser de expressões matemáticas, de um assembler básico e de um executor (simulador) para a máquina hipotética Neander, conforme proposta da Avaliação 1 da disciplina de Compiladores. A solução foi implementada na linguagem C (padrão C11), contemplando análise de expressões, tradução de assembly para código de máquina e execução do programa em memória simulada de 8 bits.

## 2. Parser de Expressões

Foi implementado um parser simples por descida recursiva para expressões matemáticas com precedência de operadores e suporte a parênteses.
O código foi mantido propositalmente direto: uso de funções pequenas e variáveis globais de controle (`entrada`, `posicao`, `erro`), evitando estruturas mais complexas para manter leitura didática.

### 2.1 Gramática EBNF

```ebnf
expressao = termo, { ("+" | "-"), termo } ;
termo     = fator, { ("*" | "/"), fator } ;
fator     = numero | "(", expressao, ")" ;
numero    = digito, { digito } ;
digito    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
```

### 2.2 Comportamento

- operadores suportados: `+`, `-`, `*`, `/`;
- precedência correta (`*` e `/` acima de `+` e `-`);
- parênteses para agrupamento;
- tratamento de erro para expressão inválida, caractere inesperado e divisão por zero.

### 2.3 Comando da CLI

O parser pode ser testado diretamente pelo executável:

```bash
./neander_cli.exe parse "2 + 3 * 4"
./neander_cli.exe parse "(10 + 5) / 3"
```

## 3. Estrutura do Assembler

O assembler foi implementado em duas passagens, conforme recomendado no enunciado.

### 3.1 Primeira passagem

Na primeira passagem, o código-fonte assembly é percorrido linha a linha com os seguintes objetivos:

- remoção de comentários e normalização de tokens;
- identificação de rótulos e validação de sintaxe;
- construção da tabela de símbolos (rótulo -> endereço);
- processamento das diretivas `ORG`, `DATA` e `SPACE` para avançar o contador de endereço;
- detecção de erros, como rótulo duplicado, diretiva inválida ou excedente de memória (limite de 256 bytes).

Essa etapa não gera bytes finais de saída; ela prepara a informação necessária para resolver referências simbólicas.

### 3.2 Segunda passagem

Na segunda passagem, o arquivo assembly é lido novamente para geração efetiva do código de máquina:

- mnemônicos são convertidos para opcodes;
- operandos são resolvidos como valor imediato (numérico) ou endereço simbólico (via tabela de símbolos);
- diretivas `DATA` e `SPACE` são emitidas na memória simulada;
- o resultado final e exportado em arquivo `.mem`.

## 4. Tabela de Símbolos

A tabela de símbolos armazena pares `(nome, endereço)` para cada rótulo declarado no programa. Sua utilização principal ocorre na segunda passagem, quando instruções e diretivas referenciam nomes simbólicos em vez de valores numéricos literais.

Com essa abordagem, o assembler desacopla declaração e uso de rótulos, permitindo referências futuras e mantendo consistência de endereçamento.

## 5. Arquitetura do Executor

O executor simula os componentes essenciais da Neander:

- Memória de 256 bytes (`0x00` a `0xFF`);
- registradores `AC`, `PC`, `IR`, `MAR`, `MDR` (8 bits);
- flags de estado `N` (negativo) e `Z` (zero).

Também foram incluídos contadores de desempenho:

- número de acessos à memória;
- número de instruções executadas.

## 6. Ciclo de Máquina (Fetch, Decode, Execute)

O ciclo de execução segue o fluxo clássico de processadores simples:

1. **Fetch (Busca)**: `MAR <- PC`, leitura de memória para `MDR`, transferência para `IR`.
2. **Decode (Decodificação)**: interpretação do opcode contido em `IR`.
3. **Execute (Execução)**: realização da operação correspondente, com atualização de registradores, memória e flags.

As instruções com operando realizam leitura adicional do byte seguinte para obter endereço/valor. As instruções de salto (`JMP`, `JN`, `JZ`) alteram diretamente o `PC` conforme condição.

## 7. Manipulação de Flags e Aritmética de 8 bits

A implementação considera aritmética modular de 8 bits para operações no acumulador, como exigido para a Neander. Em seguida, as flags são recalculadas:

- `Z = 1` quando `AC == 0`;
- `N = 1` quando o bit mais significativo de `AC` é `1` (interpretação em complemento de dois).

Isso garante o comportamento esperado para instruções condicionais dependentes de estado (`JN` e `JZ`).

## 8. Interface e Controle de Execução

Foram implementadas duas formas de execução:

- **contínua**: executa até `HLT`;
- **passo a passo (`--step`)**: exibe estado dos registradores a cada instrução e aguarda confirmação do usuário.

Também há seleção de formato de exibição:

- decimal (`--dec`);
- hexadecimal (`--hex`).

## 9. Conclusão

O projeto atende ao objetivo de integrar análise sintática de expressões, montagem e simulação para a arquitetura Neander, com tradução de código assembly, resolução simbólica, geração de arquivo de memória e execução fiel ao ciclo de máquina. A estrutura simples do parser, as duas passagens do assembler e o controle explícito de registradores e flags tornam a solução adequada para fins didáticos de arquitetura e compiladores.
