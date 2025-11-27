#include "custom_op.h"
#include "bigint.h"

// Máximo Divisor Comum (BigInt puro, sem conversão para int)
void bigint_gcd(const BigInt *a, const BigInt *b, BigInt *result) {
    BigInt A, B, T;

    // Trabalhar com cópias
    bigint_copy(&A, a);
    bigint_copy(&B, b);

    A.sign = 1;
    B.sign = 1;

    // Casos base
    if (bigint_is_zero(&A)) {
        bigint_copy(result, &B);
        return;
    }
    if (bigint_is_zero(&B)) {
        bigint_copy(result, &A);
        return;
    }

    // Algoritmo de Euclides por subtrações
    while (bigint_cmp(&A, &B) != 0) {
        if (bigint_cmp(&A, &B) > 0) {
            bigint_sub(&A, &B, &T); // A = A - B
            bigint_copy(&A, &T);
        } else {
            bigint_sub(&B, &A, &T); // B = B - A
            bigint_copy(&B, &T);
        }
    }

    // Resultado final
    bigint_copy(result, &A);
    result->sign = 1;
}
