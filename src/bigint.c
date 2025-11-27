#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bigint.h"

// Converte string para BigInt (apenas inteiros positivos)
void bigint_from_string(BigInt *n, const char *str) {
    int len = strlen(str), i;
    n->size = len;
    n->sign = 1;
    for (i = 0; i < len; i++)
        n->digits[i] = str[len - i - 1] - '0';
}

// Converte BigInt para string
void bigint_to_string(const BigInt *n, char *str) {
    int i;
    if (n->sign == -1)
        str[0] = '-';
    for (i = 0; i < n->size; i++)
        str[n->sign == -1 ? i+1 : i] = n->digits[n->size - i - 1] + '0';
    str[(n->sign == -1 ? n->size+1 : n->size)] = '\0';
}

// Soma de BigInts (apenas positivos)
void bigint_add(const BigInt *a, const BigInt *b, BigInt *result) {
    int carry = 0, i;
    result->size = (a->size > b->size ? a->size : b->size);
    for (i = 0; i < result->size; i++) {
        int sum = carry;
        if (i < a->size) sum += a->digits[i];
        if (i < b->size) sum += b->digits[i];
        result->digits[i] = sum % 10;
        carry = sum / 10;
    }
    if (carry) result->digits[result->size++] = carry;
    result->sign = 1;
}

// Subtração de BigInts (a >= b, apenas positivos)
void bigint_sub(const BigInt *a, const BigInt *b, BigInt *result) {
    int borrow = 0, i;
    result->size = a->size;
    for (i = 0; i < a->size; i++) {
        int sub = a->digits[i] - borrow;
        if (i < b->size) sub -= b->digits[i];
        if (sub < 0) {
            sub += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result->digits[i] = sub;
    }
    while (result->size > 1 && result->digits[result->size-1] == 0)
        result->size--;
    result->sign = 1;
}

// Multiplicação de BigInts (apenas positivos)
void bigint_mul(const BigInt *a, const BigInt *b, BigInt *result) {
    int i, j;
    memset(result->digits, 0, sizeof(result->digits));
    for (i = 0; i < a->size; i++) {
        int carry = 0;
        for (j = 0; j < b->size; j++) {
            int mul = a->digits[i] * b->digits[j] + result->digits[i+j] + carry;
            result->digits[i+j] = mul % 10;
            carry = mul / 10;
        }
        if (carry)
            result->digits[i + b->size] += carry;
    }
    result->size = a->size + b->size;
    while (result->size > 1 && result->digits[result->size-1] == 0)
        result->size--;
    result->sign = 1;
}

// Compara |a| e |b|
// Retorna: 1 se a > b, 0 se a == b, -1 se a < b
int bigint_cmp(const BigInt *a, const BigInt *b) {
    if (a->size > b->size) return 1;
    if (a->size < b->size) return -1;

    for (int i = a->size - 1; i >= 0; i--) {
        if (a->digits[i] > b->digits[i]) return 1;
        if (a->digits[i] < b->digits[i]) return -1;
    }
    return 0;
}

// verifica se big int é zero
int bigint_is_zero(const BigInt *n) {
    return (n->size == 1 && n->digits[0] == 0);
}

// copia big int
void bigint_copy(BigInt *dest, const BigInt *src) {
    dest->size = src->size;
    dest->sign = src->sign;
    for (int i = 0; i < src->size; i++)
        dest->digits[i] = src->digits[i];
}


// Divisão inteira e resto de BigInts (apenas positivos e b != 0)
// Implementação simplificada para inteiros pequenos
void bigint_div(const BigInt *a, const BigInt *b, BigInt *quotient, BigInt *remainder) {
    // Para simplificação: converte para int e faz a divisão clássica.
    // Para números grandes reais: implementar algoritmo de divisão para bigints.
    int ai = 0, bi = 0;
    char sa[128], sb[128];
    bigint_to_string(a, sa);
    bigint_to_string(b, sb);
    ai = atoi(sa);
    bi = atoi(sb);
    if (bi == 0) {
        if (quotient) quotient->size = 1, quotient->digits[0] = 0;
        if (remainder) remainder->size = 1, remainder->digits[0] = 0;
        return;
    }
    if (quotient) {
        int q = ai / bi;
        sprintf(sa, "%d", q);
        bigint_from_string(quotient, sa);
    }
    if (remainder) {
        int r = ai % bi;
        sprintf(sa, "%d", r);
        bigint_from_string(remainder, sa);
    }
}