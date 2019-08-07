/*
 *  Matching entities in the request text.
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : February 2019
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpMatchEntitiesInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package);
static og_status NlpMatchEntitiesInPackage1(struct nlp_match_entities_ctrl *me_ctrl);
static og_status NlpMatchEntitiesAddWord(struct nlp_match_entities_ctrl *me_ctrl, struct request_word *request_word);
static og_status NlpMatchEntitiesRemoveWord(struct nlp_match_entities_ctrl *me_ctrl);
static og_status NlpMatchEntitiesChangeToAlternativeWord(struct nlp_match_entities_ctrl *me_ctrl,
    struct request_word *request_word);
static og_status NlpMatchEntitiesInPackageRecursive(struct nlp_match_entities_ctrl *me_ctrl);
static og_bool NlpMatchEntity(struct nlp_match_entities_ctrl *me_ctrl);
static og_bool NlpMatchEntityAdd(struct nlp_match_entities_ctrl *me_ctrl, int ibuffer, unsigned char *buffer, int iout,
    unsigned char *out);

og_status NlpMatchEntities(og_nlp_th ctrl_nlp_th)
{
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  IFN(interpret_packages) DPcErr;
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = interpret_packages + i;
    if (interpret_package->package->ha_entity)
    {
      IFE(NlpMatchEntitiesInPackage(ctrl_nlp_th, interpret_packages + i));
    }
  }
  DONE;
}

static og_status NlpMatchEntitiesInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package)
{
  struct nlp_match_entities_ctrl me_ctrl[1];
  memset(me_ctrl, 0, sizeof(struct nlp_match_entities_ctrl));
  me_ctrl->ctrl_nlp_th = ctrl_nlp_th;
  me_ctrl->interpret_package = interpret_package;
  me_ctrl->expression_hash = g_hash_table_new(g_direct_hash, g_direct_equal);

  og_status status = NlpMatchEntitiesInPackage1(me_ctrl);

  g_hash_table_destroy(me_ctrl->expression_hash);

  return status;
}

static og_status NlpMatchEntitiesInPackage1(struct nlp_match_entities_ctrl *me_ctrl)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;
  struct request_word *first_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(first_request_word) DPcErr;

  for (struct request_word *rw = first_request_word; rw; rw = rw->next)
  {
    // ignore non basic word (build from ltras)
    if (rw->self_index >= ctrl_nlp_th->basic_request_word_used) break;
    if (rw->is_expression_punctuation) continue;

    IFE(NlpMatchEntitiesAddWord(me_ctrl, rw));
    IFE(NlpMatchEntitiesInPackageRecursive(me_ctrl));
    IFE(NlpMatchEntitiesRemoveWord(me_ctrl));
  }
  DONE;
}

static og_status NlpMatchEntitiesAddWord(struct nlp_match_entities_ctrl *me_ctrl, struct request_word *request_word)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;

  int string_entity_length = 0;
  if (me_ctrl->request_word_list_length > 0)
  {
    string_entity_length = me_ctrl->string_entity_length_list[me_ctrl->request_word_list_length - 1];
  }
  me_ctrl->request_word_list[me_ctrl->request_word_list_length] = request_word;
  me_ctrl->alternative_request_word_list[me_ctrl->request_word_list_length] = NULL;

  og_string normalized_string_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(normalized_string_word) DPcErr;
  int length_normalized_string_word = request_word->length;
  if (string_entity_length + length_normalized_string_word + 1 >= DOgNlpMaxEntitySize)
  {
    // We do not store very long entities
    NlpLog(DOgNlpTraceMinimal, "NlpMatchEntitiesAddWord: entity is too long");
    DONE;
  }
  memcpy(me_ctrl->string_entity + string_entity_length, normalized_string_word, length_normalized_string_word);

  string_entity_length += length_normalized_string_word;
  me_ctrl->string_entity[string_entity_length++] = ' ';
  me_ctrl->string_entity[string_entity_length] = 0;

  me_ctrl->string_entity_length_list[me_ctrl->request_word_list_length] = string_entity_length;

  me_ctrl->request_word_list_length++;
  DONE;
}

static og_status NlpMatchEntitiesRemoveWord(struct nlp_match_entities_ctrl *me_ctrl)
{
  me_ctrl->request_word_list_length--;
  int string_entity_length = me_ctrl->string_entity_length_list[me_ctrl->request_word_list_length - 1];

  me_ctrl->string_entity[string_entity_length] = 0;
  DONE;
}

static og_status NlpMatchEntitiesChangeToAlternativeWord(struct nlp_match_entities_ctrl *me_ctrl,
    struct request_word *request_word)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;

  if (me_ctrl->request_word_list_length <= 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchEntitiesChangeToAlternativeWord: no word in request_word_list");
    DPcErr;
  }

  int string_entity_length = 0;
  if (me_ctrl->request_word_list_length > 1)
  {
    string_entity_length = me_ctrl->string_entity_length_list[me_ctrl->request_word_list_length - 2];
  }

  og_string normalized_string_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(normalized_string_word) DPcErr;
  int length_normalized_string_word = request_word->length;
  if (string_entity_length + length_normalized_string_word + 1 >= DOgNlpMaxEntitySize)
  {
    // We do not store very long entities
    NlpLog(DOgNlpTraceMinimal, "NlpMatchEntitiesAddWord: entity is too long");
    DONE;
  }
  memcpy(me_ctrl->string_entity + string_entity_length, normalized_string_word, length_normalized_string_word);

  string_entity_length += length_normalized_string_word;
  me_ctrl->string_entity[string_entity_length++] = ' ';
  me_ctrl->string_entity[string_entity_length] = 0;

  me_ctrl->string_entity_length_list[me_ctrl->request_word_list_length - 1] = string_entity_length;

  me_ctrl->alternative_request_word_list[me_ctrl->request_word_list_length - 1] = request_word;

  DONE;

}

og_status NlpMatchEntitiesChangeToAlternativeString(struct nlp_match_entities_ctrl *me_ctrl,
    int length_normalized_string_word, unsigned char *normalized_string_word)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;

  if (me_ctrl->request_word_list_length <= 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchEntitiesChangeToAlternativeWord: no word in request_word_list");
    DPcErr;
  }

  int string_entity_length = 0;
  if (string_entity_length + length_normalized_string_word + 1 >= DOgNlpMaxEntitySize)
  {
    // We do not store very long entities
    NlpLog(DOgNlpTraceMinimal, "NlpMatchEntitiesAddWord: entity is too long");
    DONE;
  }
  memcpy(me_ctrl->string_entity + string_entity_length, normalized_string_word, length_normalized_string_word);

  string_entity_length += length_normalized_string_word;
  me_ctrl->string_entity[string_entity_length++] = ' ';
  me_ctrl->string_entity[string_entity_length] = 0;

  me_ctrl->string_entity_length_list[me_ctrl->request_word_list_length - 1] = string_entity_length;
  DONE;

}

static og_status NlpMatchEntitiesInPackageRecursive(struct nlp_match_entities_ctrl *me_ctrl)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;

  // Working on the basic word
  IFE(NlpMatchCurrentEntity(me_ctrl));

  // Looking for lemmatisations
  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  struct request_word *request_words = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_words) DPcErr;

  // Looking for spelling mistakes
  IFE(NlpLtrasEntity(me_ctrl));

  struct request_word *last_request_word = me_ctrl->request_word_list[me_ctrl->request_word_list_length - 1];

  for (int i = ctrl_nlp_th->basic_request_word_used; i < request_word_used; i++)
  {
    struct request_word *request_word = request_words + i;
    if (request_word->start_position == last_request_word->start_position
        && request_word->length_position == last_request_word->length_position)
    {
      IFE(NlpMatchEntitiesChangeToAlternativeWord(me_ctrl, request_word));
      IFE(NlpMatchCurrentEntity(me_ctrl));
    }
  }

  DONE;
}

og_status NlpMatchCurrentEntity(struct nlp_match_entities_ctrl *me_ctrl)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;
  NlpLog(DOgNlpTraceMatch, "NlpMatchCurrentEntity: request_word to search is '%s' length=%d", me_ctrl->string_entity,
      me_ctrl->request_word_list_length);

  og_bool found_entity = NlpMatchEntity(me_ctrl);
  IFE(found_entity);

  // Don't go any further, the rest of the entity will not match
  if (!found_entity) DONE;

  // No entity found but go on, we are on the right track, we have a partial match
  // Found an entity and go on for more entities (longer)

  struct request_word *last_request_word = me_ctrl->request_word_list[me_ctrl->request_word_list_length - 1];
  struct request_word *next_request_word = last_request_word->next;

  IFN(next_request_word) DONE;

  for (struct request_word *rw = next_request_word; rw; rw = rw->next)
  {
    // ignore non basic word (build from ltras)
    if (rw->self_index >= ctrl_nlp_th->basic_request_word_used) break;
    if (rw->is_expression_punctuation) continue;

    IFE(NlpMatchEntitiesAddWord(me_ctrl, rw));
    IFE(NlpMatchEntitiesInPackageRecursive(me_ctrl));
    IFE(NlpMatchEntitiesRemoveWord(me_ctrl));
    break;
  }

  DONE;
}

static og_bool NlpMatchEntity(struct nlp_match_entities_ctrl *me_ctrl)
{
  og_bool found_entity = FALSE;   // found full or partial entity

  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;
  package_t package = me_ctrl->interpret_package->package;
  unsigned char buffer[DPcAutMaxBufferSize + 9];
  int ibuffer = 0;

  int string_entity_length = strlen(me_ctrl->string_entity);
  memcpy(buffer + ibuffer, me_ctrl->string_entity, string_entity_length);
  ibuffer += string_entity_length;
  buffer[ibuffer] = 0;

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  // Checking if we have at least one result
  if ((retour = OgAufScanf(package->ha_entity, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states))) do
  {
    IFE(retour);
    if (retour && iout > 0)
    {
      if (out[0] == '\1')
      {
        NlpLog(DOgNlpTraceMatch, "NlpMatchEntity: found full '%s'", buffer);
        og_bool added = NlpMatchEntityAdd(me_ctrl, ibuffer, buffer, iout - 1, out + 1);
        IFE(added);
        if (added) found_entity = TRUE;
      }
      else
      {
        NlpLog(DOgNlpTraceMatch, "NlpMatchEntity: found partial '%s'", buffer);
        found_entity = TRUE;
        break;
      }
    }
  }
  while ((retour = OgAufScann(package->ha_entity, &iout, out, nstate0, &nstate1, states)));

  return found_entity;
}

static og_bool NlpMatchEntityAdd(struct nlp_match_entities_ctrl *me_ctrl, int ibuffer, unsigned char *buffer, int iout,
    unsigned char *out)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;

  long expression_ptr;
  unsigned char *p = out;
  IFE(DOgPnin8(ctrl_nlp_th->herr,&p,&expression_ptr));
  struct expression *expression = (struct expression *) expression_ptr;

  int string_entity_length = 0;
  unsigned char string_entity[DOgNlpMaxEntitySize];
  for (int i = 0; i < me_ctrl->request_word_list_length; i++)
  {
    struct request_word *request_word = me_ctrl->request_word_list[i];
    og_string normalized_string_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
    IFN(normalized_string_word) DPcErr;
    int length_normalized_string_word = request_word->length;
    if (string_entity_length + length_normalized_string_word + 1 >= DOgNlpMaxEntitySize)
    {
      // We do not store very long entities
      NlpLog(DOgNlpTraceMinimal, "NlpMatchEntityAdd: entity is too long");
      DONE;
    }
    memcpy(string_entity + string_entity_length, normalized_string_word, length_normalized_string_word);
    string_entity_length += length_normalized_string_word;
    string_entity[string_entity_length++] = ' ';
    string_entity[string_entity_length] = 0;
  }

  int iuni1;
  unsigned char uni1[DOgNlpMaxEntitySize * 2];
  int iuni2;
  unsigned char uni2[DOgNlpMaxEntitySize * 2];
  IFE(OgCpToUni(string_entity_length, string_entity, DPcPathSize, &iuni1, uni1, DOgCodePageUTF8, 0, 0));
  IFE(OgCpToUni(ibuffer, buffer, DPcPathSize, &iuni2, uni2, DOgCodePageUTF8, 0, 0));

  double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_nlp_th->hstm, iuni1, uni1, iuni2, uni2,
      ctrl_nlp_th->levenshtein_costs);
  IFE(dlevenshtein_distance);
  double score_spelling =  1.0 - dlevenshtein_distance;

  NlpLog(DOgNlpTraceMatch, "NlpMatchEntityAdd: score_spelling between original '%.*s' and found '%.*s' is %.2f",
      string_entity_length, string_entity, ibuffer, buffer, score_spelling);

  score_spelling = pow(score_spelling, 4);

  NlpLog(DOgNlpTraceMatch, "NlpMatchEntityAdd: found expression '%s' with augmented score_spelling=%.2f", expression->text,
      score_spelling);

  gpointer result = g_hash_table_lookup(me_ctrl->expression_hash, expression);
  IFX(result)
  {
    NlpLog(DOgNlpTraceMatch, "NlpMatchEntityAdd: expression '%s' already created", expression->text);
    DONE;
  }
  g_hash_table_insert(me_ctrl->expression_hash, expression, GINT_TO_POINTER(1));

  if (expression->input_parts_nb != me_ctrl->request_word_list_length)
  {
    // Typical case: "maisonde campagne", can be handled later by creating non basic words "maison" and "de"
    // and then creating the expression using those new non basic words
    NlpLog(DOgNlpTraceMatch,
        "NlpMatchEntityAdd: expression->input_parts_nb (%d) != request_word_list_length (%d), not handled yet",
        expression->input_parts_nb, me_ctrl->request_word_list_length);
    return FALSE;
  }

  if (expression->case_sensitive || expression->accent_sensitive)
  {
    for (int i = 0; i < expression->input_parts_nb; i++)
    {
      struct request_word *request_word = me_ctrl->request_word_list[i];
      IFX(me_ctrl->alternative_request_word_list[i])
      {
        request_word = me_ctrl->alternative_request_word_list[i];
      }
      og_bool matched_case_accent = NlpMatchCaseAccent(ctrl_nlp_th, request_word, expression->input_parts + i);
      IFE(matched_case_accent);
      if (!matched_case_accent)
      {
        return FALSE;
      }
    }
  }

  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct request_word *request_word = me_ctrl->request_word_list[i];
    IFX(me_ctrl->alternative_request_word_list[i])
    {
      request_word = me_ctrl->alternative_request_word_list[i];
    }
    og_status status = NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, me_ctrl->interpret_package,
        expression->input_parts[i].self_index, FALSE, score_spelling);
    IFE(status);
  }

  return TRUE;
}
