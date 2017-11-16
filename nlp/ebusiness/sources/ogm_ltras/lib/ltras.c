/*
 *  Main functions for Linguistic Trf search
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
*/

#include "ogm_ltras.h"



PUBLIC(og_status) OgLtras(void *handle, struct og_ltras_input *input
  , struct og_ltra_trfs **ptrfs)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
ogint64_t elapsed,micro_clock_start=OgMicroClock();

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleFlowChart) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog, "OgLtras: starting");
  }

ctrl_ltras->input = input;
IFE(LtrasReset(ctrl_ltras));

IFE(OgLtrasFlowChart(ctrl_ltras, input, ptrfs));

elapsed=OgMicroClock()-micro_clock_start;
ctrl_ltras->statistics.total_elapsed+=elapsed;

if (ctrl_ltras->statistics.min_elapsed > elapsed) ctrl_ltras->statistics.min_elapsed=elapsed;
if (ctrl_ltras->statistics.max_elapsed < elapsed) ctrl_ltras->statistics.max_elapsed=elapsed;

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleFlowChart) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog, "OgLtras: finished");
  }

DONE;
}



