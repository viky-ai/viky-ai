/*
 *  This is include for prefix and postfix value
 *  to control type of words in file index
 *  Copyright (c) 1997-2006	Pertimm, Inc. by Patrick Constant
 *  Dev: October 1997, January,October 1998, September, December 1999
 *  Dev: January, February,June,July,September,November 2000
 *  Dev: February,September 2001, January,August,September,October,November 2002
 *  Dev: March 2003, October, December 2003, April,May,July,October 2004
 *  Dev: January,February,March,May,July,October 2005, February,June 2006
 *  Dev: June 2008
 *  Version 4.4
*/

#ifndef _LOGFIXALIVE_


/** First Postfix for controling substitution from Java applet **/
#define DOgFixSubstit   0x2

/** Second Postfix for controling substitution from Java applet **/
#define DOgFixKeepAccentuation  0x11
#define DOgFixKeepSmallLetter   0x12
#define DOgFixKeepLigature      0x13
#define DOgFixKeepCapital       0x14
#define DOgFixKeepAll           0x15

/** First Postfix for operating system data **/
#define DOgFixOperatingSystem	0x3

/** Second Postfix for operating system data **/
#define DOgFixDriveId       0x11
#define DOgFixDriveType     0x12
#define DOgFixDirName       0x13
#define DOgFixDirExtension  0x14
#define DOgFixFileName      0x15
#define DOgFixFileExtension 0x16
#define DOgFixCreationDate  0x17
#define DOgFixLastAccess    0x18
#define DOgFixLastUpdate    0x19
#define DOgFixFileSize      0x1a
#define DOgFixTextSize      0x1b
#define DOgFixFileAttribute 0x1c
#define DOgFixFileFormat    0x1d
#define DOgFixFullFileName  0x1e
#define DOgFixFileLanguage  0x1f
/*  We don't use 0x20 because of some (minor) interference
 *  because this is the white character. */
#define DOgFixBundleType    0x21
#define DOgFixCodePage      0x22
#define DOgFixHyperLink     0x23
#define DOgFixUserLink      0x24
#define DOgFixDirNamePart   0x25
#define DOgFixFileNamePart  0x26
#define DOgFixOriginalName  0x27
#define DOgFixPathName      0x28 /** DOgFixDirName or DOgFixFileName **/
#define DOgFixPathNamePart  0x29 /** DOgFixDirNamePart or DOgFixFileNamePart **/


/* Bit value that says that no postfix is accepted. When the request
 * does not specify any postfix, then everything is accepted.
 * When the request specifies some postfix, then only the specified
 * bit-coded postfix is accepted. Thus the need to specify a none
 * postfix that works when we want to have some postfix but also 
 * no postfix. For example a request with Postfix.Name = DOgFixLinguistic
 * and Postfix.Selection = DOgBitRoot will get only the root words
 * (i.e. pointing to forms). The same request with 
 * Postfix.Selection = DOgBitRoot + DOgBitNone will get roots words
 * but also words with no postfix.
*/
#define DOgBitNone              0x80000000


/** Equivalent bit value for second Postfix for operating system data **/
#define DOgBitDriveId       0x1
#define DOgBitDriveType     0x2
#define DOgBitDirName       0x4
#define DOgBitDirExtension  0x8
#define DOgBitFileName      0x10
#define DOgBitFileExtension 0x20
#define DOgBitCreationDate  0x40
#define DOgBitLastAccess    0x80
#define DOgBitLastUpdate    0x100
#define DOgBitFileSize      0x200
#define DOgBitTextSize      0x400
#define DOgBitFileAttribute 0x800
#define DOgBitFileFormat    0x1000
#define DOgBitFullFileName  0x2000
#define DOgBitFileLanguage  0x4000
#define DOgBitBundleType    0x8000
#define DOgBitCodePage      0x10000
#define DOgBitHyperLink     0x20000
#define DOgBitUserLink      0x40000
#define DOgBitDirNamePart   0x80000
#define DOgBitFileNamePart  0x100000
#define DOgBitOriginalName  0x200000


/** First Postfix for linguistic data **/
#define DOgFixLinguistic        0x4

/** Second Postfix for linguistic data **/
#define DOgFixRoot              0x11
#define DOgFixFamily            0x12
#define DOgFixSynonym           0x13
#define DOgFixConcept           0x14
#define DOgFixSynonymName       0x15
#define DOgFixSynonymElement    0x16
#define DOgFixSynonymEquation   0x17
#define DOgFixConceptName       0x18
#define DOgFixConceptElement    0x19
/* We don't use 0x20 because of some (minor) interference
 * because this is the white character. */
#define DOgFixTranslation       0x21
#define DOgFixConceptWord       0x22
#define DOgFixForm              0x23
#define DOgFixEntity            0x24
#define DOgFixEntityName        0x25
#define DOgFixEntityElement     0x26
#define DOgFixRegular           0x27

/** Equivalent bit value for second Postfix for linguistic data **/
#define DOgBitRoot              0x1
#define DOgBitFamily            0x2
#define DOgBitSynonym           0x4
#define DOgBitConcept           0x8
#define DOgBitSynonymName       0x10
#define DOgBitSynonymElement    0x20
#define DOgBitSynonymEquation   0x40
#define DOgBitConceptName       0x80
#define DOgBitConceptElement    0x100
#define DOgBitTranslation       0x200
#define DOgBitConceptWord       0x400
#define DOgBitForm              0x800
#define DOgBitEntity            0x1000
#define DOgBitEntityName        0x2000
#define DOgBitEntityElement     0x4000
/* the second postfix exists but is not identified
 * This is typically any value used by an OEM user */
#define DOgBitOthers            0x40000000


/** First Postfix for HTML **/
#define DOgFixHtml          0x5

/** Second Postfix for HTML **/
#define DOgFixBeginTag          0x11
#define DOgFixEndTag            0x12
#define DOgFixVariableTag       0x13
#define DOgFixMetaName          0x14
#define DOgFixMetaUnknown       0x15
#define DOgFixMetaAuthor        0x16
#define DOgFixMetaKeywords      0x17
#define DOgFixMetaDescription   0x18

/** Equivalent bit value for second Postfix for HTML **/
#define DOgBitBeginTag          0x1
#define DOgBitEndTag            0x2
#define DOgBitVariableTag       0x4
#define DOgBitMetaName          0x8
#define DOgBitMetaUnknown       0x10
#define DOgBitMetaAuthor        0x20
#define DOgBitMetaKeywords      0x40
#define DOgBitMetaDescription   0x80

/** First Postfix for tagged text **/
#define DOgFixTaggedText    0x6

/** First and only Postfix for Authorization **/
#define DOgFixAuthorization 0x7

/** First Postfix for data bases, not used yet **/
#define DOgFixDataBase      0x8

/** First and only Postfix for Synonym and Concept files **/
#define DOgFixLinguisticFile    0x9


/*
 *	File name separator character for zip or other 
 *	reference of the same type, for example :
 *	  <sep>d:\filezip\ogm_fltr.zip<sep>ZIP<sep>ogm_fltr/Src/OgInso.c
 *	where <sep> is a character, and which represents
 *	the file "ogm_fltr/Src/OgInso.c" contained in the zip file
 *	whose name is "d:\filezip\ogm_fltr.zip"
 *	"ZIP" in between represents the file type. 
 *  This value should not be used for postfix, see ogm_hrd/stpfile.c
 *  for more information on the subject.
*/
#define DOgSepReference     0x1

/*  Prefix: gives the the list of available files numbers
 *      if there is only one element, available file numbers start
 *      at this number included, up to infinity.
 *      Format: <DOgAvailRefNumber><number>
*/
#define DOgAvailRefNumber       0x1

/*  Prefix: cross references. All cross references have the same format.
 *      Format: DOgXRef <type of XRef> <value of Xref>
 *      Format: <DOgXRef><number><type of XRef><value>
 *      where: <number> is the reference number and <reference>
 *      is the string containing the reference itself. <number>
 *      is before <DOgXRefName> because this way we can obtain
 *      all reference type with one single call, namely "<DOgXRef><number>*".
*/
#define DOgXRef                 0x2

/*  Prefix: Type of cross reference, formats are the following
 *    DOgXRef DOgXFile <file nb> <arborescence habituelle des Xrefs>
 *    DOgXRef DOgXParPosition <file nb> <parag nb> <position> <length>
 *    DOgXRef DOgXParExpression <file nb> <parag nb> <expression>
 *    DOgXRef DOgXRefLink <file nb> <DOgXRefHyperLink> <hyper link string>
 *    DOgXRef DOgXRefLink <file nb> <DOgXRefUserLink> <user link string>
 *    DOgXRef DOgXRefImage <base file nb> <DOgXRefImageCorrelation> <distance> <file nb>
 *    DOgXRef DOgXRefDomain <file nb> <domain>
 *    DOgXRef DOgXParDomain <file nb> <parag nb> <domain>
 *    DOgXRef DOgXSynonym <compilation file nb> DOgXSynName <value>
 *    DOgXRef DOgXSynonym <compilation file nb> DOgXSynWord <value>
 *    DOgXRef DOgXSynonym <compilation file nb> DOgXSynEquation <value>
 *    DOgXRef DOgXConcept <compilation file nb> DOgXConName <value>
 *    DOgXRef DOgXConcept <compilation file nb> DOgXConWord <value>
 *    DOgXRef DOgXConcept <compilation file nb> DOgXConPath <value>
 *    DOgXRef DOgXConcept <compilation file nb> DOgXConCooc <file nb> <parag nb>
 *    DOgXRef DOgXTag <file nb> DOgXTagName <value>
 *    DOgXRef DOgXTag <file nb> DOgXTagWeight <value>
 *    DOgXRef DOgXEntity <compilation file nb> DOgXNenName <value>
 *    DOgXRef DOgXEntity <compilation file nb> DOgXNenWord <value>
 *    DOgXRef DOgXEntity <compilation file nb> DOgXNenPath <value>
 *    DOgXRef DOgXEntity <compilation file nb> DOgXNenCooc <file nb> <parag nb>
 *    DOgXRef DOgXParEntity <file nb> <parag nb> <compilation file nb> <original_length> <named_entity>
 *    DOgXRef DOgXParFileEntity <file nb> <compilation file nb> <original_length> <named_entity>
 *    DOgXRef DOgXSort <type> <file nb> <data>
*/
#define DOgXFile                0x1
#define DOgXParPosition         0x2
#define DOgXParExpression       0x3
#define DOgXRefLink             0x4
#define DOgXRefImage            0x5
#define DOgXRefDomain           0x6
#define DOgXParDomain           0x7
#define DOgXSynonym             0x8
#define DOgXConcept             0x9
#define DOgXTag                 0x10
#define DOgXEntity              0x11
#define DOgXParEntity           0x12
#define DOgXParFileEntity       0x13
#define DOgXSort                0x14


/* All general information about a reference
 *   Format: <DOgXRef><DOgXFile><number><type of XRef><value>
 *   where: <number> is the reference number and <reference>
 *   is the string containing the reference itself. <number>
 *   is before <DOgXRefName> because this way we can obtain
 *   all reference type with one single call, namely "<DOgXRef><number>*".
 *  DOgXRefName: string representing the name of the reference, mandatory
 *  DOgXRefSize: size of the reference
 *  DOgXRefLastAccess: last access date of the reference
 *  DOgXRefLastUpdate: last update date of the reference
 *  DOgXRefLanguage: language of the reference (French, English,...)
 *  DOgXRefFileFormat: file format (TEXT, HTML WINWORD97, ...)
 *    when the file is name-only this field does not exist.
 *  DOgXRefTitle: title of the file (HTML only for the moment)
 *  DOgXRefGlimpse: one glimpse (important expression) of a file.
 *  DOgXRefNumberOfEntries: approximate number of recorded entries
 *    for the file. This gives a rough estimation of the size
 *    it takes to represent the file in the repository. The
 *    number is not completely precise because we don't keep
 *    track of the possible change of this number when a renaming
 *    of a file occurs.
 *  DOgXRefSignature: content signature of the document used in ogddir
 *  DOgXRefSignWord: word used for content signature
 *  DOgXRefWordsInFile: number of words (simple + expression) in the file
 *  DOgXRefWordsInAnchors: number of words (simple + expression) 
 *    in the file anchors (HTML only)
 *  DOgXRefNbAnchors number of anchors in the files
 *    The three last number help create an anchor density which is 
 *    either DOgXRefWordsInFile/DOgXRefWordsInFile or 
 *  DOgXRefNbAnchors/DOgXRefWordsInFile, this last value being
 *    the prefered one.
 *  DOgXRefLingType linguistic type of the file, defines
 *    a synonym list, concept, entity list file
 *  DOgXRefTagContent: a tag content, can come from tag_ctrl.dat
 *    or an XML input file.
 *  DOgXRefMetaDataMd5: an Md5 value that represents
 *    a signature of the meta data added from an XML input file.
 *    this value is used to check if the file meta-data have been
 *    changed when a re-indexation is asked for on that file.
*/
#define DOgXRefName             0x1
#define DOgXRefLastAccess       0x2
#define DOgXRefSize             0x3
#define DOgXRefLastUpdate       0x4
#define DOgXRefLanguage         0x5
#define DOgXRefFileFormat       0x6
#define DOgXRefTitle            0x7
#define DOgXRefGlimpse          0x8
#define DOgXRefNumberOfEntries  0x9
#define DOgXRefHtmlMeta         0xa
#define DOgXRefOriginalName     0xb
#define DOgXRefCodePage         0xc
#define DOgXRefAuthorization    0xd
#define DOgXRefSignature        0xe
#define DOgXRefSignWord         0xf
#define DOgXRefWordsInFile      0x10
#define DOgXRefWordsInAnchors   0x11
#define DOgXRefNbAnchors        0x12
#define DOgXRefFileId           0x13
#define DOgXRefLingType         0x14
#define DOgXRefTagContent       0x15
#define DOgXRefMetaDataMd5      0x16
#define DOgXRefPageRank         0x17
#define DOgXRefOnDepth          0x18
#define DOgXRefFilteringTime    0x19
#define DOgXRefConcept          0x20
#define DOgXRefEntity           0x21

#define DOgXRefLingTypeNil      0x0
#define DOgXRefLingTypeSynonym  0x1
#define DOgXRefLingTypeConcept  0x2
#define DOgXRefLingTypeEntity   0x3


/* All image information about a reference; format
 *   DOgXRef DOgXRefImage <base image file nb> DOgXRefImageCorrelation <distance> <image file nb>
 *   DOgXRef DOgXRefImage <image file nb> DOgXRefImageDescriptor <descriptor number> 
 *           <method> <descriptor dimension> <axis number> <real number>
 *   DOgXRef DOgXRefImage <image file nb> DOgXRefImageNbDescriptors <descriptors number> 
*/
#define DOgXRefImageCorrelation     0x1
#define DOgXRefImageDescriptor      0x2
#define DOgXRefImageNbDescriptors   0x3


/* All link information about a reference, format:
 *   DOgXRef DOgXRefLink <number> DOgXRefHyperLink <value>
 *   DOgXRef DOgXRefLink <number> DOgXRefUserLink <value>
*/
#define DOgXRefHyperLink        0x1
#define DOgXRefUserLink         0x2


/* All synonym information about a reference
 *   DOgXRef DOgXSynonym <number> DOgXSynName <value>
 *   DOgXRef DOgXSynonym <number> DOgXSynWord <value>
 *   DOgXRef DOgXSynonym <number> DOgXSynEquation <value>
 *  DOgXSynName: string representing the name of the list of synonym
 *  DOgXSynWord: string representing the added synonym 
 *  DOgXSynEquation: string representing the added synonym equation 
*/
#define DOgXSynName             0x1
#define DOgXSynWord             0x2
#define DOgXSynEquation         0x3
#define DOgXSynPath             0x4

/* All concept information about a reference
 *   DOgXRef DOgXConcept <number> DOgXConName <value>
 *   DOgXRef DOgXConcept <number> DOgXConWord <value>
 *   DOgXRef DOgXConcept <number> DOgXConPath <value>
 *   DOgXRef DOgXConcept <number> DOgXConCooc <file nb> <parag nb>
 *   DOgXRef DOgXConcept <number> DOgXConXFile <file nb>
 *  DOgXConName: string representing the name of the concept
 *  DOgXConWord: string representing the added concept element
 *  DOgXConPath: string representing concept file path element (used only for removal)
 *  DOgXConCooc: string representing cooccurrence xref of the concept (used only for removal)
 *  DOgXConXFile: string representing file xref of the concept (used only for removal)
*/
#define DOgXConName             0x1
#define DOgXConWord             0x2
#define DOgXConPath             0x3
#define DOgXConCooc             0x4
#define DOgXConXFile            0x5

/* All named entities information about a reference
 *   DOgXRef DOgXEntity <number> DOgXNenName <value>
 *   DOgXRef DOgXEntity <number> DOgXNenWord <value>
 *   DOgXRef DOgXEntity <number> DOgXNenPath <value>
 *   DOgXRef DOgXEntity <number> DOgXNenCooc <file nb> <parag nb>
 *   DOgXRef DOgXEntity <number> DOgXNenXFile <file nb>
 *  DOgXNenName: string representing the name of the entity list
 *  DOgXNenWord: string representing the added entity name
 *  DOgXNenPath: string representing entity file path element (used only for removal)
 *  DOgXNenCooc: string representing cooccurrence xref of the entity (used only for removal)
*/
#define DOgXNenName             0x1
#define DOgXNenWord             0x2
#define DOgXNenPath             0x3
#define DOgXNenCooc             0x4
#define DOgXNenXFile            0x5

/* All concept information about a reference
 *   DOgXRef DOgXTag <number> DOgXTagName <value>
 *   DOgXRef DOgXTag <number> DOgXTagWeight <value>
 *  DOgXTagName: string representing the name of the tag
 *  DOgXTagWeight: string representing the weight of the tag
*/
#define DOgXTagName             0x1
#define DOgXTagWeight           0x2

/* All Xsort information for better speed
 *   DOgXRef DOgXSort <type> <file nb> <data>
*/
#define DOgXSortLastUpdate      0x1


/*  Prefix: gives the list of stopped files numbers
 *      Format: <DOgStoppedRefNumber><number>
*/
#define DOgStoppedRefNumber     0x3


/*  Prefix: gives the list of available authorization numbers
 *      if there is only one element, available auth numbers start
 *      at this number included, up to infinity.
 *      Format: <DOgAvailAuthNumber><number>
*/
#define DOgAvailAuthNumber  0x4


/*  Prefix: cross references for authorization schemes
 *      Format: DOgXAuth <type of XAuth> <value of Xauth>
 *  For the moment, there is only one type of XAuth,
 *  but it is possible to add more.
*/
#define DOgXAuth	        0x5

/*  Authorization basic information.
 *      Format: DOgXAuth DOgXAuthorization <lengh of auth string>
 *              <auth string> <auth number> | <file number> 
*/
#define DOgXAuthorization   0x1

/*  Authorization type, first char of authorization */
#define DOgAuthorizationTypeLing    'l'
#define DOgAuthorizationTypeOem     'o'


/*  Prefix: image descriptor information
 *      Format: DOgImage <type of DOgImage> <value of DOgImage>
 *  For the moment, there is only one type of DOgImage,
 *  but it is possible to add more.
*/
#define DOgImage 	        0x6

/*  Coordinates for a descriptor of an image.
 *      Format: DOgImage DOgImageCoordinate <method> <descriptor dimension> 
 *              <axis number><coordinate><image refnum><descriptor number>
*/
#define DOgImageCoordinate 0x1


/*  Prefix: range information for dates and number
 *  Formats:
 *      DOgRange DOgRangeNumber DOgRangeNumberPure <number>
 *      DOgRange DOgRangeNumber DOgRangeNumberType <type>01<number>
 *
 *      DOgRange DOgRangeDate DOgRangeDatePure <date>
 *      DOgRange DOgRangeDate DOgRangeDateType <type>01<date>
*/
#define DOgRange            0x7

#define DOgRangeNumber      0x1
#define DOgRangeDate        0x2

#define DOgRangeSeparator   0x1

#define DOgRangeNumberPure  0x1
#define DOgRangeNumberType  0x2

#define DOgRangeDatePure    0x1
#define DOgRangeDateType    0x2


/*  Prefix: associative memory information
 *  Formats:
 *      DOgMas <type of associative memory> <private structure for the associative memory>
 *      DOgRange DOgMen
*/
#define DOgMas 0x8

/** Multiple named entities associative memory zone **/
#define DOgMen  0x1
#define DOgMts  0x2
#define DOgUrs  0x3
#define DOgUrm  0x4
#define DOgItem 0x5
#define DOgFreq 0x6
#define DOgRich 0x7
#define DOgPfix 0x8


/*  Prefix: gives the the list of available item numbers
 *      Format: <DOgAvailItemNumber><number>
 *  This is used with the DOgMas DOgItem memory
 *  if there is only one element, available item numbers start
 *  at this number included, up to infinity.
*/
#define DOgAvailItemNumber 0x9


/*  Prefix: is the date of the file as an unsigned number
 *      Format: <DOgUnsignedDate><number><file><occ>
 *  This is a temporary prefix, so we use last
 *  available prefix before white char.
*/
#define DOgUnsignedDate         0x1f


/** Second Prefix for operating system data **/

#define DOgFixCreateDateYear    0x1
#define DOgFixCreateDateMon     0x2
#define DOgFixCreateDateDay     0x3
#define DOgFixCreateDateHour    0x4
#define DOgFixeCreateDateMin    0x5
#define DOgFixCreateDateSec     0x6

#define DOgFixLastUpdateYear    0x7
#define DOgFixLastUpdateMon     0x8
#define DOgFixLastUpdateDay     0x9
#define DOgFixLastUpdateHour    0x10
#define DOgFixLastUpdateMin     0x11
#define DOgFixLastUpdateSec     0x12

#define DOgFixLastAccessYear    0x13
#define DOgFixLastAccessMon     0x14
#define DOgFixLastAccessDay     0x15
#define DOgFixLastAccessHour    0x16
#define DOgFixLastAccessMin     0x17
#define DOgFixLastAccessSec     0x18

#define _LOGFIXALIVE_

#endif

