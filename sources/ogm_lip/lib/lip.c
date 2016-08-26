/*
 *  Initialization for ogm_lip functions
 *  Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *  Dev : October,November 2006, July 2007
 *  Version 1.1
*/
#include "ogm_lip.h"

static int OgLip1(struct og_ctrl_lip *handle, int Ipawo);



PUBLIC(int) OgLip(void *handle, struct og_lip_input *input)
{
  struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *) handle;
  struct og_lip_conf *conf = input->conf;
  int i, c, state = 1, end = 0, start = 0, start_after_dot = 0;
  int Iexpression = 0, isIndivisibleExpression = FALSE, is_next_expression = FALSE;
  struct og_lip_conf_separator *currentSeparator = NULL;
  int punctuation_before_word;
  int is = 0, pw_length = 0;
  unsigned char *s;


  if (conf == NULL )
  {
    conf = &ctrl_lip->default_conf;
  }
  ctrl_lip->conf = conf;
  ctrl_lip->input = input;

  ctrl_lip->PawoUsed = 0;
  is = input->content_length;
  s = input->content;
  punctuation_before_word = 0;

  for (i = 0; !end; i += 2)
  {

    if (i < is)
    {
      c = (s[i] << 8) + s[i + 1];
    }
    else
    {
      end = 1;
      c = ' ';
    }

    switch (state)
    {
      case 1:
        if (OgUniIsspace(c) || OgLipIsPunctuation(conf, c) || OgLipIsSeparator(conf, c))
        {
          if (OgLipIsPunctuation(conf, c)) punctuation_before_word = c;

            if (OgLipIsOpenSeparator(conf, c) && currentSeparator == NULL )
            {
              if(i > 0) is_next_expression = TRUE;
              currentSeparator = OgLipGetSeparator(conf, c);
            }
            else if (OgLipIsSingleSeparator(conf, c) && currentSeparator == NULL )
            {
              if(i > 0) is_next_expression = TRUE;
            }

          state = 1;
        }
        else if (OgLipIsPunctuationWord(conf, is-i, s+i, &pw_length))
        {
          start = i;
          state = 3;
          i+=pw_length-2;
        }
        else if (c == '.') state = 1;
        else
        {
          start = i;
          state = 2;
        }
      break;

      /** In normal word **/
      case 2:

        if (is_next_expression == TRUE)
        {
          Iexpression++;
          is_next_expression = FALSE;
        }
        isIndivisibleExpression = (currentSeparator != NULL ? TRUE : FALSE);

        if (OgUniIsspace(c) || OgLipIsPunctuation(conf, c) || OgLipIsSeparator(conf, c))
        {
          IFE(LipAddPawo(ctrl_lip,start,i-start,c,punctuation_before_word,Iexpression,isIndivisibleExpression));
          if (OgLipIsPunctuation(conf, c)) punctuation_before_word = c;
          else punctuation_before_word = 0;

          if (OgLipIsCloseSeparator(currentSeparator, c))
          {
            is_next_expression = TRUE;
            currentSeparator = NULL;
          }
          else if (OgLipIsSingleSeparator(conf, c) && currentSeparator == NULL)
          {
            is_next_expression = TRUE;
          }

          state = 1;
        }
        else if (OgLipIsPunctuationWord(conf, is-i, s+i, &pw_length))
        {
          IFE(LipAddPawo(ctrl_lip,start,i-start,c,punctuation_before_word,Iexpression,isIndivisibleExpression));
          punctuation_before_word = 0;
          start = i;
          state = 3;
          i+=pw_length-2;
        }
        else if (c == '.')
        {
          int must_cut = 0;
          if (start_after_dot < start) start_after_dot = start;
          /* cut at '.' if either side is a word bigger than 1 letter.
           * Examples:
           *   MA.GE.BAT -> MA GE BAT
           *   S.N.C.F -> S.N.C.F
           *   S.N.TOTO.C.F -> S.N TOTO C.F
           *   A.RAPID.S  -> A RAPID S
           *   serrurerie.rapid -> serrurerie rapid
           *   serrurerie..rapid -> serrurerie rapid
           *   idee a suivre... */
          if (i - start_after_dot > 2) must_cut = 1;
          else
          {
            int j, d;
            for (j = i + 2; j < is; j += 2)
            {
              d = (s[j] << 8) + s[j + 1];
              if (OgUniIsspace(d) || OgLipIsPunctuation(conf, d) || OgLipIsPunctuationWord(conf, is-j, s+j, &pw_length) ||
                  OgLipIsSeparator(conf, c))
              {
                if (OgLipIsPunctuation(conf, d)) punctuation_before_word = c;

                if (OgLipIsCloseSeparator(currentSeparator, c))
                {
                  is_next_expression = TRUE;
                  currentSeparator = NULL;
                }
                else if (OgLipIsSingleSeparator(conf, c) && currentSeparator == NULL)
                {
                  is_next_expression = TRUE;
                }

                break;
              }
              if (d == '.') break;
            }
            if (j - (i + 2) > 2) must_cut = 1;
          }
          if (must_cut)
          {
            IFE(LipAddPawo(ctrl_lip,start,i-start,c,punctuation_before_word,Iexpression,isIndivisibleExpression));
            punctuation_before_word = '.';
            start = i + 2;
            state = 1;
          }
          start_after_dot = i + 2;
        }
      break;

      /** In punctuation word **/
      case 3:
        if (OgUniIsspace(c) || OgLipIsPunctuation(conf, c) || OgLipIsPunctuation(conf, c) || OgLipIsSeparator(conf, c))
        {
          IFE(LipAddPawo(ctrl_lip,start,i-start,c,punctuation_before_word,Iexpression,isIndivisibleExpression));
          if (OgLipIsPunctuation(conf, c)) punctuation_before_word = c;
          else punctuation_before_word = 0;

          if (OgLipIsCloseSeparator(currentSeparator, c))
          {
            is_next_expression = TRUE;
            currentSeparator = NULL;
          }
          else if (OgLipIsSingleSeparator(conf, c) && currentSeparator == NULL)
          {
            is_next_expression = TRUE;
          }

          state = 1;
        }
        else if (!OgLipIsPunctuationWord(conf, is-i, s+i, &pw_length))
        {
          IFE(LipAddPawo(ctrl_lip,start,i-start,c,punctuation_before_word,Iexpression,isIndivisibleExpression));
          punctuation_before_word = 0;
          start = i;
          state = 2;
          i+=pw_length-4;
        }
      break;
    }
  }


  /** This is where we will work on syntactical layers, sardane for example **/

  /** Language recognition **/
  IFE(LipLang(ctrl_lip));

  IFn(ctrl_lip->input->word_func) DONE;

  for (i = 0; i < ctrl_lip->PawoUsed; i++)
  {
    IFE(OgLip1(ctrl_lip,i));
  }

  DONE;
}





PUBLIC(int) OgLipAgain(void *handle, struct og_lip_input *input)
{
  struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *) handle;
  int i;

  ctrl_lip->input = input;

  for (i = 0; i < ctrl_lip->PawoUsed; i++)
  {
    IFE(OgLip1(ctrl_lip,i));
  }

  DONE;
}





static int OgLip1(struct og_ctrl_lip *ctrl_lip, int Ipawo)
{
IFE(ctrl_lip->input->word_func(ctrl_lip->input->context,Ipawo));
DONE;
}


PUBLIC(og_bool) OgLipIsEmptyString(void *handle, unsigned char *string, int string_length)
{
  struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *) handle;

  og_bool is_empty = TRUE;
  for (int i = 0; i < string_length; i += 2)
  {
    int c = (string[i] << 8) + string[i + 1];
    if(!OgLipIsPunctuation(ctrl_lip->conf, c) && !OgLipIsSeparator(ctrl_lip->conf, c) && (c!=32))
    {
      is_empty = FALSE;
      break;
    }
  }

  return is_empty;

}

PUBLIC(int) OgLipIsPunctuation(struct og_lip_conf *conf, int c)
{
  int i;

  // all control chars should be ignored
  // and considered as punctuation, thus word separators
  if (c < 32 && !OgUniIsspace(c)) return TRUE;

  for (i = 0; i < conf->punctuation_length; i++)
  {
    if (c == conf->punctuation[i])
    {
      return TRUE;
    }
  }
  return FALSE;
}



PUBLIC(int) OgLipIsPunctuationWord(struct og_lip_conf *conf, int is, unsigned char *s, int *plength)
{
int i,imax_size_punct=(-1);
int nb_words=conf->punctuation_word_length;
struct og_lip_conf_word *word = conf->punctuation_word;

for (i=0; i<nb_words; i++) {
  if (word[i].length > is) continue;
  if (!memcmp(word[i].string,s,word[i].length)) {
    if (imax_size_punct<0) imax_size_punct=i;
    else {
      if (word[i].length > word[imax_size_punct].length) imax_size_punct=i;
      }
    }
  }

if (imax_size_punct>=0) {
  *plength = word[imax_size_punct].length;
  return TRUE;
  }

return FALSE;
}




PUBLIC(int) OgLipIsSeparator(struct og_lip_conf *conf, int c)
{
  return (OgLipIsSingleSeparator(conf, c) || OgLipIsIndivisibleSeparator(conf, c));
}

int OgLipIsOpenSeparator(struct og_lip_conf *conf, int c)
{
  int i;

  for (i = 0; i < conf->indivisible_length; i++)
  {
    if (c == conf->indivisible[i].open)
    {
      return TRUE;
    }
  }
  return FALSE;
}

int OgLipIsCloseSeparator(struct og_lip_conf_separator *preview_open_sperator, int c)
{
  if (preview_open_sperator != NULL && c == preview_open_sperator->close)
  {
    return TRUE;
  }
  return FALSE;
}

int OgLipIsIndivisibleSeparator(struct og_lip_conf *conf, int c)
{
  int i;
  for (i = 0; i < conf->indivisible_length; i++)
  {
    if (c == conf->indivisible[i].open || c == conf->indivisible[i].close)
    {
      return TRUE;
    }
  }
  return FALSE;
}

int OgLipIsSingleSeparator(struct og_lip_conf *conf, int c)
{
  int i;

  for (i = 0; i < conf->separator_length; i++)
  {
    if (c == conf->separator[i])
    {
      return TRUE;
    }
  }
  return FALSE;
}

struct og_lip_conf_separator* OgLipGetSeparator(struct og_lip_conf *conf, int c)
{
  int i;

  for (i = 0; i < conf->indivisible_length; i++)
  {
    if (c == conf->indivisible[i].open)
    {
      return &conf->indivisible[i];
    }
  }
  return NULL;
}




