/*
 *  Reading alias file that contains lines of the form:
 *    <name in repository> <alias name> 
 *  Copyright (c) 2000  Ogmios
 *  Dev : February 2000
 *  Version 1.0
*/
#include <loggen.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>




PUBLIC(int) OgReadAlias(char *filename, struct og_aliases *aliases)
{
FILE *fd;
char erreur[DPcSzErr];
int i, nb_aliases=0, size_aliases=0;
char line[1024], *true_name, *alias_name;
int itrue_name, ialias_name;

IFn(fd=fopen(filename,"r")) {
  sprintf(erreur,"OgReadAlias: impossible to open '%s'\n", filename);
  PcErr(-1,erreur); DPcErr;
  }

while(fgets(line,1024,fd)) {
  if (line[0] == '#') continue;
  /** +2 because we put a zero at the end of both strings **/
  size_aliases+=strlen(line)+2; 
  nb_aliases++; 
  }

rewind(fd);
memset(aliases,0,sizeof(struct og_aliases));
IFn(aliases->B = (char *)malloc(size_aliases)) {
  sprintf(erreur,"OgReadAlias: impossible to malloc %d bytes for B\n"
         , size_aliases);
  PcErr(-1,erreur); DPcErr;
  }
aliases->nB = size_aliases;

IFn(aliases->alias = (struct og_alias *)malloc(nb_aliases*sizeof(struct og_alias))) {
  sprintf(erreur,"OgReadAlias: impossible to malloc %d bytes for alias\n"
         , nb_aliases*sizeof(struct og_alias));
  PcErr(-1,erreur); DPcErr;
  }
aliases->nalias = nb_aliases;

while(fgets(line,1024,fd)) {
  if (line[0] == '#') continue;
  for (i=0; line[i]!=0; i++) {
    if (line[i] == '\t') break;
    }
  if (line[i] == 0) continue; line[i] = 0;
  true_name=line; alias_name=line+i+1;
  for (i=0; true_name[i]!=0; i++) {
    if (!isspace((int)true_name[i])) { true_name += i; break; }
    }
  //for (i=0; true_name[i]!=0; i++) {
  //  if (isspace((int)true_name[i])) { true_name[i]=0; break; }
  //  }
  itrue_name = strlen(true_name);
  for (i=itrue_name-1; i>=0; i--) {
    if (!isspace((int)true_name[i])) { itrue_name=i+1; true_name[itrue_name]=0; break; }
    }
/** We don't want aliases to be lowercased on any system in fact (not only Unix) **/
#if 0
  for (i=0; true_name[i]!=0; i++) true_name[i]=tolower(true_name[i]);
#endif
  aliases->alias[aliases->ialias].true_name = aliases->iB;
  aliases->alias[aliases->ialias].itrue_name = itrue_name;
  memcpy(aliases->B+aliases->iB,true_name,itrue_name+1); aliases->iB += itrue_name+1;
  
  for (i=0; alias_name[i]!=0; i++) {
    if (!isspace((int)alias_name[i])) { alias_name += i; break; }
    }
  //for (i=0; alias_name[i]!=0; i++) {
  //  if (isspace((int)alias_name[i])) { alias_name[i]=0; break; }
  //  }
  ialias_name = strlen(alias_name);
  for (i=ialias_name-1; i>=0; i--) {
    if (!isspace((int)alias_name[i])) { ialias_name=i+1; alias_name[ialias_name]=0; break; }
    }
  if (!strcmp(alias_name,"<empty>")) { *alias_name=0; ialias_name=0; } 
  aliases->alias[aliases->ialias].alias_name = aliases->iB;
  aliases->alias[aliases->ialias].ialias_name = ialias_name;
  memcpy(aliases->B+aliases->iB,alias_name,ialias_name+1); aliases->iB += ialias_name+1;
  aliases->ialias++;
  }

fclose(fd);
DONE;
}





PUBLIC(int) OgFreeAlias(struct og_aliases *aliases)
{
IFx(aliases) {
  DPcFree(aliases->alias);
  DPcFree(aliases->B);
  }
DONE;
}





PUBLIC(int) OgGetAlias(struct og_aliases *aliases, char *name, char *aliased_name)
{
int i;
IFx(aliases) {
  for (i=0; i<aliases->ialias; i++) {
    if (memcmp(name, aliases->B + aliases->alias[i].true_name
              , aliases->alias[i].itrue_name)) continue;
    strcpy(aliased_name, aliases->B + aliases->alias[i].alias_name);
    strcpy(aliased_name + aliases->alias[i].ialias_name
          , name + aliases->alias[i].itrue_name);
    return(1);
    }
  }
strcpy(aliased_name,name);
return(0);
}



/*
 *  parses an alias string of form:
 *  "actu,institutionnel,psg,culturefoot,l1,clubsjoueurs,magazine"
 *  return 1 when an alias is found and zero when not found.
 *  it is used as a loop:
 *  int ialias,pos=0; char alias[256];
 *  while(OgParseAliases(4,"toto",&ialias,alias,&pos)) { ... }
*/

PUBLIC(int) OgParseAliases(int is, unsigned char *s, int *ialias, unsigned char *alias, int *pos)
{
int i,c,ia,state=1,end=0;

for (i=(*pos); !end; i++) {
  if (end) break;
  if (i>=is) { end=1; c=' '; }
  else c=s[i];
  switch(state) {
    case 1:
      if (OgInAliasName(c)) {
        ia=0; alias[ia++]=c;
        state=2;
        }
      break;
    case 2:
      if (OgInAliasName(c)) {
        alias[ia++]=c;
        }
      else {
        alias[ia]=0; *ialias=ia;
        *pos=i; return(1);
        }
      break;
    }
  }

alias[0]=0; *ialias=0; *pos=i;
return(0);
}





PUBLIC(int) OgInAliasName(int c)
{
return(PcIsalnum(c) || c=='_' || c=='-');
} 



