/*
 * sm_context.h
 *
 *
 */

#ifndef _OpenAPI_sm_context_H_
#define _OpenAPI_sm_context_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"
#include "ambr.h"
#include "charging_information.h"
#include "dnn_selection_mode.h"
#include "eps_bearer_info.h"
#include "eps_pdn_cnx_info.h"
#include "max_integrity_protected_data_rate.h"
#include "pdu_session_type.h"
#include "qos_flow_setup_item.h"
#include "roaming_charging_profile.h"
#include "snssai.h"
#include "tunnel_info.h"
#include "up_security.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_sm_context_s OpenAPI_sm_context_t;
typedef struct OpenAPI_sm_context_s {
    int pdu_session_id;
    char *dnn;
    struct OpenAPI_snssai_s *s_nssai;
    struct OpenAPI_snssai_s *hplmn_snssai;
    struct OpenAPI_pdu_session_type_s *pdu_session_type;
    char *gpsi;
    char *h_smf_uri;
    char *smf_uri;
    char *pdu_session_ref;
    char *pcf_id;
    char *pcf_group_id;
    char *pcf_set_id;
    struct OpenAPI_dnn_selection_mode_s *sel_mode;
    char *udm_group_id;
    char *routing_indicator;
    struct OpenAPI_ambr_s *session_ambr;
    OpenAPI_list_t *qos_flows_list;
    char *h_smf_instance_id;
    char *smf_instance_id;
    int enable_pause_charging;
    char *ue_ipv4_address;
    char *ue_ipv6_prefix;
    struct OpenAPI_eps_pdn_cnx_info_s *eps_pdn_cnx_info;
    OpenAPI_list_t *eps_bearer_info;
    struct OpenAPI_max_integrity_protected_data_rate_s *max_integrity_protected_data_rate;
    int always_on_granted;
    struct OpenAPI_up_security_s *up_security;
    char *h_smf_service_instance_id;
    char *smf_service_instance_id;
    char *recovery_time;
    int forwarding_ind;
    struct OpenAPI_tunnel_info_s *psa_tunnel_info;
    char *home_provided_charging_id;
    struct OpenAPI_charging_information_s *charging_info;
    struct OpenAPI_roaming_charging_profile_s *roaming_charging_profile;
    int nef_ext_buf_support_ind;
} OpenAPI_sm_context_t;

OpenAPI_sm_context_t *OpenAPI_sm_context_create(
    int pdu_session_id,
    char *dnn,
    OpenAPI_snssai_t *s_nssai,
    OpenAPI_snssai_t *hplmn_snssai,
    OpenAPI_pdu_session_type_t *pdu_session_type,
    char *gpsi,
    char *h_smf_uri,
    char *smf_uri,
    char *pdu_session_ref,
    char *pcf_id,
    char *pcf_group_id,
    char *pcf_set_id,
    OpenAPI_dnn_selection_mode_t *sel_mode,
    char *udm_group_id,
    char *routing_indicator,
    OpenAPI_ambr_t *session_ambr,
    OpenAPI_list_t *qos_flows_list,
    char *h_smf_instance_id,
    char *smf_instance_id,
    int enable_pause_charging,
    char *ue_ipv4_address,
    char *ue_ipv6_prefix,
    OpenAPI_eps_pdn_cnx_info_t *eps_pdn_cnx_info,
    OpenAPI_list_t *eps_bearer_info,
    OpenAPI_max_integrity_protected_data_rate_t *max_integrity_protected_data_rate,
    int always_on_granted,
    OpenAPI_up_security_t *up_security,
    char *h_smf_service_instance_id,
    char *smf_service_instance_id,
    char *recovery_time,
    int forwarding_ind,
    OpenAPI_tunnel_info_t *psa_tunnel_info,
    char *home_provided_charging_id,
    OpenAPI_charging_information_t *charging_info,
    OpenAPI_roaming_charging_profile_t *roaming_charging_profile,
    int nef_ext_buf_support_ind
    );
void OpenAPI_sm_context_free(OpenAPI_sm_context_t *sm_context);
OpenAPI_sm_context_t *OpenAPI_sm_context_parseFromJSON(cJSON *sm_contextJSON);
cJSON *OpenAPI_sm_context_convertToJSON(OpenAPI_sm_context_t *sm_context);
OpenAPI_sm_context_t *OpenAPI_sm_context_copy(OpenAPI_sm_context_t *dst, OpenAPI_sm_context_t *src);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_sm_context_H_ */

