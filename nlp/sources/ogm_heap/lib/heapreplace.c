/*
 *  Appending an array of cells at a given position instead of an existing array of cells
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include "ogm_heap.h"

/**
 * Replacing 'nb_cells_to_delete' cells by 'nb_cells_to_add' cells at 'position' position
 * 'nb_cells_to_add' can be zero, in that case, this is a delete
 * 'nb_cells_to_delete' can be zero, in that case, this is a simple addition.
 * in the case of a simple addition, it is done right before 'position'
 * OgHeapReplace(handle,position,nb_cells_to_delete,nb_cells_to_add,cells_to_add)
 *
 * \param ctrl_heap handle to a memory heap API
 * \param starting_position index of first cell to be replaced
 * \param nb_cells_to_delete number of cells to be deleted and replaced
 * \param nb_cells_to_add number of cells to add in the cells array
 * \param cells_to_add an array of memory cells
 * \return 0 if ok or -1 on error
 **/
PUBLIC(og_status) OgHeapReplace(og_heap ctrl_heap, size_t position, size_t nb_cells_to_delete, size_t nb_cells_to_add,
    void const *cells_to_add)
{
  size_t shift, start, length;
  unsigned char *dest, *src;

  if (ctrl_heap->freezed)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapReplace on '%s': is freezed you cannot repalce content",
        ctrl_heap->name);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);
    DPcErr;
  }

  if (nb_cells_to_add == 0 && nb_cells_to_delete == 0) DONE;

  if (ctrl_heap->type != DOgHeapTypeNormal)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapReplace on '%s': function not yet implemented for sliced heap",
        ctrl_heap->name);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  if (nb_cells_to_add > nb_cells_to_delete)
  {
    shift = nb_cells_to_add - nb_cells_to_delete;
    IFE(OgHeapTestReallocInternal(ctrl_heap, shift));
    start = position;
    length = ctrl_heap->cells_used - start;
    src = ctrl_heap->normal_heap + (start * ctrl_heap->cell_size);
    dest = src + (shift * ctrl_heap->cell_size);
    memmove(dest, src, length * ctrl_heap->cell_size);
    ctrl_heap->cells_used += shift;
  }
  else if (nb_cells_to_add < nb_cells_to_delete)
  {
    shift = nb_cells_to_delete - nb_cells_to_add;
    start = position + nb_cells_to_delete;
    length = ctrl_heap->cells_used - start;
    src = ctrl_heap->normal_heap + (start * ctrl_heap->cell_size);
    dest = src - (shift * ctrl_heap->cell_size);
    memmove(dest, src, length * ctrl_heap->cell_size);
    ctrl_heap->cells_used -= shift;
  }
  /* else nb_cells_to_add == nb_cells_to_delete, no memmove necessary */

  if (nb_cells_to_add > 0)
  {
    memcpy(ctrl_heap->normal_heap + (position * ctrl_heap->cell_size), cells_to_add,
        nb_cells_to_add * ctrl_heap->cell_size);
  }

  DONE;
}

