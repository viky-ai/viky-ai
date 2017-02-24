/*
 * ltraclog.c
 *
 *  Created on: 24 févr. 2017
 *      Author: caroline
 */

#include "ogm_ltrac.h"

static og_status LtracLog(struct og_ctrl_ltrac *ctrl_ltrac, void *automaton, char *filename);

og_status LtracLogLtrac(struct og_ctrl_ltrac *ctrl_ltrac)
{
  char log_file[DPcPathSize];
  if (ctrl_ltrac->WorkingDirectory[0])
  {
    sprintf(log_file, "%s/log/ogltrac_ltraf.log", ctrl_ltrac->WorkingDirectory);
  }
  else
  {
    sprintf(log_file, "log/ogltrac_ltraf.log");
  }

  IFE(LtracLog(ctrl_ltrac, ctrl_ltrac->ha_ltrac, log_file));

  DONE;
}

og_status LtracLogLtracRequests(struct og_ctrl_ltrac *ctrl_ltrac)
{
  char log_file[DPcPathSize];
  if (ctrl_ltrac->WorkingDirectory[0])
  {
    sprintf(log_file, "%s/log/ogltrac_ltraf_requests.log", ctrl_ltrac->WorkingDirectory);
  }
  else
  {
    sprintf(log_file, "log/ogltrac_ltraf_requests.log");
  }

  IFE(LtracLog(ctrl_ltrac, ctrl_ltrac->ha_expressions, log_file));

  DONE;
}

static og_status LtracLog(struct og_ctrl_ltrac *ctrl_ltrac, void *automaton, char *filename)
{
  FILE *fd = fopen(filename, "wb");
  IFn(fd)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "LtracLog: Impossible to open '%s' for writing", filename);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  unsigned char out[DOgLtracMaxWordsSize];
  oindex states[DOgLtracMaxWordsSize];
  int nstate0, nstate1, iout;
  int retour = -1;
  if ((retour = OgAutScanf(automaton, -1, "", &iout, out, &nstate0, &nstate1, states)))
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

      int iutf8 = 0;
      unsigned char utf8[DOgLtracMaxWordsSize];
      IFE(OgUniToCp(sep,out,DOgLtracMaxWordsSize,&iutf8,utf8,DOgCodePageUTF8,0,0));

      int language_code = DOgLangNil;
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));

      int Iltraf = -1;
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&Iltraf));
      IFE(Iltraf);

      struct ltraf *ltraf = ctrl_ltrac->Ltraf + Iltraf;

      fprintf(fd, "%d:%d:%s\n", ltraf->frequency, language_code, utf8);
    }
    while ((retour = OgAutScann(automaton, &iout, out, nstate0, &nstate1, states)));
  }

  fclose(fd);

  DONE;
}
