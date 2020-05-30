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
    self.ue_id_hash = ogs_hash_make();

    context_initialized = 1;
}

void ausf_context_final(void)
{
    ogs_assert(context_initialized == 1);

    ausf_ue_remove_all();

    ogs_assert(self.ue_id_hash);
    ogs_hash_destroy(self.ue_id_hash);

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

ausf_ue_t *ausf_ue_add(ogs_sbi_session_t *session, char *id)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_assert(session);
    ogs_assert(id);

    ogs_pool_alloc(&ausf_ue_pool, &ausf_ue);
    ogs_assert(ausf_ue);
    memset(ausf_ue, 0, sizeof *ausf_ue);

    ogs_sbi_session_set_data(session, ausf_ue);

    ausf_ue->id = ogs_strdup(id);
    ogs_assert(ausf_ue->id);
    ogs_hash_set(self.ue_id_hash, ausf_ue->id, strlen(ausf_ue->id), ausf_ue);

    ausf_ue->sbi_message_wait.timer = ogs_timer_add(
            self.timer_mgr, ausf_timer_sbi_message_wait_expire, session);

    ausf_ue_fsm_init(ausf_ue);

    ogs_list_add(&self.ausf_ue_list, ausf_ue);

    return ausf_ue;
}

void ausf_ue_remove(ausf_ue_t *ausf_ue)
{
    int i;

    ogs_assert(ausf_ue);

    ogs_list_remove(&self.ausf_ue_list, ausf_ue);

    ausf_ue_fsm_fini(ausf_ue);

    ogs_assert(ausf_ue->id);
    ogs_hash_set(self.ue_id_hash, ausf_ue->id, strlen(ausf_ue->id), NULL);
    ogs_free(ausf_ue->id);

    if (ausf_ue->serving_network_name)
        ogs_free(ausf_ue->serving_network_name);
    
    CLEAR_AUSF_UE_ALL_TIMERS(ausf_ue);
    ogs_timer_delete(ausf_ue->sbi_message_wait.timer);

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

ausf_ue_t *ausf_ue_find(char *id)
{
    ogs_assert(id);
    return (ausf_ue_t *)ogs_hash_get(self.ue_id_hash, id, strlen(id));
}

ausf_ue_t *ausf_ue_find_by_message(ogs_sbi_message_t *message)
{
    ausf_ue_t *ausf_ue = NULL;

    return ausf_ue;
}

ausf_ue_t *ausf_ue_add_by_message(ogs_sbi_message_t *message)
{
    ausf_ue_t *ausf_ue = NULL;

    return ausf_ue;
}
