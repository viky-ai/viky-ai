/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpRequestPositionCmp(gconstpointer ptr_request_position1, gconstpointer ptr_request_position2,
    gpointer user_data);

og_status NlpRequestPositionAdd(og_nlp_th ctrl_nlp_th, int start, int length, size_t *pIrequest_position)
{
  struct request_position *request_position = OgHeapNewCell(ctrl_nlp_th->hrequest_position, pIrequest_position);
  IFn(request_position) DPcErr;
  IF(*pIrequest_position) DPcErr;
  request_position->start = start;
  request_position->length = length;
  DONE;
}

og_status NlpRequestPositionSort(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb)
{
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_position_start);
  IFN(request_position) DPcErr;
  g_qsort_with_data(request_position, request_positions_nb, sizeof(struct request_position), NlpRequestPositionCmp,
  NULL);

  DONE;
}

og_bool NlpRequestPositionSame(og_nlp_th ctrl_nlp_th, int request_position_start1, int request_positions_nb1,
    int request_position_start2, int request_positions_nb2)
{
  if (request_positions_nb1 != request_positions_nb2) return FALSE;

  struct request_position *request_position1 = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_position_start1);
  IFN(request_position1) DPcErr;
  struct request_position *request_position2 = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_position_start2);
  IFN(request_position2) DPcErr;

  if (memcmp(request_position1, request_position2, request_positions_nb1 * sizeof(struct request_position))) return FALSE;
  return TRUE;
}

og_bool NlpRequestPositionOverlap(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb)
{
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_position_start);
  IFN(request_position) DPcErr;
  for (int i = 0; i + 1 < request_positions_nb; i++)
  {
    if (!memcmp(request_position + i, request_position + i + 1, sizeof(struct request_position))) return TRUE;
  }
  return FALSE;
}

static int NlpRequestPositionCmp(gconstpointer ptr_request_position1, gconstpointer ptr_request_position2,
    gpointer user_data)
{
  struct request_position *request_position1 = (struct request_position *) ptr_request_position1;
  struct request_position *request_position2 = (struct request_position *) ptr_request_position2;

  if (request_position1->start != request_position2->start)
  {
    return (request_position1->start - request_position2->start);
  }
  // Just to make sure it is
  return request_position1->length - request_position2->length;
}

int NlpRequestPositionString(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb, int size,
    char *string)
{
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_position_start);
  IFN(request_position) DPcErr;
  int length = 0;
  string[length] = 0;
  for (int i = 0; i < request_positions_nb; i++)
  {
    length = strlen(string);
    snprintf(string + length, size - length, "%s%d:%d", (i ? " " : ""), request_position[i].start,
        request_position[i].length);
  }

  DONE;
}

