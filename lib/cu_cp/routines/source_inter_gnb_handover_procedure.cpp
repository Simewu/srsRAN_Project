/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "source_inter_gnb_handover_procedure.h"
#include "../../ngap/ngap_asn1_helpers.h"

using namespace srsran;
using namespace srs_cu_cp;

source_inter_gnb_handover_handover_procedure::source_inter_gnb_handover_handover_procedure(
    cu_cp_ngap_control_notifier&    ngap_ctrl_notifier_,
    ngap_cu_cp_connection_notifier& cu_cp_ngap_ev_notifier_) :
  ngap_ctrl_notifier(ngap_ctrl_notifier_), cu_cp_ngap_ev_notifier(cu_cp_ngap_ev_notifier_)
{
}

void source_inter_gnb_handover_handover_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  fmt::print("Source inter gnb handover procedure\n");

  CORO_RETURN();
}

