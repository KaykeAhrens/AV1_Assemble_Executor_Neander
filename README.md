# Avaliação 1 - Assembler e Executor para a arquitetura Neander

Implementação em C de um assembler em duas passagens e de um simulador (executor) para a máquina hipotética Neander.

## Requisitos

- Sistema operacional: Windows, Linux ou macOS
- Compilador C com suporte a C11 (ex.: GCC)

## Dependências

Não há bibliotecas externas. O projeto usa apenas a biblioteca padrão da linguagem C.

## Como compilar

No diretório do projeto:

```bash
make
```

Obs.: no Windows, se o comando `make` não estiver disponível, use a compilação manual com `gcc` mostrada abaixo.

Comando manual equivalente (Windows):

```bash
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude src/main.c src/assembler.c src/executor.c -o neander_cli.exe
```

Comando manual equivalente (Linux/macOS):

```bash
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude src/main.c src/assembler.c src/executor.c -o neander_cli
```

Para limpar o binário gerado:

```bash
make clean
```

## Estrutura do projeto

```text
include/
  assembler.h
  executor.h
  parser.h
src/
  assembler.c
  executor.c
  main.c
  parser.c
```

## Modos de uso

O executável possui três modos:

- `assemble`: traduz `.asm` para `.mem`
- `run`: executa um arquivo `.mem`
- `asmrun`: monta e executa diretamente a partir de `.asm`
- `parse`: avalia expressão matemática básica

## Comandos principais (resumo rápido)

```bash
./neander_cli.exe assemble "entrada.asm" "saida.mem"
./neander_cli.exe run "saida.mem" --dec
./neander_cli.exe asmrun "entrada.asm" --hex --step
./neander_cli.exe parse "2 + 3 * (4 + 1)"
```

### 1) Montar assembly para memória (com arquivo real do projeto)

```bash
./neander_cli.exe assemble "exemplo_soma.asm" "saida.mem"
```

### 2) Executar arquivo .mem

```bash
./neander_cli.exe run "saida.mem" --dec
./neander_cli.exe run "saida.mem" --hex
./neander_cli.exe run "saida.mem" --hex --step
```

### 3) Montar e executar direto do .asm (com arquivos reais)

```bash
./neander_cli.exe asmrun "exemplo_soma.asm" --dec
./neander_cli.exe asmrun "exemplo_loop_jz.asm" --hex --step
```

### 4) Separando montagem e execução (fluxo recomendado)

```bash
./neander_cli.exe assemble "exemplo_soma.asm" "saida.mem"
./neander_cli.exe run "saida.mem" --dec
```

### 5) Usando um arquivo próprio

Se você renomear os arquivos ou criar um novo `.asm`, basta trocar o nome no comando:

```bash
./neander_cli.exe assemble "meu_programa.asm" "saida.mem"
./neander_cli.exe run "saida.mem" --dec
```

### 6) Testar o parser de expressões

```bash
./neander_cli.exe parse "2 + 3 * 4"
./neander_cli.exe parse "(10 + 5) / 3"
```

Operadores suportados: `+`, `-`, `*`, `/` e parênteses `(` `)`.
Implementação simples em C por descida recursiva (`fator`, `termo`, `expressao`), sem estruturas complexas.

## Formato do arquivo de saída (.mem)

O assembler gera:

1. Primeira linha com `ORG XX` (origem em hexadecimal)
2. Em seguida, 256 linhas com 1 byte por linha em hexadecimal (`00` a `FF`)

Esse formato é lido diretamente pelo modo `run`.

## Conjunto de instruções suportadas

- `NOP`
- `STA <end>`
- `LDA <end>`
- `ADD <end>`
- `OR <end>`
- `AND <end>`
- `NOT`
- `JMP <end>`
- `JN <end>`
- `JZ <end>`
- `HLT`

## Diretivas de assembler suportadas

- `ORG <endereço>`
- `[ROTULO:] DATA <valor>`
- `[ROTULO:] SPACE <tamanho>`

## Parser de expressões (EBNF)

```ebnf
expressao = termo, { ("+" | "-"), termo } ;
termo     = fator, { ("*" | "/"), fator } ;
fator     = numero | "(", expressao, ")" ;
numero    = digito, { digito } ;
digito    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
```

## Saída da execução

Ao executar, o simulador apresenta:

- memória antes da execução (endereços não nulos)
- estado final dos registradores (`AC`, `PC`, `IR`, `MAR`, `MDR`)
- flags `N` e `Z`
- quantidade de acessos à memória
- quantidade de instruções executadas
- memória depois da execução (endereços não nulos)

No modo `--step`, o estado de registradores é exibido a cada instrução.

## Programas de exemplo

O repositório contém dois exemplos:

- `exemplo_soma.asm`: cálculo simples com `LDA`, `ADD`, `STA`
- `exemplo_loop_jz.asm`: laço com salto condicional usando `JZ` e `JMP`
