/**
 *  Reduce heap size according to heap statistic
 *  Copyright (c) 2015 Pertimm by Brice Ruzand
 *  Dev : Mars 2015
 *  Version 1.0
 */
#include "ogm_heap.h"

#define DogHeapEnableReduction TRUE
#define DogHeapSmallBufferSize  (DPcPathSize * 2)

static og_status OgHeapResetUpdateSmoothedAverage(og_heap handle);
static og_status OgHeapResetOptimizedCellsNumber(og_heap handle);

/**
 * Setting the number of cells used in the heap to zero
 * OgHeapReset(handle)
 *
 * \param handle handle to a memory heap API
 * \return 0 if ok, -1 on error
 **/
PUBLIC(og_status) OgHeapReset(og_heap ctrl_heap)
{
  IFn(ctrl_heap) DONE;

  // To facilitate debugging with asan, heap is reallocated instead of just being reset so that asan detects buffer overflows.
#ifdef DOG_HEAP_FORCE_RESET
  OgMsg(ctrl_heap->hmsg, "", DOgMsgDestInLog,"OgHeapReset : force realloc for debug for heap %s",ctrl_heap->name);
  ctrl_heap->cells_used = 0;
  IFE(OgHeapReallocInternal(ctrl_heap, 1));
  DONE;
#endif

  if (ctrl_heap->type == DOgHeapTypeNormal && ctrl_heap->normal_cells_number > 0)
  {
    // Reset heap to 0 for string, with sliced heap no string can be used (thus no need to check)
    ctrl_heap->normal_heap[0] = 0;
  }

  // TODO remove reset counter after debuging
  ctrl_heap->reset_cmpt++;

  // compute smoothed average heap usage
  IFE(OgHeapResetUpdateSmoothedAverage(ctrl_heap));

  // reset heap used
  ctrl_heap->cells_used = 0;
  ctrl_heap->max_cells_used = 0;

  // reduce heap size if needed
  if (DogHeapEnableReduction)
  {
    IFE(OgHeapResetOptimizedCellsNumber(ctrl_heap));
  }

  DONE;
}

DEFPUBLIC(og_status) OgHeapResetWithoutReduce(og_heap ctrl_heap)
{
  IFn(ctrl_heap) DONE;

  if (ctrl_heap->type == DOgHeapTypeNormal && ctrl_heap->normal_cells_number > 0)
  {
    // Reset heap to 0 for string, with sliced heap no string can be used (thus no need to check)
    ctrl_heap->normal_heap[0] = 0;
  }

  // update max cell used
  if (ctrl_heap->cells_used > ctrl_heap->max_cells_used)
  {
    ctrl_heap->max_cells_used = ctrl_heap->cells_used;
  }

  ctrl_heap->cells_used = 0;

  DONE;
}

/**
 * Compute new moving average, using cells_used
 *
 * @param ctrl_heap heap handle
 * @return function status
 */
static og_status OgHeapResetUpdateSmoothedAverage(og_heap ctrl_heap)
{
  size_t mavg = ctrl_heap->cells_used_smoothed_average;

  size_t last_used = ctrl_heap->cells_used;
  if (ctrl_heap->max_cells_used > last_used)
  {
    last_used = ctrl_heap->max_cells_used;
  }

  // default average is heap cells number
  if (mavg == 0)
  {
    mavg = OgHeapGetCellsNumber(ctrl_heap);
  }
  else
  {

    // if last used is greater than average then average becomes last used
    if (last_used > mavg)
    {
      mavg = last_used;
    }
    else
    {
      // Compute the smoothed average
      size_t window = ctrl_heap->smoothing_window;
      mavg = ((mavg * window + last_used) / (window + 1)) + 1;
    }

  }

  ctrl_heap->cells_used_smoothed_average = mavg;
  ctrl_heap->smoothing_ratio = (float) last_used / mavg;
  ctrl_heap->smoothing_window = DOgHeapDefaultSmoothedAVGWindow
      + DOgHeapDefaultSmoothedAVGWindow * ctrl_heap->smoothing_ratio;

  DONE;
}

/*
 * Reduce cell number if needed
 * @param ctrl_heap  heap handle
 * @return function status
 */
static og_status OgHeapResetOptimizedCellsNumber(og_heap ctrl_heap)
{
  size_t mavg = ctrl_heap->cells_used_smoothed_average;
  size_t cells_number = OgHeapGetCellsNumber(ctrl_heap);
  size_t new_cells_number = cells_number * DOgHeapDecreaseFactor + 1;
  size_t cells_byte = ctrl_heap->cell_size * cells_number;

  if (mavg < new_cells_number && new_cells_number < cells_number && cells_byte > DogHeapSmallBufferSize)
  {
    if (ctrl_heap->trace & DOgHeapTraceMemory)
    {
      OgMsg(ctrl_heap->hmsg, "", DOgMsgDestInLog,
          "OgHeapResetOptimizedCellsNumber on '%s': reduce heap size from old_size=%zu bytes to new_size=%zu"
              " (old_cells_number=%zu new_cells_number=%zu, cell_size=%zu, reset_cmpt=%zu, mavg=%zu, smoothing_ratio=%.2f, smoothin_win=%zu)",
          ctrl_heap->name, cells_number * ctrl_heap->cell_size, new_cells_number * ctrl_heap->cell_size, cells_number,
          new_cells_number, ctrl_heap->cell_size, ctrl_heap->reset_cmpt, ctrl_heap->cells_used_smoothed_average,
          ctrl_heap->smoothing_ratio, ctrl_heap->smoothing_window);
    }

    // reduce cells number
    IF(OgHeapReallocInternal(ctrl_heap, new_cells_number))
    {
      og_char_buffer erreur[DOgErrorSize];
      snprintf(erreur, DOgErrorSize, "OgHeapResetOptimizedCellsNumber on '%s': heap failed to realloc to (%zu) cells.",
          ctrl_heap->name, new_cells_number);
      OgErr(ctrl_heap->herr, erreur);
      OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

      DPcErr;
    }

  }

  DONE;
}
