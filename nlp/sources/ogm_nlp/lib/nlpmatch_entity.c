/*
 *  Matching entities in the request text.
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : February 2019
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpMatchEntitiesNgrams(og_nlp_th ctrl_nlp_th, struct request_word *request_word,
    int global_max_nb_words_per_entity);
static og_status NlpMatchEntitiesNgram(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int ngram_size);

og_status NlpMatchEntities(og_nlp_th ctrl_nlp_th)
{
  int global_max_nb_words_per_entity = 0;
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    package_t package = interpret_package->package;
    if (global_max_nb_words_per_entity < package->max_nb_words_per_entity) global_max_nb_words_per_entity =
        package->max_nb_words_per_entity;
  }

  struct request_word *first_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(first_request_word) DPcErr;

  for (struct request_word *rw = first_request_word; rw; rw = rw->next)
  {
    // ignore non basic word (build from ltras)
    if (rw->self_index >= ctrl_nlp_th->basic_request_word_used) break;
    if (rw->is_expression_punctuation) continue;
    IFE(NlpMatchEntitiesNgrams(ctrl_nlp_th, rw, global_max_nb_words_per_entity));
  }

  DONE;
}

static og_status NlpMatchEntitiesNgrams(og_nlp_th ctrl_nlp_th, struct request_word *request_word,
    int global_max_nb_words_per_entity)
{
  for (int i = 2; i <= global_max_nb_words_per_entity; i++)
  {
    IFE(NlpMatchEntitiesNgram(ctrl_nlp_th, request_word, i));
  }
  DONE;
}

static og_status NlpMatchEntitiesNgram(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int ngram_size)
{
  struct request_word *request_word_list[ngram_size];
  int request_word_list_length = 0;

  if (request_word->is_punctuation) DONE;

  request_word_list[request_word_list_length++] = request_word;

  for (struct request_word *rw = request_word->next; rw; rw = rw->next)
  {
    // ignore non basic word (build from ltras)
    if (rw->self_index >= ctrl_nlp_th->basic_request_word_used) break;
    if (rw->is_expression_punctuation) continue;
    if (rw->is_punctuation) break;
    request_word_list[request_word_list_length++] = rw;
    if (request_word_list_length >= ngram_size) break;
  }

  if (request_word_list_length != ngram_size) DONE;

  int string_entity_length = 0;
  unsigned char string_entity[DOgNlpMaxEntitySize];

  for (int i = 0; i < request_word_list_length; i++)
  {
    og_string normalized_string_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word_list[i]->start);
    IFN(normalized_string_word) DPcErr;
    int length_normalized_string_word = request_word_list[i]->length;
    if (i > 0)
    {
      string_entity[string_entity_length++] = ' ';
    }
    if (string_entity_length + length_normalized_string_word + 1 >= DOgNlpMaxEntitySize)
    {
      // We do not store very long entities
      NlpLog(DOgNlpTraceMinimal, "NlpMatchEntitiesNgram: expression is too long");
      DONE;
    }
    memcpy(string_entity + string_entity_length, normalized_string_word, length_normalized_string_word);
    string_entity_length += length_normalized_string_word;
  }

  string_entity[string_entity_length] = 0;

  NlpLog(DOgNlpTraceMatch, "NlpMatchEntitiesNgram: entity to search is '%s'", string_entity);

  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    // must_spellcheck takes a lot of time when it is TRUE (do the spell check on entities
    // So, for the moment we set it to FALSE, because it is better to answer fast than to answer some corrections
    // We will see later to have an option to enable or disable spellcheck in the package
    og_status status = NlpMatchEntitiesNgramInPackage(ctrl_nlp_th, interpret_package, request_word_list,
        request_word_list_length, string_entity, string_entity_length, FALSE);
    IFE(status);
  }

  DONE;
}

og_status NlpMatchEntitiesNgramInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package,
    struct request_word **request_word_list, int request_word_list_length, unsigned char *string_entity,
    int string_entity_length, og_bool must_spellcheck)
{
  package_t package = interpret_package->package;
  if (request_word_list_length > package->max_nb_words_per_entity) DONE;
  if (package->nb_entities <= 0) DONE;

  NlpLog(DOgNlpTraceMatch, "NlpMatchEntitiesNgramInPackage: entity to search is '%s' must_spellcheck=%d", string_entity,
      must_spellcheck);

  unsigned char buffer[DPcAutMaxBufferSize + 9];
  unsigned char *p;
  int ibuffer = 0;

  p = buffer + ibuffer;
  OggNout(request_word_list_length, &p);

  ibuffer = p - buffer;
  memcpy(buffer + ibuffer, string_entity, string_entity_length);
  ibuffer += string_entity_length;
  buffer[ibuffer++] = '\1';

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  if ((retour = OgAufScanf(package->ha_entity, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      long expression_ptr;
      struct expression *expression;
      p = out;
      IFE(DOgPnin8(ctrl_nlp_th->herr,&p,&expression_ptr));
      expression = (struct expression *) expression_ptr;
      NlpLog(DOgNlpTraceMatch, "NlpMatchEntitiesNgramInPackage: found expression '%s'", expression->text);

      if (expression->input_parts_nb != request_word_list_length)
      {
        NlpThrowErrorTh(ctrl_nlp_th,
            "NlpMatchEntitiesNgramInPackage: expression->input_parts_nb (%d) != request_word_list_length (%d)",
            expression->input_parts_nb, request_word_list_length);
        DPcErr;

      }
      for (int i = 0; i < request_word_list_length; i++)
      {
        og_status status = NlpRequestInputPartAddWord(ctrl_nlp_th, request_word_list[i], interpret_package,
            expression->input_parts[i].self_index, FALSE);
        IFE(status);
      }

    }
    while ((retour = OgAufScann(package->ha_entity, &iout, out, nstate0, &nstate1, states)));
  }

  if (must_spellcheck)
  {
    og_status status = NlpLtrasEntityPackage(ctrl_nlp_th, interpret_package, request_word_list,
        request_word_list_length, string_entity, string_entity_length);
    IFE(status);
  }

  DONE;
}

