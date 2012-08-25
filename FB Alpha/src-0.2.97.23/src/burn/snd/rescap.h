#ifndef RC_MACROS_H
#define RC_MACROS_H

/* Little helpers for magnitude conversions */
#define RES_R(res) ((double)(res))
#define RES_K(res) ((double)(res) * 1e3)
#define RES_M(res) ((double)(res) * 1e6)
#define CAP_U(cap) ((double)(cap) * 1e-6)
#define CAP_N(cap) ((double)(cap) * 1e-9)
#define CAP_P(cap) ((double)(cap) * 1e-12)

/*  vin --/\r1/\-- out --/\r2/\-- gnd  */
#define RES_VOLTAGE_DIVIDER(r1, r2)		((double)(r2) / ((double)(r1) + (double)(r2)))

#define RES_2_PARALLEL(r1, r2)			(((r1) * (r2)) / ((r1) + (r2)))
#define RES_3_PARALLEL(r1, r2, r3)		(1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3)))
#define RES_4_PARALLEL(r1, r2, r3, r4)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4)))
#define RES_5_PARALLEL(r1, r2, r3, r4, r5)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5)))
#define RES_6_PARALLEL(r1, r2, r3, r4, r5, r6)  (1.0 / (1.0 / (r1) + 1.0 / (r2) + 1.0 / (r3) + 1.0 / (r4) + 1.0 / (r5) + 1.0 / (r6)))

#define RES_2_SERIAL(r1,r2)			    ((r1)+(r2))

#endif
