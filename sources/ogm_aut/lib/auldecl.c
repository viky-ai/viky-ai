/*
 *  Ajout linguistique d'une ligne de caract\`eres dans l'automate.
 *  Copyright (c) 1997-2008 Pertimm by Patrick Constant
 *  Dev: Janvier,F\'evrier,Mai 1997, July 2008
 *  Version 1.3
*/
#include "ogm_aut.h"

#define DPcMaxNbSurface    10

struct decl_surface {
  unsigned char ch[DPcMaxSizeWord];
  unsigned char sep[5];
  int nch,nsep;
  };

struct decl_all {
  int nb_surface,nb_normal,delete;
  struct decl_surface surface[DPcMaxNbSurface];
  struct decl_normal normal[DPcMaxNbNormal];
  };


STATICF(int) AulDeclClean(pr(struct decl_all *));
STATICF(int) AulDeclDelete(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *)
  pr(struct decl_all *));
STATICF(int) AulDeclSimple(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *)
  pr(struct decl_all *));
STATICF(int) AulDeclNormal(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *)
  pr(struct decl_all *));
STATICF(int) AulDeclEqual(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *)
  pr(struct decl_all *));
STATICF(int) AulDeclUpper(pr_(struct og_ctrl_aut *) pr_(struct decl_all *) pr_(int) pr(int));
STATICF(int) AulDeclUpperBuild(pr_(unsigned char *) pr(unsigned char *));
STATICF(int) AulDeclUpperAdd(pr_(struct decl_all *) pr_(int) pr_(int)
  pr(unsigned char *));
STATICF(int) AulDeclLower(pr_(struct og_ctrl_aut *) pr(struct decl_all *));
STATICF(int) AulDeclTiret(pr_(struct decl_all *) pr(int));
STATICF(int) AulDeclDelTiret(pr(struct decl_all *));
STATICF(int) AulDeclMin(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *)
  pr(struct decl_all *));
STATICF(int) AulDeclMin1(pr_(unsigned char *) pr_(unsigned char *)
  pr(unsigned char *));
STATICF(int) AulDeclKill(pr(struct decl_all *));



/*
 *  Lecture et compilation d'une d\'eclaration linguistique d'un mot.
 *  Les ':' sp\'ecifient des s\'eparations de champ.
 *  Renvoit 0 si tout va bien et (-1) sinon.
*/

int AulDecl(ctrl_aut,n,in,out,nb_out)
struct og_ctrl_aut *ctrl_aut;
int n; unsigned char *in;
unsigned char out[][DPcMaxSizeDecl];
int nb_out;
{
int i,j,state,iswarning;
char erreur[DOgErrorSize];
struct decl_all declaration;
state=1; iswarning=0; for (i=0; i<nb_out; i++) out[i][0]=0;
memset(&declaration,0,sizeof(struct decl_all));
for (i=0; in[i]!=0; i++) {
  switch(state) {

    case 1 :  /** au tout d\'ebut **/
      if (in[i]==':') {
        DPcSprintf(erreur,"line %d (%.30s) start with ':'",n,in);
        OgErr(ctrl_aut->herr,erreur); DPcErr; }
      declaration.surface[declaration.nb_surface].ch[
        declaration.surface[declaration.nb_surface].nch++]=in[i];
      if (declaration.surface[declaration.nb_surface].nch >= DPcMaxSizeWord) {
        DPcSprintf(erreur,"line %d (%.30s) : word too long",n,in);
        OgErr(ctrl_aut->herr,erreur); DPcErr; }
      state=2; break;

    case 2 :  /** \`a l'int\'erieur du champ **/
      if (in[i]==':') {
        declaration.nb_surface++;
        declaration.surface[declaration.nb_surface].sep[
          declaration.surface[declaration.nb_surface].nsep++]=in[i];
        state=3; break;
        }
      declaration.surface[declaration.nb_surface].ch[
        declaration.surface[declaration.nb_surface].nch++]=in[i];
      if (declaration.surface[declaration.nb_surface].nch >= DPcMaxSizeWord) {
        DPcSprintf(erreur,"line %d (%.30s) : word too long",n,in);
        OgErr(ctrl_aut->herr,erreur); DPcErr; }
      state=2; break;

    case 3 :  /** dans le s\'eparateur **/
      if (in[i]==':') {
        if (declaration.surface[declaration.nb_surface].nsep<2) {
          declaration.surface[declaration.nb_surface].sep[
            declaration.surface[declaration.nb_surface].nsep++]=in[i];
          state=3; break;
          }
        if (!iswarning) {
          OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
            ,"Warning line %d (%.30s) : too many ':'\n",n,in);
          iswarning=1;
          }
        state=3; break;
        }
      declaration.surface[declaration.nb_surface].ch[
        declaration.surface[declaration.nb_surface].nch++]=in[i];
      if (declaration.surface[declaration.nb_surface].nch >= DPcMaxSizeWord) {
        DPcSprintf(erreur,"line %d (%.30s) : word too long",n,in);
        OgErr(ctrl_aut->herr,erreur); DPcErr; }
      state=2; break;

    }
  }
if (declaration.nb_surface==0) DONE;
declaration.nb_surface++;

IFE(AulDeclClean(&declaration));

#ifdef DEVERMINE
for (i=0; i<declaration.nb_surface; i++) {
  printf("%s%s ",declaration.surface[i].sep,declaration.surface[i].ch);
  }
printf("\n");
#endif

IFE(AulDeclDelete(ctrl_aut,n,in,&declaration));
IFE(AulDeclSimple(ctrl_aut,n,in,&declaration));
IFE(AulDeclNormal(ctrl_aut,n,in,&declaration));

for (i=j=0; i<declaration.nb_normal; i++) {
  if (declaration.normal[i].killed) continue;
  sprintf(out[j++],"%s:%s:%s%s",declaration.normal[i].entry,
    declaration.normal[i].attribut,declaration.normal[i].value,
    (declaration.delete==1)?":-":"");
  }

#ifdef DEVERMINE
for (i=0; out[i][0]!=0; i++)
  printf("  -> %s\n",out[i]);
#endif


DONE;
}



/*
 *  Fonction de nettoyage des champs :
 *    suppression des blancs au d\'ebut et \`a la fin ;
 *    une suite de blancs est r\'eduite \`a un blanc ;
 *    les blancs autour du tiret ('-') sont supprim\'e.
*/

STATICF(int) AulDeclClean(declaration)
struct decl_all *declaration;
{
int i,j,k,b;
unsigned char *ch;
for (i=0; i<=declaration->nb_surface; i++) {
  ch=declaration->surface[i].ch;
  /** suppression des blancs au d\'ebut **/
  for (j=0; ch[j]!=0; j++) if (!PcIsspace(ch[j])) break;
  for (k=j; ch[k]!=0; k++) ch[k-j]=ch[k]; ch[k-j]=0;
  /** suppression des blancs \`a la fin **/
  for (j=k-j-1; j>=0; j--) if (!PcIsspace(ch[j])) break; ch[j+1]=0;
  /** r\'eduction des suites de blancs **/
  for (j=k=0; ch[j]!=0; ) {
    if (PcIsspace(ch[j])) {
      for (b=j+1; ch[b]!=0; b++) if (!PcIsspace(ch[b])) break;
      ch[k++]=' '; j=b;
      }
    else ch[k++]=ch[j++];
    }
  ch[k]=0;
  /** suppression des blancs autour des '-', des '+' et des '=' **/
  for (j=k=0; ch[j]!=0; j++) {
    if (ch[j]==' ' && (ch[j+1]=='-' || ch[j+1]=='+' || ch[j+1]=='='))
      { ch[k++]=ch[j+1]; j++; if (ch[j+2]==' ') j++; }
    else if ((ch[j]=='-' || ch[j]=='+' || ch[j]=='=') && ch[j+1]==' ')
      { ch[k++]=ch[j]; j++; }
    else ch[k++]=ch[j];
    }
  ch[k]=0;
  }
DONE;
}





/*
 *  Les d\'eclarations que l'on veut supprimer se terminent
 *  toutes par le champ ":-". On a deux cas :
 *    une d\'eclaration normale ou simplifi\'ee qui se termine par ":-"
 *    une entr\'ee qui est suivie de ":-".
 *  Cette fonction remplie le champ 'delete' de declaration comme suit :
 *    si on est dans le premier cas : delete=1
 *    sinon si on est dans le second cas : delete=2
 *    sinon delete=0
*/

STATICF(int) AulDeclDelete(ctrl_aut,n,in,declaration)
struct og_ctrl_aut *ctrl_aut;
int n; unsigned char *in;
struct decl_all *declaration;
{
char erreur[DPcSzErr];
struct decl_surface *last_surface;
last_surface=declaration->surface+(declaration->nb_surface-1);
if (strcmp(last_surface->ch,"-")) DONE;
if (declaration->nb_surface<2) {
  DPcSprintf(erreur,"Warning line %d ignored (%.30s), too few fields",n,in);
  OgErr(ctrl_aut->herr,erreur); DPcErr; }
/** Pour les termes simples, il faut obtenir l'attribut 'u' **/
/** pour n'effacer que les entr\'ees ayant les majuscules **/
/** au bon endroit. Pour les termes complexes, la valeur **/
/** de l'attribut 'u' n'est pas importante, puisque l'on **/
/** ne peut coder des attributs diff\'erents correspondant **/
/** \`a des attributs 'u' diff\'erents. La valeur de 'u' **/
/** n'est donc importante que pour les termes simples **/
/** On va donc utiliser un attribut diff\'erent not\'e  **/
/** '-' mais ayant les valeurs de l'attribut 'u' **/
else if (declaration->nb_surface==2) {
  struct decl_normal *normal;
  declaration->delete=2; normal=declaration->normal+0;
  strcpy(normal->entry,declaration->surface[0].ch);
  normal->nentry=declaration->surface[0].nch;
  /** AulDeclUpper a besoin de cet attribut **/
  normal->type_attribut=DPcAttribut_delete;
  IFE(AulDeclComposante(normal));
  declaration->nb_normal++;
  IFE(AulDeclUpper(ctrl_aut,declaration,0,0));
  IFE(AulDeclLower(ctrl_aut,declaration));
  IFE(AulDeclDelTiret(declaration));
  strcpy(normal->attribut,"-"); normal->nattribut=1;
  /** AulDeclUpper a effac\'e cet attribut, il faut le remettre **/
  normal->type_attribut=DPcAttribut_delete;
  }
else { declaration->delete=1; declaration->nb_surface--; }
DONE;
}





/*
 *  Traitement des d\'eclarations simplifi\'ees.
 *  Les  d\'eclarations simplifi\'ees sont repr\'esent\'es par
 *  le fait qu'il y a deux ':' sur le deuxi\`eme champ.
 *  On ne traite pas les d\'eclarations normales ':'
 *  dans cette fonction. Renvoit ERREUR lorsque la ligne
 *  ne peut \^etre prise en compte. Dans ce cas, un warning
 *  est \'emis. Un warning peut aussi \^etre \'emis sans
 *  que la fonction renvoie ERREUR. Dans ce cas, la ligne est
 *  trait\'ee. Le message indique si la ligne est ignor\'ee ou non.
 *  Renvoit CORRECT si tout s'est bien pass\'e.
*/

STATICF(int) AulDeclSimple(ctrl_aut,n,in,declaration)
struct og_ctrl_aut *ctrl_aut; int n; unsigned char *in;
struct decl_all *declaration;
{
int i,inormal;
unsigned char *ch;
char erreur[DPcSzErr];
struct decl_normal *normal,*first_normal;
if (declaration->surface[1].nsep<=1) DONE;
if (declaration->delete>=2) DONE;
if (declaration->nb_surface<2) {
  DPcSprintf(erreur,"Warning line %d ignored (%.30s), too few fields",n,in);
  OgErr(ctrl_aut->herr,erreur); DPcErr; }

/** Ajout de la d\'eclaration 'cs' ou 'c' **/
normal=declaration->normal+declaration->nb_normal;
strcpy(normal->entry,declaration->surface[0].ch);
normal->nentry=declaration->surface[0].nch;
for (i=0; declaration->surface[1].ch[i]!=0; i++)
  if (declaration->surface[1].ch[i]=='+')
    { normal->type_attribut=DPcAttribut_cs; break; }
if (!normal->type_attribut) {
  normal->type_attribut=DPcAttribut_c;
  strcpy(normal->attribut,"c"); normal->nattribut=1;
  }
else { strcpy(normal->attribut,"cs"); normal->nattribut=2; }
strcpy(normal->value,declaration->surface[1].ch);
normal->nvalue=declaration->surface[1].nch;
IFE(AulDeclComposante(normal));
inormal=declaration->nb_normal++;
IFE(AulDeclUpper(ctrl_aut,declaration,inormal,1));
IFE(AulDeclTiret(declaration,inormal));
first_normal=normal;

/** Ajout de la d\'eclaration 'f' **/
normal=declaration->normal+declaration->nb_normal;
ch=declaration->surface[1].ch;
for (i=0; ch[i]!=0 && ch[i]!='+'; i++)
  normal->entry[i]=ch[i]; normal->nentry=i;
if (!strcmp(normal->entry,"=")) {
  strcpy(normal->entry,declaration->surface[0].ch);
  normal->nentry=declaration->surface[0].nch;
  }
normal->type_attribut=DPcAttribut_f;
strcpy(normal->attribut,"f"); normal->nattribut=1;
strcpy(normal->value,declaration->surface[0].ch);
normal->nvalue=declaration->surface[0].nch;
IFE(AulDeclComposante(normal));
inormal=declaration->nb_normal++;
IFE(AulDeclUpper(ctrl_aut,declaration,inormal,1));
IFE(AulDeclTiret(declaration,inormal));

if (declaration->nb_surface<3) goto end;
/** Ajout de la d\'eclaration 's' ou 'm' **/
normal=declaration->normal+declaration->nb_normal;
strcpy(normal->entry,declaration->surface[0].ch);
normal->nentry=declaration->surface[0].nch;
if (first_normal->type_attribut==DPcAttribut_cs)
  { normal->type_attribut=DPcAttribut_m; strcpy(normal->attribut,"m"); }
else { normal->type_attribut=DPcAttribut_s; strcpy(normal->attribut,"s"); }
normal->nattribut=1;
strcpy(normal->value,declaration->surface[2].ch);
normal->nvalue=declaration->surface[2].nch;
declaration->nb_normal++;

if (declaration->nb_surface<4) goto end;
/** Ajout de la d\'eclaration 'm' ou 'v' **/
normal=declaration->normal+declaration->nb_normal;
strcpy(normal->entry,declaration->surface[0].ch);
normal->nentry=declaration->surface[0].nch;
if (first_normal->type_attribut==DPcAttribut_cs)
  { normal->type_attribut=DPcAttribut_v; strcpy(normal->attribut,"v"); }
else { normal->type_attribut=DPcAttribut_m; strcpy(normal->attribut,"m"); }
normal->nattribut=1;
strcpy(normal->value,declaration->surface[3].ch);
normal->nvalue=declaration->surface[3].nch;
declaration->nb_normal++;

if (declaration->nb_surface<5) goto end;
/** Ajout de la d\'eclaration 'v' ou 'i' **/
normal=declaration->normal+declaration->nb_normal;
strcpy(normal->entry,declaration->surface[0].ch);
normal->nentry=declaration->surface[0].nch;
if (first_normal->type_attribut==DPcAttribut_cs)
  { normal->type_attribut=DPcAttribut_i; strcpy(normal->attribut,"i"); }
else { normal->type_attribut=DPcAttribut_v; strcpy(normal->attribut,"v"); }
normal->nattribut=1;
strcpy(normal->value,declaration->surface[4].ch);
normal->nvalue=declaration->surface[4].nch;
declaration->nb_normal++;

if (declaration->nb_surface<6) goto end;
if (first_normal->type_attribut==DPcAttribut_cs) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"Warning line %d (%.30s), last field(s) ignored\n",n,in);
  goto end;
  }
/** Ajout de la d\'eclaration 'i' **/
normal=declaration->normal+declaration->nb_normal;
strcpy(normal->entry,declaration->surface[0].ch);
normal->nentry=declaration->surface[0].nch;
normal->type_attribut=DPcAttribut_i; strcpy(normal->attribut,"i");
normal->nattribut=1;
strcpy(normal->value,declaration->surface[5].ch);
normal->nvalue=declaration->surface[5].nch;
declaration->nb_normal++;

end:
IFE(AulDeclLower(ctrl_aut,declaration));
IFE(AulDeclDelTiret(declaration));
/** Met \`a jour les composantes pour les d\'eclarations normales **/
/** qui ont \'et\'e cr\'ees en dynamique **/
for (i=1; i<declaration->nb_normal; i++)
  IFE(AulDeclComposante(declaration->normal+i));
IFE(AulDeclMin(ctrl_aut,n,in,declaration));
IFE(AulDeclKill(declaration));
DONE;
}




/*
 *  Cr\'eation des d\'eclarations normales \`a partir de la d\'eclaration
 *  de surface. Les d\'eclarations normales sont repr\'esent\'es par
 *  le fait qu'il n'y a qu'un ':' sur le deuxi\`eme champ.
 *  On ne traite pas les d\'eclarations simplifi\'ees ('::')
 *  dans cette fonction. Renvoit ERREUR lorsque la ligne
 *  ne peut \^etre prise en compte. Dans ce cas, un warning
 *  est \'emis. Un warning peut aussi \^etre \'emis sans
 *  que la fonction renvoie ERREUR. Dans ce cas, la ligne est
 *  trait\'ee. Le message indique si la ligne est ignor\'ee ou non.
 *  Renvoit CORRECT si tout s'est bien pass\'e.
*/

STATICF(int) AulDeclNormal(ctrl_aut,n,in,declaration)
struct og_ctrl_aut *ctrl_aut;
int n; unsigned char *in;
struct decl_all *declaration;
{
int i;
char erreur[DPcSzErr];
struct decl_normal *normal;
if (declaration->surface[1].nsep>1) DONE;
if (declaration->delete>=2) DONE;
if (declaration->nb_surface<3) {
  DPcSprintf(erreur,"Warning line %d ignored (%.30s), 3 fields needed",n,in);
  OgErr(ctrl_aut->herr,erreur); DPcErr; }
if (declaration->nb_surface>3) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    , "Warning line %d (%.30s) : last %d field ignored\n"
    , n,in,declaration->nb_surface-3);
  }
/** On commence par recopier les trois champs **/
/** dans la premi\`ere d\'eclaration normale **/
normal=declaration->normal+0;
strcpy(normal->entry,declaration->surface[0].ch);
strcpy(normal->attribut,declaration->surface[1].ch);
strcpy(normal->value,declaration->surface[2].ch);
normal->nentry=declaration->surface[0].nch;
normal->nattribut=declaration->surface[1].nch;
normal->nvalue=declaration->surface[2].nch;
IFE(AulDeclAttribut(normal));
IFE(AulDeclComposante(normal));
declaration->nb_normal++;
if (AulDeclEqual(ctrl_aut,n,in,declaration)) DONE;
/** Suivant la forme de l'entr\'ee, certaines d\'eclarations **/
/** normales doivent \^etre ajout\'ees et la forme de l'entr\'ee **/
/** doit \^etre modifi\'ee en cons\'equence. **/
IFE(AulDeclUpper(ctrl_aut,declaration,0,1));
IFE(AulDeclTiret(declaration,0));
IFE(AulDeclLower(ctrl_aut,declaration));
IFE(AulDeclDelTiret(declaration));
/** Met \`a jour les composantes pour les d\'eclarations normales **/
/** qui ont \'et\'e cr\'ees en dynamique **/
for (i=1; i<declaration->nb_normal; i++)
  IFE(AulDeclComposante(declaration->normal+i));
IFE(AulDeclMin(ctrl_aut,n,in,declaration));
DONE;
}


/*
 *  Calcul du type de l'attribut.
*/

int AulDeclAttribut(normal)
struct decl_normal *normal;
{
unsigned char *attribut=normal->attribut;
if (!strcmp(attribut,"cs")) normal->type_attribut=DPcAttribut_cs;
else if (!strcmp(attribut,"c")) normal->type_attribut=DPcAttribut_c;
else if (!strcmp(attribut,"s")) normal->type_attribut=DPcAttribut_s;
else if (!strcmp(attribut,"f")) normal->type_attribut=DPcAttribut_f;
else if (!strcmp(attribut,"m")) normal->type_attribut=DPcAttribut_m;
else if (!strcmp(attribut,"u")) normal->type_attribut=DPcAttribut_u;
else if (!strcmp(attribut,"t")) normal->type_attribut=DPcAttribut_t;
else if (!strcmp(attribut,"v")) normal->type_attribut=DPcAttribut_v;
else if (!strcmp(attribut,"i")) normal->type_attribut=DPcAttribut_i;
else if (!strcmp(attribut,"b")) normal->type_attribut=DPcAttribut_b;
else if (!strcmp(attribut,"-")) normal->type_attribut=DPcAttribut_delete;
else if (!strcmp(attribut,"=")) normal->type_attribut=DPcAttribut_equal;
else normal->type_attribut=DPcAttribut_autre;
DONE;
}



/*
 *  Construit les chaines des diff\'erentes composantes
 *  du terme complexe. Le champ 'ncomposante_entry'
 *  donne le nombre de composantes de l'entr\'ee.
 *  Ce champ est \`a un pour les termes simples.
*/

int AulDeclComposante(normal)
struct decl_normal *normal;
{
int i,j;
unsigned char *entry=normal->entry;
unsigned char *value=normal->value;

normal->ncomposante_entry=0;
for (i=j=0; entry[i]!=0; i++) {
  if (entry[i]==' ' || entry[i]=='-') {
    normal->composante_entry[normal->ncomposante_entry][j]=0;
    j=0; normal->ncomposante_entry++;
    }
  else normal->composante_entry[normal->ncomposante_entry][j++]=entry[i];
  }
normal->composante_entry[normal->ncomposante_entry][j]=0;
normal->ncomposante_entry++;

normal->ncomposante_value=0;
if (normal->type_attribut==DPcAttribut_c
  || normal->type_attribut==DPcAttribut_f
  || normal->type_attribut==DPcAttribut_cs) {
  for (i=j=0; value[i]!=0 && value[i]!='+'; i++) {
    if (value[i]==' ' || value[i]=='-') {
      normal->composante_value[normal->ncomposante_value][j]=0;
      j=0; normal->ncomposante_value++;
      }
    else normal->composante_value[normal->ncomposante_value][j++]=value[i];
    }
  normal->composante_value[normal->ncomposante_value][j]=0;
  normal->ncomposante_value++;
  }
DONE;
}


/*
 *  Traitement de la d\'eclaration d'attribut '='
 *  il s'agit d'aller chercher par la fonction AulScan
 *  toutes les informations associ\'ees \`a la valeur de l'attribut
 *  consid\'er\'e comme une entr\'ee en matchant un maximum de lettres
 *  \`a la fin. Exemples : turber:=:danser fait une \'equivalence
 *  entre 'turb' et 'dans' et cr\'ee toutes les formes correspondantes.
 *  Cette fonction renvoit 1 si elle a trait\'e une d\'eclaration '='
 *  elle renvoit 0 sinon.
*/

STATICF(int) AulDeclEqual(ctrl_aut,n,in,declaration)
struct og_ctrl_aut *ctrl_aut;
int n; unsigned char *in;
struct decl_all *declaration;
{
struct att_value out1[70];
struct att_value out2[10];
struct decl_normal *cnormal;
unsigned char hentry[DPcMaxSizeWord];
int i,j,nentry,nvalue,ihentry,ihvalue;
unsigned char value_plus[DPcMaxSizeWord];
unsigned char lower_value[DPcMaxSizeWord];
struct decl_normal *normal=declaration->normal+0;
if (normal->type_attribut!=DPcAttribut_equal) return(0); normal->killed=1;
/** On ne traite pas les entr\'ees complexes **/
if (normal->ncomposante_entry>1) return(0);

#ifdef DEVERMINE
printf(">>%s:%s:%s%s\n",normal->entry, normal->attribut,normal->value,
  (declaration->delete==1)?":-":"");
#endif

nentry=strlen(normal->entry); nvalue=strlen(normal->value);
for (i=nentry-1,j=nvalue-1; i>=0 && j>=0; i--,j--) {
  if (normal->entry[i]!=normal->value[j]) break;
  }
ihentry=i+1; ihvalue=j+1;
memcpy(hentry,normal->entry,ihentry); hentry[ihentry]=0;
strcpy(value_plus,normal->value);
value_plus[nvalue]='+'; value_plus[nvalue+1]=0;
strcpy(lower_value,normal->value);
if (!ctrl_aut->aul_nocharcase) {
  for (i=0; lower_value[i]!=0; i++) lower_value[i]=PcTolower(lower_value[i]);
  }
IFE(OgAulScan((void *)ctrl_aut,DPcAttribut_all,lower_value,out1,70));
for (i=0; out1[i].type_attribut!=0; i++) {

  /** On ne choisit que les entr\'ee qui correspondent **/
  /** \`a l'entr\'ee propos\'ee, en tenant compte des majuscules **/
  if (strcmp(out1[i].entry,normal->value)) continue;

#ifdef DEVERMINE
  printf(">>  %s%s:%s:%s%s\n",
    hentry,out1[i].entry+ihvalue,
    out1[i].attribut,
    hentry,out1[i].value+ihvalue);
#endif

  if (out1[i].type_attribut==DPcAttribut_cs
    || out1[i].type_attribut==DPcAttribut_c
    || out1[i].type_attribut==DPcAttribut_f) {

    /** On ne choisit que les racines qui correspondent **/
    /** \`a la racine qui est demand\'ee (normal->value) **/
    if (out1[i].type_attribut==DPcAttribut_cs
      || out1[i].type_attribut==DPcAttribut_c)
      if (memcmp(value_plus,out1[i].value,nvalue+1)) continue;

    cnormal=declaration->normal+declaration->nb_normal;
    cnormal->type_attribut=out1[i].type_attribut;
    sprintf(cnormal->entry,"%s%s",hentry,out1[i].entry+ihvalue);
    strcpy(cnormal->attribut,out1[i].attribut);
    sprintf(cnormal->value,"%s%s",hentry,out1[i].value+ihvalue);
    cnormal->nentry=strlen(cnormal->entry);
    cnormal->nattribut=strlen(cnormal->attribut);
    cnormal->nvalue=strlen(cnormal->value);
    declaration->nb_normal++;

    if (out1[i].type_attribut==DPcAttribut_f) {
      /** On ne prend en compte que les 'cs' des 'f' **/
      IFE(OgAulScan((void *)ctrl_aut,DPcAttribut_cs,out1[i].value,out2,10));
      for (j=0; out2[j].type_attribut!=0; j++) {
        /** On ne choisit que les racines qui correspondent **/
        /** \`a la racine qui est demand\'ee (normal->value) **/
        if (out2[j].type_attribut==DPcAttribut_cs)
          if (memcmp(value_plus,out2[j].value,nvalue+1)) continue;

#ifdef DEVERMINE
        printf(">>    %s%s:%s:%s%s\n",
          hentry,out2[j].entry+ihvalue,
          out2[j].attribut,
          hentry,out2[j].value+ihvalue);
#endif
        cnormal=declaration->normal+declaration->nb_normal;
        cnormal->type_attribut=out2[j].type_attribut;
        sprintf(cnormal->entry,"%s%s",hentry,out2[j].entry+ihvalue);
        strcpy(cnormal->attribut,out2[j].attribut);
        sprintf(cnormal->value,"%s%s",hentry,out2[j].value+ihvalue);
        cnormal->nentry=strlen(cnormal->entry);
        cnormal->nattribut=strlen(cnormal->attribut);
        cnormal->nvalue=strlen(cnormal->value);
        declaration->nb_normal++;

        }
      }
    }
  /** Dans tous les autres cas, la seule possibilit\'e est **/
  /** simplement de reprendre la valeur telle quelle **/
  else {
    cnormal=declaration->normal+declaration->nb_normal;
    cnormal->type_attribut=out1[i].type_attribut;
    sprintf(cnormal->entry,"%s%s",hentry,out1[i].entry+ihvalue);
    strcpy(cnormal->attribut,out1[i].attribut);
    strcpy(cnormal->value,out1[i].value);
    cnormal->nentry=strlen(cnormal->entry);
    cnormal->nattribut=strlen(cnormal->attribut);
    cnormal->nvalue=strlen(cnormal->value);
    declaration->nb_normal++;
    }
  }
/** Met \`a jour les composantes pour les d\'eclarations normales **/
/** qui ont \'et\'e cr\'ees en dynamique, afin que la suite **/
/** du traitement se passe correctement **/
for (i=1; i<declaration->nb_normal; i++) {
  IFE(AulDeclComposante(declaration->normal+i));
  IFE(AulDeclUpper(ctrl_aut,declaration,i,declaration->nb_normal));
  /** Ce test permet de savoir si AulDeclUpper a cr\'e\'e **/
  /** une d\'eclaration suppl\'ementaire ou non **/
  if (declaration->normal[declaration->nb_normal].type_attribut)
    declaration->nb_normal++;
  }
IFE(AulDeclLower(ctrl_aut,declaration));
IFE(AulDeclMin(ctrl_aut,n,in,declaration));
return(1);
}





/*
 *  Traitement des majuscules. Les r\`egles sont les suivantes :
 *  Pour les entr\'ees simples :
 *    si la d\'eclaration est 'cs', l'attribut 'u' se met en +u
 *    si la d\'eclaration est 'c' ou 's' on rajoute un attribut 'u'
 *    sinon on ne fait rien.
 *  Pour les entr\'ees complexes :
 *    on rajoute un attribut 'u' dans tous les cas.
 *  le traitement des majuscules ne met rien en minuscule,
 *  c'est la fonction AulDeclLower qui fait ce travail.
*/

STATICF(int) AulDeclUpper(ctrl_aut,declaration,inormal,ibuild)
struct og_ctrl_aut *ctrl_aut;
struct decl_all *declaration;
int inormal,ibuild;
{
int i,nupper;
unsigned char *entry;
unsigned char upper[DPcMaxSizeWord];
struct decl_normal *normal=declaration->normal+inormal;

if (ctrl_aut->aul_nocharcase) DONE;

/** On v\'erifie s'il y a une lettre majuscule **/
entry=normal->entry;
for (i=nupper=0; entry[i]!=0; i++)
  if (PcIsupper(entry[i])) nupper++;
if (nupper==0 && normal->type_attribut!=DPcAttribut_delete) DONE;

/** Cas du terme complexe **/
memset(upper,0,DPcMaxSizeWord);
if (normal->ncomposante_entry>1) {
  for (i=0; i<normal->ncomposante_entry; i++)
    IFE(AulDeclUpperBuild(normal->composante_entry[i],upper+strlen(upper)));
  IFE(AulDeclUpperAdd(declaration,inormal,ibuild,upper));
  DONE;
  }

/** Cas du terme simple **/
IFE(AulDeclUpperBuild(entry,upper+1));
if (normal->type_attribut==DPcAttribut_cs
  || normal->type_attribut==DPcAttribut_f) {
  upper[0]='+'; strcpy(normal->value+strlen(normal->value),upper);
  }
else if (normal->type_attribut==DPcAttribut_c
  || normal->type_attribut==DPcAttribut_s
  || normal->type_attribut==DPcAttribut_delete) {
  IFE(AulDeclUpperAdd(declaration,inormal,ibuild,upper+1));
  }
DONE;
}


/*
 *  Fabrique la cha\^ine de caract\`eres correspondant
 *  \`a la valeur du champ 'u' et la mets dans 'upper'
*/

STATICF(int) AulDeclUpperBuild(entry,upper)
unsigned char *entry,*upper;
{
int i,nupper;
for (i=nupper=0; entry[i]!=0; i++)
  if (PcIsupper(entry[i])) nupper++;
if (nupper==0) strcpy(upper,"n");
else if (nupper==1 && PcIsupper(entry[0])) strcpy(upper,"u");
else if (nupper==i) strcpy(upper,"U");
else {
  upper[0]='u';
  for (i=0; entry[i]!=0; i++)
    if (PcIsupper(entry[i])) upper[i+1]='1';
    else upper[i+1]='0';
  upper[i+1]=0;
  }
DONE;
}



STATICF(int) AulDeclUpperAdd(declaration,inormal,ibuild,upper)
struct decl_all *declaration; int inormal,ibuild;
unsigned char *upper;
{
struct decl_normal *normal=declaration->normal+inormal;
struct decl_normal *unormal=declaration->normal+inormal+ibuild;
unormal->type_attribut=DPcAttribut_u;
strcpy(unormal->entry,normal->entry);
strcpy(unormal->attribut,"u");
strcpy(unormal->value,upper);
declaration->nb_normal+=ibuild;
DONE;
}



/*
 *  Fonction qui met en minuscule les cha\^ines suivantes :
 *    l'entr\'ee ;
 *    la valeur des champs 'c', 'f' ;
 *    la premi\`ere partie du champ 'cs' ;
 *  Le passage en minuscule s'effectue sur les d\'eclarations normales.
 *  On effectue aussi le passage en minuscule pour les composantes.
*/

STATICF(int) AulDeclLower(ctrl_aut,declaration)
struct og_ctrl_aut *ctrl_aut;
struct decl_all *declaration;
{
int i,j,k;

if (ctrl_aut->aul_nocharcase) DONE;

for (i=0; i<declaration->nb_normal; i++) {
  struct decl_normal *normal=declaration->normal+i;
  for (j=0; normal->entry[j]!=0; j++)
    normal->entry[j]=PcTolower(normal->entry[j]);
  if (normal->type_attribut==DPcAttribut_c
    || normal->type_attribut==DPcAttribut_f
    || normal->type_attribut==DPcAttribut_cs)
    for (j=0; normal->value[j]!=0 && normal->value[j]!='+'; j++)
      normal->value[j]=PcTolower(normal->value[j]);

  for(j=0; j<normal->ncomposante_entry; j++)
    for (k=0; normal->composante_entry[j][k]!=0; k++)
      normal->composante_entry[j][k]=PcTolower(normal->composante_entry[j][k]);

  for(j=0; j<normal->ncomposante_value; j++)
    for (k=0; normal->composante_value[j][k]!=0; k++)
      normal->composante_value[j][k]=PcTolower(normal->composante_value[j][k]);
  }
DONE;
}



/*
 *  Traitement des tiret. On rajoute un attribut 't' sur les entr\'ee
 *  dans tout les cas o\`u il y a un tiret.
 *  On supprime le tiret dans les cha\^ines suivantes :
 *    l'entr\'ee ;
 *    la valeur des champs 'c', 'f' ;
 *    la premi\`ere partie du champ 'cs'
*/

STATICF(int) AulDeclTiret(declaration,inormal)
struct decl_all *declaration;
int inormal;
{
int i,j,ntiret;
unsigned char *entry;
unsigned char tiret[DPcMaxSizeWord];
struct decl_normal *normal=declaration->normal+inormal;
struct decl_normal *tnormal=declaration->normal+declaration->nb_normal;
/** On v\'erifie s'il y a un tiret et on calcule **/
/** la valeur de l'attribut 't' en m\^eme temps **/
entry=normal->entry;
for (i=j=ntiret=0; entry[i]!=0; i++)
  if (entry[i]=='-') { tiret[j++]='t'; ntiret++; }
  else if (entry[i]==' ') tiret[j++]='n';
tiret[j]=0; if (ntiret==0) DONE;
/** On sait ici qu'il faut ajouter l'attribut **/
tnormal->type_attribut=DPcAttribut_t;
strcpy(tnormal->entry,normal->entry);
strcpy(tnormal->attribut,"t");
strcpy(tnormal->value,tiret);
declaration->nb_normal++;
DONE;
}




/*
 *  Suppression de tous les tirets
*/

STATICF(int) AulDeclDelTiret(declaration)
struct decl_all *declaration;
{
int i,j;
struct decl_normal *normal;
for (i=0; i<declaration->nb_normal; i++) {
  normal=declaration->normal+i;
  for (j=0; normal->entry[j]!=0; j++)
    if (normal->entry[j]=='-') normal->entry[j]=' ';
  if (normal->type_attribut==DPcAttribut_c
    || normal->type_attribut==DPcAttribut_f
    || normal->type_attribut==DPcAttribut_cs)
    for (j=0; normal->value[j]!=0 && normal->value[j]!='+'; j++)
      if (normal->value[j]=='-') normal->value[j]=' ';
  }
DONE;
}



/*
 *  Traitement des minimisations des repr\'esentations
 *  cha\^ines de caract\`eres dan les zones suivantes :
 *    la valeur des champs 'c', 'f' ;
 *    la premi\`ere partie du champ 'cs'
 *  Pour l'instant on va consid\'erer qui si le nombre
 *  de composante est diff\'erent entre l'entr\'ee
 *  et la valeur, un warning est \'emis, mais la d\'eclaration
 *  est gard\'ee telle quelle.
*/

STATICF(int) AulDeclMin(ctrl_aut,n,in,declaration)
struct og_ctrl_aut *ctrl_aut;
int n; unsigned char *in;
struct decl_all *declaration;
{
int i,j,k,len;
char erreur[DPcSzErr];
struct decl_normal *normal;
unsigned char value[DPcMaxSizeWord];
unsigned char valuemin[DPcMaxSizeWord];
for (i=0; i<declaration->nb_normal; i++) {
  normal=declaration->normal+i; len=0;
  if (normal->type_attribut==DPcAttribut_c
    || normal->type_attribut==DPcAttribut_f
    || normal->type_attribut==DPcAttribut_cs) {
    for (j=0; normal->value[j]!=0 && normal->value[j]!='+'; j++)
      value[j]=normal->value[j]; value[j]=0;
    if (!strcmp(value,"=")) {
      for (k=0; k<normal->ncomposante_entry; k++) strcpy(valuemin+2*k,"0 ");
      valuemin[len=strlen(valuemin)-1]=0;
      }
    else if (normal->ncomposante_entry!=normal->ncomposante_value) {
      DPcSprintf(erreur,
        "Warning line %d ignored (%.30s), different number of terms",n,in);
      OgErr(ctrl_aut->herr,erreur); DPcErr; }
    else {
      memset(valuemin,0,DPcMaxSizeWord);
      for (len=0,k=0; k<normal->ncomposante_entry; k++) {
        AulDeclMin1(normal->composante_entry[k],
          normal->composante_value[k],valuemin+len);
        valuemin[len=strlen(valuemin)]=' '; len++;
        }
      valuemin[--len]=0;
      }
    for (; normal->value[j]!=0; j++)
      valuemin[len++]=normal->value[j]; valuemin[len]=0;
    strcpy(normal->value,valuemin);
    }
  }
DONE;
}


STATICF(int) AulDeclMin1(entry,value,valuemin)
unsigned char *entry,*value,*valuemin;
{
int i;
for (i=0; value[i]!=0; i++) if (entry[i]!=value[i]) break;
DPcSprintf(valuemin,"%d%s",strlen(entry+i),value+i);
DONE;
}



/*
 *  Marquage des d\'eclarations normales en double.
 *  le champ 'killed' est mis \`a 1.
 *  Cela ne concerne que les attributs 'u' et 't'.
*/

STATICF(int) AulDeclKill(declaration)
struct decl_all *declaration;
{
int i,iu,it;
struct decl_normal *normal;
for (iu=it=i=0; i<declaration->nb_normal; i++) {
  normal=declaration->normal+i;
  if (normal->type_attribut==DPcAttribut_u)
    { if (iu) normal->killed=1; else iu=i; }
  else if (normal->type_attribut==DPcAttribut_t)
    { if (it) normal->killed=1; else it=i; }
  }
DONE;
}


