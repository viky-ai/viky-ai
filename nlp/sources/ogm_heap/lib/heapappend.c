/*
 *  Appending an array of cells
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include "ogm_heap.h"

static int HeapAppendSlice(struct og_ctrl_heap *ctrl_heap, size_t nb_cells, char const *cells,
    size_t *pnb_remaining_cells);

/**
 * Appending an array of memory cells to the heap
 * OgHeapAppend(handle,nb_cells,cells)
 *
 * \param ctrl_heap handle to a memory heap API
 * \param nb_cells number of cells in the cells array
 * \param cells an array of memory cells
 * \return 0 if ok or -1 on error
 **/
PUBLIC(og_status) OgHeapAppend(og_heap ctrl_heap, size_t nb_added_cells, void const *cells)
{

  if (ctrl_heap->freezed)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapAppend on '%s': is freezed you cannot add cells",
        ctrl_heap->name);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);
    DPcErr;
  }

  if (nb_added_cells <= 0) DONE;

  if (ctrl_heap->type == DOgHeapTypeSliced)
  {
    return HeapSliceAppend(ctrl_heap, nb_added_cells, cells);
  }
  else if (ctrl_heap->type == DOgHeapTypeNormal)
  {

    // Optimization : test before function call
    if (ctrl_heap->cells_used + nb_added_cells > ctrl_heap->normal_cells_number || ctrl_heap->normal_cells_number == 0)
    {
      IFE(OgHeapTestReallocInternal(ctrl_heap, nb_added_cells));
    }

    size_t preview_size = ctrl_heap->cells_used * ctrl_heap->cell_size;
    size_t new_cells_size = nb_added_cells * ctrl_heap->cell_size;

    if (cells != NULL)
    {
      memcpy(ctrl_heap->normal_heap + preview_size, cells, new_cells_size);
    }
    else
    {
      memset(ctrl_heap->normal_heap + preview_size, ctrl_heap->default_mem_set_value, new_cells_size);
    }

    // for better string management add an extra \0 at the end
    ctrl_heap->normal_heap[preview_size + new_cells_size] = 0;

    ctrl_heap->cells_used += nb_added_cells;

    DONE;
  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapAppend on '%s': function not yet implemented for heap type (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

}

int HeapSliceAppend(struct og_ctrl_heap *ctrl_heap, size_t nb_cells, void const *cells)
{
  if (nb_cells <= 0) DONE;
  size_t cell_index = ctrl_heap->cells_used;

  char const *char_cells = cells;
  size_t nb_remaining_cells = nb_cells;
  size_t remaining_size;

  while (nb_remaining_cells > 0)
  {
    IFn(char_cells) remaining_size = 0;
    else remaining_size = (nb_cells - nb_remaining_cells) * ctrl_heap->cell_size;
    IFE(HeapAppendSlice(ctrl_heap, nb_remaining_cells, char_cells + remaining_size, &nb_remaining_cells));
  }

  return (cell_index);
}

static int HeapAppendSlice(struct og_ctrl_heap *ctrl_heap, size_t nb_cells, char const *cells,
    size_t *pnb_remaining_cells)
{
  size_t slice_allocated = OgHeapGetCellsUsed(ctrl_heap->hslice);
  size_t Islice = ctrl_heap->cells_used / ctrl_heap->nb_cells_per_slice;
  size_t used_nb_cells_in_slice = ctrl_heap->cells_used % ctrl_heap->nb_cells_per_slice;
  size_t remaining_nb_cells_in_slice = 0;
  size_t nb_cells_to_copy = 0;

  if (Islice < slice_allocated)
  {
    // Already preallocated slice
    remaining_nb_cells_in_slice = ctrl_heap->nb_cells_per_slice - used_nb_cells_in_slice;
  }
  else if (Islice == slice_allocated)
  {
    // Needs to allocate a new slice
    if (used_nb_cells_in_slice != 0)
    {
      og_char_buffer erreur[DOgErrorSize];
      snprintf(erreur, DOgErrorSize, "HeapAppendSlice on '%s': used_nb_cells_in_slice (%ld) != 0", ctrl_heap->name,
          used_nb_cells_in_slice);
      OgErr(ctrl_heap->herr, erreur);
      OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

      DPcErr;
    }
    IFE(HeapSliceAlloc(ctrl_heap));
    remaining_nb_cells_in_slice = ctrl_heap->nb_cells_per_slice;
  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "HeapAppendSlice on '%s': Islice (%ld) and slice_allocated (%ld) incoherent",
        ctrl_heap->name, Islice, slice_allocated);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }
  nb_cells_to_copy = nb_cells;
  if (nb_cells_to_copy > remaining_nb_cells_in_slice)
  {
    *pnb_remaining_cells = nb_cells_to_copy - remaining_nb_cells_in_slice;
    nb_cells_to_copy = remaining_nb_cells_in_slice;
  }
  else
  {
    *pnb_remaining_cells = 0;
  }

  struct slice *slice = OgHeapGetCell(ctrl_heap->hslice, Islice);
  IFn(slice)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "HeapAppendSlice on '%s': Islice (%ld) pointer cannot be accessed", ctrl_heap->name,
        Islice);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  IFn(cells)
  {
    memset(slice->heap + (used_nb_cells_in_slice * ctrl_heap->cell_size), ctrl_heap->default_mem_set_value,
        nb_cells_to_copy * ctrl_heap->cell_size);
  }
  else
  {
    memcpy(slice->heap + (used_nb_cells_in_slice * ctrl_heap->cell_size), cells,
        nb_cells_to_copy * ctrl_heap->cell_size);
  }
  ctrl_heap->cells_used += nb_cells_to_copy;

  DONE;
}

