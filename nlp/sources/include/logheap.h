/*
 *  Header for library ogm_heap.dll
 *  Copyright (c) 2010-2011 Pertimm by Patrick Constant
 *  Dev : November 2010, March 2011
 *  Version 1.2
 */
#ifndef _LOGHEAPALIVE_
#include "lpcgentype.h"
#include "lpcounx.h"

// Use FILE api
#include <stdio.h>

#define DOgHeapBanner  "ogm_heap V1.00, Copyright (c) 2010-2011 Pertimm"
#define DOgHeapVersion 100

/** Trace levels **/
#define DOgHeapTraceMinimal          0x1
#define DOgHeapTraceMemory           0x2

typedef struct
{
  int sub_heap;
  size_t cell;
} og_heap_sharded_index;


typedef struct
{
  void *mem;
  size_t nb_cells_allocated;
  size_t nb_bytes_allocated;
} og_heap_allocated_memory;

/**
 *  og_heap lib handle
 */
typedef struct og_ctrl_heap *og_heap;
typedef struct og_ctrl_heap_sharded *og_heap_sharded;

DEFPUBLIC(og_heap) OgHeapInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells);
DEFPUBLIC(og_heap) OgHeapSliceInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells, size_t nb_cells_per_slice);
DEFPUBLIC(og_status) OgHeapFlush(og_heap handle);
DEFPUBLIC(og_status) OgHeapCheckNotSliced(og_heap handle);
DEFPUBLIC(og_status) OgHeapCheckSliced(og_heap handle);
DEFPUBLIC(og_status) OgHeapFreeze(og_heap ctrl_heap);
DEFPUBLIC(size_t) OgHeapAddCell(og_heap handle);
DEFPUBLIC(size_t) OgHeapAddCells(og_heap handle, int nb_cells);
DEFPUBLIC(void *) OgHeapNewCell(og_heap handle, size_t *p_cell_index);
DEFPUBLIC(void *) OgHeapGetCell(og_heap handle, size_t cell_index);
DEFPUBLIC(og_heap_allocated_memory) OgHeapGetFullAllocatedMemory(og_heap handle);
DEFPUBLIC(void *) OgHeapGetBufferNew(og_heap ctrl_heap, size_t buffer_cell_number);
DEFPUBLIC(void *) OgHeapGetBufferReuse(og_heap ctrl_heap, size_t buffer_cell_number);
DEFPUBLIC(og_status) OgHeapAppend(og_heap handle, size_t nb_cells, void const *cells);
DEFPUBLIC(og_status) OgHeapReplace(og_heap handle, size_t position, size_t nb_cells_to_delete, size_t nb_cells_to_add, void const *cells_to_add);

DEFPUBLIC(size_t) OgHeapGetCellsNumber(og_heap handle);
DEFPUBLIC(size_t) OgHeapGetCellsUsed(og_heap handle);
DEFPUBLIC(int)    OgHeapGetSmoothingWindowSize(og_heap handle);
DEFPUBLIC(size_t) OgHeapGetSmoothedAverage(og_heap handle);
DEFPUBLIC(float)  OgHeapGetSmoothingRatio(og_heap handle);
DEFPUBLIC(og_status) OgHeapSetCellsUsed(og_heap handle, size_t cells_used);
DEFPUBLIC(og_status) OgHeapReset(og_heap handle);
DEFPUBLIC(og_status) OgHeapResetToMinimal(og_heap ctrl_heap);
DEFPUBLIC(og_status) OgHeapResetWithoutReduce(og_heap handle);
DEFPUBLIC(og_status) OgHeapSetTrace(og_heap handle, int trace);
DEFPUBLIC(og_status) OgHeapSetDefaultValue(og_heap handle, int default_mem_set_value);
DEFPUBLIC(size_t) OgHeapGetAllocatedMemory(og_heap handle);
DEFPUBLIC(size_t) OgHeapCopy(og_heap heap_from, og_heap heap_to);

DEFPUBLIC(og_status) OgHeapFileWrite(og_heap handle, FILE *fd, size_t *p_nb_written_bytes);
DEFPUBLIC(og_status) OgHeapFileRead(og_heap handle, FILE *fd, size_t nb_bytes_to_read);

DEFPUBLIC(og_heap_sharded) OgHeapShardedInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells, int nb_sub_heaps);
DEFPUBLIC(og_heap_sharded) OgHeapShardedSliceInit(void *hmsg, unsigned char *name, size_t cell_size, size_t nb_init_cells, int nb_sub_heaps);
DEFPUBLIC(og_status) OgHeapShardedFlush(og_heap_sharded handle);
DEFPUBLIC(og_status) OgHeapShardedCheckNotSliced(og_heap_sharded handle);
DEFPUBLIC(og_status) OgHeapShardedCheckSliced(og_heap_sharded handle);
DEFPUBLIC(og_status) OgHeapShardedAppend(og_heap_sharded handle, int sub_heap, size_t nb_cells, void const *cells, og_heap_sharded_index *pcell_index);
DEFPUBLIC(void *) OgHeapShardedNewCell(og_heap_sharded sharded_ctrl_heap, int sub_heap, og_heap_sharded_index *p_sharded_cell_index);
DEFPUBLIC(void *) OgHeapShardedGetCell(og_heap_sharded handle, og_heap_sharded_index cell_index);
DEFPUBLIC(size_t) OgHeapShardedGetCellsUsed(og_heap_sharded sharded_ctrl_heap, int sub_heap);
DEFPUBLIC(size_t) OgHeapShardedGetAllocatedMemory(og_heap_sharded handle);
DEFPUBLIC(og_status) OgHeapShardedReset(og_heap_sharded handle);

#define _LOGHEAPALIVE_
#endif

