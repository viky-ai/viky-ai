/*
 *  Internal header for handling memory heaps
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include <logheap.h>
#include <logmsg.h>
#include <lpcgentype.h>

#define DOgHeapIncreaseFactorMax                 1.25
#define DOgHeapIncreaseFactorMin                 1.0
#define DOgHeapDecreaseFactor                    0.5
#define DOgHeapDefaultSmoothedAVGWindow           50
#define DOgHeapDefaultSmoothingRatio             1.0
#define DOgHeapDefaultSmoothingRatioThreshold    2.0

typedef enum
{
  DOgHeapTypeNormal = 0, DOgHeapTypeSliced = 1
} og_heap_type;

struct slice
{
  char *heap;
};

struct og_ctrl_heap
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  int trace;
  unsigned char *name;

  /** Type of the heap */
  og_heap_type type;

  /** Read only heap */
  og_bool freezed;

  /** Default value in use in memset */
  int default_mem_set_value;

  size_t cells_used;
  size_t cell_size;

  // reset stats
  /** this is intended to be the 'number of samples' to weight the memory average with */
  int smoothing_window;

  /**
   * this is a smoothing coefficient calculated as the ratio:
   * current value / average at the previous step
   * and it is used to shape both the smoothing window_and the memory increase factor
   */
  float smoothing_ratio;

  /**
   * this is the memory average, smoothed with the window and ratio formerly declared
   * (it is not a moving average as the history of samples is not stored)
   */
  size_t cells_used_smoothed_average;

  /** Max cells used between to reset **/
  size_t max_cells_used;
  size_t reset_cmpt;   // TODO reset counter is use only for debug, to be remove

  union
  {

    struct
    {
      // Normal Heap part
      size_t normal_cells_number;
      char *normal_heap;
    };

    struct
    {
      // Sliced Heap part
      size_t nb_cells_per_slice;
      og_heap hslice;   // Normal heap on slices
    };

  };

};

struct og_ctrl_heap_sharded
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  int trace;
  unsigned char *name;

  // Several Normal heap
  int nb_sub_heaps;
  og_heap *sub_heap;
};

struct og_ctrl_heap * HeapInit(void *hmsg, unsigned char *name, size_t cell_size);
int HeapSliceAppend(struct og_ctrl_heap *ctrl_heap, size_t nb_cells, void const *cells);
int HeapSliceAlloc(struct og_ctrl_heap *ctrl_heap);
void *HeapNormalGetCell(struct og_ctrl_heap *ctrl_heap, size_t cell_index);
void *HeapSliceGetCell(struct og_ctrl_heap *ctrl_heap, size_t cell_index);
og_status OgHeapTestReallocInternal(og_heap ctrl_heap, size_t nb_added_cells);
og_status OgHeapReallocInternal(og_heap ctrl_heap, size_t new_cells_number);
og_status OgHeapReallocNormal(struct og_ctrl_heap *ctrl_heap, size_t new_cells_number);

