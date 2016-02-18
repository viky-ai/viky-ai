/*
 *  This is main procedure for ogheap.exe
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : November 2010
 *  Version 1.0
*/
#include <logheap.h>
#include <logmsg.h>
#include <getopt.h>
#include <logpath.h>

struct heap1 {
  int field1;
  int field2;
  };


/* structure de contrôle du programme */
struct og_info {
  void *hmsg,*herr,*hheap; int trace;
  char input_filename[DPcPathSize],output_filename[DPcPathSize];
  int output_buffer_size; unsigned char *output_buffer;
  int input_buffer_size; unsigned char *input_buffer;
  void *hsecondary_heap;
  int test_number;
  size_t number;
  int sliced;
  int replicas;
  };


/* functions for using main api */
static int Heap(struct og_info *info);
static int HeapTest1(struct og_info *info);
static int HeapTest2(struct og_info *info);
static int HeapTest3(struct og_info *info);
static int HeapTest4(struct og_info *info);
static int HeapTest5(struct og_info *info);
static int HeapTest6(struct og_info *info);
static int HeapTest7(struct og_info *info);
static int HeapTest8(struct og_info *info);
static int HeapTest9(struct og_info *info);

static int ReadHeap1(struct og_info *info, int (*func)(struct og_info *info, struct heap1 *heap1));
static int ReadHeap1Add(struct og_info *info, struct heap1 *heap1);
static int ReadHeap1AddMulti(struct og_info *info, struct heap1 *heap1);
static int ReadHeap1Append(struct og_info *info, struct heap1 *heap1);
static int ReadHeap1SecondaryAppend(struct og_info *info, struct heap1 *heap1);
static int WriteHeap1(struct og_info *info);
static int WriteOutBuff(struct og_info *info, char* buff);


/* default function to define */
static int OgUse(struct og_info *);
static void OgExit(struct og_info *, int);



/* MAIN */
int main (int argc, char * argv[])
{
char *nil,carlu,*DOgPIPE;
struct og_info cinfo,*info=&cinfo;
int optionIndex=0;
int i,result=1;
time_t ltime;

/* definition of program options */
struct option longOptions[] = {
  {"input",              required_argument,  NULL,     'i'},
  {"test",               required_argument,  NULL,     'e'},
  {"number",             required_argument,  NULL,     'n'},
  {"output",             required_argument,  NULL,     'o'},
  {"trace",              required_argument,  NULL,     't'},
  {"help",               no_argument,        NULL,     'h'},
  {"slice",              no_argument,        NULL,     's'},
  { 0,0,0,0 }
};

/* initialization of program info structure */
memset(info,0,sizeof(struct og_info));
if((DOgPIPE=getenv("DOgPIPE"))) {
  IFn(info->hmsg=OgLogInit("ogheap",DOgPIPE, DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogPipe)) goto main_end;}
else {
  IFn(info->hmsg=OgLogInit("ogheap","ogheap", DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogFile)) goto main_end;}
info->herr = OgLogGetErr(info->hmsg);

/* parsing options */
while ((carlu=getopt_long(argc,argv,"i:e:n:o:t:hs?", longOptions, &optionIndex))!=EOF) {
  switch(carlu) {
    case 0:
      break;
    case 'e':
       info->test_number=atoi(optarg);
      break;
    case 'n':
       info->number=OgArgSize(optarg);
      break;
    case 'i':
       strcpy(info->input_filename,optarg);
      break;
    case 'o':
       strcpy(info->output_filename,optarg);
      break;
    case 's':
       info->sliced=1;
      break;
    case 't':
      info->trace = strtol(optarg,&nil,16);
      break;
    case 'h':
    case '?':
      OgUse(info);
      goto main_end;
      break;
    }
  }

/* parsing main parameters */
for(i=0;i<argc-optind;i++) {
  switch(i) {
    case 0:
      strcpy(info->input_filename,argv[optind+i]); break;
    case 1:
      strcpy(info->output_filename,argv[optind+i]); break;
    default:
      break;
    }
  }

/* optional : change the directory to program directory */
// OgGetExecutablePath(argv[0],cwd); chdir(cwd);

/* starting log */
if (info->trace&DOgHeapTraceMinimal) {
  time(&ltime);
  OgMsg(info->hmsg,"",DOgMsgDestInLog,"\nProgram %s starting with pid %x at %.24s", OgLogGetModuleName(info->hmsg), getpid(), OgGmtime(&ltime));
//  OgMsg(info->hmsg,"",DOgMsgDestInLog+DOgMsgSeverityDebug,"Command line: %s", cmd);
//  OgMsg(info->hmsg,"",DOgMsgDestInLog+DOgMsgSeverityDebug,"Current directory is '%s'",cwd);
  }

IF(Heap(info)) {
  goto main_end;
  }

/* ending log */
if (info->trace&DOgHeapTraceMinimal) {
  time(&ltime);
  OgMsg(info->hmsg,"",DOgMsgDestInLog,"\nProgram %s exiting at %.24s\n",OgLogGetModuleName(info->hmsg),OgGmtime(&ltime));
  }

result=0;
main_end:
/* flushing program */
OgExit(info,result);

return(1);
}





static int Heap(struct og_info *info)
{
char erreur[DOgErrorSize];
struct stat filestat;
FILE *fd;

if (info->sliced) {
  IFn(info->hheap=OgHeapSliceInit(info->hmsg,"heap1",sizeof(struct heap1),2,2)) DPcErr;
  }
else {
  IFn(info->hheap=OgHeapInit(info->hmsg,"heap1",sizeof(struct heap1),2)) DPcErr;
  }

if (info->input_filename[0]) {
  IFn(fd=fopen(info->input_filename,"rb")) {
    sprintf(erreur,"Heap: impossible to open '%s'",info->input_filename);
    OgErr(info->herr,erreur); DPcErr;
    }
  IF(fstat(fileno(fd),&filestat)) {
    sprintf(erreur,"Heap: impossible to fstat '%s'",info->input_filename);
    OgErr(info->herr,erreur); DPcErr;
    }
  IFn(info->input_buffer=(char *)malloc(filestat.st_size+9)) {
    sprintf(erreur,"Heap: impossible to allocate %ld bytes for '%s'",filestat.st_size,info->input_filename);
    fclose(fd); OgErr(info->herr,erreur); DPcErr;
    }
  IFn(info->input_buffer_size=fread(info->input_buffer,1,filestat.st_size,fd)) {
    sprintf(erreur,"Heap: impossible to fread '%s'",info->input_filename);
    DPcFree(info->input_buffer); fclose(fd); OgErr(info->herr,erreur); DPcErr;
    }
  fclose(fd);
  }
else if (info->test_number == 5);
else {
  /** handles stdin **/
  info->input_buffer_size = 0x100000; /* using 1Mb, should be more than enough */
  IFn(info->input_buffer=(char *)malloc(info->input_buffer_size+9)) {
    sprintf(erreur,"Heap: impossible to allocate %d bytes for stdin",info->input_buffer_size);
    OgErr(info->herr,erreur); DPcErr;
    }
  IFn(info->input_buffer_size=fread(info->input_buffer,1,info->input_buffer_size,stdin)) {
    sprintf(erreur,"Heap: impossible to fread stdin"); DPcFree(info->input_buffer);
    OgErr(info->herr,erreur); DPcErr;
    }
  }

/** on lance les tests ici **/
switch(info->test_number) {
  case 1: HeapTest1(info); break;
  case 2: HeapTest2(info); break;
  case 3: HeapTest3(info); break;
  case 4: HeapTest4(info); break;
  case 5: HeapTest5(info); break;
  case 6: HeapTest6(info); break;
  case 7: HeapTest7(info); break;
  case 8: HeapTest8(info); break;
  case 9: HeapTest9(info); break;
  default:
    sprintf(erreur,"Heap: invalid test number %d\n", info->test_number);
    OgErr(info->herr,erreur); DPcErr;
    break;
  }

IFE(OgHeapFlush(info->hheap));
DPcFree(info->input_buffer);

DONE;
}




static int HeapTest1(struct og_info *info)
{
IFE(ReadHeap1(info,ReadHeap1Add));
IFE(WriteHeap1(info));
DONE;
}




static int HeapTest2(struct og_info *info)
{
IFE(ReadHeap1(info,ReadHeap1Append));
IFE(WriteHeap1(info));
DONE;
}




static int HeapTest3(struct og_info *info)
{
void *heap1,*secondary_heap;
size_t secondary_heap_used;

IFn(info->hsecondary_heap=OgHeapInit(info->hmsg,"secondary_heap",sizeof(struct heap1),20)) DPcErr;
IFE(ReadHeap1(info,ReadHeap1SecondaryAppend));
IFn(secondary_heap_used=OgHeapGetCellsUsed(info->hsecondary_heap)) DPcErr;
IFE(OgHeapTestRealloc(info->hheap,secondary_heap_used));
IFn(secondary_heap=OgHeapGetCell(info->hsecondary_heap,0)) DPcErr;
IFn(heap1=OgHeapGetCell(info->hheap,0)) DPcErr;
memcpy(heap1,secondary_heap,secondary_heap_used*sizeof(struct heap1));
IFE(OgHeapSetCellsUsed(info->hheap,secondary_heap_used));
IFE(OgHeapFlush(info->hsecondary_heap));

IFE(WriteHeap1(info));
DONE;
}




static int HeapTest4(struct og_info *info)
{
int Iheap1;
struct heap1 *cmp_heap1;
IFE(ReadHeap1(info,ReadHeap1Add));
IFE(OgHeapSetCellsUsed(info->hheap,0));
IFE(Iheap1=OgHeapAddCell(info->hheap));
IFn(cmp_heap1=OgHeapGetCell(info->hheap,Iheap1)) DPcErr;
cmp_heap1->field1 = 30;
cmp_heap1->field2 = 40;
IFE(WriteHeap1(info));
DONE;
}





static int HeapTest5(struct og_info *info)
{
size_t Iheap1, i;
struct heap1 *cmp_heap1;

IFE(OgHeapSetCellsUsed(info->hheap,0));

for (i=0; i<info->number; i++) {
  IFE(Iheap1=OgHeapAddCell(info->hheap));
  IFn(cmp_heap1=OgHeapGetCell(info->hheap,Iheap1)) DPcErr;
  cmp_heap1->field1 = i%0x10000;
  cmp_heap1->field2 = (i+1)%0x10000;
  }

IFn(cmp_heap1=OgHeapGetCell(info->hheap,0)) DPcErr;

DONE;
}





static int HeapTest6(struct og_info *info)
{
struct heap1 heap1[3];
IFE(ReadHeap1(info,ReadHeap1Add));

heap1[0].field1=6; heap1[0].field2=7;
heap1[1].field1=7; heap1[1].field2=8;
heap1[2].field1=8; heap1[2].field2=9;
IFE(OgHeapReplace(info->hheap, 1, 2, 3, heap1));

IFE(WriteHeap1(info));
DONE;
}





static int HeapTest7(struct og_info *info)
{
struct heap1 heap1[2];
struct heap1 *basic_heap1;

IFE(ReadHeap1(info,ReadHeap1Add));

heap1[0].field1=7; heap1[0].field2=8;
heap1[1].field1=9; heap1[1].field2=10;
IFE(OgHeapAppend(info->hheap,2,heap1));

IFn(basic_heap1=OgHeapGetCell(info->hheap,0)) DPcErr;

heap1[0].field1=20; heap1[0].field2=21;
heap1[1].field1=22; heap1[1].field2=23;
IFE(OgHeapReplace(info->hheap, 1, 3, 2, heap1));

IFn(basic_heap1=OgHeapGetCell(info->hheap,0)) DPcErr;

IFE(WriteHeap1(info));
DONE;
}



static int HeapTest8(struct og_info *info)
{
  info->replicas = 666;
  IFE(ReadHeap1(info,ReadHeap1AddMulti));
  OgHeapAddCells(info->hheap, 2);
  int cellNum1 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed1 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg1 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio1 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin1 = OgHeapGetSmoothingWindowSize(info->hheap);
  OgHeapReset(info->hheap);
  int cellNum2 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed2 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg2 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio2 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin2 = OgHeapGetSmoothingWindowSize(info->hheap);

  OgHeapAddCells(info->hheap, 3131);
  int cellNum3 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed3 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg3 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio3 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin3 = OgHeapGetSmoothingWindowSize(info->hheap);
  OgHeapReset(info->hheap);
  int cellNum4 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed4 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg4 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio4 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin4 = OgHeapGetSmoothingWindowSize(info->hheap);

  OgHeapAddCells(info->hheap, 6071);
  OgHeapReset(info->hheap);
  OgHeapAddCells(info->hheap, 7080);
  OgHeapReset(info->hheap);
  OgHeapAddCells(info->hheap, 15503);
  OgHeapReset(info->hheap);
  OgHeapAddCells(info->hheap, 25000);
  OgHeapReset(info->hheap);
  int cellNum5 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed5 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg5 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio5 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin5 = OgHeapGetSmoothingWindowSize(info->hheap);
  OgHeapReset(info->hheap);
  OgHeapAddCells(info->hheap, 25000);
  int cellNum6 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed6 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg6 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio6 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin6 = OgHeapGetSmoothingWindowSize(info->hheap);

  char paramBuff[350];
  sprintf(paramBuff, "Cells Num: %i, %i, %i, %i, %i, %i\n"
      "Cells Usd: %i, %i, %i, %i, %i, %i\n"
      "Smoo  Avg: %i, %i, %i, %i, %i, %i\n"
      "Smoo  Rat: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n"
      "Smoo  Win: %i, %i, %i, %i, %i, %i\n",
      cellNum1, cellNum2, cellNum3, cellNum4, cellNum5, cellNum6,
      cellUsed1, cellUsed2, cellUsed3, cellUsed4, cellUsed5, cellUsed6,
      smooAvg1, smooAvg2, smooAvg3, smooAvg4, smooAvg5, smooAvg6,
      smooRatio1, smooRatio2, smooRatio3, smooRatio4, smooRatio5, smooRatio6,
      smooWin1, smooWin2, smooWin3, smooWin4, smooWin5, smooWin6);

  // flushing collected params
  IFE(WriteOutBuff(info, paramBuff));

  DONE;
}


static int HeapTest9(struct og_info *info)
{
  info->replicas = 666;
  IFE(ReadHeap1(info,ReadHeap1AddMulti));
  OgHeapAddCells(info->hheap, 2);
  int cellNum1 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed1 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg1 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio1 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin1 = OgHeapGetSmoothingWindowSize(info->hheap);
  OgHeapReset(info->hheap);
  int cellNum2 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed2 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg2 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio2 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin2 = OgHeapGetSmoothingWindowSize(info->hheap);

  OgHeapAddCells(info->hheap, 3131);
  int cellNum3 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed3 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg3 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio3 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin3 = OgHeapGetSmoothingWindowSize(info->hheap);
  OgHeapReset(info->hheap);
  int cellNum4 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed4 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg4 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio4 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin4 = OgHeapGetSmoothingWindowSize(info->hheap);

  OgHeapAddCells(info->hheap, 6071);
  OgHeapReset(info->hheap);
  OgHeapAddCells(info->hheap, 584);
  OgHeapReset(info->hheap);
  OgHeapAddCells(info->hheap, 5503);
  OgHeapReset(info->hheap);
  OgHeapAddCells(info->hheap, 25000);

  int cellNum5 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed5 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg5 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio5 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin5 = OgHeapGetSmoothingWindowSize(info->hheap);

  for (int i = 0; i < 60; i++)
  {
    OgHeapAddCells(info->hheap, 4790);
    OgHeapReset(info->hheap);
  }
  int cellNum6 = OgHeapGetCellsNumber(info->hheap);
  int cellUsed6 = OgHeapGetCellsUsed(info->hheap);
  int smooAvg6 = OgHeapGetSmoothedAverage(info->hheap);
  float smooRatio6 = OgHeapGetSmoothingRatio(info->hheap);
  int smooWin6 = OgHeapGetSmoothingWindowSize(info->hheap);

  char paramBuff[350];
  sprintf(paramBuff, "Cells Num: %i, %i, %i, %i, %i, %i\n"
      "Cells Usd: %i, %i, %i, %i, %i, %i\n"
      "Smoo  Avg: %i, %i, %i, %i, %i, %i\n"
      "Smoo  Rat: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n"
      "Smoo  Win: %i, %i, %i, %i, %i, %i\n",
      cellNum1, cellNum2, cellNum3, cellNum4, cellNum5, cellNum6,
      cellUsed1, cellUsed2, cellUsed3, cellUsed4, cellUsed5, cellUsed6,
      smooAvg1, smooAvg2, smooAvg3, smooAvg4, smooAvg5, smooAvg6,
      smooRatio1, smooRatio2, smooRatio3, smooRatio4, smooRatio5, smooRatio6,
      smooWin1, smooWin2, smooWin3, smooWin4, smooWin5, smooWin6);

  // flushing collected params
  IFE(WriteOutBuff(info, paramBuff));

  DONE;
}


static int ReadHeap1(struct og_info *info, int (*func)(struct og_info *info, struct heap1 *heap1))
{
char *ptr,*separators=" \t\r\n";
struct heap1 heap1[1];
int number,first=1;

ptr=strtok( info->input_buffer,separators);
while (ptr) {
  number=atoi(ptr);
  if (first) {
    heap1->field1=number;
    first=0;
    }
  else {
    heap1->field2=number;
    IFE(func(info,heap1));
    first=1;
    }
  ptr=strtok(NULL,separators);
  }

DONE;
}



static int ReadHeap1Append(struct og_info *info, struct heap1 *heap1)
{
IFE(OgHeapAppend(info->hheap,1,heap1));
DONE;
}




static int ReadHeap1SecondaryAppend(struct og_info *info, struct heap1 *heap1)
{
IFE(OgHeapAppend(info->hsecondary_heap,1,heap1));
DONE;
}




static int ReadHeap1Add(struct og_info *info, struct heap1 *heap1)
{
int Iheap1;
struct heap1 *cmp_heap1;
IFE(Iheap1=OgHeapAddCell(info->hheap));
IFn(cmp_heap1=OgHeapGetCell(info->hheap,Iheap1)) DPcErr;
cmp_heap1->field1 = heap1->field1;
cmp_heap1->field2 = heap1->field2;
DONE;
}

static int ReadHeap1AddMulti(struct og_info *info, struct heap1 *heap1)
{
  int Iheap1;
  struct heap1 *cmp_heap1;

  for (int i = 0; i < info->replicas; i++)
  {
    IFE(Iheap1=OgHeapAddCell(info->hheap));
    IFn(cmp_heap1=OgHeapGetCell(info->hheap,Iheap1)) DPcErr;
    cmp_heap1->field1 = heap1->field1;
    cmp_heap1->field2 = heap1->field2;
  }
  DONE;
}




static int WriteHeap1(struct og_info *info)
{
char erreur[DOgErrorSize];
struct heap1 *heap1;
int i,heap1_used;
FILE *fd;

IFn(info->output_filename[0]) DONE;
IFn(fd=fopen(info->output_filename,"w")) {
  sprintf(erreur,"Heap: impossible to open '%s'",info->output_filename);
  OgErr(info->herr,erreur); DPcErr;
  }

IFn(heap1_used=OgHeapGetCellsUsed(info->hheap)) DPcErr;

for (i=0; i<heap1_used; i++) {
  IFn(heap1=OgHeapGetCell(info->hheap,i)) DPcErr;
  fprintf(fd,"%d %d\n",heap1->field1,heap1->field2);
  }

fclose(fd);
DONE;
}


static int WriteOutBuff(struct og_info *info, char* buff)
{
  FILE *fd;
  char error[DOgErrorSize];

  IFn(info->output_filename[0]) DONE;

  IFn(fd = fopen(info->output_filename, "w")) {
    sprintf(error,"Heap: impossible to open '%s'", info->output_filename);
    OgErr(info->herr, error); DPcErr;
  }

  fprintf(fd,"%s", buff);

  fclose(fd);
  DONE;
}

STATICF(int) OgUse(struct og_info *info)
{
char buffer[8192];
int ibuffer=0;

ibuffer += sprintf(buffer,        "Usage : ogheap [options] [input_filename [output_filename]]\n");
ibuffer += sprintf(buffer+ibuffer,"options are:\n");
ibuffer += sprintf(buffer+ibuffer,"   -e,  --test=<number>: specifies test number, default is no test\n");
ibuffer += sprintf(buffer+ibuffer,"   -i,  --input=input_filename: specifies input filename, default is stdin\n");
ibuffer += sprintf(buffer+ibuffer,"   -o,  --output=output_filename: specifies output filename, default is stdout\n");
ibuffer += sprintf(buffer+ibuffer,"   -h,  --help prints this message\n");
ibuffer += sprintf(buffer+ibuffer,"   -s,  --slice activates the 'slice mode' heap\n");
ibuffer += sprintf(buffer+ibuffer,"   -t,  --trace=n: trace options for logging (default 0x%x)\n",info->trace);
ibuffer += sprintf(buffer+ibuffer,"         n has a combined hexadecimal value of:\n");
ibuffer += sprintf(buffer+ibuffer,"           0x1: minimal, 0x2: memory\n");

OgLogConsole(info->hmsg,"%.*s",ibuffer,buffer);

DONE;
}





static void OgExit(struct og_info *info, int result)
{
char *error;

if(info->hmsg) {
  error=OgLogLastErr(info->hmsg);
  if(error[0]) {
    IF(OgMsg(info->hmsg,"",DOgMsgDestInLog+DOgMsgSeverityError,"LAST ERROR: %s",OgLogLastErr(info->hmsg))) {exit(1);}
    }
  IF(OgLogFlush(info->hmsg)) {exit(1);}
  else {exit(result);}
  }
else {
  exit(1);
  }

exit(result);
}


