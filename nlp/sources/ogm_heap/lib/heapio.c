/*
 *  Miscellaneous API function for heap memory I/O
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
 */
#include "ogm_heap.h"

/**
 * Write all cells to 'fd' file
 *
 * @param ctrl_heap heap handle
 * @param fd file descriptor to read cell from
 * @param p_nb_written_bytes number of written bytes in file
 * @return function status
 */
PUBLIC(og_status) OgHeapFileWrite(og_heap ctrl_heap, FILE *fd, size_t *p_nb_written_bytes)
{

  if (ctrl_heap->cells_used == 0)
  {
    if (p_nb_written_bytes != NULL)
    {
      *p_nb_written_bytes = 0;
    }

    CONT;
  }

  if (ctrl_heap->type == DOgHeapTypeSliced)
  {

    size_t nb_cells_to_write = ctrl_heap->cells_used;
    size_t last_Islice_to_write = nb_cells_to_write / ctrl_heap->nb_cells_per_slice;

    size_t nb_written_cells = 0;
    for (size_t Islice = 0; Islice <= last_Islice_to_write; Islice++)
    {
      struct slice *slice = OgHeapGetCell(ctrl_heap->hslice, Islice);
      if (slice == NULL)
      {
        // check disk space
        og_char_buffer erreur[DOgErrorSize];
        snprintf(erreur, DOgErrorSize, "OgHeapFileWrite failed : unable to access to slice %zu.", Islice);
        OgErr(ctrl_heap->herr, erreur);
        OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

        DPcErr;
      }

      // read cells from file
      size_t iter_nb_cells_to_write = (nb_cells_to_write - nb_written_cells);
      if (ctrl_heap->nb_cells_per_slice < iter_nb_cells_to_write)
      {
        iter_nb_cells_to_write = ctrl_heap->nb_cells_per_slice;
      }

      // get heap buffer to write
      char *bytes = slice->heap;

      // write in fd
      size_t iter_nb_written_cells = fwrite(bytes, ctrl_heap->cell_size, iter_nb_cells_to_write, fd);
      if (iter_nb_written_cells != iter_nb_cells_to_write)
      {
        // check disk space
        og_char_buffer erreur[DOgErrorSize];
        snprintf(erreur, DOgErrorSize, "OgHeapFileWrite failed to write Islice %zu : iter_nb_written_cells (%zu)"
            " != iter_nb_cells_to_write (%zu)", Islice, iter_nb_written_cells, iter_nb_cells_to_write);
        OgErr(ctrl_heap->herr, erreur);
        OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

        DPcErr;
      }

      nb_written_cells += iter_nb_written_cells;

    }

    if (p_nb_written_bytes != NULL)
    {
      *p_nb_written_bytes = nb_written_cells * ctrl_heap->cell_size;
    }

  }
  else if (ctrl_heap->type == DOgHeapTypeNormal)
  {

    // compute nb bytes to write
    size_t nb_cells_to_write = ctrl_heap->cells_used;

    // get heap buffer to write
    char *bytes = ctrl_heap->normal_heap;

    // write in fd
    size_t nb_written_cells = fwrite(bytes, ctrl_heap->cell_size, nb_cells_to_write, fd);
    if (nb_written_cells != nb_cells_to_write)
    {
      // check disk space
      og_char_buffer erreur[DOgErrorSize];
      snprintf(erreur, DOgErrorSize, "OgHeapFileWrite failed : nb_written_cells (%zu) != nb_cells_to_write (%zu)",
          nb_written_cells, nb_cells_to_write);
      OgErr(ctrl_heap->herr, erreur);
      OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

      DPcErr;
    }

    if (p_nb_written_bytes != NULL)
    {
      *p_nb_written_bytes = nb_written_cells * ctrl_heap->cell_size;
    }

  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapFileWrite on '%s': function not yet implemented this type heap (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  DONE;
}

/**
 * Read all cells form 'fd' file
 *
 * You may need to call OgHeapReset.
 *
 * @param ctrl_heap heap handle
 * @param fd file descriptor to read cell from
 * @param nb_bytes_to_read maximum nb bytes to read
 * @return function status
 */
PUBLIC(og_status) OgHeapFileRead(og_heap ctrl_heap, FILE *fd, size_t nb_bytes_to_read)
{
  if (ctrl_heap->freezed)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapFileRead on '%s': is freezed you cannot load it from a file",
        ctrl_heap->name);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);
    DPcErr;
  }

  // add enough cells to ensure read data can be written
  size_t nb_cell_to_read = nb_bytes_to_read / ctrl_heap->cell_size;

  if (ctrl_heap->type == DOgHeapTypeSliced)
  {
    size_t buffer_nb_cells = ctrl_heap->nb_cells_per_slice;
    size_t buffer_size = ctrl_heap->cell_size * buffer_nb_cells;

    // init buffer with cell size
    char bytes[buffer_size];

    size_t nb_total_read_cells = 0;
    while (nb_total_read_cells < nb_cell_to_read)
    {
      // read cells from file
      size_t iter_nb_cell_to_read = nb_cell_to_read - nb_total_read_cells;
      if (buffer_nb_cells < iter_nb_cell_to_read)
      {
        iter_nb_cell_to_read = buffer_nb_cells;
      }

      // reset buffer
      memset(bytes, ctrl_heap->default_mem_set_value, buffer_size);

      size_t nb_cells_read = fread(bytes, ctrl_heap->cell_size, iter_nb_cell_to_read, fd);
      if (nb_cells_read != iter_nb_cell_to_read)
      {
        og_char_buffer erreur[DOgErrorSize];
        snprintf(erreur, DOgErrorSize,
            "OgHeapFileRead: failed read sliced heap : nb_cells_read (%zu) != iter_nb_cell_to_read (%zu)",
            nb_cells_read, iter_nb_cell_to_read);
        OgErr(ctrl_heap->herr, erreur);
        OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

        DPcErr;
      }

      // append data in heap
      IFE(HeapSliceAppend(ctrl_heap, nb_cells_read, bytes));

      nb_total_read_cells += nb_cells_read;
    }

  }
  else if (ctrl_heap->type == DOgHeapTypeNormal)
  {

    // add enough cells to ensure read data can be written
    size_t nb_free_cells = ctrl_heap->normal_cells_number - ctrl_heap->cells_used;
    if (nb_free_cells < nb_cell_to_read)
    {
      size_t nb_cells_to_add = nb_cell_to_read - nb_free_cells;
      IF(OgHeapAddCells(ctrl_heap, nb_cells_to_add))
      {
        og_char_buffer erreur[DOgErrorSize];
        snprintf(erreur, DOgErrorSize, "OgHeapFileRead: failed to add %zu cells", nb_cells_to_add);
        OgErr(ctrl_heap->herr, erreur);
        OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

        DPcErr;
      }
    }

    // get heap buffer to read to
    char *bytes = ctrl_heap->normal_heap;

    // read cells from file
    size_t nb_cells_read = fread(bytes, ctrl_heap->cell_size, nb_cell_to_read, fd);

    if (nb_cells_read != nb_cell_to_read)
    {
      og_char_buffer erreur[DOgErrorSize];
      snprintf(erreur, DOgErrorSize, "OgHeapFileRead: failed read heap : nb_cells_read (%zu) != nb_cell_to_read (%zu)",
          nb_cells_read, nb_cell_to_read);
      OgErr(ctrl_heap->herr, erreur);
      OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

      DPcErr;
    }

    ctrl_heap->cells_used = nb_cells_read;

  }
  else
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgHeapFileRead on '%s': function not yet implemented this type heap (%d)",
        ctrl_heap->name, ctrl_heap->type);
    OgErr(ctrl_heap->herr, erreur);
    OG_LOG_BACKTRACE(ctrl_heap->hmsg, erreur);

    DPcErr;
  }

  DONE;
}

