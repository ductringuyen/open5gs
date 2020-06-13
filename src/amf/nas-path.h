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

#ifndef AMF_NAS_5GS_PATH_H
#define AMF_NAS_5GS_PATH_H

#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

int nas_5gs_send_to_gnb(amf_ue_t *amf_ue, ogs_pkbuf_t *pkbuf);
int nas_5gs_send_to_downlink_nas_transport(
        amf_ue_t *amf_ue, ogs_pkbuf_t *pkbuf);

void nas_5gs_send_nas_reject(
        amf_ue_t *amf_ue, ogs_nas_5gmm_cause_t gmm_cause);
void nas_5gs_send_nas_reject_from_sbi(
        amf_ue_t *amf_ue, int status);

void nas_5gs_send_registration_accept(amf_ue_t *amf_ue);
void nas_5gs_send_registration_reject(
        amf_ue_t *amf_ue, ogs_nas_5gmm_cause_t gmm_cause);

void nas_5gs_send_identity_request(amf_ue_t *amf_ue);

void nas_5gs_send_authentication_request(amf_ue_t *amf_ue);
void nas_5gs_send_authentication_reject(amf_ue_t *amf_ue);

void nas_5gs_send_security_mode_command(amf_ue_t *amf_ue);

void nas_5gs_send_configuration_update_command(
        amf_ue_t *amf_ue, int ack, int red);

void nas_5gs_send_gmm_status(amf_ue_t *amf_ue, ogs_nas_5gmm_cause_t cause);

#if 0
void nas_5gs_send_detach_accept(amf_ue_t *amf_ue);

void nas_5gs_send_pdn_connectivity_reject(
    amf_sess_t *sess, ogs_nas_5gsm_cause_t gsm_cause);
void nas_5gs_send_gsm_information_request(amf_bearer_t *bearer);
void nas_5gs_send_activate_default_bearer_context_request(amf_bearer_t *bearer);
void nas_5gs_send_activate_dedicated_bearer_context_request(
        amf_bearer_t *bearer);
void nas_5gs_send_activate_all_dedicated_bearers(amf_bearer_t *default_bearer);
void nas_5gs_send_modify_bearer_context_request(
        amf_bearer_t *bearer, int qos_presence, int tft_presence);
void nas_5gs_send_deactivate_bearer_context_request(amf_bearer_t *bearer);
void nas_5gs_send_bearer_resource_allocation_reject(
    amf_bearer_t *bearer, ogs_nas_5gsm_cause_t gsm_cause);
void nas_5gs_send_bearer_resource_modification_reject(
    amf_bearer_t *bearer, ogs_nas_5gsm_cause_t gsm_cause);

void nas_5gs_send_tau_accept(
        amf_ue_t *amf_ue, S1AP_ProcedureCode_t procedureCode);
void nas_5gs_send_tau_reject(amf_ue_t *amf_ue, ogs_nas_5gsm_cause_t gmm_cause);

void nas_5gs_send_service_reject(
        amf_ue_t *amf_ue, ogs_nas_5gmm_cause_t gmm_cause);

void nas_5gs_send_cs_service_notification(amf_ue_t *amf_ue);
void nas_5gs_send_downlink_nas_transport(
        amf_ue_t *amf_ue, uint8_t *buffer, uint8_t length);
#endif

#ifdef __cplusplus
}
#endif

#endif /* AMF_NAS_5GS_PATH_H */
