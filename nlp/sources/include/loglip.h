/*
 *  Header for library ogm_lip.dll
 *  Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *  Dev: October,November 2006, May,June,July,August,December 2007
 *  Dev: November 2008, May 2009
 *  Version 1.7
*/
#ifndef _LOGLIPALIVE_
#include <lpcgentype.h>
#include <loggen.h>
#include <logthr.h>
#include <logmsg.h>


#define DOgLipBanner  "ogm_lip V1.23, Copyright (c) 2006-2013 Pertimm, Inc."
#define DOgLipVersion 123


/** trace levels **/
#define DOgLipTraceMinimal          0x1
#define DOgLipTraceMemory           0x2
#define DOgLipTraceXMLScan          0x4

#define DOgLipLangMax  3

#define DOgLipConfMaxPunctuationSize        0x200 /*  512 */
#define DOgLipConfMaxPunctuationWordSize    0x40  /*   64 */
#define DOgLipConfMaxIndivisibleSize        0x40  /*   64 */

#define DOgLipConfPunctuationFileName    "punctuation.xml"

/** real position of any unicode characters **/
struct og_lip_rpos {
  int real_position;
  };

struct og_lip_lang_score {
  double score;
  int lang;
  };

/** used for language recognition **/
struct og_lip_lang {
  struct og_lip_lang_score language[DOgLipLangMax];
  int nb_languages;
  };

struct og_lip_word
{
  int start, length;
  int real_start, real_length;
  struct og_lip_input *input;
  int punctuation_before_word;
  int has_quote, has_dot_separation;
  int last_word, nb_words;
  struct og_lip_lang lang;

  /** Expression group index */
  int Iexpression_group;

  /** flag if the expression group is nonbreaking */
  int is_indivisible_expression;

};

struct og_lip_conf_separator
{
  int open;   // unicode char
  int close;  // unicode char
};

struct og_lip_conf_word
{
  int length;
  unsigned char string[DOgLipConfMaxPunctuationWordSize]; // buffer for unicode punctuation words
};

struct og_lip_conf
{
  int punctuation_length;
  int punctuation[DOgLipConfMaxPunctuationSize]; // array of unicode char

  int punctuation_word_length;
  struct og_lip_conf_word punctuation_word[DOgLipConfMaxPunctuationWordSize]; // array of unicode punctuation words

  int separator_length;
  int separator[DOgLipConfMaxPunctuationSize]; // array of unicode char

  int indivisible_length;
  struct og_lip_conf_separator indivisible[DOgLipConfMaxIndivisibleSize];

  int conf_in_file;
};

/** content must be unicode **/
struct og_lip_input {
  struct og_lip_rpos *rpos;
  int content_length; unsigned char *content;

  /** Punct config */
  struct og_lip_conf *conf;

  int (*word_func)(void *context, int Iw);
  /** activates language recognition for the call **/
  int language_recognition;
  void *context;
  };

struct og_lip_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char filename[DPcPathSize];
  /** enables language recognition for the API **/
  char language_dictionary[DPcPathSize];

  /** Punct config */
  struct og_lip_conf *conf;
  };

DEFPUBLIC(void *) OgLipInit(struct og_lip_param *param);
DEFPUBLIC(int) OgLip(void *handle, struct og_lip_input *input);
DEFPUBLIC(int) OgLipAgain(void *handle, struct og_lip_input *input);
DEFPUBLIC(int) OgLipGetWord(void *handle, int Ipawo, struct og_lip_word *w);
DEFPUBLIC(int) OgLipGetNbWords(void *handle);
DEFPUBLIC(int) OgLipFlush(void *handle);
DEFPUBLIC(int) OgLipMem(void *handle, int must_log, int module_level, ogint64_t *pmem);
DEFPUBLIC(int) OgLipGetLanguages(void *handle, struct og_lip_lang *lang);
DEFPUBLIC(int) OgLipReadPunctConf(void *handle, char *filename, struct og_lip_conf *conf);
DEFPUBLIC(int) OgLipConfAddPunctuation(void *hmsg, struct og_lip_conf *conf, int unicode_char, int is_separator);
DEFPUBLIC(int) OgLipConfAddPunctuationIndivisible(void *hmsg, struct og_lip_conf *conf, int unicode_char_open, int unicode_char_close);
DEFPUBLIC(int) OgLipConfAddPunctuationWord(void *hmsg, struct og_lip_conf *conf, unsigned char *word);
DEFPUBLIC(int) OgLipConfLog(void *hmsg, const unsigned char *message, struct og_lip_conf *conf);
DEFPUBLIC(int) OgLipResize(void *handle, size_t max_size);
DEFPUBLIC(int) OgLipInitConfWithDefault(void *hmsg, struct og_lip_conf *conf);

DEFPUBLIC(og_bool) OgLipIsEmptyString(void *handle, unsigned char *string, int string_length);
DEFPUBLIC(int) OgLipIsPunctuation(struct og_lip_conf *conf, int c);
DEFPUBLIC(int) OgLipIsSeparator(struct og_lip_conf *conf, int c);
DEFPUBLIC(int) OgLipIsPunctuationWord(struct og_lip_conf *conf, int is, unsigned char *s, int *plength);

DEFPUBLIC(int) OgLipOutputXmlInit(void *handle ,char *xml_filename);
DEFPUBLIC(int) OgLipOutputXml(void *handle, int content_length, unsigned char *content);
DEFPUBLIC(int) OgLipOutputXmlFlush(void *handle);

#define _LOGLIPALIVE_
#endif


