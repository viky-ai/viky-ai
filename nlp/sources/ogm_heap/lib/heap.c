/*
 *  Miscellaneous API function for heap memory
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include "ogm_heap.h"

static size_t HeapSliceAllocatedMemory(struct og_ctrl_heap *ctrl_heap);

/**
 * Getting the total number of allocated cells of the heap
 * OgHeapGetCellsNumber(handle)
 *
 * \param ctrl_heap handle to a memory heap API
 * \return the total number of allocated cells of the heap
 **/
PUBLIC(size_t) OgHeapGetCellsNumber(og_heap ctrl_heap)
{
  if (ctrl_heap->type == DOgHeapTypeSliced)
  {
    size_t slice_used = OgHeapGetCellsUsed(ctrl_heap->hslice);
    return slice_used * ctrl_heap->nb_cells_per_slice;
  }
  else if (ctrl_heap->type == DOgHeapTypeNormal)
  {
    return ctrl_heap->normal_cells_number;
  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapGetCellsNumber on '%s': function not yet implemented for heap type (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }
}

/**
 * Getting the total number of used cells of the heap
 * OgHeapGetCellsUsed(handle)
 *
 * \param handle handle to a memory heap API
 * \return the total number of used cells of the heap
 **/
PUBLIC(size_t) OgHeapGetCellsUsed(og_heap ctrl_heap)
{
  return ctrl_heap->cells_used;
}

/**
 * Getting the smoothing window size of the heap
 * OgHeapGetSmoothingWindowSize(handle)
 *
 * \param handle handle to a memory heap API
 * \return the current window used to calculate the memory average of the heap
 **/
DEFPUBLIC(int) OgHeapGetSmoothingWindowSize(og_heap ctrl_heap)
{
  return ctrl_heap->smoothing_window;
}

/**
 * Getting the memory smoothed average value of the heap
 * OgHeapGetSmoothedAverage(handle)
 *
 * \param handle handle to a memory heap API
 * \return the current memory average of the heap weighed with the smoothing window
 **/
DEFPUBLIC(size_t) OgHeapGetSmoothedAverage(og_heap ctrl_heap)
{
  return ctrl_heap->cells_used_smoothed_average;
}

/**
 * Getting the memory smoothing ratio of the heap
 * OgHeapGetSmoothingRatio(handle)
 *
 * \param handle handle to a memory heap API
 * \return the current memory smoothing ratio used to shape the window size
 * and the reallocation increase factor
 **/
DEFPUBLIC(float) OgHeapGetSmoothingRatio(og_heap ctrl_heap)
{
  return ctrl_heap->smoothing_ratio;
}

/**
 * Setting the number of cells used in the heap
 * OgHeapSetCellsUsed(handle,cells_used)
 *
 * \param handle handle to a memory heap API
 * \param cells_used the number of used cells to be set
 * \return 0 if ok, -1 on error
 **/
PUBLIC(og_status) OgHeapSetCellsUsed(og_heap ctrl_heap, size_t cells_used)
{
  if (cells_used > ctrl_heap->normal_cells_number)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapSetCellsUsed on '%s': cells_used (%ld) >= ctrl_heap->cells_number (%ld)",
        ctrl_heap->name, cells_used, ctrl_heap->normal_cells_number);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  // update max cell used
  if (ctrl_heap->cells_used > ctrl_heap->max_cells_used)
  {
    ctrl_heap->max_cells_used = ctrl_heap->cells_used;
  }

  ctrl_heap->cells_used = cells_used;
  DONE;
}

/**
 * Setting the trace value to a different value than default value
 * which is : DOgHeapTraceMinimal+DOgHeapTraceMemory
 * OgHeapSetTrace(handle,trace)
 *
 * \param ctrl_heap handle to a memory heap API
 * \param trace the new value of the trace
 * \return 0 if ok, -1 on error
 **/
PUBLIC(og_status) OgHeapSetTrace(og_heap ctrl_heap, int trace)
{
  ctrl_heap->trace = trace;
  DONE;
}

/**
 * Getting the total memory allocated for the heap
 * OgHeapGetAllocatedMemory(handle)
 *
 * \param ctrl_heap handle to a memory heap API
 * \return the total size of allocated memory in bytes
 **/
PUBLIC(size_t) OgHeapGetAllocatedMemory(og_heap ctrl_heap)
{
  if (ctrl_heap == NULL)
  {
    return 0;
  }

  if (ctrl_heap->type == DOgHeapTypeSliced)
  {
    return HeapSliceAllocatedMemory(ctrl_heap);
  }
  else if (ctrl_heap->type == DOgHeapTypeNormal)
  {
    size_t size = sizeof(struct og_ctrl_heap);
    size += OgHeapGetCellsNumber(ctrl_heap) * ctrl_heap->cell_size;
    return size;
  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapGetAllocatedMemory on '%s': function not yet implemented for heap type (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }
}

static size_t HeapSliceAllocatedMemory(struct og_ctrl_heap *ctrl_heap)
{
  size_t size = sizeof(struct og_ctrl_heap);
  size_t slice_used = OgHeapGetCellsUsed(ctrl_heap->hslice);
  size += slice_used * ctrl_heap->nb_cells_per_slice * ctrl_heap->cell_size;
  size += slice_used * sizeof(struct slice);
  return (size);
}

/**
 * Set default value to use when allocation heap
 *
 * \param ctrl_heap handle to a memory heap API
 * \param default_mem_set_value value to memset
 * \return 0 if ok, -1 on error
 **/
PUBLIC(og_status) OgHeapSetDefaultValue(og_heap ctrl_heap, int default_mem_set_value)
{
  ctrl_heap->default_mem_set_value = default_mem_set_value;
  DONE;
}

