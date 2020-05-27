# Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>

# This file is part of Open5GS.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

libsbi_openapi_sources = files('''
    src/list.c
    src/apiKey.c
    external/cJSON.c
    model/object.c
    model/access_type.c
    model/amf_cond.c
    model/amf_info.c
    model/atsss_capability.c
    model/ausf_info.c
    model/bsf_info.c
    model/change_item.c
    model/change_type.c
    model/chf_info.c
    model/chf_service_info.c
    model/data_set_id.c
    model/default_notification_subscription.c
    model/dnn_smf_info_item.c
    model/dnn_upf_info_item.c
    model/event_id.c
    model/guami.c
    model/guami_list_cond.c
    model/identity_range.c
    model/inline_response_200.c
    model/interface_upf_info_item.c
    model/invalid_param.c
    model/ip_end_point.c
    model/ipv4_address_range.c
    model/ipv6_prefix_range.c
    model/link.c
    model/links_value_schema.c
    model/n1_message_class.c
    model/n2_information_class.c
    model/n2_interface_amf_info.c
    model/network_slice_cond.c
    model/nf_group_cond.c
    model/nf_instance_id_cond.c
    model/nf_profile.c
    model/nf_service.c
    model/nf_service_status.c
    model/nf_service_version.c
    model/nf_status.c
    model/nf_type.c
    model/nf_type_cond.c
    model/notif_condition.c
    model/notification_data.c
    model/notification_event_type.c
    model/notification_type.c
    model/nrf_info.c
    model/nwdaf_event.c
    model/nwdaf_info.c
    model/patch_item.c
    model/patch_operation.c
    model/pcf_info.c
    model/pdu_session_type.c
    model/plmn_id.c
    model/plmn_range.c
    model/plmn_snssai.c
    model/problem_details.c
    model/service_name_cond.c
    model/smf_info.c
    model/snssai.c
    model/snssai_smf_info_item.c
    model/snssai_upf_info_item.c
    model/subscription_data.c
    model/supi_range.c
    model/tac_range.c
    model/tai.c
    model/tai_range.c
    model/transport_protocol.c
    model/udm_info.c
    model/udr_info.c
    model/up_interface_type.c
    model/upf_info.c
    model/uri_scheme.c

    model/atom.c
    model/cnf.c
    model/cnf_unit.c
    model/complex_query.c
    model/dnf.c
    model/dnf_unit.c
    model/search_result.c
    model/stored_search_result.c

    model/af_event.c
	model/af_event_exposure_data.c
	model/an_node_type.c
	model/external_client_type.c
	model/gmlc_info.c
	model/hss_info.c
	model/imsi_range.c
	model/internal_group_id_range.c
	model/lmf_info.c
	model/nef_info.c
	model/nf_info.c
	model/nf_service_set_cond.c
	model/nf_set_cond.c
	model/pcscf_info.c
	model/pfd_data.c
	model/plmn_id_nid.c
	model/preferred_search.c
	model/rat_type.c
	model/tngf_info.c
	model/twif_info.c
	model/udsf_info.c
	model/upf_cond.c
	model/vendor_specific_feature.c
	model/w_agf_info.c

    model/auth_result.c
	model/auth_type.c
	model/authentication_info.c
	model/av5g_aka.c
	model/confirmation_data.c
	model/confirmation_data_response.c
	model/eap_session.c
	model/resynchronization_info.c
	model/rg_auth_ctx.c
	model/rg_authentication_info.c
	model/trace_data.c
	model/trace_depth.c
	model/ue_authentication_ctx.c

    model/auth_event.c
    model/authentication_info_request.c
    model/authentication_info_result.c
    model/authentication_vector.c
    model/av5_g_he_aka.c
    model/av_eap_aka_prime.c
    model/av_eps_aka.c
    model/av_ims_gba_eap_aka.c
    model/av_type.c
    model/hss_auth_type.c
    model/hss_authentication_info_request.c
    model/hss_authentication_info_result.c
    model/hss_authentication_vectors.c
    model/hss_av_type.c

    model/amf3_gpp_access_registration.c
    model/amf3_gpp_access_registration_modification.c
    model/amf_dereg_info.c
    model/amf_non3_gpp_access_registration.c
    model/amf_non3_gpp_access_registration_modification.c
    model/backup_amf_info.c
    model/cm_info.c
    model/cm_info_report.c
    model/cm_state.c
    model/deregistration_data.c
    model/deregistration_reason.c
    model/eps_interworking_info.c
    model/eps_iwk_pgw.c
    model/ims_vo_ps.c
    model/location_info.c
    model/network_node_diameter_address.c
    model/patch_result.c
    model/pcscf_restoration_notification.c
    model/pei_update_info.c
    model/registration_reason.c
    model/report_item.c
    model/smf_registration.c
    model/smf_registration_info.c
    model/smsf_registration.c
    model/trigger_request.c
    model/vgmlc_address.c
'''.split())

libsbi_openapi_inc = include_directories('.')

sbi_openapi_cc_flags = ['-DOGS_SBI_COMPILATION']

if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
    sbi_openapi_cc_flags += cc.get_supported_arguments([
                    '-Wno-strict-prototypes',
                    '-Wno-missing-prototypes',
                    '-Wno-missing-declarations',
                    '-Wno-unused-variable',
                    '-Wno-unused-label',
                    '-Wno-float-equal',
    ])
endif

libsbi_openapi = library('ogssbi-openapi',
    sources : libsbi_openapi_sources,
    version : libogslib_version,
    c_args : sbi_openapi_cc_flags,
    include_directories : [libsbi_openapi_inc, libinc],
    dependencies : libcore_dep,
    install : true)

libsbi_openapi_dep = declare_dependency(
    link_with : libsbi_openapi,
    include_directories : [libsbi_openapi_inc, libinc],
    dependencies : libcore_dep)
