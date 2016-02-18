/*
 *	Simply for reading tag_ctrl.dat
 *	Copyright (c) 1999-2000	Ogmios by Patrick Constant
 *	Dev : August 1999, May 2000
 *	Version 1.1
*/
#include <loggen.h>
#include <logtag.h>


#define DOgTagNumberOffset   1

static int GetContentType(int, char *, int *);


/*
 *  This function reads the file for controlling tags
 *  named 'filename'. It is usually data/tag_ctrl.dat.
*/

PUBLIC(int) ReadTagCtrl(char *filename, struct tag_ctrl *tag_ctrl, char *where)
{
FILE *fd;
char line[1024];
int i, n, ifield, state, title_priority, iline;
int p[DOgNbField+1]; int l[DOgNbField+1];

memset(tag_ctrl,0,sizeof(struct tag_ctrl));
/** Setting the default default tag **/
tag_ctrl->nb_tags = 1;
strcpy(tag_ctrl->t[0].string,DOgDefaultField);
tag_ctrl->t[0].length = strlen(DOgDefaultField);
tag_ctrl->t[0].value = 0;
tag_ctrl->t[0].keep_tag = 0;
tag_ctrl->t[0].keep_text = 1;
tag_ctrl->t[0].cut_word = 1;
tag_ctrl->t[0].cut_expr = 1;
strcpy(tag_ctrl->t[0].name,tag_ctrl->t[0].string);
tag_ctrl->t[0].iname = tag_ctrl->t[0].length;

IFn(fd=fopen(filename,"r")) {
  MessageInfoLog(0,where, 0
    , "file '%s' does not exist, default values assumed"
    , filename);
  DONE;
  }
while(fgets(line,1024,fd)) {
  memset(l,0,DOgNbField*sizeof(int)); ifield=0; state=1;
  iline=strlen(line); if (!isspace(line[iline-1])) { line[iline++]='\n'; line[iline]=0; }
  for (i=0; line[i]!=0; i++) {
    switch (state) {
      case 1:
        if (line[i] == '#') goto bad_line;
        if (isspace(line[i])) break;
        p[ifield] = i;
        l[ifield]++;
        state=2;
        break;
      case 2:
        if (isspace(line[i])) {
          if (ifield <= DOgNbField) { ifield++; state = 1; }
          }
        else l[ifield]++;
        break;   
      }
    }
  /** -3 because name,weight and entity are optional **/
  if (ifield >= DOgNbField-3) {
    if (line[p[0]] == '<') n = 0; /*default tag*/
    if (!Ogmemicmp(line+p[0],"<default>",9)) n = 0; /*default tag*/
    else n = tag_ctrl->nb_tags;
    if (l[0] > DOgMaxCtrlTagLength-1) l[0] = DOgMaxCtrlTagLength-1;
    memcpy(tag_ctrl->t[n].string, line + p[0], l[0]);
    tag_ctrl->t[n].string[l[0]] = 0;
    PcStrlwr(tag_ctrl->t[n].string);
    tag_ctrl->t[n].length = l[0];
    tag_ctrl->t[n].value = atoi(line + p[1]);
    /* Because value 0x1 is used as DOgTokenNothing 
     * We use values starting at 0x11 as the other second postfixes
     * We could start at 0x2 but then, why bother for 15 more values.
    */
    if (tag_ctrl->t[n].value) tag_ctrl->t[n].value += DOgTagNumberOffset;
    if (tag_ctrl->t[n].value > DOgMaxCtrlTag-1) {
      MessageInfoLog(0,where, 0
        , "tag '%s' has too big value %d (max is %d) in '%s', value set to zero"
        , tag_ctrl->t[n].string, tag_ctrl->t[n].value - DOgTagNumberOffset
        , DOgMaxCtrlTag - 1 - DOgTagNumberOffset, filename);
      tag_ctrl->t[n].value = 0;
      }
    tag_ctrl->t[n].keep_tag  = ((line[p[2]]=='t' || line[p[2]]=='T')?1:0);
    tag_ctrl->t[n].keep_text = ((line[p[3]]=='t' || line[p[3]]=='T')?1:0);
    tag_ctrl->t[n].cut_word  = ((line[p[4]]=='t' || line[p[4]]=='T')?1:0);
    tag_ctrl->t[n].cut_expr  = ((line[p[5]]=='t' || line[p[5]]=='T')?1:0);
    /** -1 because entity is after name and weight **/
    if (ifield >= DOgNbField-1) {
      tag_ctrl->t[n].iname = l[6];
      memcpy(tag_ctrl->t[n].name, line + p[6], l[6]);
      /** '-' means use first field as name **/
      if (tag_ctrl->t[n].iname==1 && tag_ctrl->t[n].name[0]=='-') {
        tag_ctrl->t[n].iname = tag_ctrl->t[n].length;
        memcpy(tag_ctrl->t[n].name, tag_ctrl->t[n].string, tag_ctrl->t[n].iname);
        }
      tag_ctrl->t[n].weight = atoi(line + p[7]);
      IFn(tag_ctrl->t[n].weight) tag_ctrl->t[n].weight = 1;
      }
    else {
      tag_ctrl->t[n].iname = tag_ctrl->t[n].length;
      memcpy(tag_ctrl->t[n].name, tag_ctrl->t[n].string, tag_ctrl->t[n].iname);
      tag_ctrl->t[n].weight = 1;
      }
    tag_ctrl->t[n].content_type=DOgTagContentTypeNil;
    if (ifield >= DOgNbField) {
      tag_ctrl->t[n].content_type=GetContentType(l[8],line+p[8],&title_priority);
      if (tag_ctrl->t[n].content_type & DOgTagContentTypeTitle) {
        IFn(title_priority) title_priority=DOgDefaultTitlePriority;
        tag_ctrl->t[n].title_priority=title_priority;
        }
      }
    if (n) tag_ctrl->nb_tags++;
    }
  bad_line:;
  }
fclose(fd);
DONE;
}





static int GetContentType(int is, char *s, int *title_priority)
{
int i,c,end=0,start=0,content_type=0;
char *file_entity="file_entity"; int ifile_entity=strlen(file_entity);
char *entity="entity"; int ientity=strlen(entity);
char *number="number"; int inumber=strlen(number);
char *title="title"; int ititle=strlen(title);
char *date="date"; int idate=strlen(date);
char *xref="xref"; int ixref=strlen(xref);
char *single="single"; int isingle=strlen(single);
*title_priority=0;

for (i=0; !end; i++) {
  if (i>=is) { end=1; c='/'; }
  else c=s[i];
  if (c=='/') {
    if (ifile_entity==i-start && !Ogmemicmp(s+start,file_entity,ifile_entity)) {
      content_type|=DOgTagContentTypeFileEntity;
      }
    else if (ientity==i-start && !Ogmemicmp(s+start,entity,ientity)) {
      content_type|=DOgTagContentTypeEntity;
      }
    else if (inumber==i-start && !Ogmemicmp(s+start,number,inumber)) {
      content_type|=DOgTagContentTypeNumber;
      }
    else if (idate==i-start && !Ogmemicmp(s+start,date,idate)) {
      content_type|=DOgTagContentTypeDate;
      }
    else if (ixref==i-start && !Ogmemicmp(s+start,xref,ixref)) {
      content_type|=DOgTagContentTypeXref;
      }
    else if (ititle<=i-start && !Ogmemicmp(s+start,title,ititle)) {
      content_type|=DOgTagContentTypeTitle;
      *title_priority=atoi(s+start+ititle);
      }
    else if (isingle==i-start && !Ogmemicmp(s+start,single,isingle)) {
      content_type|=DOgTagContentTypeSingle;
      }
    start=i+1;
    }
  }
return(content_type);
}






PUBLIC(int) PrintTagCtrl(struct tag_ctrl *tag_ctrl, char *where)
{
int i; char scontent_type[64];
MessageInfoLog(0,where, 0,"value ktag ktxt cutw cute weig name:length entity");
for (i=0; i<tag_ctrl->nb_tags; i++) {
  MessageInfoLog(0,where, 0,"%5d %4d %4d %4d %4d %4d %s:%d %s:%d %s"
        , tag_ctrl->t[i].value
        , tag_ctrl->t[i].keep_tag
        , tag_ctrl->t[i].keep_text
        , tag_ctrl->t[i].cut_word
        , tag_ctrl->t[i].cut_expr
        , tag_ctrl->t[i].weight
        , tag_ctrl->t[i].string, tag_ctrl->t[i].length
        , tag_ctrl->t[i].name, tag_ctrl->t[i].iname
        , OgGetTagContentTypeString(tag_ctrl->t[i].content_type,scontent_type)
        );
  }
DONE;
}








PUBLIC(char *) OgGetTagContentTypeString(int content_type, char *s)
{
if (content_type==0) strcpy(s,"nil");
else {
  int is; s[0]=0;
  if (content_type & DOgTagContentTypeFileEntity) sprintf(s+strlen(s),"file_entity/");
  if (content_type & DOgTagContentTypeEntity) sprintf(s+strlen(s),"entity/");
  if (content_type & DOgTagContentTypeNumber) sprintf(s+strlen(s),"number/");
  if (content_type & DOgTagContentTypeDate) sprintf(s+strlen(s),"date/");
  if (content_type & DOgTagContentTypeXref) sprintf(s+strlen(s),"xref/");
  if (content_type & DOgTagContentTypeTitle) sprintf(s+strlen(s),"title/");
  if (content_type & DOgTagContentTypeSingle) sprintf(s+strlen(s),"single/");
  is=strlen(s); if (is>0 && s[is-1]=='/') s[--is]=0;
  }
return(s);
}




