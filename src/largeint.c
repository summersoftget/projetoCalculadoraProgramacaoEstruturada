#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "largeint.h"

void li_from_string(LargeInt *n, const char *str) {
    int len = strlen(str);
    int start = 0;

    // zera todos os dígitos (segurança)
    for (int i = 0; i < MAX_DIGITS; i++) n->digits[i] = 0;

    n->sign = 1;
    if (str[0] == '-') {
        n->sign = -1;
        start = 1;
        len--;
    }

    if (len == 0) { // string vazia ou apenas '-'
        n->size = 1;
        n->digits[0] = 0;
        n->sign = 1;
        return;
    }

    // remover zeros à esquerda (ex.: "000123")
    const char *p = str + start;
    while (*p == '0' && (len > 1)) {
        p++; len--;
    }

    n->size = len;
    for (int i = 0; i < len; i++)
        n->digits[i] = p[(len - 1 - i)] - '0';

    // se o valor for 0, force sign = +1 e size = 1
    if (n->size == 1 && n->digits[0] == 0) {
        n->sign = 1;
    }
}


void li_to_string(const LargeInt *n, char *str) {
    int idx = 0;

    if (n->sign == -1)
        str[idx++] = '-';

    for (int i = n->size - 1; i >= 0; i--)
        str[idx++] = n->digits[i] + '0';

    str[idx] = '\0';
}

void li_copy(LargeInt *dest, const LargeInt *src) {
    dest->size = src->size;
    dest->sign = src->sign;
    for (int i = 0; i < src->size; i++)
        dest->digits[i] = src->digits[i];
}

int li_is_zero(const LargeInt *n) {
    return n->size == 1 && n->digits[0] == 0;
}

int li_cmp(const LargeInt *a, const LargeInt *b) {
    if (a->size > b->size) return 1;
    if (a->size < b->size) return -1;

    for (int i = a->size - 1; i >= 0; i--) {
        if (a->digits[i] > b->digits[i]) return 1;
        if (a->digits[i] < b->digits[i]) return -1;
    }
    return 0;
}
void li_add(const LargeInt *a, const LargeInt *b, LargeInt *result) {

    // Zera o resultado ANTES de começar (para evitar lixo)
    for (int i = 0; i < MAX_DIGITS; i++)
        result->digits[i] = 0;

    // Caso 1: sinais iguais -> soma normal
    if (a->sign == b->sign) {
        int carry = 0;
        int max = (a->size > b->size ? a->size : b->size);

        for (int i = 0; i < max; i++) {
            int sum = carry;
            if (i < a->size) sum += a->digits[i];
            if (i < b->size) sum += b->digits[i];

            result->digits[i] = sum % 10;
            carry = sum / 10;
        }

        result->size = max;
        if (carry)
            result->digits[result->size++] = carry;

        result->sign = a->sign;  // mantém sinal
        return;
    }

    // Caso 2: sinais diferentes -> vira subtração
    if (a->sign == 1 && b->sign == -1) {
        // A + (-B) = A - B
        LargeInt tb = *b;
        tb.sign = 1;
        li_sub(a, &tb, result);
        return;
    }

    if (a->sign == -1 && b->sign == 1) {
        // (-A) + B = B - A
        LargeInt ta = *a;
        ta.sign = 1;
        li_sub(b, &ta, result);
        return;
    }
}

void li_sub(const LargeInt *a, const LargeInt *b, LargeInt *result) {
    // Result will store |a - b| and sign will be set accordingly.
    // We assume a and b are non-negative for this raw operation (use callers to
    // ensure that when needed). li_cmp works on magnitude only, so OK.

    // Zera resultado para evitar lixo
    for (int i = 0; i < MAX_DIGITS; i++) result->digits[i] = 0;

    // Se a >= b => result = a - b, sign = +1
    // Se a <  b => result = b - a, sign = -1
    const LargeInt *larger = a;
    const LargeInt *smaller = b;
    int result_sign = 1;

    if (li_cmp(a, b) < 0) {
        larger = b;
        smaller = a;
        result_sign = -1;
    }

    int borrow = 0;
    result->size = larger->size;

    for (int i = 0; i < larger->size; i++) {
        int lv = larger->digits[i];
        int sv = (i < smaller->size) ? smaller->digits[i] : 0;

        int value = lv - borrow - sv;
        if (value < 0) {
            value += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result->digits[i] = value;
    }

    // remove zeros à esquerda
    while (result->size > 1 && result->digits[result->size - 1] == 0)
        result->size--;

    result->sign = result_sign;

    // se o valor for zero, force sign positivo
    if (result->size == 1 && result->digits[0] == 0)
        result->sign = 1;
}

void li_mul(const LargeInt *a, const LargeInt *b, LargeInt *result) {
    memset(result->digits, 0, sizeof(result->digits));

    for (int i = 0; i < a->size; i++) {
        int carry = 0;
        for (int j = 0; j < b->size; j++) {
            int sum = result->digits[i + j] + a->digits[i] * b->digits[j] + carry;
            result->digits[i + j] = sum % 10;
            carry = sum / 10;
        }
        if (carry)
            result->digits[i + b->size] += carry;
    }

    result->size = a->size + b->size;
    while (result->size > 1 && result->digits[result->size - 1] == 0)
        result->size--;

    result->sign = a->sign * b->sign;
}

// Multiplica n por um inteiro pequeno (0..9). Usa base 10 por dígito.
static void li_mul_small(const LargeInt *n, int m, LargeInt *out) {
    // assumimos 0 <= m <= 9
    memset(out->digits, 0, sizeof(out->digits));
    if (m == 0 || (n->size == 1 && n->digits[0] == 0)) {
        out->size = 1;
        out->digits[0] = 0;
        out->sign = 1;
        return;
    }

    int carry = 0;
    for (int i = 0; i < n->size; i++) {
        int prod = n->digits[i] * m + carry;
        out->digits[i] = prod % 10;
        carry = prod / 10;
    }
    int pos = n->size;
    while (carry) {
        if (pos >= MAX_DIGITS) { // proteção
            break;
        }
        out->digits[pos++] = carry % 10;
        carry /= 10;
    }
    out->size = pos;
    while (out->size > 1 && out->digits[out->size - 1] == 0) out->size--;
    out->sign = n->sign * (m == 0 ? 1 : 1); // magnitude apenas; sinal gerenciado fora
}

// Versão corrigida e segura de li_divmod
int li_divmod(const LargeInt *a, const LargeInt *b, LargeInt *q, LargeInt *r) {
    if (li_is_zero(b)) return -1; // divisão por zero

    // Cópias locais (vamos operar com magnitude positiva)
    LargeInt dividend, divisor;
    li_copy(&dividend, a);
    li_copy(&divisor, b);

    int sign_a = dividend.sign;
    int sign_b = divisor.sign;

    // operar com magnitude (forces positive)
    dividend.sign = 1;
    divisor.sign = 1;

    // inicializa q e r
    for (int i = 0; i < MAX_DIGITS; i++) q->digits[i] = 0;
    q->size = 0;
    q->sign = (sign_a * sign_b); // sinal do quociente (ajustaremos se zero)

    r->size = 1;
    for (int i = 0; i < MAX_DIGITS; i++) r->digits[i] = 0;
    r->digits[0] = 0;
    r->sign = 1;

    // laço: processa dígitos do dividend do mais-significativo para o menos
    for (int i = dividend.size - 1; i >= 0; i--) {
        // shift left de r (multiplica por 10) -> movemos todos os dígitos para cima
        if (r->size + 1 > MAX_DIGITS) return -2; // overflow de buffer
        for (int j = r->size; j > 0; j--) r->digits[j] = r->digits[j - 1];
        r->digits[0] = dividend.digits[i];
        r->size++;
        while (r->size > 1 && r->digits[r->size - 1] == 0) r->size--;

        // buscar o dígito do quociente (0..9) usando busca binária
        int low = 0, high = 9, best = 0;
        while (low <= high) {
            int mid = (low + high) / 2;
            LargeInt prod;
            li_mul_small(&divisor, mid, &prod); // prod = divisor * mid
            int cmp = li_cmp(&prod, r); // compara magnitudes
            if (cmp <= 0) { // prod <= r
                best = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        // subtrai best * divisor de r
        if (best > 0) {
            LargeInt prod;
            li_mul_small(&divisor, best, &prod);
            LargeInt tmp;
            li_sub(r, &prod, &tmp); // r = r - prod
            li_copy(r, &tmp);
        }
        // acrescenta dígito ao quociente (em ordem MSB-first)
        if (q->size >= MAX_DIGITS) return -2;
        q->digits[q->size++] = best;
    }

    // Agora q->digits[0] é MSB — precisamos inverter para convenção LSB-first
    if (q->size == 0) {
        // quociente zero
        q->size = 1;
        q->digits[0] = 0;
        q->sign = 1;
    } else {
        for (int i = 0; i < q->size / 2; i++) {
            int tmp = q->digits[i];
            q->digits[i] = q->digits[q->size - 1 - i];
            q->digits[q->size - 1 - i] = tmp;
        }
        // normalizar zeros ao topo
        while (q->size > 1 && q->digits[q->size - 1] == 0) q->size--;
        if (q->size == 1 && q->digits[0] == 0) q->sign = 1; // quociente zero deve ser positivo
    }

    
    // normalizar r
    while (r->size > 1 && r->digits[r->size - 1] == 0) r->size--;
    if (li_is_zero(r)) {
        r->size = 1;
        r->digits[0] = 0;
        r->sign = 1;
    } else {
        r->sign = 1; // escolha: resto não-negativo (mude para sign_a se preferir)
    }

    // Ajuste euclidiano final: garante a = b*q + r com 0 <= r < |b|
if (sign_a < 0 && !li_is_zero(r)) {
    LargeInt one;
    li_from_string(&one, "1");

    // q = q - sign(b)
    if (sign_b > 0) {
        li_sub(q, &one, q);
    } else {
        li_add(q, &one, q);
    }

    // r = |b| - r
    LargeInt absb;
    li_copy(&absb, &divisor); // divisor já é |b|
    LargeInt newr;
    li_sub(&absb, r, &newr);
    li_copy(r, &newr);
}
    
    return 0;
}


int li_div(const LargeInt *a, const LargeInt *b, LargeInt *q) {
    LargeInt r;
    return li_divmod(a, b, q, &r);
}

int li_mod(const LargeInt *a, const LargeInt *b, LargeInt *r) {
    LargeInt q;
    return li_divmod(a, b, &q, r);
}
