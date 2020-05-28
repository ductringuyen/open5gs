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

#include "sbi-path.h"

static ausf_context_t self;

int __ausf_log_domain;

static OGS_POOL(ausf_ue_pool, ausf_ue_t);

static int context_initialized = 0;

void ausf_context_init(void)
{
    ogs_assert(context_initialized == 0);

    /* Initialize AUSF context */
    memset(&self, 0, sizeof(ausf_context_t));

    ogs_log_install_domain(&__ausf_log_domain, "ausf", ogs_core()->log.level);

    /* Allocate TWICE the pool to check if maximum number of gNBs is reached */
    ogs_pool_init(&ausf_ue_pool, ogs_config()->pool.ue);

    ogs_list_init(&self.ausf_ue_list);
    self.imsi_ue_hash = ogs_hash_make();

    context_initialized = 1;
}

void ausf_context_final(void)
{
    ogs_assert(context_initialized == 1);

    ausf_ue_remove_all();

    ogs_assert(self.imsi_ue_hash);
    ogs_hash_destroy(self.imsi_ue_hash);

    context_initialized = 0;
}

ausf_context_t *ausf_self(void)
{
    return &self;
}

static int ausf_context_prepare(void)
{
    self.nf_type = OpenAPI_nf_type_AUSF;

    return OGS_OK;
}

static int ausf_context_validation(void)
{
    return OGS_OK;
}

int ausf_context_parse_config(void)
{
    int rv;
    yaml_document_t *document = NULL;
    ogs_yaml_iter_t root_iter;

    document = ogs_config()->document;
    ogs_assert(document);

    rv = ausf_context_prepare();
    if (rv != OGS_OK) return rv;

    ogs_yaml_iter_init(&root_iter, document);
    while (ogs_yaml_iter_next(&root_iter)) {
        const char *root_key = ogs_yaml_iter_key(&root_iter);
        ogs_assert(root_key);
        if (!strcmp(root_key, "ausf")) {
            ogs_yaml_iter_t ausf_iter;
            ogs_yaml_iter_recurse(&root_iter, &ausf_iter);
            while (ogs_yaml_iter_next(&ausf_iter)) {
                const char *ausf_key = ogs_yaml_iter_key(&ausf_iter);
                ogs_assert(ausf_key);
                if (!strcmp(ausf_key, "sbi")) {
                    /* handle config in sbi library */
                } else
                    ogs_warn("unknown key `%s`", ausf_key);
            }
        }
    }

    rv = ausf_context_validation();
    if (rv != OGS_OK) return rv;

    return OGS_OK;
}

ausf_ue_t *ausf_ue_add(void)
{
    ausf_ue_t *ausf_ue = NULL;
    ausf_event_t e;

    ogs_pool_alloc(&ausf_ue_pool, &ausf_ue);
    ogs_assert(ausf_ue);
    memset(ausf_ue, 0, sizeof *ausf_ue);

    /* Add All Timers */
    ausf_ue->discover_wait.timer = ogs_timer_add(
            self.timer_mgr, ausf_timer_discover_wait_expire, ausf_ue);

    /* Create FSM */
#if 0
    e.ausf_ue = ausf_ue;
    e.id = 0;
    ogs_fsm_create(&ausf_ue->sm, gmm_state_initial, gmm_state_final);
    ogs_fsm_init(&ausf_ue->sm, &e);
#endif

    ogs_list_add(&self.ausf_ue_list, ausf_ue);

    return ausf_ue;
}

void ausf_ue_remove(ausf_ue_t *ausf_ue)
{
    ausf_event_t e;
    int i;

    ogs_assert(ausf_ue);

    ogs_list_remove(&self.ausf_ue_list, ausf_ue);

#if 0
    e.ausf_ue = ausf_ue;
    ogs_fsm_fini(&ausf_ue->sm, &e);
    ogs_fsm_delete(&ausf_ue->sm);
#endif

    if (ausf_ue->imsi_len != 0)
        ogs_hash_set(self.imsi_ue_hash, ausf_ue->imsi, ausf_ue->imsi_len, NULL);
    
    /* Delete All Timers */
    CLEAR_AUSF_UE_ALL_TIMERS(ausf_ue);
    ogs_timer_delete(ausf_ue->discover_wait.timer);

    for (i = 0; i < OGS_SBI_MAX_NF_TYPE; i++) {
        if (ausf_ue->nf_types[i].nf_instance)
            ogs_sbi_nf_instance_remove(ausf_ue->nf_types[i].nf_instance);
    }

    ogs_pool_free(&ausf_ue_pool, ausf_ue);
}

void ausf_ue_remove_all()
{
    ausf_ue_t *ausf_ue = NULL, *next = NULL;;

    ogs_list_for_each_safe(&self.ausf_ue_list, next, ausf_ue)
        ausf_ue_remove(ausf_ue);
}

ausf_ue_t *ausf_ue_find_by_imsi_bcd(char *imsi_bcd)
{
    uint8_t imsi[OGS_MAX_IMSI_LEN];
    int imsi_len = 0;

    ogs_assert(imsi_bcd);

    ogs_bcd_to_buffer(imsi_bcd, imsi, &imsi_len);

    return ausf_ue_find_by_imsi(imsi, imsi_len);
}

ausf_ue_t *ausf_ue_find_by_imsi(uint8_t *imsi, int imsi_len)
{
    ogs_assert(imsi && imsi_len);

    return (ausf_ue_t *)ogs_hash_get(self.imsi_ue_hash, imsi, imsi_len);
}

ausf_ue_t *ausf_ue_find_by_message(ogs_sbi_message_t *message)
{
    ausf_ue_t *ausf_ue = NULL;

    return ausf_ue;
}

int ausf_ue_set_imsi(ausf_ue_t *ausf_ue, char *imsi_bcd)
{
    ausf_ue_t *old_ausf_ue = NULL;
    ogs_assert(ausf_ue && imsi_bcd);

    ogs_cpystrn(ausf_ue->imsi_bcd, imsi_bcd, OGS_MAX_IMSI_BCD_LEN+1);
    ogs_bcd_to_buffer(ausf_ue->imsi_bcd, ausf_ue->imsi, &ausf_ue->imsi_len);

    /* Check if OLD ausf_ue_t is existed */
    old_ausf_ue = ausf_ue_find_by_imsi(ausf_ue->imsi, ausf_ue->imsi_len);
    if (old_ausf_ue) {
        /* Check if OLD ausf_ue_t is different with NEW ausf_ue_t */
        if (ogs_pool_index(&ausf_ue_pool, ausf_ue) !=
            ogs_pool_index(&ausf_ue_pool, old_ausf_ue)) {
            ogs_warn("OLD UE Context Release [IMSI:%s]", ausf_ue->imsi_bcd);
            ausf_ue_remove(old_ausf_ue);
        }
    }

    ogs_hash_set(self.imsi_ue_hash, ausf_ue->imsi, ausf_ue->imsi_len, ausf_ue);

    return OGS_OK;
}
