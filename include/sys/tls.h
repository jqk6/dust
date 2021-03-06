/*
 * Copyright (c) 2018 Amol Surati
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _SYS_TLS_H_
#define _SYS_TLS_H_

#include <stdint.h>

#include <sha2.h>

#define TLS_10				0x301
#define TLS_11				0x302
#define TLS_12				0x303
#define TLS_13				0x304

enum tls_role {
	TLS_SERVER,
	TLS_CLIENT,
};

enum tls_client_state {
	TLSC_START,
	TLSC_WAIT_SH,
	TLSC_WAIT_CCS,	/* Custom state. */
	TLSC_WAIT_EE,
	TLSC_WAIT_CERT_CR,
	TLSC_WAIT_CERT,
	TLSC_WAIT_CV,
	TLSC_WAIT_FIN,
	TLSC_CONN
};

enum tls_rec_type {
	TLS_RT_HAND = 0x16,
	TLS_RT_CCS = 0x14,
	TLS_RT_DATA = 0x17,
};

enum tls_hand_type {
	TLS_HT_CHELLO = 1,
	TLS_HT_SHELLO = 2,
	TLS_HT_ENCEXT = 8,
	TLS_HT_CERT = 11,
	TLS_HT_CV = 15,
	TLS_HT_FIN = 20,
};

struct tls_hand_hw {
	uint8_t type;
	uint8_t lenhi;
	uint16_t lenlo;
} __attribute__((packed));

struct tls_rec_hw {
	uint8_t type;
	uint16_t ver;
	uint16_t len;
} __attribute__((packed));

struct tls_encext_hw {
	uint16_t exts_len;
} __attribute__((packed));

struct tls_chello_hw {
	uint16_t ver;
	uint8_t rnd[32];
	uint8_t sess_len;
	uint16_t cipher_len;	/* Our client sends only 1 cipher. */
	uint16_t cipher;
	uint8_t comp_len;
	uint8_t comp;		/* Implies comp_len must be set to 1. */
	uint16_t exts_len;
} __attribute__((packed));

struct tls_shello_hw {
	uint16_t ver;
	uint8_t rnd[32];
	uint8_t sess_len;
	uint16_t cipher;
	uint8_t comp;
	uint16_t exts_len;
} __attribute__((packed));

struct tls_ext_hw {
	uint16_t type;
	uint16_t len;
} __attribute__((packed));

/* key share entry. */
struct tls_kse_hw {
	uint16_t group;
	uint16_t klen;
} __attribute__((packed));





/* Do not use any pointers in the sw structures. */
struct tls_rec_sw {
	struct tls_rec_hw *rec;
	union {
		struct tls_hand_hw *hand;
		uint8_t *data;
	} u1;
	union {
		struct tls_chello_hw *chello;
		struct tls_shello_hw *shello;
		struct tls_encext_hw *encext;
	} u2;
	struct tls_ext_hw *exts;
};

struct tls_secrets {
	/*
	 * The sizes of these four secrets depend on the negotiated ECDHE
	 * group. The group is fixed to X25519.
	 */
	uint8_t *priv, *pub[2], *shared;
	/* pub[0] is mine, pub[1] is theirs. */

	/*
	 * These secrets depend on the negotiated Hash for their lengths.
	 * The hash is fixed to SHA256.
	 */
	uint8_t early[SHA256_DIGEST_LEN];
	uint8_t hand[SHA256_DIGEST_LEN];
	uint8_t master[SHA256_DIGEST_LEN];

	/* [0] is TLS_CLIENT, [1] is TLS_SERVER. */
	uint8_t hand_traffic[2][SHA256_DIGEST_LEN];
	uint8_t app_traffic[2][SHA256_DIGEST_LEN];

	/* Write traffic keys/IVs. */
	uint8_t hand_traffic_key[2][32];	/* size by AEAD. */
	uint8_t hand_traffic_iv[2][12];		/* size by AEAD. */
	uint8_t app_traffic_key[2][32];		/* size by AEAD. */
	uint8_t app_traffic_iv[2][12];		/* size by AEAD. */
};

struct tls_transcript {
	struct sha256_ctx hctx;	/* To save intermediate contexts. */

	/* The sizes depend on the negotiated Hash. Fixed to SHA256. */
	uint8_t empty[SHA256_DIGEST_LEN];
	uint8_t chello[SHA256_DIGEST_LEN];	/* CH */
	uint8_t shello[SHA256_DIGEST_LEN];	/* CH,SH */
	uint8_t sfin[SHA256_DIGEST_LEN];	/* CH,SH,...,SF */
	uint8_t cfin[SHA256_DIGEST_LEN];	/* CH,SH,...,SF,...,CF */
};

struct tls_ctx {
	struct tls_secrets secrets;
	struct tls_transcript transcript;
	enum tls_client_state client_state;
	enum tls_role role;
	uint64_t seq;
};
#endif
