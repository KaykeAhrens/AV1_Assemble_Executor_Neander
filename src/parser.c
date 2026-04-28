#include "parser.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>

static const char *entrada = NULL;
static size_t posicao = 0;
static int erro = 0;
static char *mensagem_saida = NULL;
static size_t tamanho_mensagem_saida = 0;

static void marcar_erro(const char *texto) {
    if (erro) {
        return;
    }
    erro = 1;
    snprintf(mensagem_saida, tamanho_mensagem_saida, "%s (posicao %u)", texto, (unsigned)(posicao + 1));
}

static void pular_espacos(void) {
    while (entrada[posicao] && isspace((unsigned char)entrada[posicao])) {
        posicao++;
    }
}

/*
Gramatica EBNF usada no parser:

expressao = termo, { ("+" | "-"), termo } ;
termo     = fator, { ("*" | "/"), fator } ;
fator     = numero | "(", expressao, ")" ;
numero    = digito, { digito } ;
digito    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
*/
static int expressao(int *resultado);

static int numero(int *resultado) {
    long valor = 0;
    int tem_digito = 0;

    pular_espacos();
    while (isdigit((unsigned char)entrada[posicao])) {
        int digito = entrada[posicao] - '0';
        tem_digito = 1;
        valor = (valor * 10) + digito;
        if (valor > INT_MAX) {
            marcar_erro("Numero muito grande");
            return 0;
        }
        posicao++;
    }

    if (!tem_digito) {
        marcar_erro("Numero esperado");
        return 0;
    }

    *resultado = (int)valor;
    return 1;
}

static int fator(int *resultado) {
    pular_espacos();

    if (entrada[posicao] == '(') {
        posicao++;
        if (!expressao(resultado)) {
            return 0;
        }
        pular_espacos();
        if (entrada[posicao] != ')') {
            marcar_erro("')' esperado");
            return 0;
        }
        posicao++;
        return 1;
    }

    return numero(resultado);
}

static int termo(int *resultado) {
    int valor;

    if (!fator(&valor)) {
        return 0;
    }

    while (!erro) {
        char operador;
        int direito;
        pular_espacos();
        operador = entrada[posicao];
        if (operador != '*' && operador != '/') {
            break;
        }
        posicao++;

        if (!fator(&direito)) {
            return 0;
        }

        if (operador == '*') {
            valor = valor * direito;
        } else {
            if (direito == 0) {
                marcar_erro("Divisao por zero");
                return 0;
            }
            valor = valor / direito;
        }
    }

    *resultado = valor;
    return 1;
}

static int expressao(int *resultado) {
    int valor;

    if (!termo(&valor)) {
        return 0;
    }

    while (!erro) {
        char operador;
        int direito;
        pular_espacos();
        operador = entrada[posicao];
        if (operador != '+' && operador != '-') {
            break;
        }
        posicao++;

        if (!termo(&direito)) {
            return 0;
        }

        if (operador == '+') {
            valor = valor + direito;
        } else {
            valor = valor - direito;
        }
    }

    *resultado = valor;
    return 1;
}

int evaluate_expression(const char *expr, int *result, char *error_msg, size_t error_msg_sz) {
    if (!expr || !result || !error_msg || error_msg_sz == 0) {
        return 0;
    }

    entrada = expr;
    posicao = 0;
    erro = 0;
    mensagem_saida = error_msg;
    tamanho_mensagem_saida = error_msg_sz;
    error_msg[0] = '\0';

    if (!expressao(result)) {
        if (!erro) {
            snprintf(error_msg, error_msg_sz, "Expressao invalida");
        }
        return 0;
    }

    pular_espacos();
    if (entrada[posicao] != '\0') {
        marcar_erro("Caractere inesperado");
        return 0;
    }

    return 1;
}
