/*
 * ltracread.c *
 *  Created on: 22 févr. 2017
 *      Author: caroline
 */

#include "ogm_ltrac.h"
#include <logis639_3166.h>

struct og_ltrac_entry
{
  int iword;
  char *word;
  int language_code;
  int frequency;
};

struct og_ltrac_update_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  int language_code;
  int frequency;
};

struct og_ltrac_found_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  int language_code;
  og_bool found;
};

static og_bool LtracGetLtraf(struct og_ctrl_ltrac *ctrl_ltrac, int ientry, unsigned char *entry, int *pIltraf);
static og_status LtracAllocLtraf(struct og_ctrl_ltrac *ctrl_ltrac, struct ltraf **pltraf);
static og_status LtracNormalise(struct og_ctrl_ltrac *ctrl_ltrac, int iword, char *word, int *iout, char *out);
static og_bool LtracIsExpression(struct og_ctrl_ltrac *ctrl_ltrac, int iword, char *word);
static og_status LtracAddExpression(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input);
static og_status LtracAddEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, og_bool replace);
static og_status LtracUpdateFrequency(void *context, struct og_ltrac_scan *scan);
static og_status LtracScanWord(struct og_ctrl_ltrac *ctrl_ltrac, int iword, og_string word,
    int (*func)(void *context, struct og_ltrac_scan *scan), void *context);
static og_status LtracIsFound(void *context, struct og_ltrac_scan *scan);

og_status LtracReadLtraf(struct og_ctrl_ltrac *ctrl_ltrac, int min_frequency)
{
  char ltrac_dir[DPcPathSize];
  sprintf(ltrac_dir, "%s/%s", ctrl_ltrac->data_directory, "ltra");

  char ltraf_file[DPcPathSize];
  sprintf(ltraf_file, "%s/ltraf.txt", ltrac_dir);
  if (!OgFileExists(ltraf_file))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "LtracRead: file '%s' does not exist. Run ogm_sidx with option -ltrac or add a 'ltraf.txt' file",
        ltraf_file);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  FILE *fd = fopen(ltraf_file, "rb");
  IFn(fd)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "LtracRead: Impossible to open '%s'", ltraf_file);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  unsigned char utf8[DOgLtracMaxWordsSize];

  int frequency = 0;
  int language_code = DOgLangNil;
  unsigned char buffer[DOgLtracMaxWordsSize];

  OgTrimString(utf8, utf8);
  int iutf8 = strlen(utf8);

  int i, n, c, start, end, start_word;
  while (fgets(utf8, DOgLtracMaxWordsSize, fd))
  {
    OgTrimString(utf8, utf8);
    iutf8 = strlen(utf8);
    for (n = 0, start = 0, end = 0, start_word = (-1), i = 0; !end; i++)
    {
      if (i >= iutf8)
      {
        end = 1;
        c = ':';
      }
      else c = utf8[i];
      if (c == ':')
      {
        sprintf(buffer, "%.*s", i - start, utf8 + start);
        switch (n)
        {
          case 0:
            frequency = atoi(buffer);
            break;
          case 1:
            // attribute number
            break;
          case 2:
            language_code = atoi(buffer);
            start_word = i + 1;
            break;
        }
        if (start_word >= 0) break;
        start = i + 1;
        n++;
      }
    }
    if (start_word < 0) continue;
    if ((frequency < min_frequency) || (frequency <= 0)) continue;

    unsigned char entry[DOgLtracMaxWordsSize];
    int ientry;
    IFE(OgCpToUni(iutf8-start_word,utf8+start_word,DOgLtracMaxWordsSize,&ientry,entry,DOgCodePageUTF8,0,0));

    struct ltrac_dic_input dic_input[1];
    memset(dic_input, 0, sizeof(struct ltrac_dic_input));
    dic_input->value_length = ientry;
    dic_input->value = entry;
    dic_input->language_code = language_code;
    dic_input->attribute_number = 0;
    dic_input->is_expression = FALSE;

    og_bool replace = TRUE;
    if (ctrl_ltrac->has_ltraf_requests)
    {
      dic_input->frequency = min_frequency;
    }
    else
    {
      dic_input->frequency = frequency;
      replace = FALSE;
    }

    IFE(LtracAddEntry(ctrl_ltrac, dic_input, replace));
  }

  fclose(fd);

  DONE;
}

static og_status LtracAddEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, og_bool replace)
{

  unsigned char entry[DOgLtracMaxWordsSize];
  memcpy(entry, dic_input->value, dic_input->value_length);
  unsigned char *p = entry + dic_input->value_length;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;

  //attribute_number
  OggNout(0, &p);
  OggNout(dic_input->language_code, &p);

  int ientry = p - entry;

  int Iltraf;
  og_bool found = LtracGetLtraf(ctrl_ltrac, ientry, entry, &Iltraf);
  IFE(found);
  if (found)
  {
    struct ltraf *ltraf = ctrl_ltrac->Ltraf + Iltraf;

    if (replace)
    {
      ltraf->frequency = dic_input->frequency;
    }
    else
    {
      ltraf->frequency += dic_input->frequency;
    }
  }
  else
  {
    struct ltraf *ltraf;
    Iltraf = LtracAllocLtraf(ctrl_ltrac, &ltraf);
    IFE(Iltraf);

    if (replace)
    {
      ltraf->frequency = dic_input->frequency;
    }
    else
    {
      ltraf->frequency += dic_input->frequency;
    }
  }

  OggNout(Iltraf, &p);
  OggNout(dic_input->is_expression, &p);

  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_ltrac, ientry, entry));
  DONE;
}

static og_bool LtracGetLtraf(struct og_ctrl_ltrac *ctrl_ltrac, int ientry, unsigned char *entry, int *pIltraf)
{
  unsigned char *p, out[DOgLtracMaxWordsSize];
  oindex states[DOgLtracMaxWordsSize];
  int retour, nstate0, nstate1, iout;

  *pIltraf = (-1);

  if ((retour = OgAutScanf(ctrl_ltrac->ha_ltrac, ientry, entry, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      p = out;
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,pIltraf));
      return TRUE;
    }
    while ((retour = OgAutScann(ctrl_ltrac->ha_ltrac, &iout, out, nstate0, &nstate1, states)));
  }

  return FALSE;
}

static og_status LtracAllocLtraf(struct og_ctrl_ltrac *ctrl_ltrac, struct ltraf **pltraf)
{
  char erreur[DOgErrorSize];
  struct ltraf *ltraf = 0;
  int i = ctrl_ltrac->LtrafNumber;

  beginAllocLtraf:

  if (ctrl_ltrac->LtrafUsed < ctrl_ltrac->LtrafNumber)
  {
    i = ctrl_ltrac->LtrafUsed++;
  }

  if (i == ctrl_ltrac->LtrafNumber)
  {
    unsigned a, b;
    struct ltraf *og_l;

    if (ctrl_ltrac->loginfo->trace & DOgSidxTraceMemory)
    {
      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracAllocLtraf: max Ltraf number (%d) reached",
          ctrl_ltrac->LtrafNumber);
    }
    a = ctrl_ltrac->LtrafNumber;
    b = a + (a >> 2) + 1;
    IFn(og_l=(struct ltraf *)malloc(b*sizeof(struct ltraf)))
    {
      sprintf(erreur, "LtracAllocLtraf: malloc error on Ltraf");
      OgErr(ctrl_ltrac->herr, erreur);
      DPcErr;
    }

    memcpy(og_l, ctrl_ltrac->Ltraf, a * sizeof(struct ltraf));
    DPcFree(ctrl_ltrac->Ltraf);
    ctrl_ltrac->Ltraf = og_l;
    ctrl_ltrac->LtrafNumber = b;

    if (ctrl_ltrac->loginfo->trace & DOgSidxTraceMemory)
    {
      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracAllocLtraf: new Ltraf number is %d\n",
          ctrl_ltrac->LtrafNumber);
    }

    goto beginAllocLtraf;
  }

  ltraf = ctrl_ltrac->Ltraf + i;
  memset(ltraf, 0, sizeof(struct ltraf));

  if (pltraf) *pltraf = ltraf;
  return (i);
}

og_status LtracReadLtrafRequest(struct og_ctrl_ltrac *ctrl_ltrac, int min_frequency)
{
  char ltrac_dir[DPcPathSize];
  sprintf(ltrac_dir, "%s/%s", ctrl_ltrac->data_directory, "ltra");

  char ltraf_file[DPcPathSize];
  sprintf(ltraf_file, "%s/ltraf_requests.txt", ltrac_dir);
  if (!OgFileExists(ltraf_file)) DONE;

  FILE *fd = fopen(ltraf_file, "rb");
  IFn(fd)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "LtracRead: Impossible to open '%s'", ltraf_file);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  unsigned char utf8[DOgLtracMaxWordsSize];

  int frequency = 0;
  int language_code = DOgLangNil;
  unsigned char buffer[DOgLtracMaxWordsSize];

  OgTrimString(utf8, utf8);
  int iutf8 = strlen(utf8);

  int i, n, c, start, end, start_word;
  while (fgets(utf8, DOgLtracMaxWordsSize, fd))
  {
    OgTrimString(utf8, utf8);
    iutf8 = strlen(utf8);
    for (n = 0, start = 0, end = 0, start_word = (-1), i = 0; !end; i++)
    {
      if (i >= iutf8)
      {
        end = 1;
        c = ':';
      }
      else c = utf8[i];
      if (c == ':')
      {
        sprintf(buffer, "%.*s", i - start, utf8 + start);
        switch (n)
        {
          case 0:
            frequency = atoi(buffer);
            break;
          case 1:
            language_code = OgCodeToIso639_3166(ctrl_ltrac->herr, buffer);
            start_word = i + 1;
            break;
        }
        if (start_word >= 0) break;
        start = i + 1;
        n++;
      }
    }
    if (start_word < 0) continue;
    if (frequency < min_frequency) continue;

    unsigned char word[DOgLtracMaxWordsSize];
    int iword;
    IFE(OgCpToUni(iutf8-start_word,utf8+start_word,DOgLtracMaxWordsSize,&iword,word,DOgCodePageUTF8,0,0));

    unsigned char word_norm[DOgLtracMaxWordsSize];
    int iword_norm = 0;
    IFE(LtracNormalise(ctrl_ltrac, iword, word, &iword_norm, word_norm));

    if (!LtracIsExpression(ctrl_ltrac, iword_norm, word_norm))
    {
      // Test si mot unique
      struct og_ltrac_update_context update_ctx[1];
      memset(update_ctx, 0, sizeof(struct og_ltrac_update_context));
      update_ctx->ctrl_ltrac = ctrl_ltrac;
      update_ctx->language_code = language_code;
      update_ctx->frequency = frequency;
      IFE(LtracScanWord(ctrl_ltrac, iword_norm, word_norm, LtracUpdateFrequency, update_ctx));
    }
    else
    {
      struct ltrac_dic_input dic_input[1];
      memset(dic_input, 0, sizeof(struct ltrac_dic_input));
      dic_input->value_length = iword_norm;
      dic_input->value = word_norm;
      dic_input->language_code = language_code;
      dic_input->frequency = frequency;
      dic_input->is_expression = TRUE;
      IFE(LtracAddExpression(ctrl_ltrac, dic_input));
    }

  }

  fclose(fd);

  DONE;
}

static og_status LtracScanWord(struct og_ctrl_ltrac *ctrl_ltrac, int iword, og_string word,
    int (*func)(void *context, struct og_ltrac_scan *scan), void *context)
{
  oindex states[DOgLtracMaxWordsSize];
  int retour, nstate0, nstate1, iout;
  unsigned char out[DOgLtracMaxWordsSize];
  if ((retour = OgAutScanf(ctrl_ltrac->ha_ltrac, iword, (char *) word, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int sep = (-1);
      for (int i = 0; i < iout; i += 2)
      {
        int c = (out[i] << 8) + out[i + 1];
        if (c == DOgLtracExtStringSeparator)
        {
          sep = i;
          break;
        }
      }
      if (sep < 0) continue;
      unsigned char *p = out + sep + 2;

      struct og_ltrac_scan scan[1];
      memset(scan, 0, sizeof(struct og_ltrac_scan));

      scan->iword = iword;
      scan->word = word;

      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->attribute_number));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->language_code));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->Iltraf));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->is_expression));

      IFE(func(context, scan));
    }
    while ((retour = OgAutScann(ctrl_ltrac->ha_ltrac, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

og_status LtracScan(struct og_ctrl_ltrac *ctrl_ltrac, int (*func)(void *context, struct og_ltrac_scan *scan),
    void *context)
{
  oindex states[DOgLtracMaxWordsSize];
  int retour, nstate0, nstate1, iout;
  unsigned char out[DOgLtracMaxWordsSize];
  if ((retour = OgAutScanf(ctrl_ltrac->ha_ltrac, (-1), "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int sep = (-1);
      for (int i = 0; i < iout; i += 2)
      {
        int c = (out[i] << 8) + out[i + 1];
        if (c == DOgLtracExtStringSeparator)
        {
          sep = i;
          break;
        }
      }
      if (sep < 0) continue;
      unsigned char *p = out + sep + 2;

      struct og_ltrac_scan scan[1];
      memset(scan, 0, sizeof(struct og_ltrac_scan));

      scan->iword = sep;
      scan->word = out;

      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->attribute_number));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->language_code));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->Iltraf));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->is_expression));

      IFE(func(context, scan));
    }
    while ((retour = OgAutScann(ctrl_ltrac->ha_ltrac, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

static og_status LtracUpdateFrequency(void *context, struct og_ltrac_scan *scan)
{
  struct og_ltrac_update_context *ctx = (struct og_ltrac_update_context *) context;
  struct og_ctrl_ltrac *ctrl_ltrac = ctx->ctrl_ltrac;

  if ((ctx->language_code == scan->language_code) || (scan->language_code == DOgLangNil))
  {
    struct ltraf *ltraf = ctrl_ltrac->Ltraf + scan->Iltraf;
    ltraf->frequency = ctx->frequency;

    if (ctrl_ltrac->loginfo->trace & DOgLtracTraceAdd)
    {
      char buffer[DPcPathSize];
      int ibuffer = 0;
      IFE(OgUniToCp(scan->iword,scan->word,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracUpdateFrequency: frequency %d updated for word '%s'",
          ltraf->frequency, buffer);
    }
  }

  DONE;
}

static og_bool LtracIsExpression(struct og_ctrl_ltrac *ctrl_ltrac, int iword, char *word)
{
  for (int i = 0; i < iword; i += 2)
  {
    int c = (word[i] << 8) + word[i + 1];
    if ((c == ' '))
    {
      return TRUE;
    }
  }
  return FALSE;
}

/** ToLower, trim and remove multiple spaces between words */
static og_status LtracNormalise(struct og_ctrl_ltrac *ctrl_ltrac, int iword, char *word, int *iout, char *out)
{
  OgUniStrlwr(iword, word, word);

  IFE(OgTrimUnicode(iword, word, &iword, word));

  og_bool space = FALSE;
  int start = 0;
  int length = 0;
  for (int i = 0; i < iword; i += 2)
  {
    int c = (word[i] << 8) + word[i + 1];
    length = i - start + 2;

    if ((c == ' ') && !space)
    {
      space = TRUE;
      memcpy(out + *iout, word + start, length);
      *iout = *iout + length;
    }

    if (space && (c != ' '))
    {
      start = i;
      space = FALSE;
    }
  }
  if (!space)
  {
    memcpy(out + *iout, word + start, length);
    *iout = *iout + length;
  }

  DONE;
}

static og_status LtracAddExpression(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{

  og_bool found = TRUE;
  int start = 0;
  int length = 0;
  og_string entry_word = NULL;

  for (int i = 0; i < dic_input->value_length; i += 2)
  {
    entry_word = dic_input->value;
    int c = (entry_word[i] << 8) + entry_word[i + 1];
    length = i - start;

    if (c == ' ')
    {
      og_string word = entry_word + start;
      int iword = length;

      struct og_ltrac_found_context found_ctx[1];
      memset(found_ctx, 0, sizeof(struct og_ltrac_found_context));
      found_ctx->ctrl_ltrac = ctrl_ltrac;
      found_ctx->language_code = dic_input->language_code;
      found_ctx->found = FALSE;

      IFE(LtracScanWord(ctrl_ltrac, iword, word, LtracIsFound, found_ctx));

      if (!found_ctx->found)
      {
        found = FALSE;
        break;
      }
      start = i + 2;
    }
  }

  if (found)
  {
    og_string word = entry_word + start;
    int iword = length;

    struct og_ltrac_found_context found_ctx[1];
    memset(found_ctx, 0, sizeof(struct og_ltrac_found_context));
    found_ctx->ctrl_ltrac = ctrl_ltrac;
    found_ctx->language_code = dic_input->language_code;
    found_ctx->found = FALSE;

    IFE(LtracScanWord(ctrl_ltrac, iword, word, LtracIsFound, found_ctx));
    if (found_ctx->found)
    {
      IFE(LtracAddEntry(ctrl_ltrac, dic_input, TRUE));
    }
  }
  else
  {
    if (ctrl_ltrac->loginfo->trace & DOgLtracTraceAdd)
    {
      char buffer[DPcPathSize];
      int ibuffer = 0;
      IFE(OgUniToCp(dic_input->value_length,dic_input->value,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracAddExpression: expression %s discarded", buffer);
    }
  }

  DONE;
}

static og_status LtracIsFound(void *context, struct og_ltrac_scan *scan)
{
  struct og_ltrac_found_context *ctx = (struct og_ltrac_found_context *) context;

  if ((ctx->language_code == scan->language_code) || (scan->language_code == DOgLangNil))
  {
    ctx->found = TRUE;
  }

  DONE;
}
