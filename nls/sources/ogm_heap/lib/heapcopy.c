/*
 *  Copying a heap into another
 *  Copyright (c) 2014 Pertimm by Patrick Constant
 *  Dev : November 2014
 *  Version 1.0
 */
#include "ogm_heap.h"

/**
 * Copying a heap into another
 * OgHeapCopy(void *heap_from, void *heap_to)
 *
 * \param heap_from handle to a memory heap API, from which to copy
 * \param heap_to handle to a memory heap API, to which to copy
 * \return 0 if ok or -1 on error
 **/
PUBLIC(size_t) OgHeapCopy(og_heap heap_from, og_heap heap_to)
{
  struct og_ctrl_heap *ctrl_heap_from = heap_from;
  struct og_ctrl_heap *ctrl_heap_to = heap_to;

  if (ctrl_heap_to->type != DOgHeapTypeNormal)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapCopy on '%s': function not yet implemented for sliced heap",
        ctrl_heap_from->name);
    OgErr(ctrl_heap_from->herr, erreur);
    DPcErr;
  }

  if (ctrl_heap_from->cell_size != ctrl_heap_to->cell_size)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapCopy: from (%s) cell_size %ld != to (%s) cell_size %ld", ctrl_heap_from->name,
        ctrl_heap_from->cell_size, ctrl_heap_to->name, ctrl_heap_to->cell_size);
    OgErr(ctrl_heap_from->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap_from->hmsg, erreur);

    DPcErr;
  }

  if (ctrl_heap_to->normal_cells_number < ctrl_heap_from->cells_used || ctrl_heap_to->normal_cells_number == 0)
  {
    IFE(OgHeapReallocNormal(heap_to, ctrl_heap_from->cells_used));
  }

  // set used cells to default value
  memset(ctrl_heap_to->normal_heap + (ctrl_heap_to->cell_size * ctrl_heap_from->cells_used),
      ctrl_heap_to->default_mem_set_value,
      (ctrl_heap_to->normal_cells_number - ctrl_heap_from->cells_used) * ctrl_heap_to->cell_size);

  memcpy(ctrl_heap_to->normal_heap, ctrl_heap_from->normal_heap,
      ctrl_heap_from->cell_size * ctrl_heap_from->cells_used);
  ctrl_heap_to->cells_used = ctrl_heap_from->cells_used;

  DONE;
}

