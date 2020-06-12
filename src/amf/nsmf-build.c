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

#include "nsmf-build.h"

ogs_sbi_request_t *amf_nsmf_pdu_session_build_create_sm_context(
        amf_ue_t *amf_ue, void *data)
{
    ogs_nas_5gs_ul_nas_transport_t *ul_nas_transport = data;
    ogs_nas_payload_container_t *payload_container = NULL;
    ogs_nas_pdu_session_identity_2_t *pdu_session_id = NULL;

    ogs_sbi_message_t sbi_message;
    ogs_sbi_request_t *request = NULL;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_header_t header;

    char buf[OGS_AMFIDSTRLEN];
    amf_sess_t *sess = NULL;

    OpenAPI_sm_context_create_data_t SMContextCreateData;
    OpenAPI_plmn_id_nid_t plmn_id_nid;
    OpenAPI_snssai_t s_nssai;
    OpenAPI_ref_to_binary_data_t n1_sm_msg;
    OpenAPI_guami_t guami;

    ogs_assert(amf_ue);
    ogs_assert(amf_ue->nas.access_type);
    ogs_assert(ul_nas_transport);
    payload_container = &ul_nas_transport->payload_container;
    ogs_assert(payload_container);
    pdu_session_id = &ul_nas_transport->pdu_session_id;
    ogs_assert(pdu_session_id);

    sess = amf_sess_find_by_psi(amf_ue, *pdu_session_id);
    ogs_assert(sess);

    memset(&sbi_message, 0, sizeof(sbi_message));
    sbi_message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    sbi_message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NSMF_PDUSESSION;
    sbi_message.h.api.version = (char *)OGS_SBI_API_V1;
    sbi_message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_SM_CONTEXTS;

    memset(&SMContextCreateData, 0, sizeof(SMContextCreateData));

    SMContextCreateData.serving_nf_id = ogs_sbi_self()->nf_instance_id;

    plmn_id_nid.mcc = ogs_plmn_id_mcc_string(&amf_ue->tai.plmn_id);
    plmn_id_nid.mnc = ogs_plmn_id_mnc_string(&amf_ue->tai.plmn_id);
    plmn_id_nid.nid = NULL;
    SMContextCreateData.serving_network = &plmn_id_nid;

    SMContextCreateData.supi = amf_ue->supi;
    SMContextCreateData.pei = amf_ue->pei;
    SMContextCreateData.pdu_session_id = sess->psi;
    SMContextCreateData.dnn = sess->dnn;

    memset(&s_nssai, 0, sizeof(s_nssai));
    if (sess->s_nssai) {
        s_nssai.sst = sess->s_nssai->sst;
        s_nssai.sd = ogs_s_nssai_sd_string(sess->s_nssai);
        SMContextCreateData.s_nssai = &s_nssai;
    }

    ogs_assert(amf_ue->guami);
    guami.amf_id = ogs_amf_id_to_string(&amf_ue->guami->amf_id, buf);
    guami.plmn_id = (OpenAPI_plmn_id_t *)&plmn_id_nid;
    SMContextCreateData.guami = &guami;

    SMContextCreateData.an_type = amf_ue->nas.access_type; 

    memset(&header, 0, sizeof(header));
    header.service.name = (char *)OGS_SBI_SERVICE_NAME_NAMF_CALLBACK;
    header.api.version = (char *)OGS_SBI_API_V1;
    header.resource.component[0] = amf_ue->supi;
    header.resource.component[1] =
        (char *)OGS_SBI_RESOURCE_NAME_SM_CONTEXT_STATUS;
    header.resource.component[2] = ogs_msprintf("%d", sess->psi);

    server = ogs_list_first(&ogs_sbi_self()->server_list);
    ogs_assert(server);
    SMContextCreateData.sm_context_status_uri =
        ogs_sbi_server_uri(server, &header);

    n1_sm_msg.content_id = (char *)OGS_SBI_MULTIPART_5GSM_ID;
    SMContextCreateData.n1_sm_msg = &n1_sm_msg;

    sbi_message.SMContextCreateData = &SMContextCreateData;

    sbi_message.part[0].content_id = (char *)OGS_SBI_MULTIPART_5GSM_ID;
    sbi_message.part[0].content_type = (char *)OGS_SBI_CONTENT_5GNAS_TYPE;
    sbi_message.part[0].pkbuf =
        ogs_pkbuf_alloc(NULL, OGS_NAS_HEADROOM+payload_container->length);
    ogs_pkbuf_reserve(sbi_message.part[0].pkbuf, OGS_NAS_HEADROOM);
    ogs_pkbuf_put_data(sbi_message.part[0].pkbuf,
            payload_container->buffer, payload_container->length);
    sbi_message.num_of_part = 1;

    request = ogs_sbi_build_request(&sbi_message);
    ogs_assert(request);

    ogs_free(plmn_id_nid.mcc);
    ogs_free(plmn_id_nid.mnc);
    ogs_free(SMContextCreateData.sm_context_status_uri);
    ogs_free(header.resource.component[2]);
    if (s_nssai.sd)
        ogs_free(s_nssai.sd);
    ogs_pkbuf_free(sbi_message.part[0].pkbuf);

    return request;
}
