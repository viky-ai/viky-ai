/*
 *	Writing the output in an XML file
 *	Copyright (c) 2013 Pertimm by Patrick Constant
 *	Dev : July 2013
 *	Version 1.0
*/
#include "ogm_lip.h"



static int OgLipOutputXmlWord(void *context, int Iw);




PUBLIC(int) OgLipOutputXmlInit(void *handle ,char *xml_filename)
{
struct og_ctrl_lip *ctrl_lip=(struct og_ctrl_lip *)handle;
struct og_lip_input *input = ctrl_lip->output_xml_input;

IFn(ctrl_lip->fdout=fopen(xml_filename,"w")) {
  OgMsg(ctrl_lip->hmsg,"",DOgMsgDestInLog
    ,"OgLipOutputXmlInit: impossible to fopen '%s', results will not be built"
    ,xml_filename);
  }
fprintf(ctrl_lip->fdout,
  "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
  "<oglip>\n"
  "<linguistic_parsed_texts>\n");

memset(ctrl_lip->output_xml_input,0,sizeof(struct og_lip_input));
input->conf = ctrl_lip->conf;
input->context=ctrl_lip;
IFx(ctrl_lip->ha_lang) {
  input->language_recognition=1;
  }
  
DONE;
}



PUBLIC(int) OgLipOutputXmlFlush(void *handle)
{
struct og_ctrl_lip *ctrl_lip=(struct og_ctrl_lip *)handle;
IFn(ctrl_lip->fdout) DONE;
fprintf(ctrl_lip->fdout,
  "</linguistic_parsed_texts>\n"
  "</oglip>\n");  
fclose(ctrl_lip->fdout);

DONE;
}




PUBLIC(int) OgLipOutputXml(void *handle, int content_length, unsigned char *content)
{
struct og_ctrl_lip *ctrl_lip=(struct og_ctrl_lip *)handle;
struct og_lip_input *input = ctrl_lip->output_xml_input;
int ib1; unsigned char b1[DPcPathSize];
int ib2; unsigned char b2[DPcPathSize];
struct og_lip_lang lang[1];
int i;

IFn(ctrl_lip->fdout) DONE;

input->content_length = content_length;
input->content = content;

fprintf(ctrl_lip->fdout,
  "  <linguistic_parsed_text>\n");

IFE(OgUniToCp(input->content_length,input->content, DPcPathSize, &ib1, b1, DOgCodePageUTF8, 0, 0));
IFE(OgXmlEncode(ib1,b1,DPcPathSize,&ib2,b2,DOgCodePageUTF8,0));
fprintf(ctrl_lip->fdout,
  "    <input_text>%s</input_text>\n",b2);

input->word_func=0;
IFE(OgLip(ctrl_lip,input));

IFE(OgLipGetLanguages(ctrl_lip,lang));
if (lang->nb_languages > 0) {
  fprintf(ctrl_lip->fdout,
    "    <languages>\n");
  for (i=0; i<lang->nb_languages; i++) {
    fprintf(ctrl_lip->fdout,
      "      <language score=\"%.10f\">%s</language>\n",lang->language[i].score,OgIso639ToCode(lang->language[i].lang));
    }
  fprintf(ctrl_lip->fdout,
    "    </languages>\n");
  }

fprintf(ctrl_lip->fdout,
  "    <output_text>\n");
input->word_func=OgLipOutputXmlWord;
IFE(OgLipAgain(ctrl_lip,input));
fprintf(ctrl_lip->fdout,
  "    </output_text>\n");



fprintf(ctrl_lip->fdout,
  "  </linguistic_parsed_text>\n");

DONE;
}



static int OgLipOutputXmlWord(void *context, int Iw)
{
struct og_ctrl_lip *ctrl_lip=(struct og_ctrl_lip *)context;
int ib1; unsigned char b1[DPcPathSize];
int ib2; unsigned char b2[DPcPathSize];
unsigned char languages[DPcPathSize];
unsigned char has_quote[DPcPathSize];
struct og_lip_word w[1];
int i,l,ilanguages;

IFE(OgLipGetWord(ctrl_lip,Iw,w));

IFE(OgUniToCp(w->length,w->input->content+w->start, DPcPathSize, &ib1, b1, DOgCodePageUTF8, 0, 0));
IFE(OgXmlEncode(ib1,b1,DPcPathSize,&ib2,b2,DOgCodePageUTF8,0));

languages[0]=0;
if (w->lang.nb_languages) {
  ilanguages=0;
  l=sprintf(languages+ilanguages," languages=\""); ilanguages+=l;
  for (i=0; i<w->lang.nb_languages; i++) {
    if (i>0) { l=sprintf(languages+ilanguages," "); ilanguages+=l; }
    l=sprintf(languages+ilanguages,"%s:%.10f",OgIso639ToCode(w->lang.language[i].lang),w->lang.language[i].score); ilanguages+=l;
    }
  l=sprintf(languages+ilanguages,"\""); ilanguages+=l;
  }

has_quote[0]=0;
if (w->has_quote) {
  sprintf(has_quote," has_quote=\"yes\"");
  }

fprintf(ctrl_lip->fdout,
  "      <output_word%s%s>%.*s</output_word>\n",has_quote,languages,ib2,b2);
  
DONE;
}

