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

#include "test-common.h"

ogs_pkbuf_t *testgmm_build_registration_request(test_ue_t *test_ue,
        ogs_nas_5gs_mobile_identity_t *mobile_identity)
{
    ogs_nas_5gs_message_t message;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_nas_5gs_registration_request_t *registration_request =
            &message.gmm.registration_request;
    ogs_nas_5gs_registration_type_t *registration_type =
            &registration_request->registration_type;
    ogs_nas_5gmm_capability_t *gmm_capability =
            &registration_request->gmm_capability;
    ogs_nas_ue_security_capability_t *ue_security_capability =
            &registration_request->ue_security_capability;

    ogs_assert(test_ue);
    ogs_assert(mobile_identity);

    memset(&message, 0, sizeof(message));
    message.gmm.h.extended_protocol_discriminator =
            OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_REGISTRATION_REQUEST;

    registration_type->type = 1;
    registration_type->follow_on_request = 1;
    registration_type->value = 1;

    registration_request->mobile_identity.length = mobile_identity->length;
    registration_request->mobile_identity.buffer = mobile_identity->buffer;

    registration_request->presencemask |=
            OGS_NAS_5GS_REGISTRATION_REQUEST_5GMM_CAPABILITY_PRESENT;
    gmm_capability->length = 1;
    gmm_capability->lte_positioning_protocol_capability = 1;
    gmm_capability->ho_attach = 1; 
    gmm_capability->s1_mode = 1; 

    registration_request->presencemask |=
            OGS_NAS_5GS_REGISTRATION_REQUEST_UE_SECURITY_CAPABILITY_PRESENT;
    ue_security_capability->length = 8;
    ue_security_capability->nea = 0xff;
    ue_security_capability->nia = 0xff;
    ue_security_capability->eps_ea = 0xff;
    ue_security_capability->eps_ia = 0xff;

    return ogs_nas_5gs_plain_encode(&message);
}

ogs_pkbuf_t *testgmm_build_authentication_response(test_ue_t *test_ue,
        uint8_t *k, uint8_t *opc)
{
    ogs_nas_5gs_message_t message;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_nas_5gs_authentication_response_t *authentication_response =
            &message.gmm.authentication_response;
    ogs_nas_authentication_response_parameter_t *authentication_response_parameter = &authentication_response->authentication_response_parameter;

    uint8_t ik[OGS_KEY_LEN];
    uint8_t ck[OGS_KEY_LEN];
    uint8_t ak[OGS_AK_LEN];
    uint8_t res[OGS_MAX_RES_LEN];
    uint8_t res_star[OGS_MAX_RES_LEN];
    uint8_t kausf[OGS_SHA256_DIGEST_SIZE];
    uint8_t kseaf[OGS_SHA256_DIGEST_SIZE];
    char *serving_network_name;

    ogs_assert(test_ue);
    ogs_assert(k);
    ogs_assert(opc);

    memset(&message, 0, sizeof(message));
    message.gmm.h.extended_protocol_discriminator =
            OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_AUTHENTICATION_RESPONSE;

    authentication_response->presencemask |=
        OGS_NAS_5GS_AUTHENTICATION_RESPONSE_AUTHENTICATION_RESPONSE_PARAMETER_PRESENT;

    milenage_f2345(opc, k, test_ue->rand, res, ck, ik, ak, NULL);
    serving_network_name = ogs_plmn_id_string(&test_self()->tai.plmn_id);
    ogs_kdf_xres_star(
            ck, ik,
            serving_network_name, test_ue->rand, res, 8,
            authentication_response_parameter->res);

    authentication_response_parameter->length = 16;

    memcpy(res_star, authentication_response_parameter->res,
            authentication_response_parameter->length);
    ogs_kdf_kausf(ck, ik, serving_network_name, test_ue->autn, kausf);
    ogs_kdf_kseaf(serving_network_name, kausf, kseaf);
    test_ue->abba_len = 2;
    ogs_kdf_kamf("imsi-2089300007487", test_ue->abba, test_ue->abba_len,
                kseaf, test_ue->kamf);

    ogs_free(serving_network_name);

    return ogs_nas_5gs_plain_encode(&message);
}
