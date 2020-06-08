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

#include "test-ngap.h"

void testngap_handle_ng_setup_response(
        test_ue_t *test_ue, ogs_ngap_message_t *message)
{
    char buf[OGS_ADDRSTRLEN];
    int i, j;

    NGAP_SuccessfulOutcome_t *successfulOutcome = NULL;
    NGAP_NGSetupResponse_t *NGSetupResponse = NULL;
    NGAP_PLMNSupportList_t *PLMNSupportList = NULL;

    NGAP_NGSetupResponseIEs_t *ie = NULL;

    ogs_assert(test_ue);
    ogs_assert(message);

    successfulOutcome = message->choice.successfulOutcome;
    ogs_assert(successfulOutcome);
    NGSetupResponse = &successfulOutcome->value.choice.NGSetupResponse;
    ogs_assert(NGSetupResponse);

    ogs_debug("NG setup response");

    for (i = 0; i < NGSetupResponse->protocolIEs.list.count; i++) {
        ie = NGSetupResponse->protocolIEs.list.array[i];
        switch (ie->id) {
        case NGAP_ProtocolIE_ID_id_PLMNSupportList:
            PLMNSupportList = &ie->value.choice.PLMNSupportList;
            break;
        default:
            break;
        }
    }

    for (i = 0; i < PLMNSupportList->list.count; i++) {
        NGAP_PLMNSupportItem_t *NGAP_PLMNSupportItem = NULL;
        NGAP_PLMNIdentity_t *pLMNIdentity = NULL;
        NGAP_SliceSupportList_t *sliceSupportList = NULL;

        sliceSupportList = (NGAP_SliceSupportList_t *)
            PLMNSupportList->list.array[i];
        for (j = 0; j < sliceSupportList->list.count; j++) {
        }
    }
}

void testngap_handle_downlink_nas_transport(
        test_ue_t *test_ue, ogs_ngap_message_t *message)
{
    int i;
    char buf[OGS_ADDRSTRLEN];

    NGAP_NGAP_PDU_t pdu;
    NGAP_InitiatingMessage_t *initiatingMessage = NULL;
    NGAP_InitialContextSetupRequest_t *InitialContextSetupRequest = NULL;

    NGAP_InitialContextSetupRequestIEs_t *ie = NULL;
    NGAP_AMF_UE_NGAP_ID_t *AMF_UE_NGAP_ID = NULL;
    NGAP_RAN_UE_NGAP_ID_t *RAN_UE_NGAP_ID = NULL;
    NGAP_NAS_PDU_t *NAS_PDU = NULL;

    ogs_assert(test_ue);
    ogs_assert(message);

    initiatingMessage = message->choice.initiatingMessage;
    ogs_assert(initiatingMessage);
    InitialContextSetupRequest =
        &initiatingMessage->value.choice.InitialContextSetupRequest;
    ogs_assert(InitialContextSetupRequest);

    for (i = 0; i < InitialContextSetupRequest->protocolIEs.list.count; i++) {
        ie = InitialContextSetupRequest->protocolIEs.list.array[i];
        switch (ie->id) {
        case NGAP_ProtocolIE_ID_id_RAN_UE_NGAP_ID:
            RAN_UE_NGAP_ID = &ie->value.choice.RAN_UE_NGAP_ID;
            break;
        case NGAP_ProtocolIE_ID_id_AMF_UE_NGAP_ID:
            AMF_UE_NGAP_ID = &ie->value.choice.AMF_UE_NGAP_ID;
            break;
        case NGAP_ProtocolIE_ID_id_NAS_PDU:
            NAS_PDU = &ie->value.choice.NAS_PDU;
            break;
        default:
            break;
        }
    }

    test_ue->ran_ue_ngap_id = *RAN_UE_NGAP_ID;
    asn_INTEGER2ulong(AMF_UE_NGAP_ID, &test_ue->amf_ue_ngap_id);

    if (NAS_PDU)
        testngap_send_to_nas(test_ue, NAS_PDU);
}

void testngap_handle_initial_context_setup_request(
        test_ue_t *test_ue, ogs_ngap_message_t *message)
{
    int i;
    char buf[OGS_ADDRSTRLEN];

    NGAP_NGAP_PDU_t pdu;
    NGAP_InitiatingMessage_t *initiatingMessage = NULL;
    NGAP_InitialContextSetupRequest_t *InitialContextSetupRequest = NULL;

    NGAP_InitialContextSetupRequestIEs_t *ie = NULL;
    NGAP_NAS_PDU_t *NAS_PDU = NULL;

    ogs_assert(test_ue);
    ogs_assert(message);

    initiatingMessage = message->choice.initiatingMessage;
    ogs_assert(initiatingMessage);
    InitialContextSetupRequest =
        &initiatingMessage->value.choice.InitialContextSetupRequest;
    ogs_assert(InitialContextSetupRequest);

    for (i = 0; i < InitialContextSetupRequest->protocolIEs.list.count; i++) {
        ie = InitialContextSetupRequest->protocolIEs.list.array[i];
        switch (ie->id) {
        case NGAP_ProtocolIE_ID_id_NAS_PDU:
            NAS_PDU = &ie->value.choice.NAS_PDU;
            break;
        default:
            break;
        }
    }

    if (NAS_PDU)
        testngap_send_to_nas(test_ue, NAS_PDU);
}
