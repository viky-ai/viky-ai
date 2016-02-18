/*
 *  Generic functions for language, based on ISO 639-1
 *  Copyright (c) 1999-2002 Pertimm by Patrick Constant
 *  Dev : October 1999, August 2002
 *  Version 1.1
*/
#include <loggen.h>
#include <logis639.h>


struct iso639 {
  unsigned char *code;
  unsigned char *en;
  unsigned char *fr;
};


static int NameIso639Initialized=0;
static struct iso639 NameIso639[200];


static unsigned char * OgInitNameIso639(void)
{
struct iso639 *n = NameIso639;
n->code="--"; n->en="Nil"; n->fr="nil"; n++;
n->code="aa"; n->en="Afar"; n->fr="afar"; n++;
n->code="ab"; n->en="Abkhazian"; n->fr="abkhazien"; n++;
n->code="af"; n->en="Afrikaans"; n->fr="afrikaans"; n++;
n->code="am"; n->en="Amharic"; n->fr="amharique"; n++;
n->code="ar"; n->en="Arabic"; n->fr="arabe"; n++;
n->code="as"; n->en="Assamese"; n->fr="assamais"; n++;
n->code="ay"; n->en="Aymara"; n->fr="aymara"; n++;
n->code="az"; n->en="Azerbaijani"; n->fr="azerba�djanais; az�ri"; n++;
n->code="ba"; n->en="Bashkir"; n->fr="bashkir"; n++;
n->code="be"; n->en="Byelorussian"; n->fr="bi�lorusse"; n++;
n->code="bg"; n->en="Bulgarian"; n->fr="bulgare"; n++;
n->code="bh"; n->en="Bihari"; n->fr="bihari"; n++;
n->code="bi"; n->en="Bislama"; n->fr="bislama"; n++;
n->code="bn"; n->en="Bengali; Bangla"; n->fr="bengali"; n++;
n->code="bo"; n->en="Tibetan"; n->fr="tib�tain"; n++;
n->code="br"; n->en="Breton"; n->fr="breton"; n++;
n->code="ca"; n->en="Catalan"; n->fr="catalan"; n++;
n->code="co"; n->en="Corsican"; n->fr="corse"; n++;
n->code="cs"; n->en="Czech"; n->fr="tch�que"; n++;
n->code="cy"; n->en="Welsh"; n->fr="gallois"; n++;
n->code="da"; n->en="Danish"; n->fr="danois"; n++;
n->code="de"; n->en="German"; n->fr="allemand"; n++;
n->code="dz"; n->en="Bhutani"; n->fr="bhoutani"; n++;
n->code="el"; n->en="Greek"; n->fr="grec"; n++;
n->code="en"; n->en="English"; n->fr="anglais"; n++;
n->code="eo"; n->en="Esperanto"; n->fr="esp�ranto"; n++;
n->code="es"; n->en="Spanish"; n->fr="espagnol"; n++;
n->code="et"; n->en="Estonian"; n->fr="estonien"; n++;
n->code="eu"; n->en="Basque"; n->fr="basque"; n++;
n->code="fa"; n->en="Persian"; n->fr="persan"; n++;
n->code="fi"; n->en="Finnish"; n->fr="finnois"; n++;
n->code="fj"; n->en="Fiji"; n->fr="fidji"; n++;
n->code="fo"; n->en="Faroese"; n->fr="f�ro�en"; n++;
n->code="fr"; n->en="French"; n->fr="fran�ais"; n++;
n->code="fy"; n->en="Frisian"; n->fr="frison"; n++;
n->code="ga"; n->en="Irish"; n->fr="irlandais"; n++;
n->code="gd"; n->en="Scots Gaelic"; n->fr="ga�lique �cossais"; n++;
n->code="gl"; n->en="Galician"; n->fr="galicien"; n++;
n->code="gn"; n->en="Guarani"; n->fr="guarani"; n++;
n->code="gu"; n->en="Gujarati"; n->fr="goujrati"; n++;
n->code="gv"; n->en="Manx Gaelic"; n->fr="ga�lique de l'�le de Man"; n++;
n->code="ha"; n->en="Hausa"; n->fr="haoussa"; n++;
n->code="he"; n->en="Hebrew"; n->fr="h�breu"; n++;
n->code="hi"; n->en="Hindi"; n->fr="hindi"; n++;
n->code="hr"; n->en="Croatian"; n->fr="croate"; n++;
n->code="hu"; n->en="Hungarian"; n->fr="hongrois"; n++;
n->code="hy"; n->en="Armenian"; n->fr="arm�nien"; n++;
n->code="ia"; n->en="Interlingua"; n->fr="interlingua"; n++;
n->code="id"; n->en="Indonesian"; n->fr="indon�sien"; n++;
n->code="ie"; n->en="Interlingue"; n->fr="interlingue"; n++;
n->code="ik"; n->en="Inupiak"; n->fr="inupiak"; n++;
n->code="is"; n->en="Icelandic"; n->fr="islandais"; n++;
n->code="it"; n->en="Italian"; n->fr="italien"; n++;
n->code="iu"; n->en="Inuktitut"; n->fr="inuktitut"; n++;
n->code="ja"; n->en="Japanese"; n->fr="japonais"; n++;
n->code="jw"; n->en="Javanese"; n->fr="javanais"; n++;
n->code="ka"; n->en="Georgian"; n->fr="g�orgien"; n++;
n->code="kk"; n->en="Kazakh"; n->fr="kazakh"; n++;
n->code="kl"; n->en="Greenlandic"; n->fr="groenlandais"; n++;
n->code="km"; n->en="Cambodian"; n->fr="cambodgien"; n++;
n->code="kn"; n->en="Kannada"; n->fr="kannada"; n++;
n->code="ko"; n->en="Korean"; n->fr="cor�en"; n++;
n->code="ks"; n->en="Kashmiri"; n->fr="kashmiri"; n++;
n->code="ku"; n->en="Kurdish"; n->fr="kurde"; n++;
n->code="kw"; n->en="Cornish"; n->fr="cornique"; n++;
n->code="ky"; n->en="Kirghiz"; n->fr="kirghiz"; n++;
n->code="la"; n->en="Latin"; n->fr="latin"; n++;
n->code="lb"; n->en="Luxemburgish"; n->fr="luxembourgeois"; n++;
n->code="ln"; n->en="Lingala"; n->fr="lingala"; n++;
n->code="lo"; n->en="Laothian"; n->fr="laotien"; n++;
n->code="lt"; n->en="Lithuanian"; n->fr="lituanien"; n++;
n->code="lv"; n->en="Latvian; Lettish"; n->fr="letton, lettonien"; n++;
n->code="mg"; n->en="Malagasy"; n->fr="malgache"; n++;
n->code="mi"; n->en="Maori"; n->fr="maori"; n++;
n->code="mk"; n->en="Macedonian"; n->fr="mac�donien"; n++;
n->code="ml"; n->en="Malayalam"; n->fr="malayalam"; n++;
n->code="mn"; n->en="Mongolian"; n->fr="mongol"; n++;
n->code="mo"; n->en="Moldavian"; n->fr="moldave"; n++;
n->code="mr"; n->en="Marathi"; n->fr="marathe"; n++;
n->code="ms"; n->en="Malay"; n->fr="malais"; n++;
n->code="mt"; n->en="Maltese"; n->fr="maltais"; n++;
n->code="my"; n->en="Burmese"; n->fr="birman"; n++;
n->code="na"; n->en="Nauru"; n->fr="nauri"; n++;
n->code="ne"; n->en="Nepali"; n->fr="n�palais"; n++;
n->code="nl"; n->en="Dutch"; n->fr="n�erlandais"; n++;
n->code="no"; n->en="Norwegian"; n->fr="norv�gien"; n++;
n->code="oc"; n->en="Occitan"; n->fr="occitan"; n++;
n->code="om"; n->en="(Afan) Oromo"; n->fr="(afan) oromo"; n++;
n->code="or"; n->en="Oriya"; n->fr="oriya"; n++;
n->code="pa"; n->en="Punjabi"; n->fr="pendjabi"; n++;
n->code="pl"; n->en="Polish"; n->fr="polonais"; n++;
n->code="ps"; n->en="Pashto, Pushto"; n->fr="pashto"; n++;
n->code="pt"; n->en="Portuguese"; n->fr="portugais"; n++;
n->code="qu"; n->en="Quechua"; n->fr="quichua"; n++;
n->code="rm"; n->en="Rhaeto-Romance"; n->fr="rh�to-roman"; n++;
n->code="rn"; n->en="Kirundi"; n->fr="kiroundi"; n++;
n->code="ro"; n->en="Romanian"; n->fr="roumain"; n++;
n->code="ru"; n->en="Russian"; n->fr="russe"; n++;
n->code="rw"; n->en="Kinyarwanda"; n->fr="kinyarwanda"; n++;
n->code="sa"; n->en="Sanskrit"; n->fr="sanscrit"; n++;
n->code="sd"; n->en="Sindhi"; n->fr="sindhi"; n++;
n->code="se"; n->en="Northern S�mi"; n->fr="s�mi du nord"; n++;
n->code="sg"; n->en="Sangho"; n->fr="sango"; n++;
n->code="sh"; n->en="Serbo-Croatian"; n->fr="serbo-croate"; n++;
n->code="si"; n->en="Singhalese"; n->fr="singhalais"; n++;
n->code="sk"; n->en="Slovak"; n->fr="slovaque"; n++;
n->code="sl"; n->en="Slovenian"; n->fr="slov�ne"; n++;
n->code="sm"; n->en="Samoan"; n->fr="samoan"; n++;
n->code="sn"; n->en="Shona"; n->fr="shona"; n++;
n->code="so"; n->en="Somali"; n->fr="somali"; n++;
n->code="sq"; n->en="Albanian"; n->fr="albanais"; n++;
n->code="sr"; n->en="Serbian"; n->fr="serbe"; n++;
n->code="ss"; n->en="Siswati"; n->fr="siswati"; n++;
n->code="st"; n->en="Sesotho"; n->fr="sesotho"; n++;
n->code="su"; n->en="Sundanese"; n->fr="soundanais"; n++;
n->code="sv"; n->en="Swedish"; n->fr="su�dois"; n++;
n->code="sw"; n->en="Swahili"; n->fr="swahili"; n++;
n->code="ta"; n->en="Tamil"; n->fr="tamil"; n++;
n->code="te"; n->en="Telugu"; n->fr="t�lougou"; n++;
n->code="tg"; n->en="Tajik"; n->fr="tadjik"; n++;
n->code="th"; n->en="Thai"; n->fr="thai"; n++;
n->code="ti"; n->en="Tigrinya"; n->fr="tigrinya"; n++;
n->code="tk"; n->en="Turkmen"; n->fr="turkm�ne"; n++;
n->code="tl"; n->en="Tagalog"; n->fr="tagal"; n++;
n->code="tn"; n->en="Setswana"; n->fr="setchwana"; n++;
n->code="to"; n->en="Tonga"; n->fr="tonga"; n++;
n->code="tr"; n->en="Turkish"; n->fr="turc"; n++;
n->code="ts"; n->en="Tsonga"; n->fr="tsonga"; n++;
n->code="tt"; n->en="Tatar"; n->fr="tatar"; n++;
n->code="tw"; n->en="Twi"; n->fr="tchi"; n++;
n->code="ug"; n->en="Uigur"; n->fr="ou�gour"; n++;
n->code="uk"; n->en="Ukrainian"; n->fr="ukrainien"; n++;
n->code="ur"; n->en="Urdu"; n->fr="ourdou"; n++;
n->code="uz"; n->en="Uzbek"; n->fr="ouzbek"; n++;
n->code="vi"; n->en="Vietnamese"; n->fr="vietnamien"; n++;
n->code="vo"; n->en="Volap�k"; n->fr="volap�k"; n++;
n->code="wo"; n->en="Wolof"; n->fr="ouolof"; n++;
n->code="xh"; n->en="Xhosa"; n->fr="xhosa"; n++;
n->code="yi"; n->en="Yiddish"; n->fr="yidich"; n++;
n->code="yo"; n->en="Yoruba"; n->fr="yorouba"; n++;
n->code="za"; n->en="Zhuang"; n->fr="zhuang"; n++;
n->code="zh"; n->en="Chinese"; n->fr="chinois"; n++;
n->code="zu"; n->en="Zulu"; n->fr="zoulou"; n++;
n->code=""; n++; n->en=""; n->fr="";
DONE;
}






/*
 *  Gets the language string in 2 characters
 *  Gets the language english name
 *  Gets the language french name
*/

PUBLIC(unsigned char *) OgIso639ToCode(int lang)
{
if (!NameIso639Initialized) { NameIso639Initialized=1; OgInitNameIso639(); }
if (1<=lang && lang<=DOgLangMax) return(NameIso639[lang].code);
return(NameIso639[0].code);
}


PUBLIC(unsigned char *) OgIso639ToEn(int lang)
{
if (!NameIso639Initialized) { NameIso639Initialized=1; OgInitNameIso639(); }
if (1<=lang && lang<=DOgLangMax) return(NameIso639[lang].en);
return(NameIso639[0].en);
}


PUBLIC(unsigned char *) OgIso639ToFr(int lang)
{
if (!NameIso639Initialized) { NameIso639Initialized=1; OgInitNameIso639(); }
if (1<=lang && lang<=DOgLangMax) return(NameIso639[lang].fr);
return(NameIso639[0].fr);
}

/*
 *  Gets the language encoding from the ISO 639 string
 *  Dichotomy should be done for optimization...
*/

PUBLIC(int) OgCodeToIso639(unsigned char *code)
{
int i;
if (!NameIso639Initialized) { NameIso639Initialized=1; OgInitNameIso639(); }
for (i=1; i<DOgLangMax; i++) {
  if (!memcmp(code, NameIso639[i].code, 2)) return(i);
  }
return(0);
}

/*
 *  Gets the language encoding from the ISO 639 string
 *  Dichotomy should be done for optimization...
*/

PUBLIC(int) OgEnglishToIso639(unsigned char *english)
{
int i;
if (!NameIso639Initialized) { NameIso639Initialized=1; OgInitNameIso639(); }
for (i=1; i<DOgLangMax; i++) {
  if (!strcmp(english, NameIso639[i].en)) return(i);
  }
return(0);
}

PUBLIC(char *) OgCodepageToStr(int codepage)
{
switch(codepage) {
  case DOgCodePageStd: return("std");
  case DOgCodePageANSI: return("ansi");
  case DOgCodePageUTF8: return("utf8");
  case DOgCodePageASCII: return("ascii");
  case DOgCodePageRuKOI: return("koi");
  case DOgCodePageRuISO: return("iso");
  case DOgCodePageJaJIS: return("jis");
  case DOgCodePageJaSJIS: return("sjis");
  case DOgCodePageJaEUC: return("euc");
  default : return("unknown");
  }
DONE;
}



PUBLIC(int) OgStrToCodepage(char *str)
{
if (!strcmp(str,"std")) return(DOgCodePageStd);
if (!strcmp(str,"ansi")) return(DOgCodePageANSI);
if (!strcmp(str,"utf8")) return(DOgCodePageUTF8);
if (!strcmp(str,"utf-8")) return(DOgCodePageUTF8);
if (!strcmp(str,"ascii")) return(DOgCodePageASCII);
if (!strcmp(str,"koi")) return(DOgCodePageRuKOI);
if (!strcmp(str,"iso")) return(DOgCodePageRuISO);
if (!strcmp(str,"jis")) return(DOgCodePageJaJIS);
if (!strcmp(str,"sjis")) return(DOgCodePageJaSJIS);
if (!strcmp(str,"euc")) return(DOgCodePageJaEUC);
return(DOgCodePageStd);
}




