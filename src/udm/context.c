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

static udm_context_t self;

int __udm_log_domain;

static OGS_POOL(udm_ue_pool, udm_ue_t);

static int context_initialized = 0;

void udm_context_init(void)
{
    ogs_assert(context_initialized == 0);

    /* Initialize UDM context */
    memset(&self, 0, sizeof(udm_context_t));

    ogs_log_install_domain(&__udm_log_domain, "udm", ogs_core()->log.level);

    /* Allocate TWICE the pool to check if maximum number of gNBs is reached */
    ogs_pool_init(&udm_ue_pool, ogs_config()->pool.ue);

    ogs_list_init(&self.udm_ue_list);
    self.ueid_hash = ogs_hash_make();

    context_initialized = 1;
}

void udm_context_final(void)
{
    ogs_assert(context_initialized == 1);

    udm_ue_remove_all();

    ogs_assert(self.ueid_hash);
    ogs_hash_destroy(self.ueid_hash);

    ogs_pool_final(&udm_ue_pool);

    context_initialized = 0;
}

udm_context_t *udm_self(void)
{
    return &self;
}

static int udm_context_prepare(void)
{
    self.nf_type = OpenAPI_nf_type_UDM;

    return OGS_OK;
}

static int udm_context_validation(void)
{
    return OGS_OK;
}

int udm_context_parse_config(void)
{
    int rv;
    yaml_document_t *document = NULL;
    ogs_yaml_iter_t root_iter;

    document = ogs_config()->document;
    ogs_assert(document);

    rv = udm_context_prepare();
    if (rv != OGS_OK) return rv;

    ogs_yaml_iter_init(&root_iter, document);
    while (ogs_yaml_iter_next(&root_iter)) {
        const char *root_key = ogs_yaml_iter_key(&root_iter);
        ogs_assert(root_key);
        if (!strcmp(root_key, "udm")) {
            ogs_yaml_iter_t udm_iter;
            ogs_yaml_iter_recurse(&root_iter, &udm_iter);
            while (ogs_yaml_iter_next(&udm_iter)) {
                const char *udm_key = ogs_yaml_iter_key(&udm_iter);
                ogs_assert(udm_key);
                if (!strcmp(udm_key, "sbi")) {
                    /* handle config in sbi library */
                } else
                    ogs_warn("unknown key `%s`", udm_key);
            }
        }
    }

    rv = udm_context_validation();
    if (rv != OGS_OK) return rv;

    return OGS_OK;
}

udm_ue_t *udm_ue_add(ogs_sbi_session_t *session, char *id)
{
    udm_event_t e;
    udm_ue_t *udm_ue = NULL;

    ogs_assert(session);
    ogs_assert(id);

    ogs_pool_alloc(&udm_ue_pool, &udm_ue);
    ogs_assert(udm_ue);
    memset(udm_ue, 0, sizeof *udm_ue);

    udm_ue->id = ogs_strdup(id);
    ogs_assert(udm_ue->id);
    ogs_hash_set(self.ueid_hash, udm_ue->id, strlen(udm_ue->id), udm_ue);

    udm_ue->sbi_client_wait.timer = ogs_timer_add(udm_self()->timer_mgr,
            udm_timer_sbi_client_wait_expire, udm_ue);

    e.udm_ue = udm_ue;
    ogs_fsm_create(&udm_ue->sm, udm_ue_state_initial, udm_ue_state_final);
    ogs_fsm_init(&udm_ue->sm, &e);

    ogs_list_add(&self.udm_ue_list, udm_ue);

    return udm_ue;
}

void udm_ue_remove(udm_ue_t *udm_ue)
{
    udm_event_t e;
    int i;

    ogs_assert(udm_ue);

    ogs_list_remove(&self.udm_ue_list, udm_ue);

    e.udm_ue = udm_ue;
    ogs_fsm_fini(&udm_ue->sm, &e);
    ogs_fsm_delete(&udm_ue->sm);

    CLEAR_UDM_UE_ALL_TIMERS(udm_ue);
    ogs_timer_delete(udm_ue->sbi_client_wait.timer);

    ogs_assert(udm_ue->id);
    ogs_hash_set(self.ueid_hash, udm_ue->id, strlen(udm_ue->id), NULL);
    ogs_free(udm_ue->id);

    if (udm_ue->serving_network_name)
        ogs_free(udm_ue->serving_network_name);
    if (udm_ue->ausf_instance_id)
        ogs_free(udm_ue->ausf_instance_id);
    if (udm_ue->ue_id)
        ogs_free(udm_ue->ue_id);
    
    for (i = 0; i < OGS_SBI_MAX_NF_TYPE; i++) {
        if (udm_ue->nf_types[i].nf_instance)
            ogs_sbi_nf_instance_remove(udm_ue->nf_types[i].nf_instance);
    }

    ogs_pool_free(&udm_ue_pool, udm_ue);
}

void udm_ue_remove_all()
{
    udm_ue_t *udm_ue = NULL, *next = NULL;;

    ogs_list_for_each_safe(&self.udm_ue_list, next, udm_ue)
        udm_ue_remove(udm_ue);
}

udm_ue_t *udm_ue_find(char *id)
{
    ogs_assert(id);
    return (udm_ue_t *)ogs_hash_get(self.ueid_hash, id, strlen(id));
}
