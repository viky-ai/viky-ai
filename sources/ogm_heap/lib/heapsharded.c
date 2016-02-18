/*
 *  Handling multiple heaps for parallelisation
 *  Copyright (c) 2015 Pertimm by Patrick Constant
 *  Dev : January 2015
 *  Version 1.0
 */
#include "ogm_heap.h"

static og_heap_sharded HeapShardedInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells,
    int nb_sub_heaps, og_heap_type type);

/**
 * Init a normal sharded heap
 */
PUBLIC(og_heap_sharded) OgHeapShardedInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells,
    int nb_sub_heaps)
{
  return HeapShardedInit(hmsg, name, cell_size, nb_init_cells, nb_sub_heaps, DOgHeapTypeNormal);
}

/**
 * Init a sliced sharded heap
 */
PUBLIC(og_heap_sharded) OgHeapShardedSliceInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells,
    int nb_sub_heaps)
{
  return HeapShardedInit(hmsg, name, cell_size, nb_init_cells, nb_sub_heaps, DOgHeapTypeSliced);
}

static og_heap_sharded HeapShardedInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells,
    int nb_sub_heaps, og_heap_type type)
{
  // adjust subheap number
  if (nb_sub_heaps <= 0)
  {
    nb_sub_heaps = 1;
  }

  size_t size = sizeof(struct og_ctrl_heap_sharded);

  int name_length = strlen(name);

  struct og_ctrl_heap_sharded *ctrl_heap = NULL;
  IFn(ctrl_heap = (struct og_ctrl_heap_sharded *) malloc(size))
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapShardedInitInternal on '%s': malloc error on ctrl_heap with size %ld bytes",
        name, size);
    OgErr(OgLogGetErr(hmsg), erreur);
    return NULL;
  }
  memset(ctrl_heap, 0, size);

  /** +2 because of the zero char at the end and +1 for safety **/
  IFn(ctrl_heap->name = (unsigned char *) malloc(name_length + 2))
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapShardedInitInternal on '%s': malloc error on name with size %d bytes", name,
        name_length);
    DPcFree(ctrl_heap);
    OgErr(OgLogGetErr(hmsg), erreur);
    return NULL;
  }

  strcpy(ctrl_heap->name, name);
  ctrl_heap->hmsg = hmsg;
  ctrl_heap->herr = OgLogGetErr(hmsg);
  ctrl_heap->hmutex = OgLogGetMutex(hmsg);
  ctrl_heap->trace = DOgHeapTraceMinimal;   // + DOgHeapTraceMemory;

  ctrl_heap->nb_sub_heaps = nb_sub_heaps;

  size_t sub_heaps_size = nb_sub_heaps * sizeof(og_heap *);
  IFn(ctrl_heap->sub_heap = (og_heap *) malloc(sub_heaps_size))
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapShardedInitInternal on '%s': malloc error on sub_heap with size %ld bytes",
        name, sub_heaps_size);
    OgErr(ctrl_heap->herr, erreur);
    return NULL;
  }
  memset(ctrl_heap->sub_heap, 0, sub_heaps_size);

  int nb_init_cells_for_sub_heap = (nb_init_cells / nb_sub_heaps) + 1;
  for (int i = 0; i < nb_sub_heaps; i++)
  {
    char name_for_sub_heap[DPcPathSize];

    char * str_type = "";
    if (type == DOgHeapTypeSliced)
    {
      str_type = "_sliced";
    }

    snprintf(name_for_sub_heap, DPcPathSize, "%s_sub_heap%s_%d", name, str_type, i);

    og_heap sub_heap = NULL;

    if (type == DOgHeapTypeNormal)
    {
      sub_heap = OgHeapInit(hmsg, name_for_sub_heap, cell_size, nb_init_cells_for_sub_heap);
    }
    else if (type == DOgHeapTypeSliced)
    {
      sub_heap = OgHeapSliceInit(hmsg, name_for_sub_heap, cell_size, 1, nb_init_cells_for_sub_heap);
    }

    IFn(sub_heap)
    {
      og_char_buffer erreur[DOgErrorSize];
      snprintf(erreur, DOgErrorSize, "OgHeapShardedInitInternal on '%s': OgHeapInit error on sub_heap %s", name,
          name_for_sub_heap);
      OgErr(ctrl_heap->herr, erreur);
      return NULL;
    }

    ctrl_heap->sub_heap[i] = sub_heap;
  }

  return ctrl_heap;
}

/**
 * Check is the heap can be sliced or not
 * \param handle handle to a memory heap API
 * \return 0 if ok or -1 on error
 */
PUBLIC(og_status) OgHeapShardedCheckNotSliced(og_heap_sharded handle)
{
  og_heap current_heap = handle->sub_heap[0];

  if (current_heap->type == DOgHeapTypeSliced)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapShardedCheckNotSliced on '%s': this heap is sliced, but the memory is used "
        "has continuous space so you cannot slice it", handle->name);
    OgErr(handle->herr, erreur);
    OG_LOG_BACKTRACE(handle->hmsg, erreur);

    DPcErr;
  }

  DONE;
}

/**
 * Check is the heap can be sliced or not
 * \param handle handle to a memory heap API
 * \return 0 if ok or -1 on error
 */
PUBLIC(og_status) OgHeapShardedCheckSliced(og_heap_sharded handle)
{
  og_heap current_heap = handle->sub_heap[0];

  if (current_heap->type != DOgHeapTypeSliced)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize,
        "OgHeapShardedCheckSliced on '%s': this heap not sliced, but pointer are keep permanently, "
            "so you must slice it", handle->name);
    OgErr(handle->herr, erreur);
    OG_LOG_BACKTRACE(handle->hmsg, erreur);

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
PUBLIC(og_status) OgHeapShardedFlush(og_heap_sharded handle)
{
  IFn(handle) DONE;
  struct og_ctrl_heap_sharded *ctrl_heap = handle;
  if (ctrl_heap == NULL)
  {
    CONT;
  }

  for (int i = 0; i < ctrl_heap->nb_sub_heaps; i++)
  {
    IFE(OgHeapFlush(ctrl_heap->sub_heap[i]));
  }
  DPcFree(ctrl_heap->sub_heap);

  DPcFree(ctrl_heap->name);
  DPcFree(ctrl_heap);
  DONE;
}

PUBLIC(og_status) OgHeapShardedAppend(og_heap_sharded sharded_ctrl_heap, int sub_heap, size_t nb_added_cells,
    void const *cells, og_heap_sharded_index *pcell_index)
{
  IFn(sharded_ctrl_heap) DONE;

  if (nb_added_cells <= 0) DONE;

  if (sub_heap < 0 || sub_heap >= sharded_ctrl_heap->nb_sub_heaps)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapShardedAppend on '%s': with cell_index.sub_heap (%d) out of range (0:%d)",
        sharded_ctrl_heap->name, sub_heap, sharded_ctrl_heap->nb_sub_heaps);
    OgErr(sharded_ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(sharded_ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  og_heap current_heap = sharded_ctrl_heap->sub_heap[sub_heap];

  if (pcell_index != NULL)
  {
    pcell_index->sub_heap = sub_heap;
    pcell_index->cell = current_heap->cells_used;
  }

  IFE(OgHeapAppend(current_heap, nb_added_cells, cells));

  DONE;
}

PUBLIC(void *) OgHeapShardedGetCell(og_heap_sharded sharded_ctrl_heap, og_heap_sharded_index sharded_cell_index)
{
  IFn(sharded_ctrl_heap) return NULL;

  og_char_buffer erreur[DOgErrorSize];

  int sharded_heap_index = sharded_cell_index.sub_heap;
  if(sharded_heap_index < 0 || sharded_heap_index >= sharded_ctrl_heap->nb_sub_heaps )
  {
    snprintf(erreur, DOgErrorSize, "OgHeapShardedGetCell on '%s': with cell_index.sub_heap (%d) out of range (0:%d)",
        sharded_ctrl_heap->name, sharded_heap_index, sharded_ctrl_heap->nb_sub_heaps);
    OgErr(sharded_ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(sharded_ctrl_heap->hmsg, erreur);

    return NULL;
  }

  // fork OgHeapGetCell for optimization current heap
  {
    og_heap current_heap = sharded_ctrl_heap->sub_heap[sharded_heap_index];

    size_t cell_index = sharded_cell_index.cell;
    if (cell_index == (size_t) -1)
    {
      snprintf(erreur, DOgErrorSize, "OgHeapShardedGetCell on '%s': cell_index == -1", current_heap->name);
      OgErr(current_heap->herr, erreur);
      OG_LOG_BACKTRACE(current_heap->hmsg, erreur);

      return NULL;
    }

    if (cell_index >= OgHeapGetCellsNumber(current_heap))
    {
      snprintf(erreur, DOgErrorSize, "OgHeapShardedGetCell on '%s': cell_index (%ld) >= OgHeapGetCellsNumber (%ld)",
          current_heap->name, cell_index, OgHeapGetCellsNumber(current_heap));
      OgErr(current_heap->herr, erreur);
      OG_LOG_BACKTRACE(current_heap->hmsg, erreur);

      return NULL;
    }

    // TODO OgHeapGetCell change to >= , it may crash a lot of test
    else if (cell_index != 0 && cell_index > current_heap->cells_used)
    {
      snprintf(erreur, DOgErrorSize, "OgHeapShardedGetCell on '%s': cell_index (%ld) >= ctrl_heap->cells_used (%ld)",

          current_heap->name, cell_index, current_heap->cells_used);
      OgErr(current_heap->herr, erreur);
      OG_LOG_BACKTRACE(current_heap->hmsg, erreur);

      return NULL;
    }

    if (current_heap->type == DOgHeapTypeSliced)
    {
      return HeapSliceGetCell(current_heap, cell_index);
    }
    else if (current_heap->type == DOgHeapTypeNormal)
    {
      return HeapNormalGetCell(current_heap, cell_index);
    }
    else
    {
      snprintf(erreur, DOgErrorSize, "OgHeapShardedGetCell on '%s': function not yet implemented for heap type (%d)",
          current_heap->name, current_heap->type);
      OgErr(current_heap->herr, erreur);
      OG_LOG_BACKTRACE(current_heap->hmsg, erreur);

      return NULL;
    }

  }

}

PUBLIC(og_status) OgHeapShardedReset(og_heap_sharded handle)
{
  IFn(handle) DONE;
  struct og_ctrl_heap_sharded *ctrl_heap = handle;

  for (int i = 0; i < ctrl_heap->nb_sub_heaps; i++)
  {
    IFE(OgHeapReset(ctrl_heap->sub_heap[i]));
  }
  DONE;
}

/**
 * Getting the total memory allocated for the heap
 * OgHeapGetAllocatedMemory(handle)
 *
 * \param handle handle to a memory heap API
 * \return the total size of allocated memory in bytes
 **/
PUBLIC(size_t) OgHeapShardedGetAllocatedMemory(og_heap_sharded handle)
{
  struct og_ctrl_heap_sharded *ctrl_heap = handle;

  size_t size = sizeof(struct og_ctrl_heap);
  size += sizeof(og_heap *) * ctrl_heap->nb_sub_heaps;
  for (int i = 0; i < ctrl_heap->nb_sub_heaps; i++)
  {
    size += OgHeapGetAllocatedMemory(ctrl_heap->sub_heap[i]);
  }
  return size;
}

/**
 * Adding new a zero initialized memory cell and getting its index and a cell pointer
 *
 * \param handle handle to a memory heap API
 * \param p_cell_index cell index
 * \return memory pointer or NULL on error
 */
PUBLIC(void *) OgHeapShardedNewCell(og_heap_sharded sharded_ctrl_heap, int sub_heap, og_heap_sharded_index *p_sharded_cell_index)
{
  IFn(sharded_ctrl_heap) return NULL;

  IF(OgHeapShardedAppend(sharded_ctrl_heap, sub_heap, 1, NULL, p_sharded_cell_index)) return NULL;

  og_heap current_heap = sharded_ctrl_heap->sub_heap[sub_heap];

  return OgHeapGetCell(current_heap, p_sharded_cell_index->cell);
}

/**
 * Getting the total number of used cells of the sub_heap 'sub_heap' of a sharded heap
 * OgHeapShardedGetCellsUsed(handle)
 *
 * \param sharded_ctrl_heap handle to a sharded memory heap API
 * \param sub_heap sub_heap 'sub_heap' of a sharded heap
 * \return the total number of used cells of the heap
 */
PUBLIC(size_t) OgHeapShardedGetCellsUsed(og_heap_sharded sharded_ctrl_heap, int sub_heap)
{
  IFn(sharded_ctrl_heap) return (size_t) 0;
  og_heap current_heap = sharded_ctrl_heap->sub_heap[sub_heap];
  return OgHeapGetCellsUsed(current_heap);
}

