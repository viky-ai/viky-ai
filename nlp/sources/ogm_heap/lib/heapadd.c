/*
 *  Adding cells
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include "ogm_heap.h"

/**
 * Adding a zero initialized memory cell and getting its index
 * OgHeapAddCell(handle)
 *
 * \param handle handle to a memory heap API
 * \return added cell index if ok or -1 on error
 **/
PUBLIC(size_t) OgHeapAddCell(og_heap handle)
{
  return OgHeapAddCells(handle, 1);
}

/**
 * Adding new a zero initialized memory cell and getting its index and a cell pointer
 *
 * \param handle handle to a memory heap API
 * \param p_cell_index cell index
 * \return memory pointer or NULL on error
 */
PUBLIC(void *) OgHeapNewCell(og_heap handle, size_t *p_cell_index)
{
  // add new cell
  size_t cell_index = OgHeapAddCell(handle);
  if(cell_index == (size_t) ERREUR)
  {
    return NULL;
  }

  // get pointer
  void *cell = OgHeapGetCell(handle, cell_index);

  memset(cell, 0, sizeof(handle->cell_size));

  // set cell index
  if (p_cell_index != NULL)
  {
    if(cell != NULL)
    {
      *p_cell_index = cell_index;
    }
    else
    {
      *p_cell_index = (size_t) ERREUR;
    }
  }

  return cell;
}

/**
 * Adding X zero initialized memory cells and getting its index
 * OgHeapAddCell(handle)
 *
 * \param ctrl_heap handle to a memory heap API
 * \param nb_cells nb cells to add, must be strictly positive
 * \return added cell index if ok or -1 on error
 **/
PUBLIC(size_t) OgHeapAddCells(og_heap ctrl_heap, int nb_cells)
{
  size_t cell_start, cell_index;

  if (ctrl_heap->freezed)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapAddCells on '%s': is freezed you cannot add cells", ctrl_heap->name);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);
    DPcErr;
  }

  if (nb_cells < 1)
  {
    OgMsg(ctrl_heap->hmsg, "", DOgMsgDestInLog,
        "OgHeapAddCells on '%s': nb_cells must be strictly positive. Do nothing", ctrl_heap->name);
    return ctrl_heap->cells_used;
  }

  if (ctrl_heap->type == DOgHeapTypeSliced)
  {
    return (HeapSliceAppend(ctrl_heap, nb_cells, NULL));
  }
  else if (ctrl_heap->type == DOgHeapTypeNormal)
  {

    if ((ctrl_heap->cells_used + nb_cells) > ctrl_heap->normal_cells_number)
    {
      float incrFactor = DOgHeapIncreaseFactorMax;
      if (ctrl_heap->smoothing_ratio > DOgHeapDefaultSmoothingRatioThreshold)
      {
        incrFactor = DOgHeapIncreaseFactorMin;
      }

      size_t old_cells_number = ctrl_heap->normal_cells_number;
      size_t new_cells_number = old_cells_number;
      while ((ctrl_heap->cells_used + nb_cells) > new_cells_number)
      {
        new_cells_number = (size_t) (incrFactor * new_cells_number) + 1;
      }

      if (ctrl_heap->trace & DOgHeapTraceMemory)
      {
        OgMsg(ctrl_heap->hmsg, "", DOgMsgDestInLog,
            "OgHeapAddCells on '%s': must reallocate with old_size=%ld bytes (old_cells_number=%ld, cell_size=%ld)",
            ctrl_heap->name, old_cells_number * ctrl_heap->cell_size, old_cells_number, ctrl_heap->cell_size);
      }

      // increase heap size
      IFE(OgHeapReallocNormal(ctrl_heap, new_cells_number));

      if (ctrl_heap->trace & DOgHeapTraceMemory)
      {
        OgMsg(ctrl_heap->hmsg, "", DOgMsgDestInLog,
            "OgHeapAddCells on '%s': reallocated with new_size=%ld bytes (new_cells_number=%ld, cell_size=%ld)",
            ctrl_heap->name, new_cells_number * ctrl_heap->cell_size, new_cells_number, ctrl_heap->cell_size);
      }

    }

    cell_index = ctrl_heap->cells_used;
    ctrl_heap->cells_used = ctrl_heap->cells_used + nb_cells;
    cell_start = cell_index * ctrl_heap->cell_size;
    memset(ctrl_heap->normal_heap + cell_start, ctrl_heap->default_mem_set_value, ctrl_heap->cell_size * nb_cells);

    return cell_index;
  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapAddCells on '%s': function not yet implemented for heap type (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }
}

