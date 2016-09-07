/*
 *  This is main procedure for ogdict.exe
 *  Copyright (c) 2016 Pertimm by Patrick Constant
 *  Dev : August 2016
 *  Version 1.0
 */
#include <logmsg.h>
#include <logaut.h>
#include <loguni.h>
#include <getopt.h>
#include <logpath.h>
#include <logis639_3166.h>

/* structure de contrôle du programme */
struct og_info
{
  void *hmsg, *herr;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];
  char input_filename[DPcPathSize], output_filename[DPcPathSize];
  char root_filename[DPcPathSize], form_filename[DPcPathSize];
  char request_text[DPcPathSize];
  int remove_composites; void *ha_roots;
  FILE *fdroot, *fdform;
  int get_root;
  int verbose;
  int lang;
};

/* functions for using main api */
static int dict(struct og_info *);
static int dict_remove_composites(struct og_info *info);
static int dict_get_root_string(struct og_info *info, int iuni, unsigned char *uni, int *piroot, unsigned char *root);
static int dict_get_form_string(struct og_info *info, int iuni, unsigned char *uni, int *piform, unsigned char *form);
static int dict_select_composite(struct og_info *info, int iroot, unsigned char *root);
static int dict_get_root(struct og_info *info);
static int dict_write_root_form(struct og_info *info, int iroot, unsigned char *root, int iform, unsigned char *form);


/* default function to define */
static int OgUse(struct og_info *);
static void OgExit(struct og_info *);

/* MAIN */
int main(int argc, char * argv[])
{
  char *nil, carlu, *DOgPIPE;
  struct og_info info[1];
  time_t ltime;

  /* definition of program options */
  struct option longOptions[] =
  {
      { "input", required_argument, NULL, 'i' },
      { "remove-composites", no_argument, NULL, 'r' },
      { "get-root", no_argument, NULL, 'g' },
      { "lang", required_argument, NULL, 'l' },
      { "output", required_argument, NULL, 'o' },
      { "help", no_argument, NULL, 'h' },
      { "trace", required_argument, NULL, 't' },
      { "verbose", no_argument, NULL, 'v' },
      { 0, 0, 0, 0 }
  };

  /* initialization of program info structure */
  memset(info, 0, sizeof(struct og_info));
  if ((DOgPIPE = getenv("DOgPIPE")))
  {
    IFn(info->hmsg=OgLogInit("ogdict",DOgPIPE, DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogPipe)) exit(1);
  }
  else
  {
    IFn(info->hmsg=OgLogInit("ogdict","ogdict", DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogFile)) exit(1);
  }
  info->herr = OgLogGetErr(info->hmsg);
  info->hmutex = OgLogGetMutex(info->hmsg);
  info->loginfo->trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  info->loginfo->where = "ogdict";


  /* parsing options */
  int optionIndex = 0;
  while ((carlu = getopt_long(argc, argv, "i:l:o:t:ghrv?", longOptions, &optionIndex)) != EOF)
  {
    switch (carlu)
    {
      case 0:
        break;
      case 'i': /* input_filename */
        strcpy(info->input_filename, optarg);
        break;
      case 'l': /* language */
        info->lang = OgCodeToIso639_3166(info->herr, optarg);
        break;
      case 'o': /* output_filename */
        strcpy(info->output_filename, optarg);
        break;
      case 't':
        info->loginfo->trace = strtol(optarg, &nil, 16);
        break;
      case 'h': /* help */
      case '?':
        OgUse(info);
        goto main_end;
        break;
      case 'r':
        info->remove_composites = 1;
        break;
      case 'g':
        info->get_root = 1;
        break;
      case 'v': /* language */
        info->verbose = 1;
        break;
    }
  }

  /* parsing main parameters */
  for (int i = 0; i < argc - optind; i++)
  {
    switch (i)
    {
      case 0:
        if (info->input_filename[0])
        {
          strcpy(info->output_filename, argv[optind + i]);
        }
        else
        {
          strcpy(info->input_filename, argv[optind + i]);
        }
        break;
      case 1:
        if (!info->output_filename[0])
        {
          strcpy(info->output_filename, argv[optind + i]);
        }
        break;
      default:
        break;
    }
  }

  if (info->loginfo->trace & DOgAutTraceMinimal)
  {
    time(&ltime);
    OgMsg(info->hmsg, "", DOgMsgDestInLog, "\nProgram %s starting with pid %x at %.24s", OgLogGetModuleName(info->hmsg),
        getpid(), OgGmtime(&ltime));
  }

  IF(dict(info))
  {
    OgExit(info);
  }

  if (info->loginfo->trace & DOgAutTraceMinimal)
  {
    time(&ltime);
    OgMsg(info->hmsg, "", DOgMsgDestInLog, "\nProgram %s exiting at %.24s\n", OgLogGetModuleName(info->hmsg),
        OgGmtime(&ltime));
  }

  main_end:
  IFE(OgLogFlush(info->hmsg));

  return (0);
}

static int dict(struct og_info *info)
{
  if (info->get_root)
  {
    return(dict_get_root(info));
  }
  else if (info->remove_composites)
  {
    return(dict_remove_composites(info));
  }
DONE;
}


static int dict_remove_composites(struct og_info *info)
{
  FILE *fd, *fdout;
  if (info->input_filename[0])
  {
    IFn(fd=fopen(info->input_filename,"rb"))
    {
      OgMsg(info->hmsg, "", DOgMsgDestInLog, "Impossible to fopen '%s' for reading", info->input_filename);
      DONE;
    }
    if (info->output_filename[0])
    {
      IFn(fdout=fopen(info->output_filename,"w"))
      {
        OgMsg(info->hmsg, "", DOgMsgDestInLog, "Impossible to fopen '%s' for writing", info->output_filename);
        DONE;
      }
    }
    else fdout = stdout;

    struct og_aut_param aut_param[1];
    memset(aut_param, 0, sizeof(struct og_aut_param));
    aut_param->herr = info->herr;
    aut_param->hmutex = info->hmutex;
    aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
    aut_param->loginfo.where = info->loginfo->where;
    aut_param->state_number = 0x1000;
    sprintf(aut_param->name, "og_dict_roots");
    IFn(info->ha_roots=OgAutInit(aut_param)) DPcErr;

    unsigned char line[DPcPathSize + 9];
    int iuni; unsigned char uni[DPcPathSize*2+9];
    int iroot; unsigned char root[DPcPathSize*2+9];
    while (fgets(line, DPcPathSize, fd))
    {
      OgTrimString(line, line);
      IFE(OgCpToUni(strlen(line), line, DPcPathSize, &iuni, uni, DOgCodePageUTF8, 0, 0));
      // Getting the root string and putting it in an automaton
      // format is : form::root+PoS
      IFE(dict_get_root_string(info,iuni,uni,&iroot,root));
      if (iroot>0)
      {
        IFE(OgAutAdd(info->ha_roots,iroot,root));
      }
    }

    rewind(fd);
    while (fgets(line, DPcPathSize, fd))
    {
      OgTrimString(line, line);
      IFE(OgCpToUni(strlen(line), line, DPcPathSize, &iuni, uni, DOgCodePageUTF8, 0, 0));
      // Getting the root string and putting it in an automaton
      // format is : form::root+PoS
      IFE(dict_get_root_string(info,iuni,uni,&iroot,root));
      if (iroot>0)
      {
        int selected;
        IFE(selected=dict_select_composite(info,iroot,root));
        if (selected)
        {
          fprintf(fdout,"%s\n",line);
        }
      }
    }


    fclose(fd);
    if (info->output_filename[0]) fclose(fdout);
  }
  else
  {
    OgUse(info);
  }

  DONE;

}



static int dict_get_root_string(struct og_info *info, int iuni, unsigned char *uni, int *piroot, unsigned char *root)
{
  int state = 1, start_root = 0, iform=0;

  *piroot = 0;

  for (int i=0; i<iuni; i+=2)
  {
    int c = (uni[i]<<8) + uni[i+1];
    switch (state)
    {
      case 1:
        if (c==':')
        {
          iform = i;
          state = 2;
        }
        break;
      case 2:
        if (c!=':')
        {
          start_root=i;
          state = 3;
        }
        break;
      case 3:
        if (c=='+')
        {
          int iroot = i-start_root;
          memcpy(root,uni+start_root,iroot);
          if (iroot==2 && !memcmp(root,"\0=",2))
          {
            memcpy(root,uni,iform);
            iroot=iform;
          }
          *piroot = iroot;
          return(1);
        }
        break;
    }
  }
  return(0);
}



static int dict_get_form_string(struct og_info *info, int iuni, unsigned char *uni, int *piform, unsigned char *form)
{
  *piform = 0;

  for (int i = 0; i < iuni; i += 2)
  {
    int c = (uni[i] << 8) + uni[i + 1];
    if (c == ':')
    {
      int iform = i;
      memcpy(form, uni, iform);
      *piform = iform;
      return (1);
    }
  }
  return (0);
}



static int dict_select_composite(struct og_info *info, int iroot, unsigned char *root)
{
  for (int i = 4; i < iroot; i += 2)
  {
    int length = iroot - i;
    if (length <= 10) break;
    int retour;
    IFE(retour = OgAutTry(info->ha_roots, length, root + i));
    if (retour == 2)
    {
      if (info->verbose)
      {
        int root_utf8_length;
        unsigned char root_utf8[DPcPathSize + 9];
        int subroot_utf8_length;
        unsigned char subroot_utf8[DPcPathSize + 9];
        IFE(OgUniToCp(iroot,root, DPcPathSize, &root_utf8_length, root_utf8, DOgCodePageUTF8, 0, 0));
        IFE(OgUniToCp(length,root + i, DPcPathSize, &subroot_utf8_length, subroot_utf8, DOgCodePageUTF8, 0, 0));
        printf("composite %s contains %s\n", root_utf8, subroot_utf8);
      }
      return (0);
    }
  }
  return (1);
}




static int dict_get_root(struct og_info *info)
{
  FILE *fd;
  if (info->input_filename[0])
  {
    IFn(fd=fopen(info->input_filename,"rb"))
    {
      OgMsg(info->hmsg, "", DOgMsgDestInLog, "Impossible to fopen '%s' for reading", info->input_filename);
      DONE;
    }
    sprintf(info->root_filename,"%s.root",info->input_filename);
    sprintf(info->form_filename,"%s.form",info->input_filename);

    IFn(info->fdroot=fopen(info->root_filename,"w"))
    {
      OgMsg(info->hmsg, "", DOgMsgDestInLog, "Impossible to fopen '%s' for writing", info->root_filename);
      DONE;
    }
    IFn(info->fdform=fopen(info->form_filename,"w"))
    {
      OgMsg(info->hmsg, "", DOgMsgDestInLog, "Impossible to fopen '%s' for writing", info->form_filename);
      DONE;
    }

    unsigned char line[DPcPathSize + 9];
    int iuni; unsigned char uni[DPcPathSize*2+9];
    int iroot; unsigned char root[DPcPathSize*2+9];
    int iform; unsigned char form[DPcPathSize*2+9];
    while (fgets(line, DPcPathSize, fd))
    {
      OgTrimString(line, line);
      IFE(OgCpToUni(strlen(line), line, DPcPathSize, &iuni, uni, DOgCodePageUTF8, 0, 0));
      // Getting the root string and putting it in an automaton
      // format is : form::root+PoS
      IFE(dict_get_root_string(info, iuni, uni, &iroot, root));
      if (iroot > 0)
      {
        IFE(dict_get_form_string(info, iuni, uni, &iform, form));
        if (iform > 0)
        {
          IFE(dict_write_root_form(info, iroot, root, iform, form));
        }
      }
    }

    fclose(fd);
    fclose(info->fdroot);
    fclose(info->fdform);
  }
  else
  {
    OgUse(info);
  }

  DONE;

}

static int dict_write_root_form(struct og_info *info, int iroot, unsigned char *root, int iform, unsigned char *form)
{
  OgUniStrlwr(iroot,root,root);
  OgUniStrlwr(iform,form,form);
  int imin = iroot;
  if (imin > iform) imin=iform;
  int ibase=imin;
  for (int i=0; i<imin; i+=2)
  {
    int croot = (root[i]<<8) + root[i+1];
    int cform = (form[i]<<8) + form[i+1];
    if (croot == cform) continue;
    ibase=i;
  }
  // root and form are identical
  if (ibase==imin && iroot==iform) DONE;

  // We build the utf8 strings
  // the root format is root:Nchange
  // the form format is form:Nchange
  // N is the number of chars that need to be removed and then change is appended

  int base_utf8_length; unsigned char base_utf8[DPcPathSize + 9];
  int root_utf8_length; unsigned char root_utf8[DPcPathSize + 9];
  int form_utf8_length; unsigned char form_utf8[DPcPathSize + 9];

  IFE(OgUniToCp(ibase,root, DPcPathSize, &base_utf8_length, base_utf8, DOgCodePageUTF8, 0, 0));
  IFE(OgUniToCp(iroot,root, DPcPathSize, &root_utf8_length, root_utf8, DOgCodePageUTF8, 0, 0));
  IFE(OgUniToCp(iform,form, DPcPathSize, &form_utf8_length, form_utf8, DOgCodePageUTF8, 0, 0));

  int backward_root = root_utf8_length - base_utf8_length;
  int backward_form = form_utf8_length - base_utf8_length;

  fprintf(info->fdroot,"%s:%d%s\n",root_utf8,backward_root,form_utf8+(form_utf8_length-backward_form));
  fprintf(info->fdform,"%s:%d%s\n",form_utf8,backward_form,root_utf8+(root_utf8_length-backward_root));

  DONE;
}


STATICF(int) OgUse(struct og_info *info)
{
  char buffer[8192];
  int ibuffer = 0;

  ibuffer += sprintf(buffer, "Usage : ogdict [options] request_text\n");
  ibuffer += sprintf(buffer + ibuffer, "options are:\n");
  ibuffer += sprintf(buffer + ibuffer, "  -i,  --input=<input_filename>: specify input filename\n");
  ibuffer += sprintf(buffer + ibuffer, "  -o,  --output=<output_filename>: specify output filename (defaut is stdout)\n");
  ibuffer += sprintf(buffer + ibuffer, "  -h,  --help prints this message\n");
  ibuffer += sprintf(buffer + ibuffer, "  -t<n>: trace options for logging (default 0x%x)\n", info->loginfo->trace);
  ibuffer += sprintf(buffer + ibuffer, "    <n> has a combined hexadecimal value of:\n");
  ibuffer += sprintf(buffer + ibuffer, "      0x1: minimal, 0x2: memory, 0x4: conf, 0x8: input\n");
  ibuffer += sprintf(buffer + ibuffer, "      0x10: ignore, 0x20: reparse, 0x40: match, 0x80: select\n");
  ibuffer += sprintf(buffer + ibuffer, "      0x100: output\n");
  ibuffer += sprintf(buffer + ibuffer, "  -r,  --remove-composites remove all composite words\n");
  ibuffer += sprintf(buffer + ibuffer, "     example: in german words like autobahn are removed because bahn exists\n");
  ibuffer += sprintf(buffer + ibuffer, "  -g,  --get-root creates <input_filename>.root and <input_filename>.form\n");

  OgLogConsole(info->hmsg, "%.*s", ibuffer, buffer);

  DONE;
}

static void OgExit(struct og_info *info)
{
  time_t ltime;

  OgMsgErr(info->hmsg, "ogfxp_error", 0, 0, 0, DOgMsgSeverityEmergency, 0);
  time(&ltime);
  OgMsg(info->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgSeverityEmergency,
      "Program %s exiting on error at %.24s\n", OgLogGetModuleName(info->hmsg), OgGmtime(&ltime));

  exit(1);
}

