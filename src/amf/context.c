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

#include "ngap-path.h"

static amf_context_t self;

int __amf_log_domain;
int __gmm_log_domain;
int __gsm_log_domain;

static OGS_POOL(amf_gnb_pool, amf_gnb_t);
static OGS_POOL(amf_ue_pool, amf_ue_t);
static OGS_POOL(ran_ue_pool, ran_ue_t);
static OGS_POOL(amf_sess_pool, amf_sess_t);
static OGS_POOL(amf_bearer_pool, amf_bearer_t);

static int context_initialized = 0;

void amf_context_init(void)
{
    ogs_assert(context_initialized == 0);

    /* Initialize AMF context */
    memset(&self, 0, sizeof(amf_context_t));

    ogs_log_install_domain(&__ogs_sctp_domain, "sctp", ogs_core()->log.level);
    ogs_log_install_domain(&__ogs_ngap_domain, "ngap", ogs_core()->log.level);
    ogs_log_install_domain(&__ogs_nas_domain, "nas", ogs_core()->log.level);
    ogs_log_install_domain(&__amf_log_domain, "amf", ogs_core()->log.level);
    ogs_log_install_domain(&__gmm_log_domain, "gmm", ogs_core()->log.level);
    ogs_log_install_domain(&__gsm_log_domain, "gsm", ogs_core()->log.level);

    ogs_list_init(&self.ngap_list);
    ogs_list_init(&self.ngap_list6);

    /* Allocate TWICE the pool to check if maximum number of gNBs is reached */
    ogs_pool_init(&amf_gnb_pool, ogs_config()->max.gnb*2);
    ogs_pool_init(&amf_ue_pool, ogs_config()->pool.ue);
    ogs_pool_init(&ran_ue_pool, ogs_config()->pool.ue);
    ogs_pool_init(&amf_sess_pool, ogs_config()->pool.sess);
    ogs_pool_init(&amf_bearer_pool, ogs_config()->pool.bearer);
    ogs_pool_init(&self.m_tmsi, ogs_config()->pool.ue);

    ogs_list_init(&self.gnb_list);
    ogs_list_init(&self.amf_ue_list);

    self.gnb_addr_hash = ogs_hash_make();
    self.gnb_id_hash = ogs_hash_make();
    self.amf_ue_ngap_id_hash = ogs_hash_make();
    self.guti_ue_hash = ogs_hash_make();
    self.suci_hash = ogs_hash_make();
    self.supi_hash = ogs_hash_make();

    context_initialized = 1;
}

void amf_context_final(void)
{
    ogs_assert(context_initialized == 1);

    amf_gnb_remove_all();
    amf_ue_remove_all();

    ogs_assert(self.gnb_addr_hash);
    ogs_hash_destroy(self.gnb_addr_hash);
    ogs_assert(self.gnb_id_hash);
    ogs_hash_destroy(self.gnb_id_hash);

    ogs_assert(self.amf_ue_ngap_id_hash);
    ogs_hash_destroy(self.amf_ue_ngap_id_hash);
    ogs_assert(self.guti_ue_hash);
    ogs_hash_destroy(self.guti_ue_hash);
    ogs_assert(self.suci_hash);
    ogs_hash_destroy(self.suci_hash);
    ogs_assert(self.supi_hash);
    ogs_hash_destroy(self.supi_hash);

    ogs_pool_final(&self.m_tmsi);
    ogs_pool_final(&amf_bearer_pool);
    ogs_pool_final(&amf_sess_pool);
    ogs_pool_final(&amf_ue_pool);
    ogs_pool_final(&ran_ue_pool);
    ogs_pool_final(&amf_gnb_pool);

    context_initialized = 0;
}

amf_context_t *amf_self(void)
{
    return &self;
}

static int amf_context_prepare(void)
{
    self.nf_type = OpenAPI_nf_type_AMF;

    self.relative_capacity = 0xff;

    self.ngap_port = OGS_NGAP_SCTP_PORT;

    return OGS_OK;
}

static int amf_context_validation(void)
{
    if (ogs_list_first(&self.ngap_list) == NULL &&
        ogs_list_first(&self.ngap_list6) == NULL) {
        ogs_error("No amf.ngap in '%s'", ogs_config()->file);
        return OGS_RETRY;
    }

    if (self.num_of_served_guami == 0) {
        ogs_error("No amf.guami in '%s'", ogs_config()->file);
        return OGS_ERROR;
    }

    if (self.num_of_served_tai == 0) {
        ogs_error("No amf.tai in '%s'", ogs_config()->file);
        return OGS_ERROR;
    }

    if (self.num_of_plmn_support == 0) {
        ogs_error("No amf.plmn in '%s'", ogs_config()->file);
        return OGS_ERROR;
    }

    if (self.plmn_support[0].num_of_s_nssai == 0) {
        ogs_error("No amf.plmn.s_nssai in '%s'", ogs_config()->file);
        return OGS_ERROR;
    }

    if (self.served_tai[0].list0.tai[0].num == 0 &&
        self.served_tai[0].list2.num == 0) {
        ogs_error("No amf.tai.plmn_id|tac in '%s'", ogs_config()->file);
        return OGS_ERROR;
    }

    if (self.amf_name == NULL) {
        ogs_error("No amf.amf_name in '%s'", ogs_config()->file);
        return OGS_ERROR;
    }

    if (self.num_of_integrity_order == 0) {
        ogs_error("No amf.security.integrity_order in '%s'",
                ogs_config()->file);
        return OGS_ERROR;
    }

    if (self.num_of_ciphering_order == 0) {
        ogs_error("no amf.security.ciphering_order in '%s'",
                ogs_config()->file);
        return OGS_ERROR;
    }

    return OGS_OK;
}

int amf_context_parse_config(void)
{
    int rv;
    yaml_document_t *document = NULL;
    ogs_yaml_iter_t root_iter;

    document = ogs_config()->document;
    ogs_assert(document);

    rv = amf_context_prepare();
    if (rv != OGS_OK) return rv;

    ogs_yaml_iter_init(&root_iter, document);
    while (ogs_yaml_iter_next(&root_iter)) {
        const char *root_key = ogs_yaml_iter_key(&root_iter);
        ogs_assert(root_key);
        if (!strcmp(root_key, "amf")) {
            ogs_yaml_iter_t amf_iter;
            ogs_yaml_iter_recurse(&root_iter, &amf_iter);
            while (ogs_yaml_iter_next(&amf_iter)) {
                const char *amf_key = ogs_yaml_iter_key(&amf_iter);
                ogs_assert(amf_key);
                if (!strcmp(amf_key, "relative_capacity")) {
                    const char *v = ogs_yaml_iter_value(&amf_iter);
                    if (v) self.relative_capacity = atoi(v);
                } else if (!strcmp(amf_key, "ngap")) {
                    ogs_yaml_iter_t ngap_array, ngap_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &ngap_array);
                    do {
                        int family = AF_UNSPEC;
                        int i, num = 0;
                        const char *hostname[OGS_MAX_NUM_OF_HOSTNAME];
                        uint16_t port = self.ngap_port;
                        const char *dev = NULL;
                        ogs_sockaddr_t *addr = NULL;

                        if (ogs_yaml_iter_type(&ngap_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&ngap_iter, &ngap_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&ngap_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&ngap_array))
                                break;
                            ogs_yaml_iter_recurse(&ngap_array, &ngap_iter);
                        } else if (ogs_yaml_iter_type(&ngap_array) ==
                            YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&ngap_iter)) {
                            const char *ngap_key =
                                ogs_yaml_iter_key(&ngap_iter);
                            ogs_assert(ngap_key);
                            if (!strcmp(ngap_key, "family")) {
                                const char *v = ogs_yaml_iter_value(&ngap_iter);
                                if (v) family = atoi(v);
                                if (family != AF_UNSPEC &&
                                    family != AF_INET && family != AF_INET6) {
                                    ogs_warn("Ignore family(%d) : "
                                        "AF_UNSPEC(%d), "
                                        "AF_INET(%d), AF_INET6(%d) ", 
                                        family, AF_UNSPEC, AF_INET, AF_INET6);
                                    family = AF_UNSPEC;
                                }
                            } else if (!strcmp(ngap_key, "addr") ||
                                    !strcmp(ngap_key, "name")) {
                                ogs_yaml_iter_t hostname_iter;
                                ogs_yaml_iter_recurse(
                                        &ngap_iter, &hostname_iter);
                                ogs_assert(ogs_yaml_iter_type(&hostname_iter) !=
                                    YAML_MAPPING_NODE);

                                do {
                                    if (ogs_yaml_iter_type(&hostname_iter) ==
                                            YAML_SEQUENCE_NODE) {
                                        if (!ogs_yaml_iter_next(&hostname_iter))
                                            break;
                                    }

                                    ogs_assert(num <= OGS_MAX_NUM_OF_HOSTNAME);
                                    hostname[num++] = 
                                        ogs_yaml_iter_value(&hostname_iter);
                                } while (
                                    ogs_yaml_iter_type(&hostname_iter) ==
                                        YAML_SEQUENCE_NODE);
                            } else if (!strcmp(ngap_key, "port")) {
                                const char *v = ogs_yaml_iter_value(&ngap_iter);
                                if (v) port = atoi(v);
                            } else if (!strcmp(ngap_key, "dev")) {
                                dev = ogs_yaml_iter_value(&ngap_iter);
                            } else
                                ogs_warn("unknown key `%s`", ngap_key);
                        }

                        addr = NULL;
                        for (i = 0; i < num; i++) {
                            rv = ogs_addaddrinfo(&addr,
                                    family, hostname[i], port, 0);
                            ogs_assert(rv == OGS_OK);
                        }

                        if (addr) {
                            if (ogs_config()->parameter.no_ipv4 == 0)
                                ogs_socknode_add(
                                        &self.ngap_list, AF_INET, addr);
                            if (ogs_config()->parameter.no_ipv6 == 0)
                                ogs_socknode_add(
                                        &self.ngap_list6, AF_INET6, addr);
                            ogs_freeaddrinfo(addr);
                        }

                        if (dev) {
                            rv = ogs_socknode_probe(
                                    ogs_config()->parameter.no_ipv4 ?
                                        NULL : &self.ngap_list,
                                    ogs_config()->parameter.no_ipv6 ?
                                        NULL : &self.ngap_list6,
                                    dev, port);
                            ogs_assert(rv == OGS_OK);
                        }

                    } while (ogs_yaml_iter_type(&ngap_array) ==
                            YAML_SEQUENCE_NODE);

                    if (ogs_list_first(&self.ngap_list) == NULL &&
                        ogs_list_first(&self.ngap_list6) == NULL) {
                        rv = ogs_socknode_probe(
                                ogs_config()->parameter.no_ipv4 ?
                                    NULL : &self.ngap_list,
                                ogs_config()->parameter.no_ipv6 ?
                                    NULL : &self.ngap_list6,
                                NULL, self.ngap_port);
                        ogs_assert(rv == OGS_OK);
                    }
                } else if (!strcmp(amf_key, "guami")) {
                    ogs_yaml_iter_t guami_array, guami_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &guami_array);
                    do {
                        const char *mcc = NULL, *mnc = NULL;
                        const char *region = NULL, *set = NULL;
                        const char *pointer = NULL;
                        ogs_assert(self.num_of_served_guami <=
                                MAX_NUM_OF_SERVED_GUAMI);

                        if (ogs_yaml_iter_type(&guami_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&guami_iter, &guami_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&guami_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&guami_array))
                                break;
                            ogs_yaml_iter_recurse(&guami_array,
                                    &guami_iter);
                        } else if (ogs_yaml_iter_type(&guami_array) ==
                            YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&guami_iter)) {
                            const char *guami_key =
                                ogs_yaml_iter_key(&guami_iter);
                            ogs_assert(guami_key);
                            if (!strcmp(guami_key, "plmn_id")) {
                                ogs_yaml_iter_t plmn_id_iter;

                                ogs_yaml_iter_recurse(&guami_iter,
                                        &plmn_id_iter);
                                while (ogs_yaml_iter_next(&plmn_id_iter)) {
                                    const char *plmn_id_key =
                                        ogs_yaml_iter_key(&plmn_id_iter);
                                    ogs_assert(plmn_id_key);
                                    if (!strcmp(plmn_id_key, "mcc")) {
                                        mcc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    } else if (!strcmp(plmn_id_key, "mnc")) {
                                        mnc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    }
                                }

                                if (mcc && mnc) {
                                    ogs_plmn_id_build(
                                        &self.served_guami[
                                            self.num_of_served_guami].
                                                plmn_id,
                                        atoi(mcc), atoi(mnc), strlen(mnc));
                                }
                            } else if (!strcmp(guami_key, "amf_id")) {
                                ogs_yaml_iter_t amf_id_iter;

                                ogs_yaml_iter_recurse(&guami_iter,
                                        &amf_id_iter);
                                while (ogs_yaml_iter_next(&amf_id_iter)) {
                                    const char *amf_id_key =
                                        ogs_yaml_iter_key(&amf_id_iter);
                                    ogs_assert(amf_id_key);
                                    if (!strcmp(amf_id_key, "region")) {
                                        region = ogs_yaml_iter_value(
                                                &amf_id_iter);
                                    } else if (!strcmp(amf_id_key, "set")) {
                                        set = ogs_yaml_iter_value(
                                                &amf_id_iter);
                                    } else if (!strcmp(amf_id_key, "pointer")) {
                                        pointer = ogs_yaml_iter_value(
                                                &amf_id_iter);
                                    }
                                }

                                if (region && set) {
                                    ogs_amf_id_build(
                                        &self.served_guami[
                                            self.num_of_served_guami].
                                                amf_id,
                                        atoi(region), atoi(set),
                                        pointer ? atoi(pointer) : 0);
                                }
                            } else
                                ogs_warn("unknown key `%s`", guami_key);
                        }

                        if (mnc && mcc && region && set) {
                            self.num_of_served_guami++;
                        } else {
                            ogs_warn("Ignore guami : "
                                    "mcc(%s), mnc(%s), region(%s), set(%s)",
                                    mcc, mnc, region, set);
                        }
                    } while (ogs_yaml_iter_type(&guami_array) ==
                            YAML_SEQUENCE_NODE);
                } else if (!strcmp(amf_key, "tai")) {
                    int num_of_list0 = 0;
                    ogs_5gs_tai0_list_t *list0 = NULL;
                    ogs_5gs_tai2_list_t *list2 = NULL;

                    ogs_assert(self.num_of_served_tai <=
                            OGS_MAX_NUM_OF_SERVED_TAI);
                    list0 = &self.served_tai[self.num_of_served_tai].list0;
                    ogs_assert(list0);
                    list2 = &self.served_tai[self.num_of_served_tai].list2;
                    ogs_assert(list2);

                    ogs_yaml_iter_t tai_array, tai_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &tai_array);
                    do {
                        const char *mcc = NULL, *mnc = NULL;
                        ogs_uint24_t tac[OGS_MAX_NUM_OF_TAI];
                        int num_of_tac = 0;

                        if (ogs_yaml_iter_type(&tai_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&tai_iter, &tai_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&tai_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&tai_array))
                                break;
                            ogs_yaml_iter_recurse(&tai_array,
                                    &tai_iter);
                        } else if (ogs_yaml_iter_type(&tai_array) ==
                                YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&tai_iter)) {
                            const char *tai_key = ogs_yaml_iter_key(&tai_iter);
                            ogs_assert(tai_key);
                            if (!strcmp(tai_key, "plmn_id")) {
                                ogs_yaml_iter_t plmn_id_iter;

                                ogs_yaml_iter_recurse(&tai_iter, &plmn_id_iter);
                                while (ogs_yaml_iter_next(&plmn_id_iter)) {
                                    const char *plmn_id_key =
                                        ogs_yaml_iter_key(&plmn_id_iter);
                                    ogs_assert(plmn_id_key);
                                    if (!strcmp(plmn_id_key, "mcc")) {
                                        mcc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    } else if (!strcmp(plmn_id_key, "mnc")) {
                                        mnc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    }
                                }
                            } else if (!strcmp(tai_key, "tac")) {
                                ogs_yaml_iter_t tac_iter;
                                ogs_yaml_iter_recurse(&tai_iter, &tac_iter);
                                ogs_assert(ogs_yaml_iter_type(&tac_iter) !=
                                    YAML_MAPPING_NODE);

                                do {
                                    const char *v = NULL;

                                    ogs_assert(num_of_tac <=
                                            OGS_MAX_NUM_OF_TAI);
                                    if (ogs_yaml_iter_type(&tac_iter) ==
                                            YAML_SEQUENCE_NODE) {
                                        if (!ogs_yaml_iter_next(&tac_iter))
                                            break;
                                    }

                                    v = ogs_yaml_iter_value(&tac_iter);
                                    if (v) {
                                        tac[num_of_tac].v = atoi(v);
                                        num_of_tac++;
                                    }
                                } while (
                                    ogs_yaml_iter_type(&tac_iter) ==
                                        YAML_SEQUENCE_NODE);
                            } else
                                ogs_warn("unknown key `%s`", tai_key);
                        }

                        if (mcc && mnc && num_of_tac) {
                            if (num_of_tac == 1) {
                                ogs_plmn_id_build(
                                    &list2->tai[list2->num].plmn_id,
                                    atoi(mcc), atoi(mnc), strlen(mnc));
                                list2->tai[list2->num].tac.v = tac[0].v;

                                list2->num++;
                                if (list2->num > 1)
                                    list2->type = OGS_TAI2_TYPE;
                                else
                                    list2->type = OGS_TAI1_TYPE;
                            } else if (num_of_tac > 1) {
                                int i;
                                ogs_plmn_id_build(
                                    &list0->tai[num_of_list0].plmn_id,
                                    atoi(mcc), atoi(mnc), strlen(mnc));
                                for (i = 0; i < num_of_tac; i++) {
                                    list0->tai[num_of_list0].tac[i].v =
                                        tac[i].v;
                                }

                                list0->tai[num_of_list0].num = num_of_tac;
                                list0->tai[num_of_list0].type = OGS_TAI0_TYPE;

                                num_of_list0++;
                            }
                        } else {
                            ogs_warn("Ignore tai : mcc(%p), mnc(%p), "
                                    "num_of_tac(%d)", mcc, mnc, num_of_tac);
                        }
                    } while (ogs_yaml_iter_type(&tai_array) ==
                            YAML_SEQUENCE_NODE);

                    if (list2->num || num_of_list0) {
                        self.num_of_served_tai++;
                    }
                } else if (!strcmp(amf_key, "plmn")) {
                    ogs_yaml_iter_t plmn_array, plmn_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &plmn_array);
                    do {
                        const char *mnc = NULL, *mcc = NULL;
                        ogs_assert(self.num_of_plmn_support <=
                                OGS_MAX_NUM_OF_PLMN);

                        if (ogs_yaml_iter_type(&plmn_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&plmn_iter, &plmn_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&plmn_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&plmn_array))
                                break;
                            ogs_yaml_iter_recurse(&plmn_array,
                                    &plmn_iter);
                        } else if (ogs_yaml_iter_type(&plmn_array) ==
                            YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&plmn_iter)) {
                            const char *plmn_key =
                                ogs_yaml_iter_key(&plmn_iter);
                            ogs_assert(plmn_key);
                            if (!strcmp(plmn_key, "plmn_id")) {
                                ogs_yaml_iter_t plmn_id_iter;

                                ogs_yaml_iter_recurse(&plmn_iter,
                                        &plmn_id_iter);
                                while (ogs_yaml_iter_next(&plmn_id_iter)) {
                                    const char *plmn_id_key =
                                        ogs_yaml_iter_key(&plmn_id_iter);
                                    ogs_assert(plmn_id_key);
                                    if (!strcmp(plmn_id_key, "mcc")) {
                                        mcc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    } else if (!strcmp(plmn_id_key, "mnc")) {
                                        mnc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    }
                                }

                                if (mcc && mnc) {
                                    ogs_plmn_id_build(
                                        &self.plmn_support[
                                            self.num_of_plmn_support].
                                                plmn_id,
                                        atoi(mcc), atoi(mnc), strlen(mnc));
                                }
                            } else if (!strcmp(plmn_key, "s_nssai")) {
                                ogs_yaml_iter_t s_nssai_array, s_nssai_iter;
                                ogs_yaml_iter_recurse(&plmn_iter,
                                        &s_nssai_array);
                                do {
                                    ogs_s_nssai_t *s_nssai = NULL;
                                    const char *sst = NULL, *sd = NULL;
                                    ogs_assert(
                                        self.plmn_support[
                                            self.num_of_plmn_support].
                                                num_of_s_nssai <=
                                            OGS_MAX_NUM_OF_S_NSSAI);
                                    s_nssai = &self.plmn_support[
                                            self.num_of_plmn_support].s_nssai[
                                                self.plmn_support[
                                                    self.num_of_plmn_support].
                                                        num_of_s_nssai];
                                    ogs_assert(s_nssai);

                                    if (ogs_yaml_iter_type(&s_nssai_array) ==
                                            YAML_MAPPING_NODE) {
                                        memcpy(&s_nssai_iter, &s_nssai_array,
                                                sizeof(ogs_yaml_iter_t));
                                    } else if (ogs_yaml_iter_type(
                                                &s_nssai_array) ==
                                                YAML_SEQUENCE_NODE) {
                                        if (!ogs_yaml_iter_next(&s_nssai_array))
                                            break;
                                        ogs_yaml_iter_recurse(&s_nssai_array,
                                                &s_nssai_iter);
                                    } else if (ogs_yaml_iter_type(
                                                &s_nssai_array) ==
                                                YAML_SCALAR_NODE) {
                                        break;
                                    } else
                                        ogs_assert_if_reached();

                                    while (ogs_yaml_iter_next(&s_nssai_iter)) {
                                        const char *s_nssai_key =
                                            ogs_yaml_iter_key(&s_nssai_iter);
                                        ogs_assert(s_nssai_key);
                                        if (!strcmp(s_nssai_key, "sst")) {
                                            sst = ogs_yaml_iter_value(
                                                    &s_nssai_iter);
                                        } else if (!strcmp(
                                                    s_nssai_key, "sd")) {
                                            sd = ogs_yaml_iter_value(
                                                    &s_nssai_iter);
                                        }
                                    }

                                    if (sst) {
                                        s_nssai->sst = atoi(sst);
                                        if (sd) {
                                            s_nssai->sd =
                                                ogs_uint24_from_string(
                                                        (char*)sd);
                                            s_nssai->len = 4;
                                        } else {
                                            s_nssai->sd.v =
                                                OGS_S_NSSAI_NO_SD_VALUE;
                                            s_nssai->len = 1;
                                        }

                                        self.plmn_support[
                                            self.num_of_plmn_support].
                                                num_of_s_nssai++;
                                    }

                                } while (ogs_yaml_iter_type(&s_nssai_array) ==
                                        YAML_SEQUENCE_NODE);
                            } else
                                ogs_warn("unknown key `%s`", plmn_key);
                        }

                        if (self.plmn_support[
                                self.num_of_plmn_support].num_of_s_nssai &&
                                mcc && mnc) {
                            self.num_of_plmn_support++;
                        } else {
                            ogs_warn("Ignore plmn : "
                                    "s_nssai(%d) mcc(%s), mnc(%s)",
                                self.plmn_support[
                                    self.num_of_plmn_support].num_of_s_nssai,
                                    mcc, mnc);
                            self.plmn_support[
                                self.num_of_plmn_support].num_of_s_nssai = 0;
                        }
                    } while (ogs_yaml_iter_type(&plmn_array) ==
                            YAML_SEQUENCE_NODE);
                } else if (!strcmp(amf_key, "security")) {
                    ogs_yaml_iter_t security_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &security_iter);
                    while (ogs_yaml_iter_next(&security_iter)) {
                        const char *security_key =
                            ogs_yaml_iter_key(&security_iter);
                        ogs_assert(security_key);
                        if (!strcmp(security_key, "integrity_order")) {
                            ogs_yaml_iter_t integrity_order_iter;
                            ogs_yaml_iter_recurse(&security_iter,
                                    &integrity_order_iter);
                            ogs_assert(ogs_yaml_iter_type(
                                        &integrity_order_iter) !=
                                YAML_MAPPING_NODE);

                            do {
                                const char *v = NULL;

                                if (ogs_yaml_iter_type(&integrity_order_iter) ==
                                        YAML_SEQUENCE_NODE) {
                                    if (!ogs_yaml_iter_next(
                                                &integrity_order_iter))
                                        break;
                                }

                                v = ogs_yaml_iter_value(&integrity_order_iter);
                                if (v) {
                                    int integrity_index = 
                                        self.num_of_integrity_order;
                                    if (strcmp(v, "NIA0") == 0) {
                                        self.integrity_order[integrity_index] = 
                                        OGS_NAS_SECURITY_ALGORITHMS_NIA0;
                                        self.num_of_integrity_order++;
                                    } else if (strcmp(v, "NIA1") == 0) {
                                        self.integrity_order[integrity_index] = 
                                        OGS_NAS_SECURITY_ALGORITHMS_128_NIA1;
                                        self.num_of_integrity_order++;
                                    } else if (strcmp(v, "NIA2") == 0) {
                                        self.integrity_order[integrity_index] = 
                                        OGS_NAS_SECURITY_ALGORITHMS_128_NIA2;
                                        self.num_of_integrity_order++;
                                    } else if (strcmp(v, "NIA3") == 0) {
                                        self.integrity_order[integrity_index] = 
                                        OGS_NAS_SECURITY_ALGORITHMS_128_NIA3;
                                        self.num_of_integrity_order++;
                                    }
                                }
                            } while (
                                ogs_yaml_iter_type(&integrity_order_iter) ==
                                    YAML_SEQUENCE_NODE);
                        } else if (!strcmp(security_key, "ciphering_order")) {
                            ogs_yaml_iter_t ciphering_order_iter;
                            ogs_yaml_iter_recurse(&security_iter,
                                    &ciphering_order_iter);
                            ogs_assert(ogs_yaml_iter_type(
                                &ciphering_order_iter) != YAML_MAPPING_NODE);

                            do {
                                const char *v = NULL;

                                if (ogs_yaml_iter_type(&ciphering_order_iter) ==
                                        YAML_SEQUENCE_NODE) {
                                    if (!ogs_yaml_iter_next(
                                                &ciphering_order_iter))
                                        break;
                                }

                                v = ogs_yaml_iter_value(&ciphering_order_iter);
                                if (v) {
                                    int ciphering_index = 
                                        self.num_of_ciphering_order;
                                    if (strcmp(v, "NEA0") == 0) {
                                        self.ciphering_order[ciphering_index] = 
                                            OGS_NAS_SECURITY_ALGORITHMS_NEA0;
                                        self.num_of_ciphering_order++;
                                    } else if (strcmp(v, "NEA1") == 0) {
                                        self.ciphering_order[ciphering_index] = 
                                        OGS_NAS_SECURITY_ALGORITHMS_128_NEA1;
                                        self.num_of_ciphering_order++;
                                    } else if (strcmp(v, "NEA2") == 0) {
                                        self.ciphering_order[ciphering_index] = 
                                        OGS_NAS_SECURITY_ALGORITHMS_128_NEA2;
                                        self.num_of_ciphering_order++;
                                    } else if (strcmp(v, "NEA3") == 0) {
                                        self.ciphering_order[ciphering_index] = 
                                        OGS_NAS_SECURITY_ALGORITHMS_128_NEA3;
                                        self.num_of_ciphering_order++;
                                    }
                                }
                            } while (
                                ogs_yaml_iter_type(&ciphering_order_iter) ==
                                    YAML_SEQUENCE_NODE);
                        }
                    }
                } else if (!strcmp(amf_key, "network_name")) {
                    ogs_yaml_iter_t network_name_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &network_name_iter);

                    while (ogs_yaml_iter_next(&network_name_iter)) {
                        const char *network_name_key =
                        ogs_yaml_iter_key(&network_name_iter);
                        ogs_assert(network_name_key);
                        if (!strcmp(network_name_key, "full")) {  
                            ogs_nas_network_name_t *network_full_name =
                                &self.full_name;
                            const char *c_network_name =
                                ogs_yaml_iter_value(&network_name_iter);
                            uint8_t size = strlen(c_network_name);
                            uint8_t i;
                            for (i = 0;i<size;i++) {
                                /* Workaround to convert the ASCII to USC-2 */
                                network_full_name->name[i*2] = 0;
                                network_full_name->name[(i*2)+1] =
                                    c_network_name[i];

                            }
                            network_full_name->length = size*2+1;
                            network_full_name->coding_scheme = 1;
                        } else if (!strcmp(network_name_key, "short")) {
                            ogs_nas_network_name_t *network_short_name =
                                &self.short_name;
                            const char *c_network_name =
                                ogs_yaml_iter_value(&network_name_iter);
                            uint8_t size = strlen(c_network_name);
                            uint8_t i;
                            for (i = 0;i<size;i++) {
                                /* Workaround to convert the ASCII to USC-2 */
                                network_short_name->name[i*2] = 0;
                                network_short_name->name[(i*2)+1] =
                                    c_network_name[i];

                            }
                            network_short_name->length = size*2+1;
                            network_short_name->coding_scheme = 1;
                        }
                    }
                } else if (!strcmp(amf_key, "amf_name")) {
                    self.amf_name = ogs_yaml_iter_value(&amf_iter);
                } else if (!strcmp(amf_key, "sbi")) {
                    /* handle config in sbi library */
                } else
                    ogs_warn("unknown key `%s`", amf_key);
            }
        }
    }

    rv = amf_context_validation();
    if (rv != OGS_OK) return rv;

    return OGS_OK;
}

amf_gnb_t *amf_gnb_add(ogs_sock_t *sock, ogs_sockaddr_t *addr)
{
    amf_gnb_t *gnb = NULL;
    amf_event_t e;

    ogs_assert(sock);
    ogs_assert(addr);

    ogs_pool_alloc(&amf_gnb_pool, &gnb);
    ogs_assert(gnb);
    memset(gnb, 0, sizeof *gnb);

    gnb->sock = sock;
    gnb->addr = addr;
    gnb->sock_type = amf_gnb_sock_type(gnb->sock);

    gnb->max_num_of_ostreams = DEFAULT_SCTP_MAX_NUM_OF_OSTREAMS;
    gnb->ostream_id = 0;
    if (ogs_config()->sockopt.sctp.max_num_of_ostreams) {
        gnb->max_num_of_ostreams =
            ogs_config()->sockopt.sctp.max_num_of_ostreams;
        ogs_info("[GNB] max_num_of_ostreams : %d", gnb->max_num_of_ostreams);
    }

    ogs_list_init(&gnb->ran_ue_list);

    if (gnb->sock_type == SOCK_STREAM) {
        gnb->poll = ogs_pollset_add(amf_self()->pollset,
            OGS_POLLIN, sock->fd, ngap_recv_upcall, sock);
        ogs_assert(gnb->poll);
    }

    ogs_hash_set(self.gnb_addr_hash, gnb->addr, sizeof(ogs_sockaddr_t), gnb);

    e.gnb = gnb;
    e.id = 0;
    ogs_fsm_create(&gnb->sm, ngap_state_initial, ngap_state_final);
    ogs_fsm_init(&gnb->sm, &e);

    ogs_list_add(&self.gnb_list, gnb);

    return gnb;
}

int amf_gnb_remove(amf_gnb_t *gnb)
{
    amf_event_t e;

    ogs_assert(gnb);
    ogs_assert(gnb->sock);

    ogs_list_remove(&self.gnb_list, gnb);

    e.gnb = gnb;
    ogs_fsm_fini(&gnb->sm, &e);
    ogs_fsm_delete(&gnb->sm);

    ogs_hash_set(self.gnb_addr_hash, gnb->addr, sizeof(ogs_sockaddr_t), NULL);
    ogs_hash_set(self.gnb_id_hash, &gnb->gnb_id, sizeof(gnb->gnb_id), NULL);

    ran_ue_remove_in_gnb(gnb);

    if (gnb->sock_type == SOCK_STREAM) {
        ogs_pollset_remove(gnb->poll);
        ogs_sctp_destroy(gnb->sock);
    }

    ogs_free(gnb->addr);

    ogs_pool_free(&amf_gnb_pool, gnb);

    return OGS_OK;
}

int amf_gnb_remove_all()
{
    amf_gnb_t *gnb = NULL, *next_gnb = NULL;

    ogs_list_for_each_safe(&self.gnb_list, next_gnb, gnb)
        amf_gnb_remove(gnb);

    return OGS_OK;
}

amf_gnb_t *amf_gnb_find_by_addr(ogs_sockaddr_t *addr)
{
    ogs_assert(addr);
    return (amf_gnb_t *)ogs_hash_get(self.gnb_addr_hash,
            addr, sizeof(ogs_sockaddr_t));

    return NULL;
}

amf_gnb_t *amf_gnb_find_by_gnb_id(uint32_t gnb_id)
{
    return (amf_gnb_t *)ogs_hash_get(self.gnb_id_hash, &gnb_id, sizeof(gnb_id));
}

int amf_gnb_set_gnb_id(amf_gnb_t *gnb, uint32_t gnb_id)
{
    ogs_assert(gnb);

    gnb->gnb_id = gnb_id;
    ogs_hash_set(self.gnb_id_hash, &gnb->gnb_id, sizeof(gnb->gnb_id), gnb);

    return OGS_OK;
}

int amf_gnb_sock_type(ogs_sock_t *sock)
{
    ogs_socknode_t *snode = NULL;

    ogs_assert(sock);

    ogs_list_for_each(&amf_self()->ngap_list, snode)
        if (snode->sock == sock) return SOCK_SEQPACKET;

    ogs_list_for_each(&amf_self()->ngap_list6, snode)
        if (snode->sock == sock) return SOCK_SEQPACKET;

    return SOCK_STREAM;
}

/** ran_ue_context handling function */
ran_ue_t *ran_ue_add(amf_gnb_t *gnb, uint32_t ran_ue_ngap_id)
{
    ran_ue_t *ran_ue = NULL;

    ogs_assert(self.amf_ue_ngap_id_hash);
    ogs_assert(gnb);

    ogs_pool_alloc(&ran_ue_pool, &ran_ue);
    ogs_assert(ran_ue);
    memset(ran_ue, 0, sizeof *ran_ue);

    ran_ue->ran_ue_ngap_id = ran_ue_ngap_id;
    ran_ue->amf_ue_ngap_id = OGS_NEXT_ID(self.amf_ue_ngap_id, 1, 0xffffffff);

    /*
     * SCTP output stream identification
     * Default ogs_config()->parameter.sctp_streams : 30
     *   0 : Non UE signalling
     *   1-29 : UE specific association 
     */
    ran_ue->gnb_ostream_id =
        OGS_NEXT_ID(gnb->ostream_id, 1, gnb->max_num_of_ostreams-1);

    ran_ue->gnb = gnb;

    ogs_hash_set(self.amf_ue_ngap_id_hash, &ran_ue->amf_ue_ngap_id,
            sizeof(ran_ue->amf_ue_ngap_id), ran_ue);
    ogs_list_add(&gnb->ran_ue_list, ran_ue);

    return ran_ue;
}

unsigned int ran_ue_count()
{
    ogs_assert(self.amf_ue_ngap_id_hash);
    return ogs_hash_count(self.amf_ue_ngap_id_hash);
}

void ran_ue_remove(ran_ue_t *ran_ue)
{
    ogs_assert(self.amf_ue_ngap_id_hash);
    ogs_assert(ran_ue);
    ogs_assert(ran_ue->gnb);

    /* De-associate S1 with NAS/EMM */
    ran_ue_deassociate(ran_ue);

    ogs_list_remove(&ran_ue->gnb->ran_ue_list, ran_ue);
    ogs_hash_set(self.amf_ue_ngap_id_hash, &ran_ue->amf_ue_ngap_id,
            sizeof(ran_ue->amf_ue_ngap_id), NULL);

    ogs_pool_free(&ran_ue_pool, ran_ue);
}

void ran_ue_remove_in_gnb(amf_gnb_t *gnb)
{
    ran_ue_t *ran_ue = NULL, *next_ran_ue = NULL;
    
    ran_ue = ran_ue_first_in_gnb(gnb);
    while (ran_ue) {
        next_ran_ue = ran_ue_next_in_gnb(ran_ue);

        ran_ue_remove(ran_ue);

        ran_ue = next_ran_ue;
    }
}

void ran_ue_switch_to_gnb(ran_ue_t *ran_ue, amf_gnb_t *new_gnb)
{
    ogs_assert(ran_ue);
    ogs_assert(ran_ue->gnb);
    ogs_assert(new_gnb);

    /* Remove from the old gnb */
    ogs_list_remove(&ran_ue->gnb->ran_ue_list, ran_ue);

    /* Add to the new gnb */
    ogs_list_add(&new_gnb->ran_ue_list, ran_ue);

    /* Switch to gnb */
    ran_ue->gnb = new_gnb;
}

ran_ue_t *ran_ue_find_by_ran_ue_ngap_id(
        amf_gnb_t *gnb, uint32_t ran_ue_ngap_id)
{
    ran_ue_t *ran_ue = NULL;
    
    ran_ue = ran_ue_first_in_gnb(gnb);
    while (ran_ue) {
        if (ran_ue_ngap_id == ran_ue->ran_ue_ngap_id)
            break;

        ran_ue = ran_ue_next_in_gnb(ran_ue);
    }

    return ran_ue;
}

ran_ue_t *ran_ue_find_by_amf_ue_ngap_id(uint64_t amf_ue_ngap_id)
{
    ogs_assert(self.amf_ue_ngap_id_hash);
    return ogs_hash_get(self.amf_ue_ngap_id_hash, 
            &amf_ue_ngap_id, sizeof(amf_ue_ngap_id));
}

ran_ue_t *ran_ue_first_in_gnb(amf_gnb_t *gnb)
{
    return ogs_list_first(&gnb->ran_ue_list);
}

ran_ue_t *ran_ue_next_in_gnb(ran_ue_t *ran_ue)
{
    return ogs_list_next(ran_ue);
}

static int amf_ue_new_guti(amf_ue_t *amf_ue)
{
    if (amf_ue->m_tmsi) {
        /* AMF has a VALID GUTI
         * As such, we need to remove previous GUTI in hash table */
        ogs_hash_set(self.guti_ue_hash,
                &amf_ue->guti, sizeof(ogs_nas_5gs_guti_t), NULL);
        ogs_assert(amf_m_tmsi_free(amf_ue->m_tmsi) == OGS_OK);
    }

    memset(&amf_ue->guti, 0, sizeof(ogs_nas_5gs_guti_t));

    ogs_assert(amf_ue->guami);
    ogs_nas_from_plmn_id(&amf_ue->guti.nas_plmn_id, &amf_ue->guami->plmn_id);
    memcpy(&amf_ue->guti.amf_id, &amf_ue->guami->amf_id, sizeof(ogs_amf_id_t));

    amf_ue->m_tmsi = amf_m_tmsi_alloc();
    ogs_assert(amf_ue->m_tmsi);
    amf_ue->guti.m_tmsi = *(amf_ue->m_tmsi);
    ogs_hash_set(self.guti_ue_hash,
            &amf_ue->guti, sizeof(ogs_nas_5gs_guti_t), amf_ue);

    return OGS_OK;
}

amf_ue_t *amf_ue_add(ran_ue_t *ran_ue)
{
    amf_gnb_t *gnb = NULL;
    amf_ue_t *amf_ue = NULL;
    amf_event_t e;

    ogs_assert(ran_ue);
    gnb = ran_ue->gnb;
    ogs_assert(gnb);

    ogs_pool_alloc(&amf_ue_pool, &amf_ue);
    ogs_assert(amf_ue);
    memset(amf_ue, 0, sizeof *amf_ue);

    ogs_list_init(&amf_ue->sess_list);

    /* TODO : Hard-coded */
    amf_ue->guami = &amf_self()->served_guami[0];
    amf_ue->nas.access_type = OGS_ACCESS_TYPE_3GPP;
    amf_ue->abba_len = 2;

    /* Create New GUTI */
    amf_ue_new_guti(amf_ue);

#if 0
    /* Clear VLR */
    amf_ue->csmap = NULL;
    amf_ue->vlr_ostream_id = 0;
#endif

    /* Add All Timers */
    amf_ue->sbi_client_wait.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_sbi_client_wait_expire, amf_ue);

    amf_ue->t3513.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_t3513_expire, amf_ue);
    amf_ue->t3513.pkbuf = NULL;
    amf_ue->t3522.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_t3522_expire, amf_ue);
    amf_ue->t3522.pkbuf = NULL;
    amf_ue->t3550.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_t3550_expire, amf_ue);
    amf_ue->t3550.pkbuf = NULL;
    amf_ue->t3555.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_t3555_expire, amf_ue);
    amf_ue->t3555.pkbuf = NULL;
    amf_ue->t3560.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_t3560_expire, amf_ue);
    amf_ue->t3560.pkbuf = NULL;
    amf_ue->t3570.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_t3570_expire, amf_ue);
    amf_ue->t3570.pkbuf = NULL;

    /* Create FSM */
    e.amf_ue = amf_ue;
    ogs_fsm_create(&amf_ue->sm, gmm_state_initial, gmm_state_final);
    ogs_fsm_init(&amf_ue->sm, &e);

    ogs_list_add(&self.amf_ue_list, amf_ue);

    return amf_ue;
}

void amf_ue_remove(amf_ue_t *amf_ue)
{
    amf_event_t e;
    int i;

    ogs_assert(amf_ue);

    ogs_list_remove(&self.amf_ue_list, amf_ue);

    e.amf_ue = amf_ue;
    ogs_fsm_fini(&amf_ue->sm, &e);
    ogs_fsm_delete(&amf_ue->sm);

    /* Clear hash table */
    if (amf_ue->m_tmsi) {
        ogs_hash_set(self.guti_ue_hash,
                &amf_ue->guti, sizeof(ogs_nas_5gs_guti_t), NULL);
        ogs_assert(amf_m_tmsi_free(amf_ue->m_tmsi) == OGS_OK);
    }
    if (amf_ue->suci) {
        ogs_hash_set(self.suci_hash, amf_ue->suci, strlen(amf_ue->suci), NULL);
        ogs_free(amf_ue->suci);
    }
    if (amf_ue->supi) {
        ogs_hash_set(self.supi_hash, amf_ue->supi, strlen(amf_ue->supi), NULL);
        ogs_free(amf_ue->supi);
    }

    if (amf_ue->sbi.request)
        ogs_sbi_request_free(amf_ue->sbi.request);

    if (amf_ue->confirmation_url_for_5g_aka)
        ogs_free(amf_ue->confirmation_url_for_5g_aka);

    /* Free UeRadioCapability */
    OGS_ASN_CLEAR_DATA(&amf_ue->ueRadioCapability);

    /* Clear Transparent Container */
    OGS_ASN_CLEAR_DATA(&amf_ue->container);

    /* Delete All Timers */
    CLEAR_AMF_UE_ALL_TIMERS(amf_ue);
    ogs_timer_delete(amf_ue->sbi_client_wait.timer);
    ogs_timer_delete(amf_ue->t3513.timer);
    ogs_timer_delete(amf_ue->t3522.timer);
    ogs_timer_delete(amf_ue->t3550.timer);
    ogs_timer_delete(amf_ue->t3555.timer);
    ogs_timer_delete(amf_ue->t3560.timer);
    ogs_timer_delete(amf_ue->t3570.timer);

    amf_ue_deassociate(amf_ue);

    amf_sess_remove_all(amf_ue);
    amf_pdn_remove_all(amf_ue);

    for (i = 0; i < OGS_SBI_MAX_NF_TYPE; i++) {
        if (amf_ue->nf_types[i].nf_instance)
            ogs_sbi_nf_instance_remove(amf_ue->nf_types[i].nf_instance);
    }

    ogs_pool_free(&amf_ue_pool, amf_ue);
}

void amf_ue_remove_all()
{
    amf_ue_t *amf_ue = NULL, *next = NULL;;

    ogs_list_for_each_safe(&self.amf_ue_list, next, amf_ue)
        amf_ue_remove(amf_ue);
}

amf_ue_t *amf_ue_find_by_guti(ogs_nas_5gs_guti_t *guti)
{
    ogs_assert(guti);

    return (amf_ue_t *)ogs_hash_get(
            self.guti_ue_hash, guti, sizeof(ogs_nas_5gs_guti_t));
}

amf_ue_t *amf_ue_find_by_teid(uint32_t teid)
{
    return ogs_pool_find(&amf_ue_pool, teid);
}

amf_ue_t *amf_ue_find_by_suci(char *suci)
{
    ogs_assert(suci);
    return (amf_ue_t *)ogs_hash_get(self.suci_hash, suci, strlen(suci));
}

amf_ue_t *amf_ue_find_by_supi(char *supi)
{
    ogs_assert(supi);
    return (amf_ue_t *)ogs_hash_get(self.supi_hash, supi, strlen(supi));
}

amf_ue_t *amf_ue_find_by_message(ogs_nas_5gs_message_t *message)
{
    amf_ue_t *amf_ue = NULL;
    ogs_nas_5gs_registration_request_t *registration_request = NULL;
#if 0
    ogs_nas_5gs_tracking_area_update_request_t *tau_request = NULL;
    ogs_nas_5gs_extended_service_request_t *extended_service_request = NULL;
#endif
    ogs_nas_5gs_mobile_identity_t *mobile_identity = NULL;
    ogs_nas_5gs_mobile_identity_header_t *mobile_identity_header = NULL;
    ogs_nas_5gs_mobile_identity_guti_t *mobile_identity_guti = NULL;
    ogs_nas_5gs_guti_t nas_guti;

    char *suci = NULL;

    ogs_assert(message);

    registration_request = &message->gmm.registration_request;
    ogs_assert(registration_request);
    mobile_identity = &registration_request->mobile_identity;
    ogs_assert(mobile_identity);
    mobile_identity_header =
            (ogs_nas_5gs_mobile_identity_header_t *)mobile_identity->buffer;
    ogs_assert(mobile_identity_header);

    switch (message->gmm.h.message_type) {
    case OGS_NAS_5GS_REGISTRATION_REQUEST:
        switch (mobile_identity_header->type) {
        case OGS_NAS_5GS_MOBILE_IDENTITY_SUCI:
            suci = ogs_nas_5gs_suci_from_mobile_identity(mobile_identity);
            amf_ue = amf_ue_find_by_suci(suci);
            if (amf_ue) {
                ogs_trace("[%s] known UE by SUCI", suci);
            } else {
                ogs_trace("[%s] Unknown UE by SUCI", suci);
            }
            ogs_free(suci);
            break;
        case OGS_NAS_5GS_MOBILE_IDENTITY_GUTI:
            mobile_identity_guti =
                (ogs_nas_5gs_mobile_identity_guti_t *)mobile_identity->buffer;
            ogs_assert(mobile_identity_guti);

            memcpy(&nas_guti.nas_plmn_id,
                    &mobile_identity_guti->nas_plmn_id, OGS_PLMN_ID_LEN);
            memcpy(&nas_guti.amf_id,
                    &mobile_identity_guti->amf_id, sizeof(ogs_amf_id_t));
            nas_guti.m_tmsi = be32toh(mobile_identity_guti->m_tmsi);

            amf_ue = amf_ue_find_by_guti(&nas_guti);
            if (amf_ue) {
                ogs_debug("Known UE by 5G-S_TMSI[AMF_ID:0x%x,M_TMSI:0x%x]",
                    ogs_amf_id_hexdump(&nas_guti.amf_id), nas_guti.m_tmsi);
            } else {
                ogs_warn("Unknown UE by 5G-S_TMSI[AMF_ID:0x%x,M_TMSI:0x%x]",
                    ogs_amf_id_hexdump(&nas_guti.amf_id), nas_guti.m_tmsi);
            }
            break;
        default:
            ogs_error("Unknown SUCI type [%d]", mobile_identity_header->type);
            break;
        }
        break;
#if 0
    case OGS_NAS_5GS_TRACKING_AREA_UPDATE_REQUEST:
        tau_request = &message->gmm.tracking_area_update_request;
        eps_mobile_identity = &tau_request->old_guti;

        switch(eps_mobile_identity->imsi.type) {
        case OGS_NAS_5GS_MOBILE_IDENTITY_GUTI:
            eps_mobile_identity_guti = &eps_mobile_identity->guti;

            ogs_nas_guti.nas_plmn_id = eps_mobile_identity_guti->nas_plmn_id;
            ogs_nas_guti.amf_gid = eps_mobile_identity_guti->amf_gid;
            ogs_nas_guti.amf_code = eps_mobile_identity_guti->amf_code;
            ogs_nas_guti.m_tmsi = eps_mobile_identity_guti->m_tmsi;

            amf_ue = amf_ue_find_by_guti(&ogs_nas_guti);
            if (amf_ue) {
                ogs_trace("Known UE by GUTI[G:%d,C:%d,M_TMSI:0x%x]",
                        ogs_nas_guti.amf_gid,
                        ogs_nas_guti.amf_code,
                        ogs_nas_guti.m_tmsi);
            } else {
                ogs_warn("Unknown UE by GUTI[G:%d,C:%d,M_TMSI:0x%x]",
                        ogs_nas_guti.amf_gid,
                        ogs_nas_guti.amf_code,
                        ogs_nas_guti.m_tmsi);
            }
            break;
        default:
            ogs_error("Unknown IMSI type [%d]", eps_mobile_identity->imsi.type);
            break;
        }
        break;
    case OGS_NAS_5GS_EXTENDED_SERVICE_REQUEST:
        extended_service_request = &message->gmm.extended_service_request;
        mobile_identity = &extended_service_request->m_tmsi;

        switch(mobile_identity->tmsi.type) {
        case OGS_NAS_MOBILE_IDENTITY_TMSI:
            mobile_identity_tmsi = &mobile_identity->tmsi;
            served_guami = &amf_self()->served_guami[0];

            /* Use the first configured plmn_id and amf group id */
            ogs_nas_from_plmn_id(
                    &ogs_nas_guti.nas_plmn_id, &served_guami->plmn_id[0]);
            ogs_nas_guti.amf_gid = served_guami->amf_gid[0];
            ogs_nas_guti.amf_code = served_guami->amf_code[0];
            ogs_nas_guti.m_tmsi = mobile_identity_tmsi->tmsi;

            amf_ue = amf_ue_find_by_guti(&ogs_nas_guti);
            if (amf_ue) {
                ogs_trace("Known UE by GUTI[G:%d,C:%d,M_TMSI:0x%x]",
                        ogs_nas_guti.amf_gid,
                        ogs_nas_guti.amf_code,
                        ogs_nas_guti.m_tmsi);
            } else {
                ogs_warn("Unknown UE by GUTI[G:%d,C:%d,M_TMSI:0x%x]",
                        ogs_nas_guti.amf_gid,
                        ogs_nas_guti.amf_code,
                        ogs_nas_guti.m_tmsi);
            }
            break;
        default:
            ogs_error("Unknown TMSI type [%d]", mobile_identity->tmsi.type);
            break;
        }
        break;
#endif
    default:
        break;
    }

    return amf_ue;
}

void amf_ue_set_suci(amf_ue_t *amf_ue,
        ogs_nas_5gs_mobile_identity_t *mobile_identity)
{
    amf_ue_t *old_amf_ue = NULL;
    char *suci = NULL;

    ogs_assert(amf_ue);
    ogs_assert(mobile_identity);

    suci = ogs_nas_5gs_suci_from_mobile_identity(mobile_identity);
    ogs_assert(suci);

    /* Check if OLD amf_ue_t is existed */
    old_amf_ue = amf_ue_find_by_suci(suci);
    if (old_amf_ue) {
        /* Check if OLD amf_ue_t is different with NEW amf_ue_t */
        if (ogs_pool_index(&amf_ue_pool, amf_ue) !=
            ogs_pool_index(&amf_ue_pool, old_amf_ue)) {
            ogs_warn("[%s] OLD UE Context Release", suci);
            if (old_amf_ue->ran_ue)
                ran_ue_deassociate(old_amf_ue->ran_ue);
            amf_ue_remove(old_amf_ue);
        }
    }

    if (amf_ue->suci) {
        ogs_hash_set(self.suci_hash, amf_ue->suci, strlen(amf_ue->suci), NULL);
        ogs_free(amf_ue->suci);
    }
    amf_ue->suci = suci;
    ogs_hash_set(self.suci_hash, amf_ue->suci, strlen(amf_ue->suci), amf_ue);

    amf_ue->guti_present = 1;
}

void amf_ue_set_supi(amf_ue_t *amf_ue, char *supi)
{
    ogs_assert(supi);

    if (amf_ue->supi) {
        ogs_hash_set(self.supi_hash, amf_ue->supi, strlen(amf_ue->supi), NULL);
        ogs_free(amf_ue->supi);
    }
    amf_ue->supi = ogs_strdup(supi);
    ogs_assert(amf_ue->supi);
    ogs_hash_set(self.supi_hash, amf_ue->supi, strlen(amf_ue->supi), amf_ue);
}

int amf_ue_have_indirect_tunnel(amf_ue_t *amf_ue)
{
    amf_sess_t *sess = NULL;

    sess = amf_sess_first(amf_ue);
    while (sess) {
        amf_bearer_t *bearer = amf_bearer_first(sess);
        while (bearer) {
            if (AMF_HAVE_GNB_DL_INDIRECT_TUNNEL(bearer) ||
                AMF_HAVE_GNB_UL_INDIRECT_TUNNEL(bearer) ||
                AMF_HAVE_SMF_DL_INDIRECT_TUNNEL(bearer) ||
                AMF_HAVE_SMF_UL_INDIRECT_TUNNEL(bearer)) {
                return 1;
            }

            bearer = amf_bearer_next(bearer);
        }
        sess = amf_sess_next(sess);
    }

    return 0;
}

int amf_ue_clear_indirect_tunnel(amf_ue_t *amf_ue)
{
    amf_sess_t *sess = NULL;

    ogs_assert(amf_ue);

    sess = amf_sess_first(amf_ue);
    while (sess) {
        amf_bearer_t *bearer = amf_bearer_first(sess);
        while (bearer) {
            CLEAR_INDIRECT_TUNNEL(bearer);

            bearer = amf_bearer_next(bearer);
        }
        sess = amf_sess_next(sess);
    }

    return OGS_OK;
}

void amf_ue_associate_ran_ue(amf_ue_t *amf_ue, ran_ue_t *ran_ue)
{
    ogs_assert(amf_ue);
    ogs_assert(ran_ue);

    amf_ue->ran_ue = ran_ue;
    ran_ue->amf_ue = amf_ue;
}

void ran_ue_deassociate(ran_ue_t *ran_ue)
{
    ogs_assert(ran_ue);
    ran_ue->amf_ue = NULL;
}

void amf_ue_deassociate(amf_ue_t *amf_ue)
{
    ogs_assert(amf_ue);
    amf_ue->ran_ue = NULL;
}

void source_ue_associate_target_ue(
        ran_ue_t *source_ue, ran_ue_t *target_ue)
{
    amf_ue_t *amf_ue = NULL;

    ogs_assert(source_ue);
    ogs_assert(target_ue);
    amf_ue = source_ue->amf_ue;
    ogs_assert(amf_ue);

    target_ue->amf_ue = amf_ue;
    target_ue->source_ue = source_ue;
    source_ue->target_ue = target_ue;
}

void source_ue_deassociate_target_ue(ran_ue_t *ran_ue)
{
    ran_ue_t *source_ue = NULL;
    ran_ue_t *target_ue = NULL;
    ogs_assert(ran_ue);

    if (ran_ue->target_ue) {
        source_ue = ran_ue;
        target_ue = ran_ue->target_ue;

        ogs_assert(source_ue->target_ue);
        ogs_assert(target_ue->source_ue);
        source_ue->target_ue = NULL;
        target_ue->source_ue = NULL;
    } else if (ran_ue->source_ue) {
        target_ue = ran_ue;
        source_ue = ran_ue->source_ue;

        ogs_assert(source_ue->target_ue);
        ogs_assert(target_ue->source_ue);
        source_ue->target_ue = NULL;
        target_ue->source_ue = NULL;
    }
}

amf_sess_t *amf_sess_add(amf_ue_t *amf_ue, uint8_t pti)
{
    amf_sess_t *sess = NULL;
    amf_bearer_t *bearer = NULL;

    ogs_assert(amf_ue);
    ogs_assert(pti != OGS_NAS_PROCEDURE_TRANSACTION_IDENTITY_UNASSIGNED);

    ogs_pool_alloc(&amf_sess_pool, &sess);
    ogs_assert(sess);
    memset(sess, 0, sizeof *sess);

    ogs_list_init(&sess->bearer_list);

    sess->amf_ue = amf_ue;
    sess->pti = pti;

    bearer = amf_bearer_add(sess);
    ogs_assert(bearer);

    ogs_list_add(&amf_ue->sess_list, sess);

    return sess;
}

void amf_sess_remove(amf_sess_t *sess)
{
    ogs_assert(sess);
    ogs_assert(sess->amf_ue);
    
    ogs_list_remove(&sess->amf_ue->sess_list, sess);

    amf_bearer_remove_all(sess);

    OGS_NAS_CLEAR_DATA(&sess->ue_pco);
    OGS_TLV_CLEAR_DATA(&sess->pgw_pco);

    ogs_pool_free(&amf_sess_pool, sess);
}

void amf_sess_remove_all(amf_ue_t *amf_ue)
{
    amf_sess_t *sess = NULL, *next_sess = NULL;
    
    sess = amf_sess_first(amf_ue);
    while (sess) {
        next_sess = amf_sess_next(sess);

        amf_sess_remove(sess);

        sess = next_sess;
    }
}

amf_sess_t *amf_sess_find_by_pti(amf_ue_t *amf_ue, uint8_t pti)
{
    amf_sess_t *sess = NULL;

    sess = amf_sess_first(amf_ue);
    while(sess) {
        if (pti == sess->pti)
            return sess;

        sess = amf_sess_next(sess);
    }

    return NULL;
}

amf_sess_t *amf_sess_find_by_ebi(amf_ue_t *amf_ue, uint8_t ebi)
{
    amf_bearer_t *bearer = NULL;

    bearer = amf_bearer_find_by_ue_ebi(amf_ue, ebi);
    if (bearer)
        return bearer->sess;

    return NULL;
}

amf_sess_t *amf_sess_find_by_dnn(amf_ue_t *amf_ue, char *dnn)
{
    amf_sess_t *sess = NULL;

    sess = amf_sess_first(amf_ue);
    while (sess) {
        if (sess->pdn && strcmp(sess->pdn->apn, dnn) == 0)
            return sess;

        sess = amf_sess_next(sess);
    }

    return NULL;
}

amf_sess_t *amf_sess_first(amf_ue_t *amf_ue)
{
    return ogs_list_first(&amf_ue->sess_list);
}

amf_sess_t *amf_sess_next(amf_sess_t *sess)
{
    return ogs_list_next(sess);
}

unsigned int amf_sess_count(amf_ue_t *amf_ue)
{
    unsigned int count = 0;
    amf_sess_t *sess = NULL;

    sess = amf_sess_first(amf_ue);
    while (sess) {
        sess = amf_sess_next(sess);
        count++;
    }

    return count;
}

amf_bearer_t *amf_bearer_add(amf_sess_t *sess)
{
    amf_event_t e;

    amf_bearer_t *bearer = NULL;
    amf_ue_t *amf_ue = NULL;

    ogs_assert(sess);
    amf_ue = sess->amf_ue;
    ogs_assert(amf_ue);

    ogs_pool_alloc(&amf_bearer_pool, &bearer);
    ogs_assert(bearer);
    memset(bearer, 0, sizeof *bearer);

    bearer->ebi = OGS_NEXT_ID(amf_ue->ebi,
            MIN_5GS_BEARER_ID, MAX_5GS_BEARER_ID);

    bearer->amf_ue = amf_ue;
    bearer->sess = sess;

    ogs_list_add(&sess->bearer_list, bearer);

    bearer->t3589.timer = ogs_timer_add(
            self.timer_mgr, amf_timer_t3589_expire, bearer);
    bearer->t3589.pkbuf = NULL;
    
    e.bearer = bearer;
    e.id = 0;
    ogs_fsm_create(&bearer->sm, gsm_state_initial, gsm_state_final);
    ogs_fsm_init(&bearer->sm, &e);

    return bearer;
}

void amf_bearer_remove(amf_bearer_t *bearer)
{
    amf_event_t e;

    ogs_assert(bearer);
    ogs_assert(bearer->sess);

    e.bearer = bearer;
    ogs_fsm_fini(&bearer->sm, &e);
    ogs_fsm_delete(&bearer->sm);

    CLEAR_BEARER_ALL_TIMERS(bearer);
    ogs_timer_delete(bearer->t3589.timer);

    ogs_list_remove(&bearer->sess->bearer_list, bearer);

    OGS_TLV_CLEAR_DATA(&bearer->tft);
    
    ogs_pool_free(&amf_bearer_pool, bearer);
}

void amf_bearer_remove_all(amf_sess_t *sess)
{
    amf_bearer_t *bearer = NULL, *next_bearer = NULL;

    ogs_assert(sess);
    
    bearer = amf_bearer_first(sess);
    while (bearer) {
        next_bearer = amf_bearer_next(bearer);

        amf_bearer_remove(bearer);

        bearer = next_bearer;
    }
}

amf_bearer_t *amf_bearer_find_by_sess_ebi(amf_sess_t *sess, uint8_t ebi)
{
    amf_bearer_t *bearer = NULL;

    ogs_assert(sess);

    bearer = amf_bearer_first(sess);
    while (bearer) {
        if (ebi == bearer->ebi)
            return bearer;

        bearer = amf_bearer_next(bearer);
    }

    return NULL;
}

amf_bearer_t *amf_bearer_find_by_ue_ebi(amf_ue_t *amf_ue, uint8_t ebi)
{
    amf_sess_t *sess = NULL;
    amf_bearer_t *bearer = NULL;
    
    ogs_assert(amf_ue);

    sess = amf_sess_first(amf_ue);
    while (sess) {
        bearer = amf_bearer_find_by_sess_ebi(sess, ebi);
        if (bearer) {
            return bearer;
        }

        sess = amf_sess_next(sess);
    }

    return NULL;
}

amf_bearer_t *amf_bearer_find_or_add_by_message(
        amf_ue_t *amf_ue, ogs_nas_5gs_message_t *message)
{
#if 0
    uint8_t pti = OGS_NAS_PROCEDURE_TRANSACTION_IDENTITY_UNASSIGNED;
    uint8_t ebi = OGS_NAS_5GS_BEARER_IDENTITY_UNASSIGNED;
#endif

    amf_bearer_t *bearer = NULL;
#if 0
    amf_sess_t *sess = NULL;
#endif

#if 0
    ogs_assert(amf_ue);
    ogs_assert(message);

    pti = message->esm.h.procedure_transaction_identity;
    ebi = message->esm.h.eps_bearer_identity;

    ogs_debug("amf_bearer_find_or_add_by_message() [PTI:%d, EBI:%d]",
            pti, ebi);

    if (ebi != OGS_NAS_5GS_BEARER_IDENTITY_UNASSIGNED) {
        bearer = amf_bearer_find_by_ue_ebi(amf_ue, ebi);
        if (!bearer) {
            ogs_error("No Bearer : EBI[%d]", ebi);
            nas_eps_send_attach_reject(amf_ue,
                EMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED,
                ESM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);
            return NULL;
        }

        return bearer;
    }

    if (pti == OGS_NAS_PROCEDURE_TRANSACTION_IDENTITY_UNASSIGNED) {
        ogs_error("Both PTI[%d] and EBI[%d] are 0", pti, ebi);
        nas_eps_send_attach_reject(amf_ue,
            EMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED,
            ESM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);
        return NULL;
    }

    if (message->esm.h.message_type == OGS_NAS_5GS_PDN_DISCONNECT_REQUEST) {
        ogs_nas_eps_pdn_disconnect_request_t *pdn_disconnect_request = 
            &message->esm.pdn_disconnect_request;
        ogs_nas_linked_eps_bearer_identity_t *linked_eps_bearer_identity =
            &pdn_disconnect_request->linked_eps_bearer_identity;

        bearer = amf_bearer_find_by_ue_ebi(amf_ue,
                linked_eps_bearer_identity->eps_bearer_identity);
        if (!bearer) {
            ogs_error("No Bearer : Linked-EBI[%d]", 
                    linked_eps_bearer_identity->eps_bearer_identity);
            nas_eps_send_attach_reject(amf_ue,
                EMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED,
                ESM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);
            return NULL;
        }
    } else if (message->esm.h.message_type ==
            OGS_NAS_5GS_BEARER_RESOURCE_ALLOCATION_REQUEST) {
        ogs_nas_eps_bearer_resource_allocation_request_t
            *bearer_allocation_request =
                &message->esm.bearer_resource_allocation_request;
        ogs_nas_linked_eps_bearer_identity_t *linked_eps_bearer_identity =
            &bearer_allocation_request->linked_eps_bearer_identity;

        bearer = amf_bearer_find_by_ue_ebi(amf_ue,
                linked_eps_bearer_identity->eps_bearer_identity);
        if (!bearer) {
            ogs_error("No Bearer : Linked-EBI[%d]", 
                    linked_eps_bearer_identity->eps_bearer_identity);
            nas_eps_send_attach_reject(amf_ue,
                EMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED,
                ESM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);
            return NULL;
        }
    } else if (message->esm.h.message_type ==
            OGS_NAS_5GS_BEARER_RESOURCE_MODIFICATION_REQUEST) {
        ogs_nas_eps_bearer_resource_modification_request_t
            *bearer_modification_request =
                &message->esm.bearer_resource_modification_request;
        ogs_nas_linked_eps_bearer_identity_t *linked_eps_bearer_identity =
            &bearer_modification_request->eps_bearer_identity_for_packet_filter;

        bearer = amf_bearer_find_by_ue_ebi(amf_ue,
                linked_eps_bearer_identity->eps_bearer_identity);
        if (!bearer) {
            ogs_error("No Bearer : Linked-EBI[%d]", 
                    linked_eps_bearer_identity->eps_bearer_identity);
            nas_eps_send_attach_reject(amf_ue,
                EMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED,
                ESM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);
            return NULL;
        }
    }

    if (bearer) {
        sess = bearer->sess;
        ogs_assert(sess);
        sess->pti = pti;

        return bearer;
    }

    if (message->esm.h.message_type == OGS_NAS_5GS_PDN_CONNECTIVITY_REQUEST) {
        ogs_nas_eps_pdn_connectivity_request_t *pdn_connectivity_request =
            &message->esm.pdn_connectivity_request;
        if (pdn_connectivity_request->presencemask &
                OGS_NAS_5GS_PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_PRESENT)
            sess = amf_sess_find_by_dnn(amf_ue,
                    pdn_connectivity_request->access_point_name.dnn);
        else
            sess = amf_sess_first(amf_ue);

        if (!sess)
            sess = amf_sess_add(amf_ue, pti);
        else
            sess->pti = pti;

        ogs_assert(sess);
    } else {
        sess = amf_sess_find_by_pti(amf_ue, pti);
        ogs_assert(sess);
    }

    bearer = amf_default_bearer_in_sess(sess);
    ogs_assert(bearer);
#endif

    return bearer;
}

amf_bearer_t *amf_default_bearer_in_sess(amf_sess_t *sess)
{
    ogs_assert(sess);
    return amf_bearer_first(sess);
}

amf_bearer_t *amf_linked_bearer(amf_bearer_t *bearer)
{
    amf_sess_t *sess = NULL;

    ogs_assert(bearer);
    sess = bearer->sess;
    ogs_assert(sess);

    return amf_default_bearer_in_sess(sess);
}

amf_bearer_t *amf_bearer_first(amf_sess_t *sess)
{
    ogs_assert(sess);

    return ogs_list_first(&sess->bearer_list);
}

amf_bearer_t *amf_bearer_next(amf_bearer_t *bearer)
{
    ogs_assert(bearer);
    return ogs_list_next(bearer);
}

int amf_bearer_is_inactive(amf_ue_t *amf_ue)
{
    amf_sess_t *sess = NULL;
    ogs_assert(amf_ue);

    sess = amf_sess_first(amf_ue);
    while (sess) {
        amf_bearer_t *bearer = amf_bearer_first(sess);
        while (bearer) {
            if (AMF_HAVE_GNB_S1U_PATH(bearer)) {
                return 0;
            }

            bearer = amf_bearer_next(bearer);
        }
        sess = amf_sess_next(sess);
    }

    return 1;
}

int amf_bearer_set_inactive(amf_ue_t *amf_ue)
{
    amf_sess_t *sess = NULL;

    ogs_assert(amf_ue);
    sess = amf_sess_first(amf_ue);
    while (sess) {
        amf_bearer_t *bearer = amf_bearer_first(sess);
        while (bearer) {
            CLEAR_GNB_S1U_PATH(bearer);

            bearer = amf_bearer_next(bearer);
        }
        sess = amf_sess_next(sess);
    }

    return OGS_OK;
}

void amf_pdn_remove_all(amf_ue_t *amf_ue)
{
#if 0
    ogs_diam_s6a_subscription_data_t *subscription_data = NULL;

    ogs_assert(amf_ue);
    subscription_data = &amf_ue->subscription_data;
    ogs_assert(subscription_data);

    subscription_data->num_of_pdn = 0;
#endif
}

ogs_pdn_t *amf_pdn_find_by_dnn(amf_ue_t *amf_ue, char *dnn)
{
#if 0
    ogs_diam_s6a_subscription_data_t *subscription_data = NULL;
    ogs_pdn_t *pdn = NULL;
    int i = 0;
    
    ogs_assert(amf_ue);
    ogs_assert(dnn);

    subscription_data = &amf_ue->subscription_data;
    ogs_assert(subscription_data);

    for (i = 0; i < subscription_data->num_of_pdn; i++) {
        pdn = &subscription_data->pdn[i];
        if (strcmp(pdn->dnn, dnn) == 0)
            return pdn;
    }
#endif

    return NULL;
}

ogs_pdn_t *amf_default_pdn(amf_ue_t *amf_ue)
{
#if 0
    ogs_diam_s6a_subscription_data_t *subscription_data = NULL;
    ogs_pdn_t *pdn = NULL;
    int i = 0;
    
    ogs_assert(amf_ue);
    subscription_data = &amf_ue->subscription_data;
    ogs_assert(subscription_data);

    for (i = 0; i < subscription_data->num_of_pdn; i++) {
        pdn = &subscription_data->pdn[i];
        if (pdn->context_identifier == subscription_data->context_identifier)
            return pdn;
    }
#endif

    return NULL;
}

int amf_find_served_tai(ogs_5gs_tai_t *tai)
{
    int i = 0, j = 0, k = 0;

    ogs_assert(tai);

    for (i = 0; i < self.num_of_served_tai; i++) {
        ogs_5gs_tai0_list_t *list0 = &self.served_tai[i].list0;
        ogs_assert(list0);
        ogs_5gs_tai2_list_t *list2 = &self.served_tai[i].list2;
        ogs_assert(list2);

        for (j = 0; list0->tai[j].num; j++) {
            ogs_assert(list0->tai[j].type == OGS_TAI0_TYPE);
            ogs_assert(list0->tai[j].num < OGS_MAX_NUM_OF_TAI);

            for (k = 0; k < list0->tai[j].num; k++) {
                if (memcmp(&list0->tai[j].plmn_id,
                            &tai->plmn_id, OGS_PLMN_ID_LEN) == 0 && 
                    list0->tai[j].tac[k].v == tai->tac.v) {
                    return i;
                }
            }
        }

        if (list2->num) {
            ogs_assert(list2->type == OGS_TAI1_TYPE ||
                        list2->type == OGS_TAI2_TYPE);
            ogs_assert(list2->num < OGS_MAX_NUM_OF_TAI);

            for (j = 0; j < list2->num; j++) {
                if (memcmp(&list2->tai[j].plmn_id,
                            &tai->plmn_id, OGS_PLMN_ID_LEN) == 0 && 
                    list2->tai[j].tac.v == tai->tac.v) {
                    return i;
                }
            }
        }
    }

    return -1;
}

int amf_m_tmsi_pool_generate()
{
    int i, j;
    int index = 0;

    ogs_trace("M-TMSI Pool try to generate...");
    for (i = 0; index < ogs_config()->pool.ue; i++) {
        amf_m_tmsi_t *m_tmsi = NULL;
        int conflict = 0;

        m_tmsi = &self.m_tmsi.array[index];
        ogs_assert(m_tmsi);
        *m_tmsi = ogs_random32();

        /* for mapped-GUTI */
        *m_tmsi |= 0xc0000000;
        *m_tmsi &= 0xff00ffff;

        for (j = 0; j < index; j++) {
            if (*m_tmsi == self.m_tmsi.array[j]) {
                conflict = 1;
                ogs_trace("[M-TMSI CONFLICT]  %d:0x%x == %d:0x%x",
                        index, *m_tmsi, j, self.m_tmsi.array[j]);
                break;
            }
        }
        if (conflict == 1) {
            continue;
        }

        index++;
    }
    self.m_tmsi.size = index;
    ogs_trace("M-TMSI Pool generate...done");

    return OGS_OK;
}

amf_m_tmsi_t *amf_m_tmsi_alloc()
{
    amf_m_tmsi_t *m_tmsi = NULL;

    ogs_pool_alloc(&self.m_tmsi, &m_tmsi);
    ogs_assert(m_tmsi);

    return m_tmsi;
}

int amf_m_tmsi_free(amf_m_tmsi_t *m_tmsi)
{
    ogs_assert(m_tmsi);
    ogs_pool_free(&self.m_tmsi, m_tmsi);

    return OGS_OK;
}

uint8_t amf_selected_int_algorithm(amf_ue_t *amf_ue)
{
    int i;

    ogs_assert(amf_ue);

    for (i = 0; i < amf_self()->num_of_integrity_order; i++) {
        if (amf_ue->ue_security_capability.nia &
                (0x80 >> amf_self()->integrity_order[i])) {
            return amf_self()->integrity_order[i];
        }
    }

    return 0;
}

uint8_t amf_selected_enc_algorithm(amf_ue_t *amf_ue)
{
    int i;

    ogs_assert(amf_ue);

    for (i = 0; i < amf_self()->num_of_ciphering_order; i++) {
        if (amf_ue->ue_security_capability.nea &
                (0x80 >> amf_self()->ciphering_order[i])) {
            return amf_self()->ciphering_order[i];
        }
    }

    return 0;
}
