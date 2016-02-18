/*
 *  Reallocation of the heap, works with any value of new_cells_number
 *  works also whether new_cells_number >= old_cells_number is true or not
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include "ogm_heap.h"

static og_status OgHeapReallocSliced(struct og_ctrl_heap *ctrl_heap, size_t new_cells_number);

/**
 * Reallocated the heap so that its new size is 'new_cells_number' cells
 * it works with all possible values :
 * if new_cells_number == 0, the heap is desallocated to zero, and no new allocation is done
 * else the heap is reallocated (bigger or smaller) with the new size
 * OgHeapRealloc(handle,new_cells_number)
 *
 * \deprecated You should not realloc a heap manually use : OgHeapAddCells, OgHeapGetBufferNew, OgHeapGetBufferReuse instead.
 *
 * \param ctrl_heap handle to a memory heap API
 * \param new_cells_number new total number of cells
 * \return 0 if ok or -1 on error
 **/
PUBLIC(og_status) OgHeapRealloc(og_heap ctrl_heap, size_t new_cells_number)
{
  return OgHeapReallocInternal(ctrl_heap, new_cells_number);
}

og_status OgHeapReallocInternal(og_heap ctrl_heap, size_t new_cells_number)
{
  if (new_cells_number < ctrl_heap->cells_used)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapRealloc on '%s': you cannot realloc a heap with a smaller cells number than "
        "current cells used (%zu < %zu)", ctrl_heap->name, new_cells_number, ctrl_heap->cells_used);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  if (ctrl_heap->type == DOgHeapTypeNormal)
  {
    IFE(OgHeapReallocNormal(ctrl_heap, new_cells_number));
  }
  else if (ctrl_heap->type == DOgHeapTypeSliced)
  {
    IFE(OgHeapReallocSliced(ctrl_heap, new_cells_number));
  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapRealloc on '%s': function not yet implemented for heap type (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  DONE;
}

int HeapSliceAlloc(struct og_ctrl_heap *ctrl_heap)
{
  struct slice slice[1];

  size_t size = ctrl_heap->nb_cells_per_slice * ctrl_heap->cell_size;

  IFn(slice->heap = (char *) malloc(size))
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "HeapSliceAlloc on '%s': malloc error with size=%ld bytes", ctrl_heap->name, size);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  IFE(OgHeapAppend(ctrl_heap->hslice, 1, slice));

  DONE;
}

og_status OgHeapReallocNormal(struct og_ctrl_heap *ctrl_heap, size_t new_cells_number)
{

  if (new_cells_number == 0)
  {
    ctrl_heap->normal_cells_number = 0;
    DPcFree(ctrl_heap->normal_heap);
  }
  else
  {
    size_t old_size = ctrl_heap->normal_cells_number * ctrl_heap->cell_size;

    /** +9 because we want to have always extra chars at the end for safety reasons **/
    size_t new_size = ((new_cells_number + 1) * ctrl_heap->cell_size) + 9;

    // use a temporary pointer to avoid realloc error
    char *new_heap = (char *) realloc(ctrl_heap->normal_heap, new_size);
    IFn(new_heap)
    {
      og_char_buffer erreur[DOgErrorSize];
      snprintf(erreur, DOgErrorSize, "HeapRealloc on '%s': realloc error with"
          " new_size=%zu bytes (new_cells_number=%zu)", ctrl_heap->name, new_size, new_cells_number);
      OgErr(ctrl_heap->herr, erreur);
      OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

      DPcErr;
    }
    ctrl_heap->normal_heap = new_heap;

    // Memset new allocated memory part only
    if (new_size > old_size)
    {
      memset(ctrl_heap->normal_heap + old_size, ctrl_heap->default_mem_set_value, (new_size - old_size));
    }

    // set new cells number
    ctrl_heap->normal_cells_number = new_cells_number;

  }

  DONE;
}

/**
 * Realloc up/down the number of slice according to a new number of cells.
 *
 * "new_cells_number" while be round to coreponding to an uppper nb_cells_per_slice modulo
 *
 * @param ctrl_heap heap
 * @param new_cells_number expect new cell number
 * @return function status
 */
static og_status OgHeapReallocSliced(struct og_ctrl_heap *ctrl_heap, size_t new_cells_number)
{
  og_heap hslice = ctrl_heap->hslice;
  size_t new_slice_number = new_cells_number / ctrl_heap->nb_cells_per_slice + 1;
  size_t minimum_slice_number = ctrl_heap->cells_used / ctrl_heap->nb_cells_per_slice + 1;
  size_t current_slice_number = OgHeapGetCellsUsed(hslice);

  if (current_slice_number < new_slice_number)
  {
    // increase slice number
    while (current_slice_number < new_slice_number)
    {
      IFE(HeapSliceAlloc(ctrl_heap));

      current_slice_number++;
    }

  }
  else if (current_slice_number > new_cells_number)
  {

    // decrease slice number
    while (current_slice_number > new_cells_number && minimum_slice_number >= current_slice_number)
    {
      current_slice_number--;

      struct slice *slice = OgHeapGetCell(hslice, current_slice_number);
      if (slice != NULL)
      {
        DPcFree(slice->heap);
      }
      IFE(OgHeapSetCellsUsed(hslice, current_slice_number));
    }

  }

  DONE;
}

/**
 * Makes sure there is enough room for adding 'nb_added_cells' cells
 * this can trigger a reallocation of the heap
 * No cell is actually added
 * OgHeapTestRealloc(handle,nb_added_cells)
 *
 * \deprecated You should not realloc a heap manually use : OgHeapAddCells, OgHeapGetBufferNew, OgHeapGetBufferReuse instead.
 *
 * \param ctrl_heap handle to a memory heap API
 * \param nb_added_cells number of cells in the cells array
 * \return 0 if ok or -1 on error
 **/
PUBLIC(og_status) OgHeapTestRealloc(og_heap ctrl_heap, size_t nb_added_cells)
{
  og_status status = OgHeapTestReallocInternal(ctrl_heap, nb_added_cells);

  // update max cell used, because nb_added_cells will be use by caller anyway
  if (ctrl_heap->cells_used + nb_added_cells > ctrl_heap->max_cells_used)
  {
    ctrl_heap->max_cells_used = ctrl_heap->cells_used + nb_added_cells;
  }

  return status;
}

og_status OgHeapTestReallocInternal(og_heap ctrl_heap, size_t nb_added_cells)
{

  if (ctrl_heap->type != DOgHeapTypeNormal)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapTestRealloc on '%s': function not yet implemented for heap type (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  if (ctrl_heap->cells_used + nb_added_cells < ctrl_heap->normal_cells_number) DONE;

  float incrFactor = DOgHeapIncreaseFactorMax;
  if (ctrl_heap->smoothing_ratio > DOgHeapDefaultSmoothingRatioThreshold)
  {
    incrFactor = DOgHeapIncreaseFactorMin;
  }

  size_t old_cells_number = ctrl_heap->normal_cells_number;
  size_t needed_cells_number = old_cells_number + nb_added_cells;
  size_t new_cells_number = incrFactor * needed_cells_number + 1;
  if (ctrl_heap->trace & DOgHeapTraceMemory)
  {
    OgMsg(ctrl_heap->hmsg, "", DOgMsgDestInLog,
        "HeapTestRealloc on '%s': must reallocate with old_size=%ld bytes (old_cells_number=%ld, cell_size=%ld)",
        ctrl_heap->name, old_cells_number * ctrl_heap->cell_size, old_cells_number, ctrl_heap->cell_size);
  }

  IFE(OgHeapReallocNormal(ctrl_heap, new_cells_number));

  if (ctrl_heap->trace & DOgHeapTraceMemory)
  {
    OgMsg(ctrl_heap->hmsg, "", DOgMsgDestInLog,
        "HeapTestRealloc on '%s': reallocated with new_size=%ld bytes (new_cells_number=%ld, cell_size=%ld)",
        ctrl_heap->name, new_cells_number * ctrl_heap->cell_size, new_cells_number, ctrl_heap->cell_size);
  }

  DONE;
}
