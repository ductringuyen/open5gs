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

#ifndef NAS_5GS_SECURITY_H
#define NAS_5GS_SECURITY_H

#include "ogs-crypt.h"
#include "context.h"

/* Algorithm Type Distinguishers */
#define AMF_KDF_NAS_ENC_ALG 0x01
#define AMF_KDF_NAS_INT_ALG 0x02

/* Algorithm Identity defined in nas_ies.h 
#define NAS_SECURITY_ALGORITHMS_EIA0_EEA0 0
#define NAS_SECURITY_ALGORITHMS_128_EIA1_EEA1 1
#define NAS_SECURITY_ALGORITHMS_128_EIA1_EEA2 2
#define NAS_SECURITY_ALGORITHMS_128_EIA3_EEA3 3
*/

#ifdef __cplusplus
extern "C" {
#endif

ogs_pkbuf_t *nas_5gs_security_encode(
    amf_ue_t *amf_ue, ogs_nas_5gs_message_t *message);
int nas_5gs_security_decode(amf_ue_t *amf_ue, 
    ogs_nas_security_header_type_t security_header_type, ogs_pkbuf_t *pkbuf);

#ifdef __cplusplus
}
#endif

#endif /* NAS_5GS_SECURITY_H */

