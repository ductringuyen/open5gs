/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ogs-crypt.h"

#define MAX_NUM_OF_KDF_PARAM                    16

#define FC_FOR_CK_PRIME_IK_PRIME_DERIVATION     0x20
#define FC_FOR_5GS_ALGORITHM_KEY_DERIVATION     0x69
#define FC_FOR_KAUSF_DERIVATION                 0x6A
#define FC_FOR_RES_STAR_XRES_STAR_DERIVATION    0x6B
#define FC_FOR_KSEAF_DERIVATION                 0x6C
#define FC_FOR_KAMF_DERIVATION                  0x6D
#define FC_FOR_KGNB_KN3IWF_DERIVATION           0x6E
#define FC_FOR_KGNB_NH_DERIVATION               0x6F

#define FC_FOR_KENB_DERIVATION                  0x11
#define FC_FOR_KENB_NH_DERIVATION               0x12
#define FC_FOR_EPS_ALGORITHM_KEY_DERIVATION     0x15

typedef struct kdf_param_s {
    uint8_t *buf;
    uint16_t len;
} kdf_param_t[MAX_NUM_OF_KDF_PARAM];

/* KDF function : TS.33220 cluase B.2.0 */
static void ogs_kdf_common(uint8_t *key, uint32_t key_size,
        uint8_t fc, kdf_param_t param, uint8_t *output)
{
    int i = 0, pos;
    uint8_t *s = NULL;

    ogs_assert(key);
    ogs_assert(key_size);
    ogs_assert(fc);
    ogs_assert(param[0].buf);
    ogs_assert(param[0].len);
    ogs_assert(output);

    pos = 1; /* FC Value */

    /* Calculate buffer length */
    for (i = 0; i < MAX_NUM_OF_KDF_PARAM && param[i].buf && param[i].len; i++) {
        pos += (param[i].len + 2);
    }

    s = ogs_calloc(1, pos);
    ogs_assert(s);

    /* Copy buffer from param */
    pos = 0;
    s[pos++] = fc;
    for (i = 0; i < MAX_NUM_OF_KDF_PARAM && param[i].buf && param[i].len; i++) {
        uint16_t len;

        memcpy(&s[pos], param[i].buf, param[i].len);
        pos += param[i].len;
        len = htobe16(param[i].len);
        memcpy(&s[pos], &len, sizeof(len));
        pos += 2;
    }

    ogs_hmac_sha256(key, key_size, s, pos, output, OGS_SHA256_DIGEST_SIZE);

    ogs_free(s);
}

/* TS33.501 Annex A.2 : Kausf derviation function */
void ogs_kdf_kausf(
        uint8_t *ck, uint8_t *ik,
        char *serving_network_name, uint8_t *autn,
        uint8_t *kausf)
{
    kdf_param_t param;
    uint8_t key[OGS_KEY_LEN*2];

    ogs_assert(ck);
    ogs_assert(ik);
    ogs_assert(serving_network_name);
    ogs_assert(autn);
    ogs_assert(kausf);

    memcpy(key, ck, OGS_KEY_LEN);
    memcpy(key+OGS_KEY_LEN, ik, OGS_KEY_LEN);

    memset(param, 0, sizeof(param));
    param[0].buf = (uint8_t *)serving_network_name;
    param[0].len = strlen(serving_network_name);
    param[1].buf = autn;
    param[1].len = OGS_SQN_XOR_AK_LEN;

    ogs_kdf_common(key, OGS_KEY_LEN*2,
            FC_FOR_KAUSF_DERIVATION, param, kausf);
}

/* TS33.501 Annex A.4 : RES* and XRES* derivation function */
void ogs_kdf_xres_star(
        uint8_t *ck, uint8_t *ik,
        char *serving_network_name, uint8_t *rand,
        uint8_t *xres, size_t xres_len,
        uint8_t *xres_star)
{
    kdf_param_t param;
    uint8_t key[OGS_KEY_LEN*2];
    uint8_t output[OGS_SHA256_DIGEST_SIZE];

    ogs_assert(ck);
    ogs_assert(ik);
    ogs_assert(serving_network_name);
    ogs_assert(rand);
    ogs_assert(xres);
    ogs_assert(xres_len);

    memcpy(key, ck, OGS_KEY_LEN);
    memcpy(key+OGS_KEY_LEN, ik, OGS_KEY_LEN);

    memset(param, 0, sizeof(param));
    param[0].buf = (uint8_t *)serving_network_name;
    param[0].len = strlen(serving_network_name);
    param[1].buf = rand;
    param[1].len = OGS_RAND_LEN;
    param[2].buf = xres;
    param[2].len = xres_len;

    ogs_kdf_common(key, OGS_KEY_LEN*2,
            FC_FOR_RES_STAR_XRES_STAR_DERIVATION, param, output);

    memcpy(xres_star, output+OGS_KEY_LEN, OGS_KEY_LEN);
}

/* TS33.501 Annex A.5 : HRES* and HXRES* derivation function */
void ogs_kdf_hxres_star(uint8_t *rand, uint8_t *xres_star, uint8_t *hxres_star)
{
    uint8_t message[OGS_RAND_LEN + OGS_KEY_LEN];
    uint8_t output[OGS_SHA256_DIGEST_SIZE];

    ogs_assert(rand);
    ogs_assert(xres_star);
    ogs_assert(hxres_star);

    memcpy(message, rand, OGS_RAND_LEN);
    memcpy(message+OGS_RAND_LEN, xres_star, OGS_KEY_LEN);

    ogs_sha256(message, OGS_RAND_LEN+OGS_KEY_LEN, output);

    memcpy(hxres_star, output+OGS_KEY_LEN, OGS_KEY_LEN);
}

/* TS33.501 Annex A.6 : Kseaf derivation function */
void ogs_kdf_kseaf(char *serving_network_name, uint8_t *kausf, uint8_t *kseaf)
{
    kdf_param_t param;

    ogs_assert(serving_network_name);
    ogs_assert(kausf);
    ogs_assert(kseaf);

    memset(param, 0, sizeof(param));
    param[0].buf = (uint8_t *)serving_network_name;
    param[0].len = strlen(serving_network_name);

    ogs_kdf_common(kausf, OGS_SHA256_DIGEST_SIZE,
            FC_FOR_KSEAF_DERIVATION, param, kseaf);
}

/* TS33.501 Annex A.7 : Kamf derivation function */
void ogs_kdf_kamf(char *supi, uint8_t *abba, uint8_t abba_len,
        uint8_t *kseaf, uint8_t *kamf)
{
    kdf_param_t param;

    ogs_assert(supi);
    ogs_assert(abba);
    ogs_assert(abba_len);
    ogs_assert(kseaf);
    ogs_assert(kamf);

    memset(param, 0, sizeof(param));
    param[0].buf = (uint8_t *)supi;
    param[0].len = strlen(supi);
    param[1].buf = abba;
    param[1].len = abba_len;

    ogs_kdf_common(kseaf, OGS_SHA256_DIGEST_SIZE,
            FC_FOR_KAMF_DERIVATION, param, kamf);
}

/* TS33.401 Annex A.3 KeNB derivation function */
void ogs_kdf_kenb(const uint8_t *kasme, uint32_t ul_count, uint8_t *kenb)
{
    uint8_t s[7];

    s[0] = FC_FOR_KENB_DERIVATION;

    ul_count = htobe32(ul_count);
    memcpy(s+1, &ul_count, 4);

    s[5] = 0x00;
    s[6] = 0x04;

    ogs_hmac_sha256(kasme, 32, s, 7, kenb, 32);

    ogs_fatal("ENB");
    ogs_log_hexdump(OGS_LOG_FATAL, kenb, 32);
}

/* TS33.401 Annex A.4 NH derivation function */
void ogs_kdf_nh_enb(const uint8_t *kasme,
        const uint8_t *sync_input, uint8_t *kenb)
{
    uint8_t s[35];

    s[0] = FC_FOR_KENB_NH_DERIVATION;

    memcpy(s+1, sync_input, 32);

    s[33] = 0x00;
    s[34] = 0x20;

    ogs_hmac_sha256(kasme, 32, s, 35, kenb, 32);
    ogs_fatal("NH");
    ogs_log_hexdump(OGS_LOG_FATAL, kenb, 32);
}

/* TS33.401 Annex A.7 Algorithm key derivation functions */
void ogs_kdf_nas_eps(uint8_t algorithm_type_distinguishers,
    uint8_t algorithm_identity, const uint8_t *kasme, uint8_t *knas)
{
    uint8_t s[7];
    uint8_t out[32];

    s[0] = FC_FOR_EPS_ALGORITHM_KEY_DERIVATION;

    s[1] = algorithm_type_distinguishers;
    s[2] = 0x00;
    s[3] = 0x01;

    s[4] = algorithm_identity;
    s[5] = 0x00;
    s[6] = 0x01;

    ogs_hmac_sha256(kasme, 32, s, 7, out, 32);
    memcpy(knas, out+16, 16);

    ogs_fatal("KNAS");
    ogs_log_hexdump(OGS_LOG_FATAL, knas, 16);
}

