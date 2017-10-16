/*
 *	Scanning linguistique de l'automate.
 *	Copyright (c) 1997	Patrick Constant
 *	Dev : F\'evrier,Mai 1997
 *	Version 1.1
*/
#include "ogm_aut.h"



STATICF(int) AulScan(pr_(int) pr_(struct decl_normal *) pr_(int)
                     pr_(unsigned char *) pr_(struct att_value *) pr(int));
STATICF(int) AulScanDeleteAttribut(pr_(struct decl_normal *) pr(int));
STATICF(int) AulScanDeleteEntry(pr_(struct decl_normal *) pr(int));
STATICF(int) AulScanParse(pr(struct decl_normal *));
STATICF(int) AulScanExpand(pr(struct decl_normal *));
STATICF(int) AulScanExpand1(pr_(unsigned char *) pr_(unsigned char *)
                            pr(unsigned char *));
STATICF(int) AulScanUpper(pr_(struct decl_normal *) pr(int));
STATICF(int) AulScanUpper1(pr_(struct decl_normal *) pr_(int)
                           pr(unsigned char *));
STATICF(int) IsValueU(pr(unsigned char *));
STATICF(int) AulScanTiret(pr_(struct decl_normal *) pr(int));




PUBLIC(int) OgAulAddScan(handle,line,out,nb_out)
void *handle; unsigned char *line;
struct att_value *out;
int nb_out;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
unsigned char buffer[DPcMaxNbNormal][DPcMaxSizeDecl];
unsigned char entry[DPcMaxSizeDecl];
struct decl_normal normal;
int i,j;

IFE(AulDecl(ctrl_aut,0,line,buffer,DPcMaxNbNormal));
strcpy(entry,buffer[0]);
for (i=0; entry[i]!=0; i++)
  if (entry[i]==':') { entry[i]=0; break; }
out[0].type_attribut=0;
for (i=0; buffer[i][0]!=0; i++) {
  strcpy(entry,buffer[i]);
  for (j=0; entry[j]!=0; j++)
    if (entry[j]==':') { entry[j]=0; break; }
  strcpy(normal.scanned,buffer[i]+j+1);
  AulScan(DPcAttribut_all,&normal,1,entry,out+i,nb_out);
  }
DONE;
}




/*
 *	R\'ecup\'eration des informations de l'automate 'ha'
 *	en fonction des types d'attributs donn\'es par 'type_attribut'
 *	et stockage dans 'out' avec un maximum d'information 
 *	de 'nb_out' informations. On privil\'egie l'automate rapide
 *	s'il existe. Les majuscules et les tirets sont r\'etablies.
 *	Les informations n\'egatives sont prises en compte.
*/

PUBLIC(int) OgAulScan(handle,type_attribut,line,out,nb_out)
void *handle; int type_attribut; unsigned char *line;
struct att_value *out;
int nb_out;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
struct decl_normal normal[DPcMaxNbNormal];
unsigned char lineb[DPcAutMaxBufferSize];
unsigned char buffer[DPcMaxSizeDecl];
oindex states[DPcAutMaxBufferSize];
int i,nb_normal_base;
int retour,nb_normal;
int nstate0,nstate1;

out[0].type_attribut=0;

nb_normal=0; sprintf(lineb,"%s:",line);
if ((retour=OgAufScanf(handle,-1,lineb,0,buffer,&nstate0,&nstate1,states))) {
  do { IFE(retour); strcpy(normal[nb_normal++].scanned,buffer); }
  while((retour=OgAufScann(handle,0,buffer,nstate0,&nstate1,states)));
  }

nb_normal_base=nb_normal;
if ((retour=OgAutScanf(handle,-1,lineb,0,buffer,&nstate0,&nstate1,states))) {
  do { IFE(retour);
    for (i=0; i<nb_normal_base; i++)
      if (!strcmp(normal[i].scanned,buffer)) break;
    if (i>=nb_normal_base) strcpy(normal[nb_normal++].scanned,buffer);
    }
  while((retour=OgAutScann(handle,0,buffer,nstate0,&nstate1,states)));
  }

return(AulScan(type_attribut,normal,nb_normal,line,out,nb_out));
}




STATICF(int) AulScan(type_attribut,normal,nb_normal,line,out,nb_out)
int type_attribut; struct decl_normal *normal; int nb_normal;
unsigned char *line; struct att_value *out; int nb_out;
{
int i,j;
if (nb_normal==0) DONE;
for (i=0; i<nb_normal; i++) normal[i].killed=0;
AulScanDeleteAttribut(normal,nb_normal);

for (i=0; i<nb_normal; i++) {
  if (normal[i].killed) continue;
  strcpy(normal[i].entry,line);
  AulScanParse(normal+i);
  AulDeclComposante(normal+i);
  AulScanExpand(normal+i);
  }
AulScanUpper(normal,nb_normal);
AulScanTiret(normal,nb_normal);

AulScanDeleteEntry(normal,nb_normal);

#ifdef DEVERMINE
/** Attention, les attributs d\'etruits ne sont pas mis totalement **/
/** \`a jour, il ne faut compter que sur normal[i].scanned **/
/** (ce qui est imprim\'e entre parenth\`eses) **/
for (i=0; i<nb_normal; i++) {
  printf("  %s:%s:%s (%s)%s\n",normal[i].entry,normal[i].attribut,
    normal[i].value,normal[i].scanned,(normal[i].killed)?" killed":"");
  }
#endif

for (i=j=0; i<nb_normal; i++) {
  if (normal[i].killed) continue;
  /** on remplie jusqu'\`a nb_out-1 pour que la derni\`ere structure **/
  /** puisse marquer la fin (champ 'type_attribut' \`a z\'ero) **/
  if (normal[i].type_attribut&type_attribut && j<nb_out-1) {
    out[j].type_attribut=normal[i].type_attribut;
    memcpy(out[j].entry,normal[i].entry,DPcMaxSizeWord);
    memcpy(out[j].attribut,normal[i].attribut,DPcMaxSizeWord);
    memcpy(out[j].value,normal[i].value,DPcMaxSizeWord);
    j++;
    }
  }
out[j].type_attribut=0;
DONE;
}



/*
 *	Destruction dynamique des entr\'ees marqu\'ees \`a d\'etruire
 *	Il n'y a normalement que deux cas :
 *	  1 - destruction simple d'un attribut ;
 *	  2 - destruction d'une entr\'ee.
 *	Ici on ne d\'etruit que les attributs.
*/

STATICF(int) AulScanDeleteAttribut(normal,nb_normal)
struct decl_normal *normal;
int nb_normal;
{
int i,j,len;
char del[DPcMaxSizeWord];
for (i=0; i<nb_normal; i++) {
  if (normal[i].killed) continue;
  len=strlen(normal[i].scanned);
  if (!strcmp(":-",normal[i].scanned+len-2)) {
    strcpy(del,normal[i].scanned); del[len-2]=0;
    for (j=0; j<nb_normal; j++)
      if (j!=i && !strcmp(normal[j].scanned,del)) { normal[j].killed=1; break; }
    normal[i].killed=1;
    }
  }
DONE;
}



/*
 *	Destruction dynamique des entr\'ees.
*/

STATICF(int) AulScanDeleteEntry(normal,nb_normal)
struct decl_normal *normal;
int nb_normal;
{
int i,j;
for (i=0; i<nb_normal; i++) {
  if (normal[i].killed) continue;
  if (normal[i].type_attribut==DPcAttribut_delete)
    for (j=0; j<nb_normal; j++)
      if (!strcmp(normal[j].entry,normal[i].entry)) normal[j].killed=1;
  }
DONE;
}






/*
 *	Mise \`a jour des champs 'attribut', 'value'
 *	et 'type_attribut'.
*/

STATICF(int) AulScanParse(normal)
struct decl_normal *normal;
{
int i,j;
unsigned char *scanned=normal->scanned;
for (i=0; scanned[i]!=0; i++) {
  if (scanned[i]==':') break;
  normal->attribut[i]=scanned[i];
  }
normal->attribut[i++]=0;
AulDeclAttribut(normal); if (scanned[i-1]==0) DONE;
for (j=i; scanned[j]!=0; j++) normal->value[j-i]=scanned[j];
normal->value[j-i]=0;
DONE;
}



/*
 *	Expansion des informations sur les champs cs,c,f
 *	On pratique aussi l'expansion des mots compos\'es.
*/

STATICF(int) AulScanExpand(normal)
struct decl_normal *normal;
{
int j,k,len;
unsigned char valueexp[DPcMaxSizeWord];
if (normal->type_attribut==DPcAttribut_c
  || normal->type_attribut==DPcAttribut_f
  || normal->type_attribut==DPcAttribut_cs) {
  /** La valeur n'a pas le m\^eme nombre de composantes **/
  /** il n'y a donc pas eu de minimisation des d\'eclaration **/
  /** et donc logiquement, il n'y a pas d'expansion **/
  if (normal->ncomposante_entry!=normal->ncomposante_value) DONE;
  for (j=0; normal->value[j]!=0 && normal->value[j]!='+'; j++);
  memset(valueexp,0,DPcMaxSizeWord);
  for (len=0,k=0; k<normal->ncomposante_entry; k++) {
    AulScanExpand1(normal->composante_entry[k],
      normal->composante_value[k],valueexp+len);
    valueexp[len=strlen(valueexp)]=' '; len++;
    }
  valueexp[--len]=0;
  for (; normal->value[j]!=0; j++)
    valueexp[len++]=normal->value[j]; valueexp[len]=0;
  strcpy(normal->value,valueexp);
  }
DONE;
}





/*
 *	Expansion effective de 'value' dans 'valueexp'
 *	\`a partir de 'value' et de 'line'.
 *	Recopie  'valueexp' dans 'value'.
*/

STATICF(int) AulScanExpand1(entry,value,valueexp)
unsigned char *entry,*value,*valueexp;
{
int i,j,idel;
int lenentry=strlen(entry);
unsigned char add[DPcMaxSizeWord];
unsigned char del[DPcMaxSizeWord];
for (i=0; value[i]!=0; i++) {
  if (!PcIsdigit(value[i])) break;
  del[i]=value[i];
  }
del[i]=0;
for (j=i; value[j]!=0; j++) {
  if (value[j]=='+') break;
  add[j-i]=value[j];
  }
add[j-i]=0;
idel=atoi(del);
/*  sept 10th 2004, Patrick Constant. An entry such as "page:cs:pa1/4+SUB"
 *  generates "page:cs:21/4+SUB", which is an ambiguity: idel should
 *  be 2 instead of 21. In order to avoid crashing with a negative
 *  memcpy length, we secure that zone. When the length is negative,
 *  valueexp is not correct, but it will not crash. At this moment
 *  we consider that words with numbers in them are not interesting
 *  and should be removed from the automaton's source */
if (lenentry-idel > 0) {
  memcpy(del,entry,lenentry-idel); del[lenentry-idel]=0;
  }
else del[0]=0;
sprintf(valueexp,"%s%s%s",del,add,value+j);
strcpy(value,valueexp);
DONE;
}






STATICF(int) AulScanUpper(normal,nb_normal)
struct decl_normal *normal;
int nb_normal;
{
int i,j,iu,lenu;
for (iu=i=0; i<nb_normal; i++) {
  if (normal[i].killed) continue;
  if (normal[i].type_attribut==DPcAttribut_u)
    { iu=i; lenu=strlen(normal[i].value); break; }
  }
if (iu) {
  for (i=0; i<nb_normal; i++) {
    if (normal[i].killed) continue;
    AulScanUpper1(normal,i,normal[iu].value);
    }
  }
else {
  /** On mets les majuscules sur les attributs 'cs' et 'f' **/
  /** qui sont les seuls attributs \`a accepter des majuscules **/
  for (i=0; i<nb_normal; i++) {
    if (normal[i].killed) continue;
    if (normal[i].type_attribut==DPcAttribut_f
      || normal[i].type_attribut==DPcAttribut_cs) {
      unsigned char *stru;
      for (j=strlen(normal[i].value)-1; j>=0; j--)
        if (normal[i].value[j]=='+') break;
      if (j<0) DONE;
      stru=normal[i].value+j+1;
      if (!IsValueU(stru)) continue;
      AulScanUpper1(normal,i,stru);
      normal[i].value[j]=0;
      }
    else if (normal[i].type_attribut==DPcAttribut_delete) {
      AulScanUpper1(normal,i,normal[i].value);
      }
    }
  }
DONE;
}



STATICF(int) AulScanUpper1(normal,i,stru)
struct decl_normal *normal;
int i; unsigned char *stru;
{
int j,k,lenu,minu,len;
unsigned char upper[DPcMaxSizeWord];
unsigned char tmp_value[DPcMaxSizeWord];

lenu=strlen(stru);
if (lenu<normal[i].ncomposante_entry) minu=lenu;
else minu=normal[i].ncomposante_entry;
for (j=0; j<minu; j++) {
  if (stru[j]=='u') normal[i].composante_entry[j][0]=
    PcToupper(normal[i].composante_entry[j][0]);
  else if (stru[j]=='U') {
    for (k=0; normal[i].composante_entry[j][k]!=0; k++)
      normal[i].composante_entry[j][k]=
        PcToupper(normal[i].composante_entry[j][k]);
    }
  }
for (upper[0]=0,len=0,k=0; k<normal[i].ncomposante_entry; k++) {
  strcpy(upper+len,normal[i].composante_entry[k]);
  upper[len=strlen(upper)]=' '; len++;
  }
upper[--len]=0; strcpy(normal[i].entry,upper);
    if (normal[i].type_attribut==DPcAttribut_c
      || normal[i].type_attribut==DPcAttribut_f
      || normal[i].type_attribut==DPcAttribut_cs) {
      for (j=0; j<minu; j++) {
        if (stru[j]=='u') normal[i].composante_value[j][0]=
          PcToupper(normal[i].composante_value[j][0]);
        else if (stru[j]=='U') {
          for (k=0; normal[i].composante_value[j][k]!=0; k++)
            normal[i].composante_value[j][k]=
              PcToupper(normal[i].composante_value[j][k]);
          }
        }
      for (j=0; normal[i].value[j]!=0 && normal[i].value[j]!='+'; j++);
      for (upper[0]=0,len=0,k=0; k<normal[i].ncomposante_value; k++) {
        strcpy(upper+len,normal[i].composante_value[k]);
        upper[len=strlen(upper)]=' '; len++;
        }
      upper[--len]=0; sprintf(tmp_value,"%s%s",upper,normal[i].value+j);
      strcpy(normal[i].value,tmp_value);
      }
DONE;
}







/*
 *	Teste si 'ch' a la forme d'une valeur d'attribut 'u'.
 *	Renvoit 1 si c'est vrai et 0 sinon.
*/

STATICF(int) IsValueU(ch)
unsigned char *ch;
{
int i,state=1;
for (i=0; ch[i]!=0; i++) {
  switch(state) {
    case 1 :
      if (ch[i]=='n' || ch[i]=='u' || ch[i]=='U'
        || ch[i]=='o' || ch[i]=='O') state=1;
      /*else if (ch[i]=='b' || ch[i]=='B') state=2;*/
      else return(0);
      break;
    case 2 :
      if (ch[i]=='n' || ch[i]=='u' || ch[i]=='U'
        || ch[i]=='o' || ch[i]=='O') state=1;
      /*else if (ch[i]=='b' || ch[i]=='B') return(0);*/
      else if (ch[i]=='1' || ch[i]=='0') state=2;
      else return(0);
      break;
    }
  }
return(1);
}





STATICF(int) AulScanTiret(normal,nb_normal)
struct decl_normal *normal;
int nb_normal;
{
int i,j,k,it,lent=0;
unsigned char *strt=0;
for (it=i=0; i<nb_normal; i++) {
  if (normal[i].killed) continue;
  if (normal[i].type_attribut==DPcAttribut_t)
    { it=i; strt=normal[i].value; lent=strlen(strt); break; }
  }
if (!it) DONE;
for (i=0; i<nb_normal; i++) {
  if (normal[i].killed) continue;
  for (k=j=0; normal[i].entry[j]!=0; j++)
    if (normal[i].entry[j]==' ')
      if (k<lent && strt[k++]=='t') normal[i].entry[j]='-';
  if (normal[i].type_attribut==DPcAttribut_c
    || normal[i].type_attribut==DPcAttribut_f
    || normal[i].type_attribut==DPcAttribut_cs) {
    for (k=j=0; normal[i].value[j]!=0 && normal[i].value[j]!='+'; j++)
      if (normal[i].value[j]==' ')
        if (k<lent && strt[k++]=='t') normal[i].value[j]='-';
    }
  }
DONE;
}











