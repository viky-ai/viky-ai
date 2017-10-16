/*
 * Definitions of country codes according to ISO 3166
 *  Copyright (c) 2014 Pertimm by Patrick Constant
 *  Dev : September 2014
 * Version 1.0
*/

#ifndef _LOGIS3166ALIVE_

#include <lpcosys.h>

#define DOgCountryNil  0
#define DOgCountryAF   1 /* Afghanistan */
#define DOgCountryAX   2 /* Åland Islands */
#define DOgCountryAL   3 /* Albania */
#define DOgCountryDZ   4 /* Algeria */
#define DOgCountryAS   5 /* American Samoa */
#define DOgCountryAD   6 /* Andorra */
#define DOgCountryAO   7 /* Angola */
#define DOgCountryAI   8 /* Anguilla */
#define DOgCountryAQ   9 /* Antarctica */
#define DOgCountryAG  10 /* Antigua and Barbuda */
#define DOgCountryAR  11 /* Argentina */
#define DOgCountryAM  12 /* Armenia */
#define DOgCountryAW  13 /* Aruba */
#define DOgCountryAU  14 /* Australia */
#define DOgCountryAT  15 /* Austria */
#define DOgCountryAZ  16 /* Azerbaijan */
#define DOgCountryBS  17 /* Bahamas */
#define DOgCountryBH  18 /* Bahrain */
#define DOgCountryBD  19 /* Bangladesh */
#define DOgCountryBB  20 /* Barbados */
#define DOgCountryBY  21 /* Belarus */
#define DOgCountryBE  22 /* Belgium */
#define DOgCountryBZ  23 /* Belize */
#define DOgCountryBJ  24 /* Benin */
#define DOgCountryBM  25 /* Bermuda */
#define DOgCountryBT  26 /* Bhutan */
#define DOgCountryBO  27 /* Bolivia, Plurinational State of */
#define DOgCountryBQ  28 /* Bonaire, Sint Eustatius and Saba */
#define DOgCountryBA  29 /* Bosnia and Herzegovina */
#define DOgCountryBW  30 /* Botswana */
#define DOgCountryBV  31 /* Bouvet Island */
#define DOgCountryBR  32 /* Brazil */
#define DOgCountryIO  33 /* British Indian Ocean Territory */
#define DOgCountryBN  34 /* Brunei Darussalam */
#define DOgCountryBG  35 /* Bulgaria */
#define DOgCountryBF  36 /* Burkina Faso */
#define DOgCountryBI  37 /* Burundi */
#define DOgCountryKH  38 /* Cambodia */
#define DOgCountryCM  39 /* Cameroon */
#define DOgCountryCA  40 /* Canada */
#define DOgCountryCV  41 /* Cabo Verde */
#define DOgCountryKY  42 /* Cayman Islands */
#define DOgCountryCF  43 /* Central African Republic */
#define DOgCountryTD  44 /* Chad */
#define DOgCountryCL  45 /* Chile */
#define DOgCountryCN  46 /* China */
#define DOgCountryCX  47 /* Christmas Island */
#define DOgCountryCC  48 /* Cocos (Keeling) Islands */
#define DOgCountryCO  49 /* Colombia */
#define DOgCountryKM  50 /* Comoros */
#define DOgCountryCG  51 /* Congo */
#define DOgCountryCD  52 /* Congo, the Democratic Republic of the */
#define DOgCountryCK  53 /* Cook Islands */
#define DOgCountryCR  54 /* Costa Rica */
#define DOgCountryCI  55 /* Côte d'Ivoire */
#define DOgCountryHR  56 /* Croatia */
#define DOgCountryCU  57 /* Cuba */
#define DOgCountryCW  58 /* Curaçao */
#define DOgCountryCY  59 /* Cyprus */
#define DOgCountryCZ  60 /* Czech Republic */
#define DOgCountryDK  61 /* Denmark */
#define DOgCountryDJ  62 /* Djibouti */
#define DOgCountryDM  63 /* Dominica */
#define DOgCountryDO  64 /* Dominican Republic */
#define DOgCountryEC  65 /* Ecuador */
#define DOgCountryEG  66 /* Egypt */
#define DOgCountrySV  67 /* El Salvador */
#define DOgCountryGQ  68 /* Equatorial Guinea */
#define DOgCountryER  69 /* Eritrea */
#define DOgCountryEE  70 /* Estonia */
#define DOgCountryET  71 /* Ethiopia */
#define DOgCountryFK  72 /* Falkland Islands (Malvinas) */
#define DOgCountryFO  73 /* Faroe Islands */
#define DOgCountryFJ  74 /* Fiji */
#define DOgCountryFI  75 /* Finland */
#define DOgCountryFR  76 /* France */
#define DOgCountryGF  77 /* French Guiana */
#define DOgCountryPF  78 /* French Polynesia */
#define DOgCountryTF  79 /* French Southern Territories */
#define DOgCountryGA  80 /* Gabon */
#define DOgCountryGM  81 /* Gambia */
#define DOgCountryGE  82 /* Georgia */
#define DOgCountryDE  83 /* Germany */
#define DOgCountryGH  84 /* Ghana */
#define DOgCountryGI  85 /* Gibraltar */
#define DOgCountryGR  86 /* Greece */
#define DOgCountryGL  87 /* Greenland */
#define DOgCountryGD  88 /* Grenada */
#define DOgCountryGP  89 /* Guadeloupe */
#define DOgCountryGU  90 /* Guam */
#define DOgCountryGT  91 /* Guatemala */
#define DOgCountryGG  92 /* Guernsey */
#define DOgCountryGN  93 /* Guinea */
#define DOgCountryGW  94 /* Guinea-Bissau */
#define DOgCountryGY  95 /* Guyana */
#define DOgCountryHT  96 /* Haiti */
#define DOgCountryHM  97 /* Heard Island and McDonald Islands */
#define DOgCountryVA  98 /* Holy See (Vatican City State) */
#define DOgCountryHN  99 /* Honduras */
#define DOgCountryHK 100 /* Hong Kong */
#define DOgCountryHU 101 /* Hungary */
#define DOgCountryIS 102 /* Iceland */
#define DOgCountryIN 103 /* India */
#define DOgCountryID 104 /* Indonesia */
#define DOgCountryIR 105 /* Iran, Islamic Republic of */
#define DOgCountryIQ 106 /* Iraq */
#define DOgCountryIE 107 /* Ireland */
#define DOgCountryIM 108 /* Isle of Man */
#define DOgCountryIL 109 /* Israel */
#define DOgCountryIT 110 /* Italy */
#define DOgCountryJM 111 /* Jamaica */
#define DOgCountryJP 112 /* Japan */
#define DOgCountryJE 113 /* Jersey */
#define DOgCountryJO 114 /* Jordan */
#define DOgCountryKZ 115 /* Kazakhstan */
#define DOgCountryKE 116 /* Kenya */
#define DOgCountryKI 117 /* Kiribati */
#define DOgCountryKP 118 /* Korea, Democratic People's Republic of */
#define DOgCountryKR 119 /* Korea, Republic of */
#define DOgCountryKW 120 /* Kuwait */
#define DOgCountryKG 121 /* Kyrgyzstan */
#define DOgCountryLA 122 /* Lao People's Democratic Republic */
#define DOgCountryLV 123 /* Latvia */
#define DOgCountryLB 124 /* Lebanon */
#define DOgCountryLS 125 /* Lesotho */
#define DOgCountryLR 126 /* Liberia */
#define DOgCountryLY 127 /* Libya */
#define DOgCountryLI 128 /* Liechtenstein */
#define DOgCountryLT 129 /* Lithuania */
#define DOgCountryLU 130 /* Luxembourg */
#define DOgCountryMO 131 /* Macao */
#define DOgCountryMK 132 /* Macedonia, the former Yugoslav Republic of */
#define DOgCountryMG 133 /* Madagascar */
#define DOgCountryMW 134 /* Malawi */
#define DOgCountryMY 135 /* Malaysia */
#define DOgCountryMV 136 /* Maldives */
#define DOgCountryML 137 /* Mali */
#define DOgCountryMT 138 /* Malta */
#define DOgCountryMH 139 /* Marshall Islands */
#define DOgCountryMQ 140 /* Martinique */
#define DOgCountryMR 141 /* Mauritania */
#define DOgCountryMU 142 /* Mauritius */
#define DOgCountryYT 143 /* Mayotte */
#define DOgCountryMX 144 /* Mexico */
#define DOgCountryFM 145 /* Micronesia, Federated States of */
#define DOgCountryMD 146 /* Moldova, Republic of */
#define DOgCountryMC 147 /* Monaco */
#define DOgCountryMN 148 /* Mongolia */
#define DOgCountryME 149 /* Montenegro */
#define DOgCountryMS 150 /* Montserrat */
#define DOgCountryMA 151 /* Morocco */
#define DOgCountryMZ 152 /* Mozambique */
#define DOgCountryMM 153 /* Myanmar */
#define DOgCountryNA 154 /* Namibia */
#define DOgCountryNR 155 /* Nauru */
#define DOgCountryNP 156 /* Nepal */
#define DOgCountryNL 157 /* Netherlands */
#define DOgCountryNC 158 /* New Caledonia */
#define DOgCountryNZ 159 /* New Zealand */
#define DOgCountryNI 160 /* Nicaragua */
#define DOgCountryNE 161 /* Niger */
#define DOgCountryNG 162 /* Nigeria */
#define DOgCountryNU 163 /* Niue */
#define DOgCountryNF 164 /* Norfolk Island */
#define DOgCountryMP 165 /* Northern Mariana Islands */
#define DOgCountryNO 166 /* Norway */
#define DOgCountryOM 167 /* Oman */
#define DOgCountryPK 168 /* Pakistan */
#define DOgCountryPW 169 /* Palau */
#define DOgCountryPS 170 /* Palestine, State of */
#define DOgCountryPA 171 /* Panama */
#define DOgCountryPG 172 /* Papua New Guinea */
#define DOgCountryPY 173 /* Paraguay */
#define DOgCountryPE 174 /* Peru */
#define DOgCountryPH 175 /* Philippines */
#define DOgCountryPN 176 /* Pitcairn */
#define DOgCountryPL 177 /* Poland */
#define DOgCountryPT 178 /* Portugal */
#define DOgCountryPR 179 /* Puerto Rico */
#define DOgCountryQA 180 /* Qatar */
#define DOgCountryRE 181 /* Réunion */
#define DOgCountryRO 182 /* Romania */
#define DOgCountryRU 183 /* Russian Federation */
#define DOgCountryRW 184 /* Rwanda */
#define DOgCountryBL 185 /* Saint Barthélemy */
#define DOgCountrySH 186 /* Saint Helena, Ascension and Tristan da Cunha */
#define DOgCountryKN 187 /* Saint Kitts and Nevis */
#define DOgCountryLC 188 /* Saint Lucia */
#define DOgCountryMF 189 /* Saint Martin (French part) */
#define DOgCountryPM 190 /* Saint Pierre and Miquelon */
#define DOgCountryVC 191 /* Saint Vincent and the Grenadines */
#define DOgCountryWS 192 /* Samoa */
#define DOgCountrySM 193 /* San Marino */
#define DOgCountryST 194 /* Sao Tome and Principe */
#define DOgCountrySA 195 /* Saudi Arabia */
#define DOgCountrySN 196 /* Senegal */
#define DOgCountryRS 197 /* Serbia */
#define DOgCountrySC 198 /* Seychelles */
#define DOgCountrySL 199 /* Sierra Leone */
#define DOgCountrySG 200 /* Singapore */
#define DOgCountrySX 201 /* Sint Maarten (Dutch part) */
#define DOgCountrySK 202 /* Slovakia */
#define DOgCountrySI 203 /* Slovenia */
#define DOgCountrySB 204 /* Solomon Islands */
#define DOgCountrySO 205 /* Somalia */
#define DOgCountryZA 206 /* South Africa */
#define DOgCountryGS 207 /* South Georgia and the South Sandwich Islands */
#define DOgCountrySS 208 /* South Sudan */
#define DOgCountryES 209 /* Spain */
#define DOgCountryLK 210 /* Sri Lanka */
#define DOgCountrySD 211 /* Sudan */
#define DOgCountrySR 212 /* Suriname */
#define DOgCountrySJ 213 /* Svalbard and Jan Mayen */
#define DOgCountrySZ 214 /* Swaziland */
#define DOgCountrySE 215 /* Sweden */
#define DOgCountryCH 216 /* Switzerland */
#define DOgCountrySY 217 /* Syrian Arab Republic */
#define DOgCountryTW 218 /* Taiwan, Province of China */
#define DOgCountryTJ 219 /* Tajikistan */
#define DOgCountryTZ 220 /* Tanzania, United Republic of */
#define DOgCountryTH 221 /* Thailand */
#define DOgCountryTL 222 /* Timor-Leste */
#define DOgCountryTG 223 /* Togo */
#define DOgCountryTK 224 /* Tokelau */
#define DOgCountryTO 225 /* Tonga */
#define DOgCountryTT 226 /* Trinidad and Tobago */
#define DOgCountryTN 227 /* Tunisia */
#define DOgCountryTR 228 /* Turkey */
#define DOgCountryTM 229 /* Turkmenistan */
#define DOgCountryTC 230 /* Turks and Caicos Islands */
#define DOgCountryTV 231 /* Tuvalu */
#define DOgCountryUG 232 /* Uganda */
#define DOgCountryUA 233 /* Ukraine */
#define DOgCountryAE 234 /* United Arab Emirates */
#define DOgCountryGB 235 /* United Kingdom */
#define DOgCountryUS 236 /* United States */
#define DOgCountryUM 237 /* United States Minor Outlying Islands */
#define DOgCountryUY 238 /* Uruguay */
#define DOgCountryUZ 239 /* Uzbekistan */
#define DOgCountryVU 240 /* Vanuatu */
#define DOgCountryVE 241 /* Venezuela, Bolivarian Republic of */
#define DOgCountryVN 242 /* Viet Nam */
#define DOgCountryVG 243 /* Virgin Islands, British */
#define DOgCountryVI 244 /* Virgin Islands, U.S. */
#define DOgCountryWF 245 /* Wallis and Futuna */
#define DOgCountryEH 246 /* Western Sahara */
#define DOgCountryYE 247 /* Yemen */
#define DOgCountryZM 248 /* Zambia */
#define DOgCountryZW 249 /* Zimbabwe */

/* Code element restricts */
#define DOgCountryAC 250 /* Ascension Island */
#define DOgCountryIC 251 /* Canary Islands */
#define DOgCountryEA 252 /* Ceuta and Melilla */
#define DOgCountryCP 253 /* Clipperton Island */
#define DOgCountryDG 254 /* Diego Garcia */
#define DOgCountryEU 255 /* European Union */
#define DOgCountryEZ 256 /* International negotiable securities */
#define DOgCountryFX 257 /* Metropolitan France */
#define DOgCountrySU 258 /* Soviet Union */
#define DOgCountryTA 259 /* Tristan da Cunha */
#define DOgCountryUK 260 /* United Kingdom */
#define DOgCountryUN 261 /* United Nations */

#define DOgCountryEmpty 262 /* EMPTY */
#define DOgCountryMax   263

DEFPUBLIC(unsigned char *) OgIso3166ToCode(int country);
DEFPUBLIC(unsigned char *) OgIso3166ToEn(int country);
DEFPUBLIC(int) OgCodeToIso3166(unsigned char *code);

#define _LOGIS3166ALIVE_

#endif


