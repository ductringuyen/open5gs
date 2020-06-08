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

#include "test-5gc.h"

static void test1_func(abts_case *tc, void *data)
{
    int rv;
    ogs_socknode_t *ngap;
    ogs_socknode_t *gtpu;
    ogs_pkbuf_t *gmmbuf;
    ogs_pkbuf_t *nasbuf;
    ogs_pkbuf_t *sendbuf;
    ogs_pkbuf_t *recvbuf;
    ogs_ngap_message_t message;
    int i;
    int msgindex = 0;

    ogs_nas_5gs_mobile_identity_t mobile_identity;
    ogs_nas_5gs_mobile_identity_imsi_t mobile_identity_imsi;
    test_ue_t test_ue;

    uint8_t tmp[OGS_MAX_SDU_LEN];

    const char *_k_string = "5122250214c33e723a5dd523fc145fc0";
    uint8_t k[OGS_KEY_LEN];
    const char *_opc_string = "981d464c7c52eb6e5036234984ad0bcf";
    uint8_t opc[OGS_KEY_LEN];

    mongoc_collection_t *collection = NULL;
    bson_t *doc = NULL;
    int64_t count = 0;
    bson_error_t error;
    const char *json =
      "{"
        "\"_id\" : { \"$oid\" : \"597223158b8861d7605378c6\" }, "
        "\"imsi\" : \"2089300007487\", "
        "\"pdn\" : ["
          "{"
            "\"apn\" : \"internet\", "
            "\"_id\" : { \"$oid\" : \"597223158b8861d7605378c7\" }, "
            "\"ambr\" : {"
              "\"uplink\" : { \"$numberLong\" : \"1024000\" }, "
              "\"downlink\" : { \"$numberLong\" : \"1024000\" } "
            "},"
            "\"qos\" : { "
              "\"qci\" : 9, "
              "\"arp\" : { "
                "\"priority_level\" : 8,"
                "\"pre_emption_vulnerability\" : 1, "
                "\"pre_emption_capability\" : 1"
              "} "
            "}, "
            "\"type\" : 2"
          "}"
        "],"
        "\"ambr\" : { "
          "\"uplink\" : { \"$numberLong\" : \"1024000\" }, "
          "\"downlink\" : { \"$numberLong\" : \"1024000\" } "
        "},"
        "\"subscribed_rau_tau_timer\" : 12,"
        "\"network_access_mode\" : 2, "
        "\"subscriber_status\" : 0, "
        "\"access_restriction_data\" : 32, "
        "\"security\" : { "
          "\"k\" : \"5122250214c33e723a5dd523fc145fc0\", "
          "\"opc\" : \"981d464c7c52eb6e5036234984ad0bcf\", "
          "\"amf\" : \"8000\", "
          "\"sqn\" : { \"$numberLong\" : \"25235952177090\" }, "
          "\"rand\" : \"20080C38 18183B52 2614162C 07601D0D\" "
        "}, "
        "\"__v\" : 0 "
      "}";

    /* gNB connects to AMF */
    ngap = testgnb_ngap_client("127.0.0.1");
    ABTS_PTR_NOTNULL(tc, ngap);

#if 0
    /* eNB connects to SGW */
    gtpu = testgnb_gtpu_server("127.0.0.5");
    ABTS_PTR_NOTNULL(tc, gtpu);
#endif

    /* Send NG-Setup Reqeust */
    sendbuf = testngap_build_ng_setup_request(0x000102);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testgnb_ngap_send(ngap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    /* Receive NG-Setup Response */
    recvbuf = testgnb_ngap_read(ngap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    ogs_pkbuf_free(recvbuf);

    /* Setup Test UE Context */
    memset(&test_ue, 0, sizeof(test_ue));

    test_ue.nas.registration.type = 1; /* TSC[0], KSI[1] */
    test_ue.nas.registration.follow_on_request = 1;
    test_ue.nas.registration.value = 1; /* Initial Registration */

    memset(&mobile_identity_imsi, 0, sizeof(mobile_identity_imsi));
    mobile_identity.length = 12;
    mobile_identity.buffer = &mobile_identity_imsi;

    mobile_identity_imsi.h.supi_format = OGS_NAS_5GS_SUPI_FORMAT_IMSI;
    mobile_identity_imsi.h.type = OGS_NAS_5GS_MOBILE_IDENTITY_SUCI;
    ogs_nas_from_plmn_id(&mobile_identity_imsi.nas_plmn_id,
            &test_self()->tai.plmn_id);
    mobile_identity_imsi.routing_indicator1 = 0;
    mobile_identity_imsi.routing_indicator2 = 0xf;
    mobile_identity_imsi.routing_indicator3 = 0xf;
    mobile_identity_imsi.routing_indicator4 = 0xf;
    mobile_identity_imsi.protection_scheme_id = OGS_NAS_5GS_NULL_SCHEME;
    mobile_identity_imsi.home_network_pki_value = 0;
    mobile_identity_imsi.scheme_output[0] = 0;
    mobile_identity_imsi.scheme_output[1] = 0;
    mobile_identity_imsi.scheme_output[2] = 0x47;
    mobile_identity_imsi.scheme_output[3] = 0x78;

    test_ue_set_mobile_identity(&test_ue, &mobile_identity);

    test_ue.nas.access_type = OGS_ACCESS_TYPE_3GPP;
    test_ue.abba_len = 2;

    OGS_HEX(_k_string, strlen(_k_string), test_ue.k);
    OGS_HEX(_opc_string, strlen(_opc_string), test_ue.opc);

    /********** Insert Subscriber in Database */
    collection = mongoc_client_get_collection(
        ogs_mongoc()->client, ogs_mongoc()->name, "subscribers");
    ABTS_PTR_NOTNULL(tc, collection);

    doc = bson_new_from_json((const uint8_t *)json, -1, &error);;
    ABTS_PTR_NOTNULL(tc, doc);
    ABTS_TRUE(tc, mongoc_collection_insert(collection,
                MONGOC_INSERT_NONE, doc, NULL, &error));
    bson_destroy(doc);

    doc = BCON_NEW("imsi", BCON_UTF8(test_ue.imsi));
    ABTS_PTR_NOTNULL(tc, doc);
    do {
        count = mongoc_collection_count (
            collection, MONGOC_QUERY_NONE, doc, 0, 0, NULL, &error);
    } while (count == 0);
    bson_destroy(doc);

    /* Send Registration request */
    gmmbuf = testgmm_build_registration_request(&test_ue, &mobile_identity);
    ABTS_PTR_NOTNULL(tc, gmmbuf);
    sendbuf = testngap_build_initial_ue_message(gmmbuf);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testgnb_ngap_send(ngap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    /* Receive Authentication request */
    recvbuf = testgnb_ngap_read(ngap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    testngap_recv(&test_ue, recvbuf);

    /* Send Authentication response */
    gmmbuf = testgmm_build_authentication_response(&test_ue);
    ABTS_PTR_NOTNULL(tc, gmmbuf);
    sendbuf = testngap_build_uplink_nas_transport(&test_ue, gmmbuf);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testgnb_ngap_send(ngap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    /* Receive Security mode command */
    recvbuf = testgnb_ngap_read(ngap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    testngap_recv(&test_ue, recvbuf);

    /* Send Security mode complete */
    nasbuf = testgmm_build_registration_request(&test_ue, &mobile_identity);
    ABTS_PTR_NOTNULL(tc, nasbuf);
    gmmbuf = testgmm_build_security_mode_complete(&test_ue, nasbuf);
    ABTS_PTR_NOTNULL(tc, gmmbuf);
    sendbuf = testngap_build_uplink_nas_transport(&test_ue, gmmbuf);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testgnb_ngap_send(ngap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);
    ogs_pkbuf_free(nasbuf);

    /* Receive Initial Context Setup Request */
    recvbuf = testgnb_ngap_read(ngap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    testngap_recv(&test_ue, recvbuf);

#if 0
    /* Send UE Capability Info Indication */
    rv = testngap_build_ue_capability_info_indication(&sendbuf, msgindex);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);
    rv = testgnb_ngap_send(ngap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);
#endif

    ogs_msleep(50);

    /* Send Initial Context Setup Response */
    sendbuf = testngap_build_initial_context_setup_response(&test_ue);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testgnb_ngap_send(ngap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    ogs_msleep(300);

#if 0
    /* Send Attach Complete + Activate default EPS bearer cotext accept */
    rv = testngap_build_attach_complete(&sendbuf, msgindex);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);
    rv = testgnb_ngap_send(ngap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    ogs_msleep(50);

    /* Receive EMM information */
    recvbuf = testgnb_ngap_read(ngap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    OGS_HEX(_emm_information, strlen(_emm_information), tmp);
    ABTS_TRUE(tc, memcmp(recvbuf->data, tmp, 28) == 0);
    ABTS_TRUE(tc, memcmp(recvbuf->data+32, tmp+32, 20) == 0);
    ogs_pkbuf_free(recvbuf);

    /* Send GTP-U ICMP Packet */
    rv = testgtpu_build_ping(&sendbuf, 1, "10.45.0.2", "10.45.0.1");
    ABTS_INT_EQUAL(tc, OGS_OK, rv);
    rv = testgnb_gtpu_send(gtpu, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    /* Receive GTP-U ICMP Packet */
    recvbuf = testgnb_gtpu_read(gtpu);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    ogs_pkbuf_free(recvbuf);
#endif

    /********** Remove Subscriber in Database */
    doc = BCON_NEW("imsi", BCON_UTF8(test_ue.imsi));
    ABTS_PTR_NOTNULL(tc, doc);
    ABTS_TRUE(tc, mongoc_collection_remove(collection,
            MONGOC_REMOVE_SINGLE_REMOVE, doc, NULL, &error)) 
    bson_destroy(doc);

    mongoc_collection_destroy(collection);

    /* Clear Test UE Context */
    test_ue_remove(&test_ue);

#if 0
    /* eNB disonncect from SGW */
    testgnb_gtpu_close(gtpu);
#endif

    /* gNB disonncect from AMF */
    testgnb_ngap_close(ngap);
}

abts_suite *test_minimal(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test1_func, NULL);

    return suite;
}
