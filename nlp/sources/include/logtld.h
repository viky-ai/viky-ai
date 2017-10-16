/* 
 * Top level domains for Internet per www.iana.org
 * Copyright (c) Pertimm 2004 by Patrick Constant
 * Dev : March 2004
 * Version 1.0
*/

#ifndef _LOGTLDALIVE_

#include <lpcosys.h>

struct og_tld {
  int len; char *ch;
  short code,generic;
  char *name;
  };

#define DOgTldnil	0
#define DOgTldaero	1 /* air-transport industry */
#define DOgTldbiz	2 /* businesses */
#define DOgTldcom	3 /* most common */
#define DOgTldcoop	4 /* cooperative associations */
#define DOgTldinfo	5 /* unspecified */
#define DOgTldmuseum	6 /* museums */
#define DOgTldname	7 /* individuals */
#define DOgTldnet	8 /* internet */
#define DOgTldorg	9 /* noncommercial community, but all are eligible to register within .org */
#define DOgTldpro	10 /* credentialed professionals and related entities */
#define DOgTldgov	11 /* United States Government */
#define DOgTldedu	12 /* U.S. Department of Education's list of Nationally Recognized Accrediting Agencies */
#define DOgTldmil	13 /* United States Military */
#define DOgTldint	14 /* established by international treaties between governments */
#define DOgTldgMax	15

#define DOgTldMin 30
#define DOgTldac  30 /* Ascension Island */
#define DOgTldad  31 /* Andorra */
#define DOgTldae  32 /* United Arab Emirates */
#define DOgTldaf  33 /* Afghanistan */
#define DOgTldag  34 /* Antigua and Barbuda */
#define DOgTldai  35 /* Anguilla */
#define DOgTldal  36 /* Albania */
#define DOgTldam  37 /* Armenia */
#define DOgTldan  38 /* Netherlands Antilles */
#define DOgTldao  39 /* Angola */
#define DOgTldaq  40 /* Antarctica */
#define DOgTldar  41 /* Argentina */
#define DOgTldas  42 /* American Samoa */
#define DOgTldat  43 /* Austria */
#define DOgTldau  44 /* Australia */
#define DOgTldaw  45 /* Aruba */
#define DOgTldaz  46 /* Azerbaijan */
#define DOgTldba  47 /* Bosnia and Herzegovina */
#define DOgTldbb  48 /* Barbados */
#define DOgTldbd  49 /* Bangladesh */
#define DOgTldbe  50 /* Belgium */
#define DOgTldbf  51 /* Burkina Faso */
#define DOgTldbg  52 /* Bulgaria */
#define DOgTldbh  53 /* Bahrain */
#define DOgTldbi  54 /* Burundi */
#define DOgTldbj  55 /* Benin */
#define DOgTldbm  56 /* Bermuda */
#define DOgTldbn  57 /* Brunei Darussalam */
#define DOgTldbo  58 /* Bolivia */
#define DOgTldbr  59 /* Brazil */
#define DOgTldbs  60 /* Bahamas */
#define DOgTldbt  61 /* Bhutan */
#define DOgTldbv  62 /* Bouvet Island */
#define DOgTldbw  63 /* Botswana */
#define DOgTldby  64 /* Belarus */
#define DOgTldbz  65 /* Belize */
#define DOgTldca  66 /* Canada */
#define DOgTldcc  67 /* Cocos (Keeling) Islands */
#define DOgTldcd  68 /* Congo, Democratic Republic of the */
#define DOgTldcf  69 /* Central African Republic */
#define DOgTldcg  70 /* Congo, Republic of */
#define DOgTldch  71 /* Switzerland */
#define DOgTldci  72 /* Cote d'Ivoire */
#define DOgTldck  73 /* Cook Islands */
#define DOgTldcl  74 /* Chile */
#define DOgTldcm  75 /* Cameroon */
#define DOgTldcn  76 /* China */
#define DOgTldco  77 /* Colombia */
#define DOgTldcr  78 /* Costa Rica */
#define DOgTldcu  79 /* Cuba */
#define DOgTldcv  80 /* Cap Verde */
#define DOgTldcx  81 /* Christmas Island */
#define DOgTldcy  82 /* Cyprus */
#define DOgTldcz  83 /* Czech Republic */
#define DOgTldde  84 /* Germany */
#define DOgTlddj  85 /* Djibouti */
#define DOgTlddk  86 /* Denmark */
#define DOgTlddm  87 /* Dominica */
#define DOgTlddo  88 /* Dominican Republic */
#define DOgTlddz  89 /* Algeria */
#define DOgTldec  90 /* Ecuador */
#define DOgTldee  91 /* Estonia */
#define DOgTldeg  92 /* Egypt */
#define DOgTldeh  93 /* Western Sahara */
#define DOgTlder  94 /* Eritrea */
#define DOgTldes  95 /* Spain */
#define DOgTldet  96 /* Ethiopia */
#define DOgTldfi  97 /* Finland */
#define DOgTldfj  98 /* Fiji */
#define DOgTldfk  99 /* Falkland Islands (Malvina) */
#define DOgTldfm 100 /* Micronesia, Federal State of */
#define DOgTldfo 101 /* Faroe Islands */
#define DOgTldfr 102 /* France */
#define DOgTldga 103 /* Gabon */
#define DOgTldgd 104 /* Grenada */
#define DOgTldge 105 /* Georgia */
#define DOgTldgf 106 /* French Guiana */
#define DOgTldgg 107 /* Guernsey */
#define DOgTldgh 108 /* Ghana */
#define DOgTldgi 109 /* Gibraltar */
#define DOgTldgl 110 /* Greenland */
#define DOgTldgm 111 /* Gambia */
#define DOgTldgn 112 /* Guinea */
#define DOgTldgp 113 /* Guadeloupe */
#define DOgTldgq 114 /* Equatorial Guinea */
#define DOgTldgr 115 /* Greece */
#define DOgTldgs 116 /* South Georgia and the South Sandwich Islands */
#define DOgTldgt 117 /* Guatemala */
#define DOgTldgu 118 /* Guam */
#define DOgTldgw 119 /* Guinea-Bissau */
#define DOgTldgy 120 /* Guyana */
#define DOgTldhk 121 /* Hong Kong */
#define DOgTldhm 122 /* Heard and McDonald Islands */
#define DOgTldhn 123 /* Honduras */
#define DOgTldhr 124 /* Croatia/Hrvatska */
#define DOgTldht 125 /* Haiti */
#define DOgTldhu 126 /* Hungary */
#define DOgTldid 127 /* Indonesia */
#define DOgTldie 128 /* Ireland */
#define DOgTldil 129 /* Israel */
#define DOgTldim 130 /* Isle of Man */
#define DOgTldin 131 /* India */
#define DOgTldio 132 /* British Indian Ocean Territory */
#define DOgTldiq 133 /* Iraq */
#define DOgTldir 134 /* Iran (Islamic Republic of) */
#define DOgTldis 135 /* Iceland */
#define DOgTldit 136 /* Italy */
#define DOgTldje 137 /* Jersey */
#define DOgTldjm 138 /* Jamaica */
#define DOgTldjo 139 /* Jordan */
#define DOgTldjp 140 /* Japan */
#define DOgTldke 141 /* Kenya */
#define DOgTldkg 142 /* Kyrgyzstan */
#define DOgTldkh 143 /* Cambodia */
#define DOgTldki 144 /* Kiribati */
#define DOgTldkm 145 /* Comoros */
#define DOgTldkn 146 /* Saint Kitts and Nevis */
#define DOgTldkp 147 /* Korea, Democratic People's Republic */
#define DOgTldkr 148 /* Korea, Republic of */
#define DOgTldkw 149 /* Kuwait */
#define DOgTldky 150 /* Cayman Islands */
#define DOgTldkz 151 /* Kazakhstan */
#define DOgTldla 152 /* Lao People's Democratic Republic */
#define DOgTldlb 153 /* Lebanon */
#define DOgTldlc 154 /* Saint Lucia */
#define DOgTldli 155 /* Liechtenstein */
#define DOgTldlk 156 /* Sri Lanka */
#define DOgTldlr 157 /* Liberia */
#define DOgTldls 158 /* Lesotho */
#define DOgTldlt 159 /* Lithuania */
#define DOgTldlu 160 /* Luxembourg */
#define DOgTldlv 161 /* Latvia */
#define DOgTldly 162 /* Libyan Arab Jamahiriya */
#define DOgTldma 163 /* Morocco */
#define DOgTldmc 164 /* Monaco */
#define DOgTldmd 165 /* Moldova, Republic of */
#define DOgTldmg 166 /* Madagascar */
#define DOgTldmh 167 /* Marshall Islands */
#define DOgTldmk 168 /* Macedonia, Former Yugoslav Republic */
#define DOgTldml 169 /* Mali */
#define DOgTldmm 170 /* Myanmar */
#define DOgTldmn 171 /* Mongolia */
#define DOgTldmo 172 /* Macau */
#define DOgTldmp 173 /* Northern Mariana Islands */
#define DOgTldmq 174 /* Martinique */
#define DOgTldmr 175 /* Mauritania */
#define DOgTldms 176 /* Montserrat */
#define DOgTldmt 177 /* Malta */
#define DOgTldmu 178 /* Mauritius */
#define DOgTldmv 179 /* Maldives */
#define DOgTldmw 180 /* Malawi */
#define DOgTldmx 181 /* Mexico */
#define DOgTldmy 182 /* Malaysia */
#define DOgTldmz 183 /* Mozambique */
#define DOgTldna 184 /* Namibia */
#define DOgTldnc 185 /* New Caledonia */
#define DOgTldne 186 /* Niger */
#define DOgTldnf 187 /* Norfolk Island */
#define DOgTldng 188 /* Nigeria */
#define DOgTldni 189 /* Nicaragua */
#define DOgTldnl 190 /* Netherlands */
#define DOgTldno 191 /* Norway */
#define DOgTldnp 192 /* Nepal */
#define DOgTldnr 193 /* Nauru */
#define DOgTldnu 194 /* Niue */
#define DOgTldnz 195 /* New Zealand */
#define DOgTldom 196 /* Oman */
#define DOgTldpa 197 /* Panama */
#define DOgTldpe 198 /* Peru */
#define DOgTldpf 199 /* French Polynesia */
#define DOgTldpg 200 /* Papua New Guinea */
#define DOgTldph 201 /* Philippines */
#define DOgTldpk 202 /* Pakistan */
#define DOgTldpl 203 /* Poland */
#define DOgTldpm 204 /* St. Pierre and Miquelon */
#define DOgTldpn 205 /* Pitcairn Island */
#define DOgTldpr 206 /* Puerto Rico */
#define DOgTldps 207 /* Palestinian Territories */
#define DOgTldpt 208 /* Portugal */
#define DOgTldpw 209 /* Palau */
#define DOgTldpy 210 /* Paraguay */
#define DOgTldqa 211 /* Qatar */
#define DOgTldre 212 /* Reunion Island */
#define DOgTldro 213 /* Romania */
#define DOgTldru 214 /* Russian Federation */
#define DOgTldrw 215 /* Rwanda */
#define DOgTldsa 216 /* Saudi Arabia */
#define DOgTldsb 217 /* Solomon Islands */
#define DOgTldsc 218 /* Seychelles */
#define DOgTldsd 219 /* Sudan */
#define DOgTldse 220 /* Sweden */
#define DOgTldsg 221 /* Singapore */
#define DOgTldsh 222 /* St. Helena */
#define DOgTldsi 223 /* Slovenia */
#define DOgTldsj 224 /* Svalbard and Jan Mayen Islands */
#define DOgTldsk 225 /* Slovak Republic */
#define DOgTldsl 226 /* Sierra Leone */
#define DOgTldsm 227 /* San Marino */
#define DOgTldsn 228 /* Senegal */
#define DOgTldso 229 /* Somalia */
#define DOgTldsr 230 /* Suriname */
#define DOgTldst 231 /* Sao Tome and Principe */
#define DOgTldsv 232 /* El Salvador */
#define DOgTldsy 233 /* Syrian Arab Republic */
#define DOgTldsz 234 /* Swaziland */
#define DOgTldtc 235 /* Turks and Caicos Islands */
#define DOgTldtd 236 /* Chad */
#define DOgTldtf 237 /* French Southern Territories */
#define DOgTldtg 238 /* Togo */
#define DOgTldth 239 /* Thailand */
#define DOgTldtj 240 /* Tajikistan */
#define DOgTldtk 241 /* Tokelau */
#define DOgTldtm 242 /* Turkmenistan */
#define DOgTldtn 243 /* Tunisia */
#define DOgTldto 244 /* Tonga */
#define DOgTldtp 245 /* East Timor */
#define DOgTldtr 246 /* Turkey */
#define DOgTldtt 247 /* Trinidad and Tobago */
#define DOgTldtv 248 /* Tuvalu */
#define DOgTldtw 249 /* Taiwan */
#define DOgTldtz 250 /* Tanzania */
#define DOgTldua 251 /* Ukraine */
#define DOgTldug 252 /* Uganda */
#define DOgTlduk 253 /* United Kingdom */
#define DOgTldum 254 /* US Minor Outlying Islands */
#define DOgTldus 255 /* United States */
#define DOgTlduy 256 /* Uruguay */
#define DOgTlduz 257 /* Uzbekistan */
#define DOgTldva 258 /* Holy See (City Vatican State) */
#define DOgTldvc 259 /* Saint Vincent and the Grenadines */
#define DOgTldve 260 /* Venezuela */
#define DOgTldvg 261 /* Virgin Islands (British) */
#define DOgTldvi 262 /* Virgin Islands (USA) */
#define DOgTldvn 263 /* Vietnam */
#define DOgTldvu 264 /* Vanuatu */
#define DOgTldwf 265 /* Wallis and Futuna Islands */
#define DOgTldws 266 /* Western Samoa */
#define DOgTldye 267 /* Yemen */
#define DOgTldyt 268 /* Mayotte */
#define DOgTldyu 269 /* Yugoslavia */
#define DOgTldza 270 /* South Africa */
#define DOgTldzm 271 /* Zambia */
#define DOgTldzw 272 /* Zimbabwe */


DEFPUBLIC(int) OgTldToInfo(pr_(char *) pr(struct og_tld **));
                 
#define _LOGTLDALIVE_
                 
#endif           


