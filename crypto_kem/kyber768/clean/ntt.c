#include "ntt.h"
#include "inttypes.h"
#include "params.h"
#include "reduce.h"

extern const uint16_t PQCLEAN_KYBER768_omegas_inv_bitrev_montgomery[];
extern const uint16_t PQCLEAN_KYBER768_psis_inv_montgomery[];
extern const uint16_t PQCLEAN_KYBER768_zetas[];

/*************************************************
 * Name:        ntt
 *
 * Description: Computes negacyclic number-theoretic transform (NTT) of
 *              a polynomial (vector of 256 coefficients) in place;
 *              inputs assumed to be in normal order, output in bitreversed
 *order
 *
 * Arguments:   - uint16_t *p: pointer to in/output polynomial
 **************************************************/
void PQCLEAN_KYBER768_ntt(uint16_t *p) {
    int level, start, j, k;
    uint16_t zeta, t;

    k = 1;
    for (level = 7; level >= 0; level--) {
        for (start = 0; start < KYBER_N; start = j + (1 << level)) {
            zeta = PQCLEAN_KYBER768_zetas[k++];
            for (j = start; j < start + (1 << level); ++j) {
                t = PQCLEAN_KYBER768_montgomery_reduce((uint32_t)zeta *
                                                       p[j + (1 << level)]);

                p[j + (1 << level)] =
                    PQCLEAN_KYBER768_barrett_reduce(p[j] + 4 * KYBER_Q - t);

                if (level & 1) {     /* odd level */
                    p[j] = p[j] + t; /* Omit reduction (be lazy) */
                } else {
                    p[j] = PQCLEAN_KYBER768_barrett_reduce(p[j] + t);
                }
            }
        }
    }
}

/*************************************************
 * Name:        invntt
 *
 * Description: Computes inverse of negacyclic number-theoretic transform (NTT)
 *of a polynomial (vector of 256 coefficients) in place; inputs assumed to be in
 *bitreversed order, output in normal order
 *
 * Arguments:   - uint16_t *a: pointer to in/output polynomial
 **************************************************/
void PQCLEAN_KYBER768_invntt(uint16_t *a) {
    int start, j, jTwiddle, level;
    uint16_t temp, W;
    uint32_t t;

    for (level = 0; level < 8; level++) {
        for (start = 0; start < (1 << level); start++) {
            jTwiddle = 0;
            for (j = start; j < KYBER_N - 1; j += 2 * (1 << level)) {
                W = PQCLEAN_KYBER768_omegas_inv_bitrev_montgomery[jTwiddle++];
                temp = a[j];

                if (level & 1) { /* odd level */
                    a[j] = PQCLEAN_KYBER768_barrett_reduce(
                        (temp + a[j + (1 << level)]));
                } else {
                    a[j] = (temp +
                            a[j + (1 << level)]); /* Omit reduction (be lazy) */
                }

                t = (W * ((uint32_t)temp + 4 * KYBER_Q - a[j + (1 << level)]));

                a[j + (1 << level)] = PQCLEAN_KYBER768_montgomery_reduce(t);
            }
        }
    }

    for (j = 0; j < KYBER_N; j++) {
        a[j] = PQCLEAN_KYBER768_montgomery_reduce(
            (a[j] * PQCLEAN_KYBER768_psis_inv_montgomery[j]));
    }
}
