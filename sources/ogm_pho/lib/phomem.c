/*
 *  Calculating memory used for ogm_pho
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : March 2009
 *  Version 1.0
*/
#include "ogm_pho.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"%s%s:%s %16s",header,y,z,v); } }

static void PhoMemCallback(gpointer key_pt, gpointer value_pt, gpointer ctx);


struct og_pho_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_pho,ha_rules,Rule,Matching;
  ogint64_t Ba,Char_class,BaClass;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

struct PhoMemCallback_ctx{
    struct og_pho_memory *m;
    int module_level;
    og_bool must_log;
    struct og_ctrl_pho *ctrl_pho;
};

PUBLIC(int) OgPhoMem(void *hpho, int must_log, int module_level, ogint64_t *pmem)
{
  struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *) hpho;
  char header[DPcPathSize],v[128];

  if (pmem) *pmem = 0;
  IFn(hpho) DONE;

  struct og_pho_memory m[1];
  memset(m, 0, sizeof(struct og_pho_memory));

  struct PhoMemCallback_ctx ctx[1];
  memset(ctx, 0, sizeof(struct PhoMemCallback_ctx));
  ctx->m = m;
  ctx->module_level = module_level;
  ctx->must_log = must_log;
  ctx->ctrl_pho = ctrl_pho;

  g_hash_table_foreach(ctrl_pho->lang_context_map, PhoMemCallback, ctx);

  m->total_module += m->ctrl_pho;
  DOgShowMem(m->total_module, "total_module pho", " ")

  if (pmem) *pmem = m->total;

  DONE;
}




static void PhoMemCallback(gpointer key_pt, gpointer value_pt, gpointer ctx)
{
  struct PhoMemCallback_ctx *context = (struct PhoMemCallback_ctx *)ctx;
  struct og_pho_memory *m = context->m;
  int must_log = context->must_log;
  int module_level = context->module_level;
  struct og_ctrl_pho *ctrl_pho = context->ctrl_pho;
  char header[DPcPathSize],v[128];

  struct lang_context *lang_context = (struct lang_context *) value_pt;


  if (must_log)
  {
    int i = 0;
    for (i = 0; i < module_level; i++)
      header[i] = ' ';
    header[i] = 0;
  }

  OgAutMem(lang_context->ha_rules, 0, module_level + 2, &m->ha_rules);

  m->Rule = lang_context->RuleNumber * sizeof(struct rule);
  m->Matching = lang_context->MatchingNumber * sizeof(struct matching);
  m->Ba = lang_context->BaSize * sizeof(unsigned char);
  m->Char_class = lang_context->Char_classNumber * sizeof(struct char_class);
  m->BaClass = lang_context->BaClassSize * sizeof(unsigned char);

  m->total_module = m->ha_rules + m->Rule + m->Matching + m->Ba + m->Char_class + m->BaClass;

  m->total += m->total_module;


  DOgShowMem(m->ctrl_pho, "ctrl_pho", "         ")
  DOgShowMem(m->ha_rules, "ha_rules", "         ")
  DOgShowMem(m->Rule, "Rule", "             ")
  DOgShowMem(m->Matching, "Matching", "         ")
  DOgShowMem(m->Ba, "Ba", "               ")
  DOgShowMem(m->Char_class, "Char_class", "       ")
  DOgShowMem(m->BaClass, "BaClass", "          ")

}



