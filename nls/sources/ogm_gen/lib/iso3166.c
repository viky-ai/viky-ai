/*
 *  Generic functions for country, based on ISO 3166-1
 *  Copyright (c) 2014 Pertimm by Patrick Constant
 *  Dev : September 2014
 *  Version 1.0
*/
#include <loggen.h>
#include <logis3166.h>
#include <pthread.h>


struct iso3166 {
  unsigned char *code;
  unsigned char *en;
};


static int NameIso3166Initialized=0;
static struct iso3166 NameIso3166[DOgCountryMax];

// use pthread directly to have static init
static pthread_mutex_t NameIso3166Mutex = PTHREAD_MUTEX_INITIALIZER;

static int OgInitNameIso3166NoSync(void);

static int OgInitNameIso3166(void)
{
  // lock init access
  pthread_mutex_lock(&NameIso3166Mutex);

  int status = OgInitNameIso3166NoSync();

  pthread_mutex_unlock(&NameIso3166Mutex);

  return status;
}

static int OgInitNameIso3166NoSync(void)
{
  if (NameIso3166Initialized)
  {
    DONE;
  }

  struct iso3166 *n = NameIso3166;
  memset(n, 0, sizeof(struct iso3166) * DOgCountryMax);

  n->code="--"; n->en="Nil"; n++;
  n->code="AF"; n->en="Afghanistan"; n++;
  n->code="AX"; n->en="Åland Islands"; n++;
  n->code="AL"; n->en="Albania"; n++;
  n->code="DZ"; n->en="Algeria"; n++;
  n->code="AS"; n->en="American Samoa"; n++;
  n->code="AD"; n->en="Andorra"; n++;
  n->code="AO"; n->en="Angola"; n++;
  n->code="AI"; n->en="Anguilla"; n++;
  n->code="AQ"; n->en="Antarctica"; n++;
  n->code="AG"; n->en="Antigua and Barbuda"; n++;
  n->code="AR"; n->en="Argentina"; n++;
  n->code="AM"; n->en="Armenia"; n++;
  n->code="AW"; n->en="Aruba"; n++;
  n->code="AU"; n->en="Australia"; n++;
  n->code="AT"; n->en="Austria"; n++;
  n->code="AZ"; n->en="Azerbaijan"; n++;
  n->code="BS"; n->en="Bahamas"; n++;
  n->code="BH"; n->en="Bahrain"; n++;
  n->code="BD"; n->en="Bangladesh"; n++;
  n->code="BB"; n->en="Barbados"; n++;
  n->code="BY"; n->en="Belarus"; n++;
  n->code="BE"; n->en="Belgium"; n++;
  n->code="BZ"; n->en="Belize"; n++;
  n->code="BJ"; n->en="Benin"; n++;
  n->code="BM"; n->en="Bermuda"; n++;
  n->code="BT"; n->en="Bhutan"; n++;
  n->code="BO"; n->en="Bolivia, Plurinational State of"; n++;
  n->code="BQ"; n->en="Bonaire, Sint Eustatius and Saba"; n++;
  n->code="BA"; n->en="Bosnia and Herzegovina"; n++;
  n->code="BW"; n->en="Botswana"; n++;
  n->code="BV"; n->en="Bouvet Island"; n++;
  n->code="BR"; n->en="Brazil"; n++;
  n->code="IO"; n->en="British Indian Ocean Territory"; n++;
  n->code="BN"; n->en="Brunei Darussalam"; n++;
  n->code="BG"; n->en="Bulgaria"; n++;
  n->code="BF"; n->en="Burkina Faso"; n++;
  n->code="BI"; n->en="Burundi"; n++;
  n->code="KH"; n->en="Cambodia"; n++;
  n->code="CM"; n->en="Cameroon"; n++;
  n->code="CA"; n->en="Canada"; n++;
  n->code="CV"; n->en="Cabo Verde"; n++;
  n->code="KY"; n->en="Cayman Islands"; n++;
  n->code="CF"; n->en="Central African Republic"; n++;
  n->code="TD"; n->en="Chad"; n++;
  n->code="CL"; n->en="Chile"; n++;
  n->code="CN"; n->en="China"; n++;
  n->code="CX"; n->en="Christmas Island"; n++;
  n->code="CC"; n->en="Cocos (Keeling) Islands"; n++;
  n->code="CO"; n->en="Colombia"; n++;
  n->code="KM"; n->en="Comoros"; n++;
  n->code="CG"; n->en="Congo"; n++;
  n->code="CD"; n->en="Congo, the Democratic Republic of the"; n++;
  n->code="CK"; n->en="Cook Islands"; n++;
  n->code="CR"; n->en="Costa Rica"; n++;
  n->code="CI"; n->en="Côte d'Ivoire"; n++;
  n->code="HR"; n->en="Croatia"; n++;
  n->code="CU"; n->en="Cuba"; n++;
  n->code="CW"; n->en="Curaçao"; n++;
  n->code="CY"; n->en="Cyprus"; n++;
  n->code="CZ"; n->en="Czech Republic"; n++;
  n->code="DK"; n->en="Denmark"; n++;
  n->code="DJ"; n->en="Djibouti"; n++;
  n->code="DM"; n->en="Dominica"; n++;
  n->code="DO"; n->en="Dominican Republic"; n++;
  n->code="EC"; n->en="Ecuador"; n++;
  n->code="EG"; n->en="Egypt"; n++;
  n->code="SV"; n->en="El Salvador"; n++;
  n->code="GQ"; n->en="Equatorial Guinea"; n++;
  n->code="ER"; n->en="Eritrea"; n++;
  n->code="EE"; n->en="Estonia"; n++;
  n->code="ET"; n->en="Ethiopia"; n++;
  n->code="FK"; n->en="Falkland Islands (Malvinas)"; n++;
  n->code="FO"; n->en="Faroe Islands"; n++;
  n->code="FJ"; n->en="Fiji"; n++;
  n->code="FI"; n->en="Finland"; n++;
  n->code="FR"; n->en="France"; n++;
  n->code="GF"; n->en="French Guiana"; n++;
  n->code="PF"; n->en="French Polynesia"; n++;
  n->code="TF"; n->en="French Southern Territories"; n++;
  n->code="GA"; n->en="Gabon"; n++;
  n->code="GM"; n->en="Gambia"; n++;
  n->code="GE"; n->en="Georgia"; n++;
  n->code="DE"; n->en="Germany"; n++;
  n->code="GH"; n->en="Ghana"; n++;
  n->code="GI"; n->en="Gibraltar"; n++;
  n->code="GR"; n->en="Greece"; n++;
  n->code="GL"; n->en="Greenland"; n++;
  n->code="GD"; n->en="Grenada"; n++;
  n->code="GP"; n->en="Guadeloupe"; n++;
  n->code="GU"; n->en="Guam"; n++;
  n->code="GT"; n->en="Guatemala"; n++;
  n->code="GG"; n->en="Guernsey"; n++;
  n->code="GN"; n->en="Guinea"; n++;
  n->code="GW"; n->en="Guinea-Bissau"; n++;
  n->code="GY"; n->en="Guyana"; n++;
  n->code="HT"; n->en="Haiti"; n++;
  n->code="HM"; n->en="Heard Island and McDonald Islands"; n++;
  n->code="VA"; n->en="Holy See (Vatican City State)"; n++;
  n->code="HN"; n->en="Honduras"; n++;
  n->code="HK"; n->en="Hong Kong"; n++;
  n->code="HU"; n->en="Hungary"; n++;
  n->code="IS"; n->en="Iceland"; n++;
  n->code="IN"; n->en="India"; n++;
  n->code="ID"; n->en="Indonesia"; n++;
  n->code="IR"; n->en="Iran, Islamic Republic of"; n++;
  n->code="IQ"; n->en="Iraq"; n++;
  n->code="IE"; n->en="Ireland"; n++;
  n->code="IM"; n->en="Isle of Man"; n++;
  n->code="IL"; n->en="Israel"; n++;
  n->code="IT"; n->en="Italy"; n++;
  n->code="JM"; n->en="Jamaica"; n++;
  n->code="JP"; n->en="Japan"; n++;
  n->code="JE"; n->en="Jersey"; n++;
  n->code="JO"; n->en="Jordan"; n++;
  n->code="KZ"; n->en="Kazakhstan"; n++;
  n->code="KE"; n->en="Kenya"; n++;
  n->code="KI"; n->en="Kiribati"; n++;
  n->code="KP"; n->en="Korea, Democratic People's Republic of"; n++;
  n->code="KR"; n->en="Korea, Republic of"; n++;
  n->code="KW"; n->en="Kuwait"; n++;
  n->code="KG"; n->en="Kyrgyzstan"; n++;
  n->code="LA"; n->en="Lao People's Democratic Republic"; n++;
  n->code="LV"; n->en="Latvia"; n++;
  n->code="LB"; n->en="Lebanon"; n++;
  n->code="LS"; n->en="Lesotho"; n++;
  n->code="LR"; n->en="Liberia"; n++;
  n->code="LY"; n->en="Libya"; n++;
  n->code="LI"; n->en="Liechtenstein"; n++;
  n->code="LT"; n->en="Lithuania"; n++;
  n->code="LU"; n->en="Luxembourg"; n++;
  n->code="MO"; n->en="Macao"; n++;
  n->code="MK"; n->en="Macedonia, the former Yugoslav Republic of"; n++;
  n->code="MG"; n->en="Madagascar"; n++;
  n->code="MW"; n->en="Malawi"; n++;
  n->code="MY"; n->en="Malaysia"; n++;
  n->code="MV"; n->en="Maldives"; n++;
  n->code="ML"; n->en="Mali"; n++;
  n->code="MT"; n->en="Malta"; n++;
  n->code="MH"; n->en="Marshall Islands"; n++;
  n->code="MQ"; n->en="Martinique"; n++;
  n->code="MR"; n->en="Mauritania"; n++;
  n->code="MU"; n->en="Mauritius"; n++;
  n->code="YT"; n->en="Mayotte"; n++;
  n->code="MX"; n->en="Mexico"; n++;
  n->code="FM"; n->en="Micronesia, Federated States of"; n++;
  n->code="MD"; n->en="Moldova, Republic of"; n++;
  n->code="MC"; n->en="Monaco"; n++;
  n->code="MN"; n->en="Mongolia"; n++;
  n->code="ME"; n->en="Montenegro"; n++;
  n->code="MS"; n->en="Montserrat"; n++;
  n->code="MA"; n->en="Morocco"; n++;
  n->code="MZ"; n->en="Mozambique"; n++;
  n->code="MM"; n->en="Myanmar"; n++;
  n->code="NA"; n->en="Namibia"; n++;
  n->code="NR"; n->en="Nauru"; n++;
  n->code="NP"; n->en="Nepal"; n++;
  n->code="NL"; n->en="Netherlands"; n++;
  n->code="NC"; n->en="New Caledonia"; n++;
  n->code="NZ"; n->en="New Zealand"; n++;
  n->code="NI"; n->en="Nicaragua"; n++;
  n->code="NE"; n->en="Niger"; n++;
  n->code="NG"; n->en="Nigeria"; n++;
  n->code="NU"; n->en="Niue"; n++;
  n->code="NF"; n->en="Norfolk Island"; n++;
  n->code="MP"; n->en="Northern Mariana Islands"; n++;
  n->code="NO"; n->en="Norway"; n++;
  n->code="OM"; n->en="Oman"; n++;
  n->code="PK"; n->en="Pakistan"; n++;
  n->code="PW"; n->en="Palau"; n++;
  n->code="PS"; n->en="Palestine, State of"; n++;
  n->code="PA"; n->en="Panama"; n++;
  n->code="PG"; n->en="Papua New Guinea"; n++;
  n->code="PY"; n->en="Paraguay"; n++;
  n->code="PE"; n->en="Peru"; n++;
  n->code="PH"; n->en="Philippines"; n++;
  n->code="PN"; n->en="Pitcairn"; n++;
  n->code="PL"; n->en="Poland"; n++;
  n->code="PT"; n->en="Portugal"; n++;
  n->code="PR"; n->en="Puerto Rico"; n++;
  n->code="QA"; n->en="Qatar"; n++;
  n->code="RE"; n->en="Réunion"; n++;
  n->code="RO"; n->en="Romania"; n++;
  n->code="RU"; n->en="Russian Federation"; n++;
  n->code="RW"; n->en="Rwanda"; n++;
  n->code="BL"; n->en="Saint Barthélemy"; n++;
  n->code="SH"; n->en="Saint Helena, Ascension and Tristan da Cunha"; n++;
  n->code="KN"; n->en="Saint Kitts and Nevis"; n++;
  n->code="LC"; n->en="Saint Lucia"; n++;
  n->code="MF"; n->en="Saint Martin (French part)"; n++;
  n->code="PM"; n->en="Saint Pierre and Miquelon"; n++;
  n->code="VC"; n->en="Saint Vincent and the Grenadines"; n++;
  n->code="WS"; n->en="Samoa"; n++;
  n->code="SM"; n->en="San Marino"; n++;
  n->code="ST"; n->en="Sao Tome and Principe"; n++;
  n->code="SA"; n->en="Saudi Arabia"; n++;
  n->code="SN"; n->en="Senegal"; n++;
  n->code="RS"; n->en="Serbia"; n++;
  n->code="SC"; n->en="Seychelles"; n++;
  n->code="SL"; n->en="Sierra Leone"; n++;
  n->code="SG"; n->en="Singapore"; n++;
  n->code="SX"; n->en="Sint Maarten (Dutch part)"; n++;
  n->code="SK"; n->en="Slovakia"; n++;
  n->code="SI"; n->en="Slovenia"; n++;
  n->code="SB"; n->en="Solomon Islands"; n++;
  n->code="SO"; n->en="Somalia"; n++;
  n->code="ZA"; n->en="South Africa"; n++;
  n->code="GS"; n->en="South Georgia and the South Sandwich Islands"; n++;
  n->code="SS"; n->en="South Sudan"; n++;
  n->code="ES"; n->en="Spain"; n++;
  n->code="LK"; n->en="Sri Lanka"; n++;
  n->code="SD"; n->en="Sudan"; n++;
  n->code="SR"; n->en="Suriname"; n++;
  n->code="SJ"; n->en="Svalbard and Jan Mayen"; n++;
  n->code="SZ"; n->en="Swaziland"; n++;
  n->code="SE"; n->en="Sweden"; n++;
  n->code="CH"; n->en="Switzerland"; n++;
  n->code="SY"; n->en="Syrian Arab Republic"; n++;
  n->code="TW"; n->en="Taiwan, Province of China"; n++;
  n->code="TJ"; n->en="Tajikistan"; n++;
  n->code="TZ"; n->en="Tanzania, United Republic of"; n++;
  n->code="TH"; n->en="Thailand"; n++;
  n->code="TL"; n->en="Timor-Leste"; n++;
  n->code="TG"; n->en="Togo"; n++;
  n->code="TK"; n->en="Tokelau"; n++;
  n->code="TO"; n->en="Tonga"; n++;
  n->code="TT"; n->en="Trinidad and Tobago"; n++;
  n->code="TN"; n->en="Tunisia"; n++;
  n->code="TR"; n->en="Turkey"; n++;
  n->code="TM"; n->en="Turkmenistan"; n++;
  n->code="TC"; n->en="Turks and Caicos Islands"; n++;
  n->code="TV"; n->en="Tuvalu"; n++;
  n->code="UG"; n->en="Uganda"; n++;
  n->code="UA"; n->en="Ukraine"; n++;
  n->code="AE"; n->en="United Arab Emirates"; n++;
  n->code="GB"; n->en="United Kingdom"; n++;
  n->code="US"; n->en="United States"; n++;
  n->code="UM"; n->en="United States Minor Outlying Islands"; n++;
  n->code="UY"; n->en="Uruguay"; n++;
  n->code="UZ"; n->en="Uzbekistan"; n++;
  n->code="VU"; n->en="Vanuatu"; n++;
  n->code="VE"; n->en="Venezuela, Bolivarian Republic of"; n++;
  n->code="VN"; n->en="Viet Nam"; n++;
  n->code="VG"; n->en="Virgin Islands, British"; n++;
  n->code="VI"; n->en="Virgin Islands, U.S."; n++;
  n->code="WF"; n->en="Wallis and Futuna"; n++;
  n->code="EH"; n->en="Western Sahara"; n++;
  n->code="YE"; n->en="Yemen"; n++;
  n->code="ZM"; n->en="Zambia"; n++;
  n->code="ZW"; n->en="Zimbabwe"; n++;

  /* Tag restricts */
  n->code="AC"; n->en="Ascension Island"; n++;
  n->code="IC"; n->en="Canary Islands"; n++;
  n->code="EA"; n->en="Ceuta and Melilla"; n++;
  n->code="CP"; n->en="Clipperton Island"; n++;
  n->code="DG"; n->en="Diego Garcia"; n++;
  n->code="EU"; n->en="European Union"; n++;
  n->code="EZ"; n->en="International negotiable securities"; n++;
  n->code="FX"; n->en="Metropolitan France"; n++;
  n->code="SU"; n->en="Soviet Union"; n++;
  n->code="TA"; n->en="Tristan da Cunha"; n++;
  n->code="UK"; n->en="United Kingdom"; n++;
  n->code="UN"; n->en="United Nations"; n++;

  n->code=""; n->en="";

  NameIso3166Initialized = 1;

  DONE;
}






/*
 *  Gets the country string in 2 characters
 *  Gets the country english name
 */

PUBLIC(unsigned char *) OgIso3166ToCode(int country)
{
  if (!NameIso3166Initialized)
  {
    OgInitNameIso3166();
  }

  if (1 <= country && country < DOgCountryMax)
  {
    return(NameIso3166[country].code);
  }

  return(NameIso3166[0].code);
}

PUBLIC(unsigned char *) OgIso3166ToEn(int country)
{
  if (!NameIso3166Initialized)
  {
    OgInitNameIso3166();
  }

  if (1 <= country && country < DOgCountryMax)
  {
    return(NameIso3166[country].en);
  }

  return(NameIso3166[0].en);
}

/*
 *  Gets the country encoding from the ISO 3166 string
 *  Dichotomy should be done for optimization...
 */

PUBLIC(int) OgCodeToIso3166(unsigned char *code)
{
  if (!NameIso3166Initialized)
  {
    OgInitNameIso3166();
  }

  for (int i = 1; i < DOgCountryMax; i++)
  {
    if (!Ogstricmp(code, NameIso3166[i].code)) return (i);
  }

  DONE;
}

