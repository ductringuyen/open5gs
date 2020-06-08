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

#include "nas-security.h"
#include "gmm-build.h"
#include "amf-sm.h"

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __gmm_log_domain

ogs_pkbuf_t *gmm_build_registration_accept(amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_nas_5gs_registration_accept_t *registration_accept = &message.gmm.registration_accept;
    ogs_nas_5gs_registration_result_t *registration_result =
        &registration_accept->registration_result;
    ogs_nas_5gs_mobile_identity_t *mobile_identity = &registration_accept->guti;
    ogs_nas_5gs_mobile_identity_guti_t mobile_identity_guti;
    int served_tai_index = 0;
    ogs_nas_nssai_t *allowed_nssai = &registration_accept->allowed_nssai;
    int i, j, num_of_nssai;
    ogs_nas_gprs_timer_3_t *t3512_value = &registration_accept->t3512_value;

    ogs_assert(amf_ue);

    ogs_debug("[%s] Registration accept", amf_ue->supi);

    memset(&message, 0, sizeof(message));
    message.h.security_header_type = 
#if 0
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHERED;
#else
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED;
#endif
    message.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;

    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_REGISTRATION_ACCEPT;

    /* Registration Result */
    registration_result->length = 1;
    registration_result->value = amf_ue->nas.access_type;

    /* Set GUTI */
    ogs_debug("[%s]    %s 5G-S_GUTI[AMF_ID:0x%x,M_TMSI:0x%x]", amf_ue->supi,
            amf_ue->guti_present ? "[V]" : "[N]",
            ogs_amf_id_hexdump(&amf_ue->guti.amf_id), amf_ue->guti.m_tmsi);
    if (amf_ue->guti_present) {
        registration_accept->presencemask |= OGS_NAS_5GS_REGISTRATION_ACCEPT_5G_GUTI_PRESENT;
        memset(&mobile_identity_guti, 0, sizeof(mobile_identity_guti));

        mobile_identity_guti.h.type = OGS_NAS_5GS_MOBILE_IDENTITY_GUTI;

        memcpy(&mobile_identity_guti.nas_plmn_id,
                &amf_ue->guti.nas_plmn_id, OGS_PLMN_ID_LEN);
        memcpy(&mobile_identity_guti.amf_id,
                &amf_ue->guti.amf_id, sizeof(ogs_amf_id_t));
        mobile_identity_guti.m_tmsi = htobe32(amf_ue->guti.m_tmsi);

        mobile_identity->length = sizeof(mobile_identity_guti);
        mobile_identity->buffer = &mobile_identity_guti;
    }
    amf_ue->guti_present = 0;

    /* Set TAI List */
    registration_accept->presencemask |= OGS_NAS_5GS_REGISTRATION_ACCEPT_TAI_LIST_PRESENT;

    ogs_debug("[%s]    TAI[PLMN_ID:%06x,TAC:%d]", amf_ue->supi,
            ogs_plmn_id_hexdump(&amf_ue->tai.plmn_id), amf_ue->tai.tac.v);
    ogs_debug("[%s]    NR_CGI[PLMN_ID:%06x,CELL_ID:0x%llx]", amf_ue->supi,
            ogs_plmn_id_hexdump(&amf_ue->cgi.plmn_id),
            (long long)amf_ue->cgi.cell_id);

    served_tai_index = amf_find_served_tai(&amf_ue->tai);
    ogs_debug("[%s]    SERVED_TAI_INDEX[%d]", amf_ue->supi, served_tai_index);
    ogs_assert(served_tai_index >= 0 &&
            served_tai_index < OGS_MAX_NUM_OF_SERVED_TAI);

    ogs_nas_5gs_tai_list_build(&registration_accept->tai_list,
            &amf_self()->served_tai[served_tai_index].list0,
            &amf_self()->served_tai[served_tai_index].list2);

    /* Set Allowed NSSAI */
    num_of_nssai = 0;
    allowed_nssai->length = 0;
    for (i = 0; i < amf_self()->num_of_plmn_support; i++) {
        if (memcmp(&amf_ue->tai.plmn_id,
                &amf_self()->plmn_support[i].plmn_id, OGS_PLMN_ID_LEN) != 0)
            continue;

        ogs_debug("[%s]    NSSAI[PLMN_ID:%06x]", amf_ue->supi,
                ogs_plmn_id_hexdump(&amf_self()->plmn_support[i].plmn_id));
        for (j = 0; j < amf_self()->plmn_support[i].num_of_s_nssai; j++) {
            ogs_debug("[%s]         [sst:%d, sd:%06x]", amf_ue->supi,
                    amf_self()->plmn_support[i].s_nssai[j].sst,
                    amf_self()->plmn_support[i].s_nssai[j].sd.v);
            if (num_of_nssai < OGS_MAX_NUM_OF_S_NSSAI) {
                allowed_nssai->s_nssai[num_of_nssai].len =
                        amf_self()->plmn_support[i].s_nssai[j].len;
                allowed_nssai->s_nssai[num_of_nssai].sst =
                        amf_self()->plmn_support[i].s_nssai[j].sst;
                if (amf_self()->plmn_support[i].s_nssai[j].sd.v !=
                        OGS_S_NSSAI_NO_SD_VALUE) {
                    allowed_nssai->s_nssai[num_of_nssai].sd =
                        ogs_htobe24(amf_self()->plmn_support[i].s_nssai[j].sd);
                }
                allowed_nssai->length +=
                    allowed_nssai->s_nssai[num_of_nssai].len;
                num_of_nssai++;

            }
        }
    }
    if (allowed_nssai->length) {
        registration_accept->presencemask |= OGS_NAS_5GS_REGISTRATION_ACCEPT_ALLOWED_NSSAI_PRESENT;
        allowed_nssai->length += 2;
    }

    /* Set T3512 */
    registration_accept->presencemask |= OGS_NAS_5GS_REGISTRATION_ACCEPT_T3512_VALUE_PRESENT;
    t3512_value->length = 1;
    t3512_value->unit = OGS_NAS_GRPS_TIMER_3_UNIT_MULTIPLES_OF_1_HH;
    t3512_value->value = 9;

#if 0
    /* Set T3502 */
    registration_accept->presencemask |= OGS_NAS_5GS_REGISTRATION_ACCEPT_T3502_VALUE_PRESENT;
    registration_accept->t3502_value.length = 1;
    registration_accept->t3502_value.unit = OGS_NAS_GRPS_TIMER_UNIT_MULTIPLES_OF_1_MM;
    registration_accept->t3502_value.value = 12;
#endif

    pkbuf = nas_5gs_security_encode(amf_ue, &message);

    return pkbuf;
}

ogs_pkbuf_t *gmm_build_registration_reject(ogs_nas_5gmm_cause_t gmm_cause)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_registration_reject_t *registration_reject =
        &message.gmm.registration_reject;

    memset(&message, 0, sizeof(message));
    message.gmm.h.extended_protocol_discriminator =
            OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_REGISTRATION_REJECT;

    registration_reject->gmm_cause = gmm_cause;

    return ogs_nas_5gs_plain_encode(&message);
}

ogs_pkbuf_t *gmm_build_identity_request(amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_identity_request_t *identity_request = 
        &message.gmm.identity_request;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_IDENTITY_REQUEST;

    /* Request IMSI */
    ogs_debug("    Identity Type 2 : SUCI");
    identity_request->identity_type.value = OGS_NAS_5GS_MOBILE_IDENTITY_SUCI;

    return ogs_nas_5gs_plain_encode(&message);
}

ogs_pkbuf_t *gmm_build_authentication_request(amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_authentication_request_t *authentication_request = 
        &message.gmm.authentication_request;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.gmm.h.extended_protocol_discriminator =
            OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_AUTHENTICATION_REQUEST;

    authentication_request->ngksi.tsc = amf_ue->nas.tsc;
    authentication_request->ngksi.value = amf_ue->nas.ksi;
    authentication_request->abba.length = amf_ue->abba_len;
    memcpy(authentication_request->abba.value, amf_ue->abba, amf_ue->abba_len);

    authentication_request->presencemask |=
    OGS_NAS_5GS_AUTHENTICATION_REQUEST_AUTHENTICATION_PARAMETER_RAND_PRESENT;
    authentication_request->presencemask |=
    OGS_NAS_5GS_AUTHENTICATION_REQUEST_AUTHENTICATION_PARAMETER_AUTN_PRESENT;

    memcpy(authentication_request->authentication_parameter_rand.rand,
            amf_ue->rand, OGS_RAND_LEN);
    memcpy(authentication_request->authentication_parameter_autn.autn,
            amf_ue->autn, OGS_AUTN_LEN);
    authentication_request->authentication_parameter_autn.length = 
            OGS_AUTN_LEN;

    return ogs_nas_5gs_plain_encode(&message);
}

ogs_pkbuf_t *gmm_build_authentication_reject(void)
{
    ogs_nas_5gs_message_t message;

    memset(&message, 0, sizeof(message));

    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_AUTHENTICATION_REJECT;

    return ogs_nas_5gs_plain_encode(&message);
}

ogs_pkbuf_t *gmm_build_security_mode_command(amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_security_mode_command_t *security_mode_command = 
        &message.gmm.security_mode_command;
    ogs_nas_security_algorithms_t *selected_nas_security_algorithms =
        &security_mode_command->selected_nas_security_algorithms;
    ogs_nas_key_set_identifier_t *ngksi = &security_mode_command->ngksi;
    ogs_nas_ue_security_capability_t *replayed_ue_security_capabilities = 
        &security_mode_command->replayed_ue_security_capabilities;
    ogs_nas_imeisv_request_t *imeisv_request =
        &security_mode_command->imeisv_request;
    ogs_nas_additional_5g_security_information_t
        *additional_security_information =
            &security_mode_command->additional_security_information;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.h.security_header_type = 
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_NEW_SECURITY_CONTEXT;
    message.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;

    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_SECURITY_MODE_COMMAND;

    amf_ue->selected_int_algorithm = amf_selected_int_algorithm(amf_ue);
    amf_ue->selected_enc_algorithm = amf_selected_enc_algorithm(amf_ue);

    selected_nas_security_algorithms->type_of_integrity_protection_algorithm =
        amf_ue->selected_int_algorithm;
    selected_nas_security_algorithms->type_of_ciphering_algorithm =
        amf_ue->selected_enc_algorithm;

    ngksi->tsc = amf_ue->nas.tsc;
    ngksi->value = amf_ue->nas.ksi;

    replayed_ue_security_capabilities->nea = amf_ue->ue_security_capability.nea;
    replayed_ue_security_capabilities->nia = amf_ue->ue_security_capability.nia;
    replayed_ue_security_capabilities->eps_ea =
        amf_ue->ue_security_capability.eps_ea;
    replayed_ue_security_capabilities->eps_ia =
        amf_ue->ue_security_capability.eps_ia;

    replayed_ue_security_capabilities->length =
        sizeof(replayed_ue_security_capabilities->nea) +
        sizeof(replayed_ue_security_capabilities->nia);
    if (replayed_ue_security_capabilities->eps_ea ||
        replayed_ue_security_capabilities->eps_ia)
        replayed_ue_security_capabilities->length =
            sizeof(replayed_ue_security_capabilities->nea) +
            sizeof(replayed_ue_security_capabilities->nia) +
            sizeof(replayed_ue_security_capabilities->eps_ea) +
            sizeof(replayed_ue_security_capabilities->eps_ia);
    replayed_ue_security_capabilities->length = 8;
    ogs_debug("    Replayed UE SEC[LEN:%d NEA:0x%x NIA:0x%x EEA:0x%x EIA:0x%x",
            replayed_ue_security_capabilities->length,
            replayed_ue_security_capabilities->nea,
            replayed_ue_security_capabilities->nia,
            replayed_ue_security_capabilities->eps_ea,
            replayed_ue_security_capabilities->eps_ia);
    ogs_debug("    Selected[Integrity:0x%x Encrypt:0x%x]",
            amf_ue->selected_int_algorithm, amf_ue->selected_enc_algorithm);

    security_mode_command->presencemask |=
        OGS_NAS_5GS_SECURITY_MODE_COMMAND_IMEISV_REQUEST_PRESENT;
    imeisv_request->type = OGS_NAS_IMEISV_TYPE;
    imeisv_request->value = OGS_NAS_IMEISV_REQUESTED;

    security_mode_command->presencemask |=
        OGS_NAS_5GS_SECURITY_MODE_COMMAND_ADDITIONAL_5G_SECURITY_INFORMATION_PRESENT;
    additional_security_information->length = 1;

    if (amf_ue->selected_int_algorithm == OGS_NAS_SECURITY_ALGORITHMS_EIA0) {
        ogs_error("Encrypt[0x%x] can be skipped with NEA0, "
            "but Integrity[0x%x] cannot be bypassed with NIA0",
            amf_ue->selected_enc_algorithm, amf_ue->selected_int_algorithm);
        return NULL;
    }

    ogs_kdf_nas_5gs(OGS_KDF_NAS_INT_ALG, amf_ue->selected_int_algorithm,
            amf_ue->kamf, amf_ue->knas_int);
    ogs_kdf_nas_5gs(OGS_KDF_NAS_ENC_ALG, amf_ue->selected_enc_algorithm,
            amf_ue->kamf, amf_ue->knas_enc);

    return nas_5gs_security_encode(amf_ue, &message);
}

#if 0
ogs_pkbuf_t *gmm_build_detach_accept(amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.h.security_header_type = 
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHERED;
    message.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;

    ogs_debug("Detach accept");
    ogs_debug("    IMSI[%s]", amf_ue->imsi_bcd);

    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_DETACH_ACCEPT;

    return nas_5gs_security_encode(amf_ue, &message);
}

ogs_pkbuf_t *gmm_build_tau_accept(amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_tracking_area_update_accept_t *tau_accept = 
        &message.gmm.tracking_area_update_accept;
    int served_tai_index = 0;

    amf_sess_t *sess = NULL;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.h.security_header_type = 
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHERED;
    message.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;

    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_TRACKING_AREA_UPDATE_ACCEPT;

    tau_accept->eps_update_result.result = amf_ue->nas_5gs.update.value;

    /* Set T3512 */
    tau_accept->presencemask |=
        OGS_NAS_5GS_TRACKING_AREA_UPDATE_ACCEPT_T3512_VALUE_PRESENT ;
    tau_accept->t3512_value.unit = OGS_NAS_GRPS_TIMER_UNIT_MULTIPLES_OF_DECI_HH;
    tau_accept->t3512_value.value = 9;

    /* Set TAI */
    tau_accept->presencemask |=
        OGS_NAS_5GS_TRACKING_AREA_UPDATE_ACCEPT_TAI_LIST_PRESENT;

    ogs_debug("    TAI[PLMN_ID:%06x,TAC:%d]",
            ogs_plmn_id_hexdump(&amf_ue->tai.plmn_id),
            amf_ue->tai.tac);
    ogs_debug("    E_CGI[PLMN_ID:%06x,CELL_ID:%d]",
            ogs_plmn_id_hexdump(&amf_ue->e_cgi.plmn_id),
            amf_ue->e_cgi.cell_id);
    served_tai_index = amf_find_served_tai(&amf_ue->tai);
    ogs_debug("    SERVED_TAI_INDEX[%d]", served_tai_index);
    ogs_assert(served_tai_index >= 0 &&
            served_tai_index < OGS_MAX_NUM_OF_SERVED_TAI);
    ogs_nas_tai_list_build(&tau_accept->tai_list,
            &amf_self()->served_tai[served_tai_index].list0,
            &amf_self()->served_tai[served_tai_index].list2);

    /* Set EPS bearer context status */
    tau_accept->presencemask |=
        OGS_NAS_5GS_TRACKING_AREA_UPDATE_ACCEPT_EPS_BEARER_CONTEXT_STATUS_PRESENT;
    tau_accept->eps_bearer_context_status.length = 2;
    sess = amf_sess_first(amf_ue);
    while (sess) {
        amf_bearer_t *bearer = amf_bearer_first(sess);
        while (bearer) {
            switch (bearer->ebi) {
            case 5: tau_accept->eps_bearer_context_status.ebi5 = 1; break;
            case 6: tau_accept->eps_bearer_context_status.ebi6 = 1; break;
            case 7: tau_accept->eps_bearer_context_status.ebi7 = 1; break;
            case 8: tau_accept->eps_bearer_context_status.ebi8 = 1; break;
            case 9: tau_accept->eps_bearer_context_status.ebi9 = 1; break;
            case 10: tau_accept->eps_bearer_context_status.ebi10 = 1; break;
            case 11: tau_accept->eps_bearer_context_status.ebi11 = 1; break;
            case 12: tau_accept->eps_bearer_context_status.ebi12 = 1; break;
            case 13: tau_accept->eps_bearer_context_status.ebi13 = 1; break;
            case 14: tau_accept->eps_bearer_context_status.ebi14 = 1; break;
            case 15: tau_accept->eps_bearer_context_status.ebi15 = 1; break;
            default: break;
            }

            bearer = amf_bearer_next(bearer);
        }
        sess = amf_sess_next(sess);
    }

#if 0 /* Need not to include T3502 */
    /* Set T3502 */
    tau_accept->presencemask |=
        OGS_NAS_5GS_TRACKING_AREA_UPDATE_ACCEPT_T3502_VALUE_PRESENT;
    tau_accept->t3502_value.unit = OGS_NAS_GRPS_TIMER_UNIT_MULTIPLES_OF_1_MM;
    tau_accept->t3502_value.value = 12;
#endif

    /* Set T3523 */
    tau_accept->presencemask |=
        OGS_NAS_5GS_TRACKING_AREA_UPDATE_ACCEPT_T3523_VALUE_PRESENT;
    tau_accept->t3523_value.unit = OGS_NAS_GRPS_TIMER_UNIT_MULTIPLES_OF_DECI_HH;
    tau_accept->t3523_value.value = 9;

    /* Set EPS network feature support */
    tau_accept->presencemask |=
        OGS_NAS_5GS_TRACKING_AREA_UPDATE_ACCEPT_EPS_NETWORK_FEATURE_SUPPORT_PRESENT;
    tau_accept->eps_network_feature_support.length = 1;
    tau_accept->eps_network_feature_support.ims_vops = 1;

    return nas_5gs_security_encode(amf_ue, &message);
}

ogs_pkbuf_t *gmm_build_tau_reject(
        ogs_nas_5gmm_cause_t gmm_cause, amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_tracking_area_update_reject_t *tau_reject = 
        &message.gmm.tracking_area_update_reject;

    ogs_assert(amf_ue);

    ogs_debug("Tracking area update reject");
    ogs_debug("    IMSI[%s] Cause[%d]",
            MME_UE_HAVE_IMSI(amf_ue) ? amf_ue->imsi_bcd : "Unknown", gmm_cause);

    memset(&message, 0, sizeof(message));
    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_TRACKING_AREA_UPDATE_REJECT;

    tau_reject->gmm_cause = gmm_cause;

    return ogs_nas_5gs_plain_encode(&message);
}

ogs_pkbuf_t *gmm_build_service_reject(
        ogs_nas_5gmm_cause_t gmm_cause, amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_service_reject_t *service_reject = &message.gmm.service_reject;

    ogs_assert(amf_ue);

    ogs_debug("Service reject");
    ogs_debug("    Cause[%d]", gmm_cause);

    memset(&message, 0, sizeof(message));
    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_SERVICE_REJECT;

    service_reject->gmm_cause = gmm_cause;

    return ogs_nas_5gs_plain_encode(&message);
}

ogs_pkbuf_t *gmm_build_cs_service_notification(amf_ue_t *amf_ue)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_cs_service_notification_t *cs_service_notification = 
        &message.gmm.cs_service_notification;
    ogs_nas_paging_identity_t *paging_identity =
        &cs_service_notification->paging_identity;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.h.security_header_type = 
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHERED;
    message.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;

    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_CS_SERVICE_NOTIFICATION;

    /* FIXME : Does it right to use TMSI */
    paging_identity->identity = OGS_NAS_PAGING_IDENTITY_TMSI;
    ogs_debug("    Paging Identity[%d]", paging_identity->identity);

    /* FIXME : What optional filed should be included in this message? */  

    return nas_5gs_security_encode(amf_ue, &message);
}

ogs_pkbuf_t *gmm_build_downlink_nas_transport(
        amf_ue_t *amf_ue, uint8_t *buffer, uint8_t length)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_downlink_nas_transport_t *downlink_nas_transport = 
        &message.gmm.downlink_nas_transport;
    ogs_nas_5gs_message_container_t *nas_message_container =
        &downlink_nas_transport->nas_message_container;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.h.security_header_type = 
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHERED;
    message.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;

    message.gmm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM;
    message.gmm.h.message_type = OGS_NAS_5GS_DOWNLINK_NAS_TRANSPORT;

    nas_message_container->length = length;
    memcpy(nas_message_container->buffer, buffer, length);

    return nas_5gs_security_encode(amf_ue, &message);
}
#endif
