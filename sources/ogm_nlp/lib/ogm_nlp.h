/*
 *  Internal header for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev: September 2017
 *  Version 1.0
 */
#include <lognlp.h>

struct og_ctrl_nlp
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];
};

