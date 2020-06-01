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

void ogs_kdf_xres_star(
        char *serving_network_name, uint8_t *rand,
        uint8_t *xres, size_t xres_len,
        uint8_t *xres_star)
{
#if 0
    uint8_t *s = ogs_calloc(1 +
            strlen(serving_network_name) + 2 +
            OGS_RAND_LEN + 2 + xres_len + 2);
    uint16_t len;
    uint8_t out[32];

    s[0] = 0x6b; /* FC Value */

    strcpy(s+1, serving_network_name);
    len = strlen(serving_network_name);
    memcpy(s+1+len, &
    s[1] = algorithm_type_distinguishers;
    s[2] = 0x00;
    s[3] = 0x01;

    s[4] = algorithm_identity;
    s[5] = 0x00;
    s[6] = 0x01;

    ogs_hmac_sha256(kasme, 32, s, 7, out, 32);
    memcpy(knas, out+16, 16);

    ogs_free(s);
#endif
}
