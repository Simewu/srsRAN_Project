/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "cu_cp_routine_manager.h"
#include "../routines/mobility/inter_cu_handover_target_routine.h"
#include "../routines/mobility/inter_du_handover_routine.h"
#include "../routines/pdu_session_resource_modification_routine.h"
#include "../routines/pdu_session_resource_release_routine.h"
#include "../routines/pdu_session_resource_setup_routine.h"
#include "../routines/reestablishment_context_modification_routine.h"
#include "../routines/ue_context_release_routine.h"
#include "srsran/support/async/coroutine.h"

using namespace srsran;
using namespace srs_cu_cp;

cu_cp_routine_manager::cu_cp_routine_manager(ue_manager&                  ue_mng_,
                                             const security_indication_t& default_security_indication_,
                                             srslog::basic_logger&        logger_) :

  ue_mng(ue_mng_),
  default_security_indication(default_security_indication_),
  logger(logger_),

  main_ctrl_loop(128)
{
}

bool cu_cp_routine_manager::schedule_async_task(async_task<void> task)
{
  return main_ctrl_loop.schedule(std::move(task));
}

async_task<cu_cp_pdu_session_resource_setup_response> cu_cp_routine_manager::start_pdu_session_resource_setup_routine(
    const cu_cp_pdu_session_resource_setup_request& setup_msg,
    const srsran::security::sec_as_config&          security_cfg,
    e1ap_bearer_context_manager&                    e1ap_bearer_ctxt_mng,
    f1ap_ue_context_manager&                        f1ap_ue_ctxt_mng,
    du_processor_rrc_ue_control_message_notifier&   rrc_ue_ctrl_notifier,
    up_resource_manager&                            rrc_ue_up_resource_manager)
{
  return launch_async<pdu_session_resource_setup_routine>(setup_msg,
                                                          ue_mng.get_ue_config(),
                                                          security_cfg,
                                                          default_security_indication,
                                                          e1ap_bearer_ctxt_mng,
                                                          f1ap_ue_ctxt_mng,
                                                          rrc_ue_ctrl_notifier,
                                                          rrc_ue_up_resource_manager,
                                                          logger);
}

async_task<cu_cp_pdu_session_resource_modify_response>
cu_cp_routine_manager::start_pdu_session_resource_modification_routine(
    const cu_cp_pdu_session_resource_modify_request& modify_msg,
    e1ap_bearer_context_manager&                     e1ap_bearer_ctxt_mng,
    f1ap_ue_context_manager&                         f1ap_ue_ctxt_mng,
    du_processor_rrc_ue_control_message_notifier&    rrc_ue_ctrl_notifier,
    up_resource_manager&                             rrc_ue_up_resource_manager)
{
  return launch_async<pdu_session_resource_modification_routine>(
      modify_msg, e1ap_bearer_ctxt_mng, f1ap_ue_ctxt_mng, rrc_ue_ctrl_notifier, rrc_ue_up_resource_manager, logger);
}

async_task<cu_cp_pdu_session_resource_release_response>
cu_cp_routine_manager::start_pdu_session_resource_release_routine(
    const cu_cp_pdu_session_resource_release_command& release_cmd,
    e1ap_bearer_context_manager&                      e1ap_bearer_ctxt_mng,
    f1ap_ue_context_manager&                          f1ap_ue_ctxt_mng,
    ngap_control_message_handler&                     ngap_handler,
    du_processor_rrc_ue_control_message_notifier&     rrc_ue_ctrl_notifier,
    du_processor_ue_task_scheduler&                   task_sched,
    up_resource_manager&                              rrc_ue_up_resource_manager)
{
  return launch_async<pdu_session_resource_release_routine>(release_cmd,
                                                            e1ap_bearer_ctxt_mng,
                                                            f1ap_ue_ctxt_mng,
                                                            ngap_handler,
                                                            rrc_ue_ctrl_notifier,
                                                            task_sched,
                                                            rrc_ue_up_resource_manager,
                                                            logger);
}

async_task<cu_cp_ue_context_release_complete>
cu_cp_routine_manager::start_ue_context_release_routine(const cu_cp_ue_context_release_command& command,
                                                        e1ap_bearer_context_manager*            e1ap_bearer_ctxt_mng,
                                                        f1ap_ue_context_manager&                f1ap_ue_ctxt_mng,
                                                        cu_cp_ue_removal_handler&               ue_removal_handler)
{
  return launch_async<ue_context_release_routine>(
      command, e1ap_bearer_ctxt_mng, f1ap_ue_ctxt_mng, ue_removal_handler, ue_mng, logger);
}

async_task<bool> cu_cp_routine_manager::start_reestablishment_context_modification_routine(
    ue_index_t                                    ue_index,
    const security::sec_as_config&                up_sec,
    e1ap_bearer_context_manager&                  e1ap_bearer_ctxt_mng,
    f1ap_ue_context_manager&                      f1ap_ue_ctxt_mng,
    du_processor_rrc_ue_control_message_notifier& rrc_ue_ctrl_notifier,
    up_resource_manager&                          ue_up_resource_manager)
{
  return launch_async<reestablishment_context_modification_routine>(
      ue_index, up_sec, e1ap_bearer_ctxt_mng, f1ap_ue_ctxt_mng, rrc_ue_ctrl_notifier, ue_up_resource_manager, logger);
}

async_task<cu_cp_inter_du_handover_response>
cu_cp_routine_manager::start_inter_du_handover_routine(const cu_cp_inter_du_handover_request& request,
                                                       const byte_buffer&                     target_cell_sib1,
                                                       e1ap_bearer_context_manager&           e1ap_bearer_ctxt_mng,
                                                       f1ap_ue_context_manager&               source_f1ap_ue_ctxt_mng,
                                                       f1ap_ue_context_manager&               target_f1ap_ue_ctxt_mng,
                                                       cu_cp_ue_context_release_handler& ue_context_release_handler,
                                                       cu_cp_ue_removal_handler&         ue_removal_handler,
                                                       cu_cp_ue_context_manipulation_handler& cu_cp_handler)
{
  return launch_async<inter_du_handover_routine>(request,
                                                 target_cell_sib1,
                                                 e1ap_bearer_ctxt_mng,
                                                 source_f1ap_ue_ctxt_mng,
                                                 target_f1ap_ue_ctxt_mng,
                                                 ue_context_release_handler,
                                                 ue_removal_handler,
                                                 cu_cp_handler,
                                                 ue_mng,
                                                 logger);
}

async_task<ngap_handover_resource_allocation_response>
cu_cp_routine_manager::start_inter_cu_handover_target_routine(const ngap_handover_request& request_,
                                                              e1ap_bearer_context_manager& e1ap_bearer_ctxt_mng,
                                                              f1ap_ue_context_manager&     f1ap_ue_ctxt_mng,
                                                              cu_cp_ue_removal_handler&    ue_removal_handler)
{
  return launch_async<inter_cu_handover_target_routine>(request_,
                                                        e1ap_bearer_ctxt_mng,
                                                        f1ap_ue_ctxt_mng,
                                                        ue_removal_handler,
                                                        ue_mng,
                                                        default_security_indication,
                                                        logger);
}
