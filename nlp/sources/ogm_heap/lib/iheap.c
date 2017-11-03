/*
 *  Initialization for ogm_heap functions
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include "ogm_heap.h"

/**
 * Initialization for the memory heap API
 * OgHeapInit(hmsg,name,cell_size,nb_init_cells)
 *
 * \param hsmg handle on msg librairy
 * \param name name of the heap, only for logging purposes
 * \param cell_size, size of the memory cell, usually sizeof(struct mystruct)
 * \param nb_init_cells, number of cells to be malloc-ed at initialization
 * \return pointer to heap handle to be used for all other functions or 0 if error
 **/
PUBLIC(og_heap) OgHeapInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells)
{
  struct og_ctrl_heap *ctrl_heap = HeapInit(hmsg, name, cell_size);
  IFn(ctrl_heap) return NULL;

  ctrl_heap->type = DOgHeapTypeNormal;

  IF(OgHeapReallocInternal(ctrl_heap, nb_init_cells))
  {
    OgHeapFlush(ctrl_heap);
    return NULL;
  }

  return ctrl_heap;
}

/**
 * Initialization for the memory heap API
 * OgHeapInit(hmsg,name,cell_size,nb_init_cells,nb_cells_per_slice)
 *
 * \param hsmg handle on msg librairy
 * \param name name of the heap, only for logging purposes
 * \param cell_size, size of the memory cell, usually sizeof(struct mystruct)
 * \param nb_init_cells, number of cells to be malloc-ed at initialization
 * \param nb_cells_per_slice, number of cells per slice (allocated chunk)
 * \return pointer to heap handle to be used for all other functions or 0 if error
 **/
PUBLIC(og_heap) OgHeapSliceInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells,
    size_t nb_cells_per_slice)
{
  struct og_ctrl_heap *ctrl_heap = HeapInit(hmsg, name, cell_size);
  IFn(ctrl_heap) return (0);

  ctrl_heap->type = DOgHeapTypeSliced;
  ctrl_heap->nb_cells_per_slice = nb_cells_per_slice;
  IFn(ctrl_heap->hslice=OgHeapInit(ctrl_heap->hmsg,name,sizeof(struct slice),1)) return (0);
  IF(OgHeapAddCells(ctrl_heap, nb_init_cells)) return (0);
  IF(OgHeapSetCellsUsed(ctrl_heap,0)) return (0);

  return ctrl_heap;
}

struct og_ctrl_heap *HeapInit(void *hmsg, unsigned char *name, size_t cell_size)
{
  size_t size = sizeof(struct og_ctrl_heap);
  struct og_ctrl_heap *ctrl_heap;
  int name_length = strlen(name);

  IFn(ctrl_heap = (struct og_ctrl_heap *) malloc(size))
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapInit on '%s': malloc error on ctrl_heap with size %ld bytes", name, size);
    OgErr(OgLogGetErr(hmsg), erreur);
    return NULL;
  }
  memset(ctrl_heap, 0, size);

  /** +2 because of the zero char at the end and +1 for safety **/
  IFn(ctrl_heap->name=(unsigned char *)malloc(name_length+2))
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapInit on '%s': malloc error on name with size %d bytes", name, name_length);
    DPcFree(ctrl_heap);
    OgErr(OgLogGetErr(hmsg), erreur);
    return NULL;
  }

  strcpy(ctrl_heap->name, name);
  ctrl_heap->hmsg = hmsg;
  ctrl_heap->herr = OgLogGetErr(hmsg);
  ctrl_heap->hmutex = OgLogGetMutex(hmsg);
  ctrl_heap->trace = DOgHeapTraceMinimal;   // + DOgHeapTraceMemory;

  ctrl_heap->cell_size = cell_size;
  ctrl_heap->smoothing_ratio = DOgHeapDefaultSmoothingRatio;
  ctrl_heap->smoothing_window = DOgHeapDefaultSmoothedAVGWindow
      + DOgHeapDefaultSmoothedAVGWindow * ctrl_heap->smoothing_ratio;

  return ctrl_heap;
}

/**
 * Reduce heap size to current heap used and mark it has readonly to avoid append
 */
PUBLIC(og_status) OgHeapFreeze(og_heap ctrl_heap)
{
  if (ctrl_heap->freezed)
  {
    CONT;
  }

  size_t cells_number_min = ctrl_heap->cells_used;
  if (cells_number_min <= 0)
  {
    cells_number_min = 1;
  }

  // reduce cells number to cells used
 // IFE(OgHeapReallocInternal(ctrl_heap, cells_number_min));

  ctrl_heap->freezed = TRUE;

  DONE;
}

/**
 * Check is the heap can be sliced or not
 * \param handle handle to a memory heap API
 * \return 0 if ok or -1 on error
 */
PUBLIC(og_status) OgHeapCheckNotSliced(og_heap ctrl_heap)
{
  if (ctrl_heap->type == DOgHeapTypeSliced)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapCheckNotSliced on '%s': this heap is sliced, but the memory is used "
        "has continuous space so you cannot slice it", ctrl_heap->name);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }
  DONE;
}

/**
 * Check is the heap can be sliced or not
 * \param handle handle to a memory heap API
 * \return 0 if ok or -1 on error
 */
PUBLIC(og_status) OgHeapCheckSliced(og_heap ctrl_heap)
{
  if (ctrl_heap->type == DOgHeapTypeNormal)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapCheckSliced on '%s': this heap not sliced, but pointer are keep permanently, "
        "so you must slice it", ctrl_heap->name);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }
  DONE;
}

/**
 * Terminating the memory heap API
 * OgHeapFlush(handle)
 *
 * \param handle handle to a memory heap API
 * \return 0 if ok or -1 on error
 **/
PUBLIC(og_status) OgHeapFlush(og_heap ctrl_heap)
{
  if (ctrl_heap == NULL)
  {
    CONT;
  }

  if (ctrl_heap->type == DOgHeapTypeSliced)
  {

    size_t slice_used = OgHeapGetCellsUsed(ctrl_heap->hslice);
    for (size_t i = 0; i < slice_used; i++)
    {
      struct slice *slice = OgHeapGetCell(ctrl_heap->hslice, i);
      if (slice != NULL)
      {
        DPcFree(slice->heap);
      }
    }
    IFE(OgHeapFlush(ctrl_heap->hslice));

  }
  else if (ctrl_heap->type == DOgHeapTypeNormal)
  {
    DPcFree(ctrl_heap->normal_heap);
  }

  DPcFree(ctrl_heap->name);
  DPcFree(ctrl_heap);
  DONE;
}

