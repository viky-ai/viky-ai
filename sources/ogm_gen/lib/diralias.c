/*
 *  Reading alias file that contains lines of the form:
 *    <directory in select format> <external name of directory> 
 *  Copyright (c) 2003 Pertimm by Patrick Constant
 *  Dev : June 2003
 *  Version 1.0
*/
#include <loggen.h>

#define DOgMaxToken 10

struct og_token {
  int start, end;
  };

/* Format is the following:
 *   directory          equivalent name
 *   /umr/              <fils>
 *   /presse/           "La presse"
 *   /presse/journal/   "Le journal du CNRS"
 * General syntax is:
 *   <dir spécification>[<tab>(<fils>|<nom visible du répertoire>)
 * <dir specification> is a specification of type [DirSelect] from ogm_conf.txt
*/

PUBLIC(int) OgReadDirAlias(void *herr, char *filename, struct og_dir_aliases *aliases)
{
FILE *fd;
char erreur[DOgErrorSize];
int i, nb_aliases=0, size_aliases=0;
struct og_token *t, token[DOgMaxToken];
unsigned char line[DPcPathSize];
int is; unsigned char *s;

IFn(fd=fopen(filename,"r")) {
  sprintf(erreur,"OgReadDirAlias: impossible to open '%s'\n", filename);
  OgErr(herr,erreur); DPcErr;
  }

while(fgets(line,1024,fd)) {
  if (line[0] == '#') continue;
  size_aliases+=strlen(line); 
  nb_aliases++; 
  }

rewind(fd);
memset(aliases,0,sizeof(struct og_dir_aliases));
IFn(aliases->B = (char *)malloc(size_aliases)) {
  sprintf(erreur,"OgReadAlias: impossible to malloc %d bytes for B\n"
         , size_aliases);
  OgErr(herr,erreur); DPcErr;
  }
aliases->nB = size_aliases;

IFn(aliases->alias = (struct og_dir_alias *)malloc(nb_aliases*sizeof(struct og_dir_alias))) {
  sprintf(erreur,"OgReadAlias: impossible to malloc %d bytes for alias\n"
         , nb_aliases*sizeof(struct og_dir_alias));
  OgErr(herr,erreur); DPcErr;
  }
aliases->nalias = nb_aliases;

while(fgets(line,1024,fd)) {
  int state, itoken, start_token=0, end_token=0;
  struct og_dir_alias *a;
  if (line[0] == '#') continue;
  state=1; itoken=0;
  for (i=0; line[i]!=0; i++) {
    int c = line[i];
    switch (state) {
      /** before token **/
      case 1:
        if (isspace(c)) state=1;
        else {
          start_token=i;
          end_token=i;
          state=2;
          }
        break;
      /** in token **/
      case 2:
        if (isspace(c)) {
          if (end_token == start_token) end_token=i;
          }
        else end_token = start_token;
        if (c=='\t') {
          token[itoken].start = start_token;
          token[itoken].end = end_token;
          itoken++;
          if (itoken >= DOgMaxToken) goto end_line;
          state=1;
          }
        break;
      }
    }
  end_line:

  if (state==2) {
    if (end_token == start_token) end_token=i;
    token[itoken].start = start_token;
    token[itoken].end = end_token;
    itoken++;
    }

  if (itoken != 2) continue;

  a = aliases->alias + aliases->ialias;
  a->aliases = aliases;

  i = 0; t = token + i;
  a->dir = aliases->iB;  a->idir = t->end - t->start;
  is = a->idir; s = line+t->start;
  /** removing '/' at end **/
  for (i=is-1; i>=0; i--) {
    if (s[i]!='/' && s[i]!='\\') break;
    }
  if (i<is-1) a->idir = i+1;
  memcpy(aliases->B+aliases->iB, line+t->start, a->idir);
  aliases->B[aliases->iB + a->idir]=0;
  aliases->iB += a->idir+1;
  
  i = 1; t = token + i;
  if (line[t->start]=='"') t->start++;
  if (line[t->end-1]=='"') t->end--;
  a->alias = aliases->iB;  a->ialias = t->end - t->start;
  memcpy(aliases->B+aliases->iB, line+t->start, a->ialias);
  aliases->B[aliases->iB + a->ialias]=0;
  aliases->iB += a->ialias+1;

  aliases->ialias++;  
  }

fclose(fd);
DONE;
}





int OgFreeDirAlias(struct og_dir_aliases *aliases)
{
DPcFree(aliases->alias);
DPcFree(aliases->B);
DONE;
}



