/* 
 * Definitions of language codes according to ISO 639 
 * Copyright (c) Pertimm 1992-2002 by Patrick Constant
 * Dev : Juillet, Ao\^ut 1992, Mai 1993, Septembre 1998, October 1999
 * Dev : August 2002
 * Version 1.4
*/

#ifndef _LOGIS639ALIVE_

#include <lpcosys.h>

#define DOgLangNil  0
#define DOgLangAA   1   /* Afar - afar */
#define DOgLangAB   2   /* Abkhazian - abkhazien */
#define DOgLangAF   3   /* Afrikaans - afrikaans */
#define DOgLangAM   4   /* Amharic - amharique */
#define DOgLangAR   5   /* Arabic - arabe */
#define DOgLangAS   6   /* Assamese - assamais */
#define DOgLangAY   7   /* Aymara - aymara */
#define DOgLangAZ   8   /* Azerbaijani - azerbaïdjanais; azéri */
#define DOgLangBA   9   /* Bashkir - bashkir */
#define DOgLangBE  10   /* Byelorussian - biélorusse */
#define DOgLangBG  11   /* Bulgarian - bulgare */
#define DOgLangBH  12   /* Bihari - bihari */
#define DOgLangBI  13   /* Bislama - bislama */
#define DOgLangBN  14   /* Bengali; Bangla - bengali */
#define DOgLangBO  15   /* Tibetan - tibétain */
#define DOgLangBR  16   /* Breton - breton */
#define DOgLangCA  17   /* Catalan - catalan */
#define DOgLangCO  18   /* Corsican - corse */
#define DOgLangCS  19   /* Czech - tchèque */
#define DOgLangCY  20   /* Welsh - gallois */
#define DOgLangDA  21   /* Danish - danois */
#define DOgLangDE  22   /* German - allemand */
#define DOgLangDZ  23   /* Bhutani - bhoutani */
#define DOgLangEL  24   /* Greek - grec */
#define DOgLangEN  25   /* English - anglais */
#define DOgLangEO  26   /* Esperanto - espéranto */
#define DOgLangES  27   /* Spanish - espagnol */
#define DOgLangET  28   /* Estonian - estonien */
#define DOgLangEU  29   /* Basque - basque */
#define DOgLangFA  30   /* Persian - persan */
#define DOgLangFI  31   /* Finnish - finnois */
#define DOgLangFJ  32   /* Fiji - fidji */
#define DOgLangFO  33   /* Faroese - féroïen */
#define DOgLangFR  34   /* French - français */
#define DOgLangFY  35   /* Frisian - frison */
#define DOgLangGA  36   /* Irish - irlandais */
#define DOgLangGD  37   /* Scots Gaelic - gaélique écossais */
#define DOgLangGL  38   /* Galician - galicien */
#define DOgLangGN  39   /* Guarani - guarani */
#define DOgLangGU  40   /* Gujarati - goujrati */
#define DOgLangGV  41   /* Manx Gaelic - gaélique de l'Île de Man */
#define DOgLangHA  42   /* Hausa - haoussa */
#define DOgLangHE  43   /* Hebrew - hébreu */
#define DOgLangHI  44   /* Hindi - hindi */
#define DOgLangHR  45   /* Croatian - croate */
#define DOgLangHU  46   /* Hungarian - hongrois */
#define DOgLangHY  47   /* Armenian - arménien */
#define DOgLangIA  48   /* Interlingua - interlingua */
#define DOgLangID  49   /* Indonesian - indonésien */
#define DOgLangIE  50   /* Interlingue - interlingue */
#define DOgLangIK  51   /* Inupiak - inupiak */
#define DOgLangIS  52   /* Icelandic - islandais */
#define DOgLangIT  53   /* Italian - italien */
#define DOgLangIU  54   /* Inuktitut - inuktitut */
#define DOgLangJA  55   /* Japanese - japonais */
#define DOgLangJW  56   /* Javanese - javanais */
#define DOgLangKA  57   /* Georgian - géorgien */
#define DOgLangKK  58   /* Kazakh - kazakh */
#define DOgLangKL  59   /* Greenlandic - groenlandais */
#define DOgLangKM  60   /* Cambodian - cambodgien */
#define DOgLangKN  61   /* Kannada - kannada */
#define DOgLangKO  62   /* Korean - coréen */
#define DOgLangKS  63   /* Kashmiri - kashmiri */
#define DOgLangKU  64   /* Kurdish - kurde */
#define DOgLangKW  65   /* Cornish - cornique */
#define DOgLangKY  66   /* Kirghiz - kirghiz */
#define DOgLangLA  67   /* Latin - latin */
#define DOgLangLB  68   /* Luxemburgish - luxembourgeois */
#define DOgLangLN  69   /* Lingala - lingala */
#define DOgLangLO  70   /* Laothian - laotien */
#define DOgLangLT  71   /* Lithuanian - lituanien */
#define DOgLangLV  72   /* Latvian; Lettish - letton, lettonien */
#define DOgLangMG  73   /* Malagasy - malgache */
#define DOgLangMI  74   /* Maori - maori */
#define DOgLangMK  75   /* Macedonian - macédonien */
#define DOgLangML  76   /* Malayalam - malayalam */
#define DOgLangMN  77   /* Mongolian - mongol */
#define DOgLangMO  78   /* Moldavian - moldave */
#define DOgLangMR  79   /* Marathi - marathe */
#define DOgLangMS  80   /* Malay - malais */
#define DOgLangMT  81   /* Maltese - maltais */
#define DOgLangMY  82   /* Burmese - birman */
#define DOgLangNA  83   /* Nauru - nauri */
#define DOgLangNE  84   /* Nepali - népalais */
#define DOgLangNL  85   /* Dutch - néerlandais */
#define DOgLangNO  86   /* Norwegian - norvégien */
#define DOgLangOC  87   /* Occitan - occitan */
#define DOgLangOM  88   /* (Afan) Oromo - (afan) oromo */
#define DOgLangOR  89   /* Oriya - oriya */
#define DOgLangPA  90   /* Punjabi - pendjabi */
#define DOgLangPL  91   /* Polish - polonais */
#define DOgLangPS  92   /* Pashto, Pushto - pashto */
#define DOgLangPT  93   /* Portuguese - portugais */
#define DOgLangQU  94   /* Quechua - quichua */
#define DOgLangRM  95   /* Rhaeto-Romance - rhéto-roman */
#define DOgLangRN  96   /* Kirundi - kiroundi */
#define DOgLangRO  97   /* Romanian - roumain */
#define DOgLangRU  98   /* Russian - russe */
#define DOgLangRW  99   /* Kinyarwanda - kinyarwanda */
#define DOgLangSA 100   /* Sanskrit - sanscrit */
#define DOgLangSD 101   /* Sindhi - sindhi */
#define DOgLangSE 102   /* Northern Sámi - sámi du nord */
#define DOgLangSG 103   /* Sangho - sango */
#define DOgLangSH 104   /* Serbo-Croatian - serbo-croate */
#define DOgLangSI 105   /* Singhalese - singhalais */
#define DOgLangSK 106   /* Slovak - slovaque */
#define DOgLangSL 107   /* Slovenian - slovène */
#define DOgLangSM 108   /* Samoan - samoan */
#define DOgLangSN 109   /* Shona - shona */
#define DOgLangSO 110   /* Somali - somali */
#define DOgLangSQ 111   /* Albanian - albanais */
#define DOgLangSR 112   /* Serbian - serbe */
#define DOgLangSS 113   /* Siswati - siswati */
#define DOgLangST 114   /* Sesotho - sesotho */
#define DOgLangSU 115   /* Sundanese - soundanais */
#define DOgLangSV 116   /* Swedish - suédois */
#define DOgLangSW 117   /* Swahili - swahili */
#define DOgLangTA 118   /* Tamil - tamil */
#define DOgLangTE 119   /* Telugu - télougou */
#define DOgLangTG 120   /* Tajik - tadjik */
#define DOgLangTH 121   /* Thai - thaï */
#define DOgLangTI 122   /* Tigrinya - tigrinya */
#define DOgLangTK 123   /* Turkmen - turkmène */
#define DOgLangTL 124   /* Tagalog - tagal */
#define DOgLangTN 125   /* Setswana - setchwana */
#define DOgLangTO 126   /* Tonga - tonga */
#define DOgLangTR 127   /* Turkish - turc */
#define DOgLangTS 128   /* Tsonga - tsonga */
#define DOgLangTT 129   /* Tatar - tatar */
#define DOgLangTW 130   /* Twi - tchi */
#define DOgLangUG 131   /* Uigur - ouïgour */
#define DOgLangUK 132   /* Ukrainian - ukrainien */
#define DOgLangUR 133   /* Urdu - ourdou */
#define DOgLangUZ 134   /* Uzbek - ouzbek */
#define DOgLangVI 135   /* Vietnamese - vietnamien */
#define DOgLangVO 136   /* Volapük - volapük */
#define DOgLangWO 137   /* Wolof - ouolof */
#define DOgLangXH 138   /* Xhosa - xhosa */
#define DOgLangYI 139   /* Yiddish - yidich */
#define DOgLangYO 140   /* Yoruba - yorouba */
#define DOgLangZA 141   /* Zhuang - zhuang */
#define DOgLangZH 142   /* Chinese - chinois */
#define DOgLangZU 143   /* Zulu - zoulou */
#define DOgLangMax 144

DEFPUBLIC(unsigned char *) OgIso639ToCode(pr(int));
DEFPUBLIC(unsigned char *) OgIso639ToEn(pr(int));
DEFPUBLIC(unsigned char *) OgIso639ToFr(pr(int));
DEFPUBLIC(int) OgCodeToIso639(pr(unsigned char *));
DEFPUBLIC(int) OgEnglishToIso639(pr(unsigned char *));

#define _LOGIS639ALIVE_

#endif


