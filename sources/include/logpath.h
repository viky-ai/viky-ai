/*
 *	All macros for file names and system names
 *	Copyright (c) 1998-2002 Pertimm, Inc. by Pertimm Team.
 *	Dev: May 1998, July,October 2000, September,October 2001
 *	Dev: November 2002, April, July 2003, November 2004, January,March,July,August 2005
 *  Dev: March,July 2006
 *	Version 1.36
*/
#ifndef _LOGPATHALIVE_

#define DogMaxPathSize				1024
#define DOgMinFreeSpace				2097152	 /* 2 M */
#define DOgDefaultMaxFilePerSlice	30
#define DOgDefaultUnisize			262144 /*  256K  */
#define DOgDefaultMaxWordSize		256
#define DOgMaxUnisize				0x80000000 /*  2G */
#define DOgMinUnisize				4096 /*  4 K */

#define TIMER_PERIOD_SOFT 1000  //ms
#define TIMER_PERIOD_HARD 1000  //ms
#define TIMER_PERIOD_OCEA 1000  //ms
#define TIMER_PERIOD_FLTR 1000  //ms
#define TIMER_PERIOD_MONI 1000  //ms
#define TIMER_PERIOD_SYNC 1000  //ms
#define TIMER_PERIOD_CMD  1000  //ms  
#define TIMER_PERIOD_S95  1000  //ms  

/** Used in all files to define windows names **/
/** and control file names **/
#define SNT_EXE		"ogm_snt" 
#define S2K_EXE		"ogm_s2k" 
#define TSR95_EXE	"ogm_s95" 
#define TSR9X_EXE   "ogm_s9x"
#define FLTR_EXE	"ogm_fltr" 
#define MONI_EXE	"ogm_moni" 
#define OCEA_EXE	"ogm_ocea" 
#define JSRV_EXE	"ogm_jsrv" 
#define PHOX_EXE	"ogm_phox" 
#define STOP_EXE	"ogm_stop" 
#define DSP_EXE		"ogm_dsp"
#define FIND_EXE	"ogm_find"
#define SYNC_EXE	"ogm_sync"
#define UPDT_EXE	"ogm_updt"
#define NODE_EXE    "ogm_node"
#define MTSE_EXE    "ogm_mtse"
#define URLS_EXE    "ogm_urls"
#define URLM_EXE    "ogm_urlm"
#define SIDX_EXE    "ogm_sidx"
#define SSRV_EXE    "ogm_ssrv"
#define SCTA_EXE    "ogm_scta"
#define CLEAN_EXE    "ogmclean"
#define CDSTAR_EXE	"ogcdstar" 
#define OGGL_EXE    "oggl"
#define URUN_EXE    "ogurun"
#define UCIS_EXE    "ogucis"

/** Used in process control **/
#define DOgOceaName     "ocea"
#define DOgFltrName     "fltr"
#define DOgMoniName     "moni"
#define DOgJsrvName     "jsrv"
#define DOgSyncName     "sync"
#define DOgPhoxName     "phox"
#define DOgUpdtName     "updt"
#define DOgNodeName     "node"
#define DOgMtseName     "mtse"
#define DOgUrlsName     "urls"
#define DOgUrlmName     "urlm"
#define DOgSidxName     "sidx"
#define DOgSsrvName     "ssrv"
#define DOgSctaName     "scta"
#define DOgCdStarName   "cdstar"
#define DOgUrunName     "urun"
#define DOgUcisName     "ucis"

#define DOgLetterFltr	"f"
#define DOgLetterOcea	"a"

#define DOgDirHard          "hard"
#define DOgDirSoft          "soft"
#define DOgDirStalled       "stalled"
#define DOgDirFilter        "filter"
#define DOgDirService       "service"
#define DOgDirControl       "control"
#define DOgDirData          "data"
#define DOgDirLog           "log"
#define DOgDirCodepage      "codepage"
#define DOgDirUnzip         "unzip"
#define DOgDirViewer        "viewer"
#define DOgDirOcea          "ocea"
#define DOgDirOceaMirror    "ocea/mirror"
#define DOgDirShared        "shared"
#define DOgDirHtml          "html"

#define DOgExt_Trs	".trs"
#define DOgExt_Ogm	".ogm"
#define DOgExt_Exp	".exp"
#define DOgExt_Dsp	".dsp"
#define DOgExt_Tmp	".tmp"
#define DOgExt_Txt	".txt"
#define DOgExt_Dat	".dat"
#define DOgExt_Rem	".rem"
#define DOgExt_Sto      ".sto"
#define DOgExt_Mdf	".mdf"
#define DOgExt_Jo	".jo"

/** file compteur **/
#define DOgExt_Nul	".nul"

/** file compteur **/
#define DOgExt_Cpt	".cpt"
#define DOgExt_Tot	".tot"

/** space file **/
#define DOgExt_Spa	".spa"

/** Starting the program **/
#define DOgExt_Sta	".sta"

/** Stopping the program as soon as possible **/
#define DOgExt_Stp	".stp"

/** Stopping the program as soon as possible but **/ 
/** but the program finishes it's current task) **/
/** Example: ogm_hard stops only after finishing a soft file **/
#define DOgExt_Spd	".spd"

/** Return flag of the program (not implemented) **/
/** Flag set by the program if it has returned correctly **/
#define DOgExt_Rok	".rok"

#define DOgTempDirectory    "temp_directory"

#define DOgFileOgmConf_Txt	"ogm_conf.txt"
#define DOgFileOgmConf_Txt_instance	"conf/ogm_conf.txt"

#define DOgFileOgmRen_Dat	"ogm_ren.dat"

#define DOgFileHardReposit_Ory		"hard/reposit.ory"

#define DOgFileCodepageUnicode_Dlm	"codepage/unicode.dlm"

#define DOgFileServiceOgmSnt_Inc	"service/ogm_snt.inc"
#define DOgFileServiceOgmOpen_Dat	"service/ogm_open.dat"
#define DOgFileServiceOgmDel_Dat	"service/ogm_del.dat"
#define DOgFileServiceOgmRen_Dat	"service/ogm_ren.dat"
#define DOgFileServiceOgmOpen_Tmp	"service/ogm_open.tmp"
#define DOgFileServiceOgmDel_Tmp	"service/ogm_del.tmp"
#define DOgFileServiceOgmRen_Tmp	"service/ogm_ren.tmp"
#define DOgFileServiceOgmServ_Ren	"service/ogm_serv.ren"
#define DOgFileServiceOgmServ_Std	"service/ogm_serv.std"
#define DOgFileServiceOgmServ_Stp	"service/ogm_serv.stp"

#define DOgFileDataOgmRef_Dat	"data/ogm_ref.dat"
#define DOgFileDataOgmStor_Dat	"data/ogm_stor.dat"
#define DOgFileDataOgmSlct_Dat	"data/ogm_slct.dat"
#define DOgFileDataOgmEras_Dat	"data/ogm_eras.dat"
#define DOgFileDataOgmName_Dat	"data/ogm_name.dat"
#define DOgFileDataOgmRen_Dat	"data/ogm_ren.dat"
#define DOgFileDataPatterns_Dat	"data/patterns.dat"
#define DOgFileDataTagCtrl_Dat	"data/tag_ctrl.dat"
#define DOgFileDataOgmMoni_Dat	"data/ogm_moni.dat"
#define DOgFileDataDiralias_Dat	"data/diralias.dat"
#define DOgFileDataDiralias_Auf	"data/diralias.auf"
#define DOgFileDataOrialias_Dat	"data/orialias.dat"

#define DOgFileDataOgmRef_Tmp	"data/ogm_ref.tmp"
#define DOgFileDataOgmStor_Tmp	"data/ogm_stor.tmp"
#define DOgFileDataOgmSlct_Tmp	"data/ogm_slct.tmp"
#define DOgFileDataOgmEras_Tmp	"data/ogm_eras.tmp"
#define DOgFileDataOgmName_Tmp	"data/ogm_name.tmp"
#define DOgFileDataOgmRen_Tmp	"data/ogm_ren.tmp"
#define DOgFileDataOgmDsp_Dat	"data/ogm_dsp.dat"
#define DOgFileDataOgmDsp_Tmp	"data/ogm_dsp.tmp"

#define DOgFileDataOgmRen_New	"data/ogm_ren.new"

#define DOgFileLogOgmFltr_Log	"log/ogm_fltr.log"
#define DOgFileLogOgmSnt_Log	"log/ogm_snt.log"
#define DOgFileLogOgmCmd_Log	"log/ogm_cmd.log"
#define DOgFileLogOgmS95_Log	"log/ogm_s95.log"
#define DOgFileLogOgmBulk_Log	"log/ogm_bulk.log"
#define DOgFileLogOgmDate_Log	"log/ogm_date.log"

#define DOgFileLingStop_Auf     "ling/stop.auf"

#define	DOgFileFilterMetadata_fil "filter/metadata.fil"

#define	DOgFileControlsync_on        "control/sync.on"
#define	DOgFileControlOnepass_run    "control/onepass.run"
#define	DOgFileControlOnepass_end    "control/onepass.end"
#define	DOgFileControlMupdate_now    "control/mupdate.now"
#define	DOgFileControlMcontinue_now  "control/mcontinue.now"
#define	DOgFileControlMcancel_now    "control/mcancel.now"
#define	DOgFileControlMwait_inf      "control/mwait.inf"

#define DOgFileHtmlFile_Html        "html/file.html"

#define DOgFileWhereogm_Dt	"whereogm.dt"

#define DOgFileConfEngineMatching_Txt   "conf/engine_matching.txt"

/* defining path for instances */
#define DOgPathMAINtoINSTANCES	"private/instances"
#define DOgPathMAINtoVAR	"var"

#define _LOGPATHALIVE_
#endif


