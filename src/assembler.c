#include "assembler.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINHAS 1000
#define MAX_SIMBOLOS 256
#define MAX_TOKEN 64
#define MAX_LINHA_TEXTO 256

typedef struct {
    char nome[MAX_TOKEN];
    uint8_t endereco;
} Simbolo;

typedef struct {
    char original[MAX_LINHA_TEXTO];
    char rotulo[MAX_TOKEN];
    char instrucao[MAX_TOKEN];
    char operando[MAX_TOKEN];
    int numero_linha;
} Linha;

Simbolo tabela_simbolos[MAX_SIMBOLOS];
int num_simbolos = 0;

Linha linhas[MAX_LINHAS];
int num_linhas = 0;

uint8_t memoria_montada[256];
int endereco_atual = 0;
uint8_t origem_programa = 0;
int origem_definida = 0;

void trim(char *str) {
    size_t len;
    while (*str && isspace((unsigned char)*str)) {
        memmove(str, str + 1, strlen(str));
    }
    len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

void limpar_string(char *str) {
    char *comentario = strchr(str, ';');
    if (comentario) {
        *comentario = '\0';
    }
    trim(str);
}

int eh_numero(const char *str) {
    int i = 0;
    if (!str || !*str) {
        return 0;
    }
    if (str[0] == '#') {
        str++;
    }
    if (str[0] == '$') {
        str++;
        if (!*str) return 0;
        while (str[i]) {
            if (!isxdigit((unsigned char)str[i])) return 0;
            i++;
        }
        return 1;
    }
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        i = 2;
        if (!str[i]) return 0;
        while (str[i]) {
            if (!isxdigit((unsigned char)str[i])) return 0;
            i++;
        }
        return 1;
    }
    while (str[i]) {
        if (!isdigit((unsigned char)str[i])) return 0;
        i++;
    }
    return 1;
}

int converter_numero(const char *str) {
    long v;
    if (!str || !*str) {
        return -1;
    }
    if (str[0] == '#') {
        str++;
    }
    if (str[0] == '$') {
        v = strtol(str + 1, NULL, 16);
    } else if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        v = strtol(str, NULL, 16);
    } else {
        v = strtol(str, NULL, 10);
    }
    if (v < 0 || v > 255) {
        return -1;
    }
    return (int)v;
}

int eh_rotulo_valido(const char *nome) {
    int i;
    if (!nome || !nome[0]) {
        return 0;
    }
    if (!isalpha((unsigned char)nome[0]) && nome[0] != '_') {
        return 0;
    }
    for (i = 1; nome[i]; i++) {
        if (!isalnum((unsigned char)nome[i]) && nome[i] != '_') {
            return 0;
        }
    }
    return 1;
}

void to_upper_str(char *dst, const char *src, size_t dst_sz) {
    size_t i = 0;
    if (dst_sz == 0) {
        return;
    }
    while (src[i] && i < dst_sz - 1) {
        dst[i] = (char)toupper((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
}

int equals_ignore_case(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] && b[i]) {
        if (toupper((unsigned char)a[i]) != toupper((unsigned char)b[i])) {
            return 0;
        }
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';
}

uint8_t obter_opcode(const char *mnemonico) {
    char m[MAX_TOKEN];
    to_upper_str(m, mnemonico, sizeof(m));
    if (strcmp(m, "NOP") == 0) return 0x00;
    if (strcmp(m, "STA") == 0) return 0x10;
    if (strcmp(m, "LDA") == 0) return 0x20;
    if (strcmp(m, "ADD") == 0) return 0x30;
    if (strcmp(m, "OR") == 0) return 0x40;
    if (strcmp(m, "AND") == 0) return 0x50;
    if (strcmp(m, "NOT") == 0) return 0x70;
    if (strcmp(m, "JMP") == 0) return 0x80;
    if (strcmp(m, "JN") == 0) return 0x90;
    if (strcmp(m, "JZ") == 0) return 0xA0;
    if (strcmp(m, "HLT") == 0) return 0xF0;
    return 0xFF;
}

int instrucao_tem_operando(uint8_t opcode) {
    return !(opcode == 0x00 || opcode == 0x70 || opcode == 0xF0);
}

int buscar_simbolo(const char *nome) {
    int i;
    for (i = 0; i < num_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].nome, nome) == 0) {
            return tabela_simbolos[i].endereco;
        }
    }
    return -1;
}

int adicionar_simbolo(const char *nome, uint8_t endereco) {
    if (num_simbolos >= MAX_SIMBOLOS) return 0;
    if (buscar_simbolo(nome) != -1) return 0;
    strncpy(tabela_simbolos[num_simbolos].nome, nome, MAX_TOKEN - 1);
    tabela_simbolos[num_simbolos].nome[MAX_TOKEN - 1] = '\0';
    tabela_simbolos[num_simbolos].endereco = endereco;
    num_simbolos++;
    return 1;
}

void parsear_linha(const char *linha_original, Linha *linha) {
    char copia[MAX_LINHA_TEXTO];
    char *token;
    int len;

    strncpy(linha->original, linha_original, sizeof(linha->original) - 1);
    linha->original[sizeof(linha->original) - 1] = '\0';
    limpar_string(linha->original);

    linha->rotulo[0] = '\0';
    linha->instrucao[0] = '\0';
    linha->operando[0] = '\0';

    if (strlen(linha->original) == 0) return;

    strncpy(copia, linha->original, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';
    token = strtok(copia, " \t\r\n");
    if (!token) return;

    len = (int)strlen(token);
    if (len > 0 && token[len - 1] == ':') {
        token[len - 1] = '\0';
        to_upper_str(linha->rotulo, token, sizeof(linha->rotulo));
        token = strtok(NULL, " \t\r\n");
    }

    if (token) {
        to_upper_str(linha->instrucao, token, sizeof(linha->instrucao));
        token = strtok(NULL, " \t\r\n");
        if (token) {
            to_upper_str(linha->operando, token, sizeof(linha->operando));
        }
    }
}

int ler_arquivo_asm(const char *asm_path, char *error_msg, size_t error_msg_sz) {
    FILE *f = fopen(asm_path, "r");
    char buffer[MAX_LINHA_TEXTO];
    int numero_linha = 1;

    if (!f) {
        snprintf(error_msg, error_msg_sz, "Não foi possível abrir %s", asm_path);
        return 0;
    }

    num_linhas = 0;
    while (fgets(buffer, sizeof(buffer), f) && num_linhas < MAX_LINHAS) {
        linhas[num_linhas].numero_linha = numero_linha++;
        parsear_linha(buffer, &linhas[num_linhas]);
        /* guarda so linha util pra simplificar as passagens */
        if (strlen(linhas[num_linhas].instrucao) > 0 || strlen(linhas[num_linhas].rotulo) > 0) {
            num_linhas++;
        }
    }
    fclose(f);
    return 1;
}

int resolver_operando(const char *token, uint8_t *valor) {
    int n;
    int endereco;
    if (eh_numero(token)) {
        n = converter_numero(token);
        if (n < 0) return 0;
        *valor = (uint8_t)n;
        return 1;
    }
    endereco = buscar_simbolo(token);
    if (endereco < 0) {
        return 0;
    }
    *valor = (uint8_t)endereco;
    return 1;
}

int primeira_passagem(char *error_msg, size_t error_msg_sz) {
    int i;
    num_simbolos = 0;
    endereco_atual = 0;
    origem_programa = 0;
    origem_definida = 0;

    for (i = 0; i < num_linhas; i++) {
        Linha *l = &linhas[i];
        int valor;
        uint8_t opcode;

        if (strlen(l->rotulo) > 0) {
            if (!eh_rotulo_valido(l->rotulo)) {
                snprintf(error_msg, error_msg_sz, "Linha %d: rótulo inválido", l->numero_linha);
                return 0;
            }
            if (!adicionar_simbolo(l->rotulo, (uint8_t)endereco_atual)) {
                snprintf(error_msg, error_msg_sz, "Linha %d: rótulo duplicado (%s)", l->numero_linha, l->rotulo);
                return 0;
            }
        }

        if (strlen(l->instrucao) == 0) continue;

        if (equals_ignore_case(l->instrucao, "ORG")) {
            if (!eh_numero(l->operando)) {
                snprintf(error_msg, error_msg_sz, "Linha %d: ORG inválido", l->numero_linha);
                return 0;
            }
            valor = converter_numero(l->operando);
            if (valor < 0) {
                snprintf(error_msg, error_msg_sz, "Linha %d: ORG inválido", l->numero_linha);
                return 0;
            }
            endereco_atual = valor;
            if (!origem_definida) {
                /* primeiro ORG define de onde o programa comeca */
                origem_programa = (uint8_t)valor;
                origem_definida = 1;
            }
        } else if (equals_ignore_case(l->instrucao, "DATA")) {
            if (!eh_numero(l->operando) && strlen(l->operando) > 0 && !eh_rotulo_valido(l->operando)) {
                snprintf(error_msg, error_msg_sz, "Linha %d: DATA inválido", l->numero_linha);
                return 0;
            }
            endereco_atual += 1;
        } else if (equals_ignore_case(l->instrucao, "SPACE")) {
            if (!eh_numero(l->operando)) {
                snprintf(error_msg, error_msg_sz, "Linha %d: SPACE inválido", l->numero_linha);
                return 0;
            }
            valor = converter_numero(l->operando);
            if (valor < 0) {
                snprintf(error_msg, error_msg_sz, "Linha %d: SPACE inválido", l->numero_linha);
                return 0;
            }
            endereco_atual += valor;
        } else {
            /* aqui e so calcular tamanho, sem gerar byte ainda */
            opcode = obter_opcode(l->instrucao);
            if (opcode == 0xFF) {
                snprintf(error_msg, error_msg_sz, "Linha %d: instrução inválida (%s)", l->numero_linha, l->instrucao);
                return 0;
            }
            endereco_atual += instrucao_tem_operando(opcode) ? 2 : 1;
        }

        if (endereco_atual > 256) {
            snprintf(error_msg, error_msg_sz, "Linha %d: programa excede 256 bytes", l->numero_linha);
            return 0;
        }
    }

    if (!origem_definida) {
        origem_programa = 0x00;
    }
    return 1;
}

int segunda_passagem(char *error_msg, size_t error_msg_sz) {
    int i;
    endereco_atual = origem_programa;
    memset(memoria_montada, 0, sizeof(memoria_montada));

    for (i = 0; i < num_linhas; i++) {
        Linha *l = &linhas[i];
        int valor;
        uint8_t opcode;
        uint8_t operando;

        if (strlen(l->instrucao) == 0) continue;

        if (equals_ignore_case(l->instrucao, "ORG")) {
            valor = converter_numero(l->operando);
            if (valor < 0) {
                snprintf(error_msg, error_msg_sz, "Linha %d: ORG inválido", l->numero_linha);
                return 0;
            }
            endereco_atual = valor;
            continue;
        }

        if (equals_ignore_case(l->instrucao, "DATA")) {
            if (strlen(l->operando) == 0) {
                memoria_montada[(uint8_t)endereco_atual++] = 0;
            } else {
                if (!resolver_operando(l->operando, &operando)) {
                    snprintf(error_msg, error_msg_sz, "Linha %d: DATA inválido", l->numero_linha);
                    return 0;
                }
                memoria_montada[(uint8_t)endereco_atual++] = operando;
            }
            continue;
        }

        if (equals_ignore_case(l->instrucao, "SPACE")) {
            valor = converter_numero(l->operando);
            if (valor < 0) {
                snprintf(error_msg, error_msg_sz, "Linha %d: SPACE inválido", l->numero_linha);
                return 0;
            }
            endereco_atual += valor;
            continue;
        }

        opcode = obter_opcode(l->instrucao);
        if (opcode == 0xFF) {
            snprintf(error_msg, error_msg_sz, "Linha %d: instrução inválida (%s)", l->numero_linha, l->instrucao);
            return 0;
        }

        memoria_montada[(uint8_t)endereco_atual++] = opcode;
        if (instrucao_tem_operando(opcode)) {
            /* operando pode ser numero direto ou rotulo da tabela */
            if (strlen(l->operando) == 0 || !resolver_operando(l->operando, &operando)) {
                snprintf(error_msg, error_msg_sz, "Linha %d: operando inválido para %s", l->numero_linha, l->instrucao);
                return 0;
            }
            memoria_montada[(uint8_t)endereco_atual++] = operando;
        }
    }
    return 1;
}

int escrever_arquivo_mem(const char *path, uint8_t origin, const uint8_t memory[256]) {
    FILE *f = fopen(path, "w");
    int i;
    if (!f) return 0;
    fprintf(f, "ORG %02X\n", origin);
    for (i = 0; i < 256; i++) {
        fprintf(f, "%02X\n", memory[i]);
    }
    fclose(f);
    return 1;
}

int assemble_to_memory(const char *asm_path, uint8_t *origin, uint8_t memory[256], char *error_msg, size_t error_msg_sz) {
    if (!ler_arquivo_asm(asm_path, error_msg, error_msg_sz)) return 0;
    if (!primeira_passagem(error_msg, error_msg_sz)) return 0;
    if (!segunda_passagem(error_msg, error_msg_sz)) return 0;
    *origin = origem_programa;
    memcpy(memory, memoria_montada, 256);
    return 1;
}

int assemble_file(const char *asm_path, const char *mem_path, char *error_msg, size_t error_msg_sz) {
    uint8_t origem;
    uint8_t memory[256];
    if (!assemble_to_memory(asm_path, &origem, memory, error_msg, error_msg_sz)) {
        return 0;
    }
    if (!escrever_arquivo_mem(mem_path, origem, memory)) {
        snprintf(error_msg, error_msg_sz, "Erro ao escrever arquivo %s", mem_path);
        return 0;
    }
    return 1;
}

int load_mem_file(const char *path, uint8_t *origin, uint8_t memory[256], char *error_msg, size_t error_msg_sz) {
    FILE *f = fopen(path, "r");
    char line[MAX_LINHA_TEXTO];
    int i = 0;

    if (!f) {
        snprintf(error_msg, error_msg_sz, "Não foi possível abrir %s", path);
        return 0;
    }
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        snprintf(error_msg, error_msg_sz, "Arquivo .mem vazio: %s", path);
        return 0;
    }
    trim(line);
    if (sscanf(line, "ORG %hhx", origin) != 1) {
        fclose(f);
        snprintf(error_msg, error_msg_sz, "Formato inválido de ORG em %s", path);
        return 0;
    }
    while (i < 256 && fgets(line, sizeof(line), f)) {
        unsigned int v;
        trim(line);
        if (line[0] == '\0') continue;
        if (sscanf(line, "%x", &v) != 1 || v > 0xFFu) {
            fclose(f);
            snprintf(error_msg, error_msg_sz, "Byte inválido no .mem em %s", path);
            return 0;
        }
        memory[i++] = (uint8_t)v;
    }
    fclose(f);

    if (i != 256) {
        snprintf(error_msg, error_msg_sz, "Arquivo .mem incompleto (esperado 256 bytes): %s", path);
        return 0;
    }
    return 1;
}
