/***************************************************************************/
/*                                                                         */
/* COMPILE.CPP,                                                            */
/*     a sample file for use with the FrontDoor Nodelist Write Code        */
/*                                                                         */
/* (c) 1997,1998 Colin Turner                                              */
/*                                                                         */
/* Please see FDNODE.DOC for details on the conditions attached to this    */
/* code.                                                                   */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/* This is /very/ rudimentary example of the usage of this code, little or */
/*                     no "idiot" checking is performed.                   */
/*                                                                         */
/***************************************************************************/

#include "fdnode.h"     // Nodelist class declarations
#include "fdwcache.h"   // Cached Write Class, much faster
#include "ctl.h"        // FrontDoor SETUP.FD structure
#include <dos.h>
#include <ctype.h>
#include <stdio.h>
#include <conio.h>

// Comment this to use the base class only

#define CacheOn


// Prototypes

void            main(int argc, char *argv[]);
void            PrintBanner();

void            ReadFD(void);

char *          ReadControlLine(FILE * fp, char * buffer);

void            ProcessDialTable(char * Defaults, FILE * control);
void            ProcessCostTable(FILE * control);
void            ProcessNodeFile(char * Filename, long int Whence);
void            ProcessPointFile(char * Filename);

unsigned short  GetNodelistDetails(char * stem, char * buffer);
int             AddToPrivate(char * Parameter);
int             AddToPoint(char * Parameter);

int             AppendFile(char * FileName, char * Existing, char * Header);
struct find_t * CheckFile(char * filename);


char NAME[]="FDWNode Test";
char VERSION[]="1.03";
char FDNodelistDir[72]="";
char FDSemaphoreDir[72]="";
unsigned short Country;
char Buffer[1024];

// Global write nodelist class
#ifdef CacheOn
FDWCachedNode * Nodelist;
#else
FrontDoorWNode * Nodelist;
#endif

void main(int argc, char *argv[])
{
  unsigned short ExtInt;
  char NLExt[5];
  int  quit=0;
  char stuff[72];
  char * p;
  FILE * control;
  
  // Get the FrontDoor config information we need.
  ReadFD();
  PrintBanner();

  // As the class is started as a global variable with
  // default constructor, we need to initialise it.

  // This little bit of code is used to override the location of the nodelist
  // index set. Used for testing. Note that a trailing backslash /is/ required.
  // (For this file, not for the class as such).
  if(argc==2) strcpy(FDNodelistDir, argv[1]);

  sprintf(stuff, "%sNODELIST.*", FDNodelistDir);
  ExtInt = GetNodelistDetails(stuff, NULL);
  if(ExtInt == 0xFFFFU) sprintf(NLExt, "PVT");
  else sprintf(NLExt, "%03u", ExtInt);

  #ifdef CacheOn
  // If you're loading with some sort of cache, and need to override the OnThaw()
  // function, you must NOT expect the class to be thawed for you.
  // Alternatively, build this into the derived constructor
  Nodelist = new FDWCachedNode(FDNodelistDir, NLExt, Country, WFDNodeOverWrite);
  #else  
  Nodelist = new FrontDoorWNode(FDNodelistDir, NLExt, Country, WFDNodeOverWrite);
  #endif
  if(!Nodelist){
    printf("\nMemory allocation error");
    exit(10);
  }

  printf("\nFD nodelist in %s\n\n", FDNodelistDir);

  if(Nodelist->IsFrozen()){
    printf("\nError opening nodelist indices.");
    printf("\nEither run this program in the nodelist directory, the FD system\nDirectory, or correctly set the FD environment variable.\n\n");
    exit(1);
  }

  // The following line is reasonably technical and should be avoided unless
  // you understand what it does. Essentially it tailors the NODELIST.FDX
  // adding a little which results in slightly smaller index files when you
  // add nodelist files in approximately zone order
  Nodelist->SetTreeFlags(NFDXIndex, 0, 24);

  sprintf(stuff, "%sFDNODE.CTL", FDNodelistDir);
  control=_fsopen(stuff, "r", SH_DENYRW);
  if(!control){
    perror("Unable to open FDNODE.CTL");
    quit=1;
  }

  sprintf(stuff, "%sFDNET.PVT", FDNodelistDir);
  remove(stuff);
  sprintf(stuff, "%sFDPOINT.PVT", FDNodelistDir);
  remove(stuff);
  
  while(!quit){
    p = ReadControlLine(control, Buffer);
    if(!p) quit=1;
    else{
      if(!strnicmp(p, "PVTLIST", 7))   AddToPrivate(p+7);
      if(!strnicmp(p, "POINTLIST", 9)) AddToPoint(p+9);
      if(!strnicmp(p, "DIAL", 4))      ProcessDialTable(p+4, control);
      if(!strnicmp(p, "COST", 4))      ProcessCostTable(control);
    }
  }
  fclose(control);

  if(ExtInt != 0xFFFFU){
    sprintf(stuff, "%sNODELIST.%03u", FDNodelistDir, ExtInt);
    ProcessNodeFile(stuff, WFDNOfficial);
  }
  sprintf(stuff, "%sFDNET.PVT", FDNodelistDir);
  ProcessNodeFile(stuff, WFDNPrivate);
  sprintf(stuff, "%sFDPOINT.PVT", FDNodelistDir);
  ProcessPointFile(stuff);
  delete Nodelist;

}


void PrintBanner()
{
  printf("\nFrontDoor (TM) Write NodeList Index Tests, Version %s\nColin Turner, 2:443/13.0\nCompiled at %s on %s\n", VERSION, __TIME__, __DATE__);
}


/*
**    ReadFD
**
** A highly non-sophisticated function which reads the NodelistDir from the
** SETUP.FD FrontDoor configuration file.
**
** NO Macro Expansion is performed.
**
**/
void ReadFD(void)
{
  FILE *fp;
  char filename[72]="";
  struct _ctl *fdsetup;

  fdsetup = new _ctl;
  if(!fdsetup){
         printf("\nUnable to allocate memory for SETUP.FD\n");
         exit(11);
  }
  if(getenv("FD")){
         strcpy(filename, getenv("FD"));
         if(filename[strlen(filename)-1]!='\\') strcat(filename, "\\");
         strcat(filename,"SETUP.FD");
  }
  else strcpy(filename, "SETUP.FD");
  fp=_fsopen(filename,"rb",SH_DENYWR);
  if(fp){

    if((fread(fdsetup,sizeof(struct _ctl),1,fp))!=1) printf("\n SETUP.FD read error (structure packing in compiler?)\n");
    fclose(fp);

    strcpy(FDNodelistDir, fdsetup->s.nodelistpath);
    strcpy(FDSemaphoreDir, fdsetup->s.rescanpath);
    if(!strlen(FDSemaphoreDir)) strcpy(FDSemaphoreDir, fdsetup->s.systempath);
    Country = fdsetup->s.countrycode;

  }
  else{
    printf("\nSETUP.FD must be in the current directory, or pointed to by\nan FD environment variable\n");
    delete fdsetup;
    exit(11);
  }
  delete fdsetup;

}


/*
**      ReadControlLine
**
** This function simply reads from the specified file into the specified buffer
** until it gets a "valid" line. That is, one that does not begin with a ';'
** character.
**
** White space is also ignored.
**
**      Parameters
**
**      fp      Pointer to the file from which to read the control line.
**      buffer  Pointer to the region in which we should place the read line.
**
**      Returns
**
**      NULL signifies EOF condition, otherwise it returns buffer.
**/
char * ReadControlLine(FILE * fp, char * buffer)
{
  int found=0;

  while(!found /*&& !feof(fp)*/){

    if (fgets(buffer, 1024, fp) == NULL) return NULL;
  
    // skip any leading spaces
    while(*buffer==' ') buffer++;
    if(!((*buffer=='\r') || (*buffer==';') || (*buffer=='\n'))) found=1;
  }
  if(found) return(buffer);
  return(NULL);

}


/*
**    ProcessDialTable
**
** This attempts to parse a standard FDNODE.CTL Dial table. I'm sure it won't
** cope with anything fancy.
**
**    Parameters
**
**    Defaults    A pointer to the string which starts the table (that is the
**                default national and international dial translation).
**    control     A file pointer to the FDNODE.CTL from which to read the
**                subsequent commands.
**
*/
void ProcessDialTable(char * Defaults, FILE * control)
{
  char * p = Defaults;
  char * q;
  char Match[30];
  char XLT[30];
  int quit = 0;

  printf("(+) Processing DIAL translations\n");
  
  // Read the default data from the control line
  while(*p==' ') p++;
  q = XLT;
  while(*p!=' ' && *p!='\r' && *p!='\n'){
    *q = *p;
    q++; p++;
  }
  *q = 0;
  Nodelist->AddRecord("DOM", XLT, 0);
  
  while(*p==' ') p++;
  q = XLT;
  while(*p!=' ' && *p!='\r' && *p!='\n'){
    *q = *p;
    q++; p++;
  }
  *q = 0;
  Nodelist->AddRecord("INTL", XLT, 0);  
  
  while(!quit){
    p = ReadControlLine(control, Buffer);
    if(!p) quit = 1;
    else{
      if(!strnicmp(p, "END", 3)) quit=1;
      else{
        // Process Dial XLT line
        // Copy first segment
        q = Match;
        while(*p!=' ' && *p!='\r' && *p!='\n'){
          *q = *p;
          q++; p++;
        }
        *q = 0;

        while(*p==' ') p++;
        if(*p=='\r') *XLT=0;
        else{
          q = XLT;
          while(*p!=' ' && *p!='\r' && *p!='\n'){
            *q = *p;
            q++; p++;
          }
          *q = 0;
        }
        Nodelist->AddRecord(Match, XLT, 0);
      }
    }
  }
}


/*
**    ProcessCostTable
**
** This attempts to parse a standard FDNODE.CTL Cost table. I'm sure it won't
** cope with anything fancy.
**
**    Parameters
**
**    Defaults    A pointer to the string which starts the table (that is the
**                default national and international cost). THIS IS *NOT* used
**                at this stage. Purely because I cannot be bother to implement
**                the code requried to enter it with the dial codes above for a
**                simple test program such as this.
**    control     A file pointer to the FDNODE.CTL from which to read the
**                subsequent commands.
**
*/
void ProcessCostTable(FILE * control)
{
  char * p;
  char * q;
  char Match[30];
  char Cost[30];
  int quit = 0;

  printf("(+) Processing COST table\n");

  // Please note I do NOT bother with the defaults in this implementation

  while(!quit){
    p = ReadControlLine(control, Buffer);
    if(!p) quit = 1;
    else{
      if(!strnicmp(p, "END", 3)) quit=1;
      else{
        // Process Dial XLT line

        // Copy first segment
        q = Match;
        while(*p!=' ' && *p!='\r' && *p!='\n'){
          *q = *p;
          q++; p++;
        }
        *q = 0;

        while(*p==' ') p++;
        if(*p=='\r') *Cost=0;
        else{
          q = Cost;
          while(*p!=' ' && *p!='\r' && *p!='\n'){
            *q = *p;
            q++; p++;
          }
          *q = 0;
        }
        Nodelist->AddRecord(Match, "=", (unsigned short) atoi(Cost));
      }
    }
  }
}


/*
**    ProcessNodeFile
**
** This is a highly unintelligent NodeList parser, but it should serve the
** purpose for experimental compiling.
**
**    Parameters
**
**    Filename    The name of the file to compile, usually FDNET.PVT in some
**                path, or the official NODELIST.xxx file.
**
*/
void ProcessNodeFile(char * Filename, long int Whence)
{
  unsigned short Zone, Region, Net, Hub, Node, Point;
  unsigned short RNet, RNode;
  FILE *NodeFile;
  int quit = 0;
  char * p;
  unsigned long Number=0;
  char Status = 0;
  long Offset;

  Zone = Region = Net = Node = Hub = Point = RNet = RNode = 0;
  NodeFile = _fsopen(Filename, "r", SH_DENYWR);
  if(!NodeFile){
    perror("Unable to open Data file");
    // Cannot find file
    exit(3);
  }
  printf("(+) Compiling %s\n", Filename);
  while(!quit){
    p = ReadControlLine(NodeFile, Buffer);
    if(!p) quit = 1;
    else{
      Number++;
      if(*p!=','){

        if(!strnicmp(p, "Zone", 4)){
          Zone   = (unsigned short) atoi(p+5); RNet = 0; RNode = 0; Status = ISZC; Region = Node = Point = 0; Net = Zone;
        }
        if(!strnicmp(p, "Region", 6)){
          Region = (unsigned short) atoi(p+7); RNet = Net = Region; RNode = 0; Status = ISRC; Node = Point = 0;
        }
        if(!strnicmp(p, "Host", 4)){
          Net    = (unsigned short) atoi(p+5); RNet = Net; RNode = 0; Status = ISNC; Hub = Node = Point = 0;
        }
        if(!strnicmp(p, "Hub", 3)){
          Hub    = (unsigned short) atoi(p+4); RNet = Net; RNode = Hub; Status = ISHUB; Node = Hub;
        }
        if(!strnicmp(p, "Hold", 4)){
          Node   = (unsigned short) atoi(p+5); Status = ISHOLD; Point = 0;
        }
        if(!strnicmp(p, "Down", 4)){
          Node   = (unsigned short) atoi(p+5); Status = ISDOWN; Point = 0;
        }
        if(!strnicmp(p, "PVT", 3)){
          Node   = (unsigned short) atoi(p+4); Status = ISPVT; Point = 0;
        }
        if(!strnicmp(p, "Point", 5)){
          Point  = (unsigned short) atoi(p+6); Status = ISPOINT;
        }
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
      }
      else{
        p++;
        Node = (unsigned short) atoi(p);
        p = strchr(p, ',') + 1;
        Status = 0;
        RNet = Net; RNode = Hub;
        Point = 0;
      }
      // Ok, we're ready to dissect the line and add it to the Database
      Offset = ftell(NodeFile) - strlen(p) - 1;
      switch(Status){
        case ISNC:
          Nodelist->AddRecord(Zone, Net, Node, Point, Region, 0, Status, Whence, Offset);
          break;
        case ISPOINT:
          Nodelist->AddRecord(Zone, Net, Node, Point, Net, Node, Status, Whence, Offset);
          break;  
        default:
          Nodelist->AddRecord(Zone, Net, Node, Point, RNet, RNode, Status, Whence, Offset);
          break;
      }
      Nodelist->AddRecord(Zone, Net, Node, Point, strchr(strchr(p, ',')+1, ',')+1, Status, Whence, Offset);
      if(Status==ISZC) printf("%5lu Zone %5u\n",Number, Zone);
    }
  }
  fclose(NodeFile);
}


/*
**    ProcessPointFile
**
** This is a highly unintelligent PointList parser, but it should serve the
** purpose for experimental compiling.
**
**    Parameters
**
**    Filename    The name of the file to compile, usually FDPOINT.PVT in some
**                path.
**
*/
void ProcessPointFile(char * Filename)
{
  unsigned short Zone, Node, Net, Point;
  FILE *NodeFile;
  int quit = 0;
  char *p;
  long Offset;

  Zone = Net = Node = Point = 0;
  NodeFile = _fsopen(Filename, "r", SH_DENYWR);

  if(!NodeFile){
    perror("Unable to open Data file");
    // Cannot find file
    exit(3);
  }
  printf("(+) Compiling %s\n", Filename);
  while(!quit){
    p = ReadControlLine(NodeFile, Buffer);
    if(!p) quit = 1;
    else{
      if(*p!=','){
        if(!strnicmp(p, "Boss", 4)){
          p = strchr(p, ',') + 1;
          Zone = (unsigned short) atoi(p);
          p = strchr(p, ':') + 1;
          Net  = (unsigned short) atoi(p);
          p = strchr(p, '/') + 1;
          Node = (unsigned short) atoi(p);

          // Ok, read next line
          p = ReadControlLine(NodeFile, Buffer);
        }

        if(!strnicmp(p, "Hold", 4) || !strnicmp(p, "Down", 4)){
          p+=4;
        }
      }
      if(*p!=','){
        printf("(!) Did not understand PointList line\n[%s]\n", p);
        exit(12);
      }
      else{
        p++;
        Point = (unsigned short) atoi(p);
        p = strchr(p, ',') + 1;
      }
      // Ok, we're ready to dissect the line and add it to the Database
      Offset = ftell(NodeFile) - strlen(p) - 1;

      // Add data to NODELIST.FDX
      Nodelist->AddRecord(Zone, Net, Node, Point, Net, Node, ISPOINT, WFDNPoint, Offset);
      // Add data to USERLIST.FDX
      Nodelist->AddRecord(Zone, Net, Node, Point, strchr(strchr(p, ',')+1, ',')+1, ISPOINT, WFDNPoint, Offset);
    }
  }
  fclose(NodeFile);
}
          
                       
/*
**    GetNodelistDetails
**
** Most nodelists included are done so in wildcarded form, eg. NODELIST.*.
** This function is used to pass in these nodelist names, and attempts to find
** the "best" match.
**
** It will examine only entries with numeric extensions (to avoid NODELIST.FDX
** for example), and then will consider the entry with greatest numeric value.
**
** It can be used to determine the index in question, (needed for the index
** set), and also the actual pathname of the file it did in the end select.
**
**    Parameters
**
**    filename    The filemask for the nodelist file in question
**    buffer      A region of memory to hold the found filename
**
**    Returns
**
**    0xFFFF      No relevant file was found
**    other       The numeric extension of the found file, if relevant
**
**/
unsigned short GetNodelistDetails(char * filename, char * buffer)
{
  // Takes a stem name and determines the nodelist extension
  char          Stub[72];
  char          BestFile[72];
  char          extension[4];
  struct        find_t found;
  int           done=0;
  int           IsFound=0;
  int           BestIndex=0;
  char *        p;

  // Get the directory name (if any) and put it in Stub
  strcpy(Stub, filename);
  p = strrchr(Stub, '\\');
  if(p) *p=0; else *Stub = 0;

  done=_dos_findfirst(filename, 0, &found);
  while(!done){
    // Let's have a look at the extension
    if(strstr(found.name, ".")){
      strcpy(extension, strstr(found.name, ".")+1);
      if(isdigit(extension[0]) && isdigit(extension[1]) && isdigit(extension[2])){
        // We have an entry with a numeric extension
        IsFound=1;
        if(atoi(extension) > BestIndex){
          strcpy(BestFile, found.name);
          BestIndex=atoi(extension);
        }
      }
    }
    done=_dos_findnext(&found);
  }
  
  if(!IsFound){
    printf("(!) Error, no file found matching %s\n", filename);
    return(0xFFFFU);
  }
  else{
    if(buffer) sprintf(buffer, "%s\\%s", Stub, BestFile);
    return((unsigned short) BestIndex);
  }
}


/*
**    AddToPrivate
**
** Takes a filename which it is fed and adds it to the end of the FDNET.PVT
** file.
**
**    Parameters
**
**    Parameter   The string front FDNODE.CTL, which may have leading and/or
**                trailing spaces.
**
**    Returns
**
**    1 on success, 0 on failure
**
**/
int AddToPrivate(char * Parameter){
  char * StemName;
  char ExpandedName[72];
  char PrivateList[72];
  unsigned short ext;

  while(*Parameter == ' ') Parameter++;
  StemName = strtok(Parameter, "\n\r ");

  sprintf(PrivateList, "%sFDNET.PVT", FDNodelistDir);

  if(!strstr(StemName, "*") && !strstr(StemName, "?")) return (AppendFile(StemName, PrivateList, ""));

  // Ok, we probably need to look for a numeric extension in this case

  ext = GetNodelistDetails(StemName, ExpandedName);
  if(ext != 0xFFFF){
    return(AppendFile(ExpandedName, PrivateList, ""));
  }
  return(0);
}


/*
**    AddToPoint
**
** Takes a filename which it is fed and adds it to the end of the FDPOINT.PVT
** file.
**
**    Parameters
**
**    Parameter   The string front FDNODE.CTL, which may have leading and/or
**                trailing spaces.
**
**    Returns
**
**    1 on success, 0 on failure
**
**/
int AddToPoint(char * Parameter){
  char * StemName;
  char * Boss;
  char ExpandedName[72];
  char PrivateList[72];
  char Header[72]="";
  unsigned short ext;

  while(*Parameter == ' ') Parameter++;
  StemName = strtok(Parameter, "\n\r ");
  Boss     = strtok(NULL, "\n\r ");

  sprintf(PrivateList, "%sFDPOINT.PVT", FDNodelistDir);
  if(Boss) sprintf(Header, "Boss,%s\r\n", Boss);

  if(!strstr(StemName, "*") && !strstr(StemName, "?")) return (AppendFile(StemName, PrivateList, Header));

  // Ok, we probably need to look for a numeric extension in this case

  ext = GetNodelistDetails(StemName, ExpandedName);
  if(ext != 0xFFFF){
    return(AppendFile(ExpandedName, PrivateList, Header));
  }
  return(0);
}


/*
**    AppendFile
**
** Appends a file to an existing file, optionally adding a brief header line.
**
**    Parameters
**
**    FileName    The file to append
**    Existing    The file to add FileName to
**    Header      The option header line which needs to be written    
**
**    Returns
**
**    1 on success, 0 on failure
**
**/
int AppendFile(char * FileName, char * Existing, char * Header)
{
  FILE * NewSection, * ExistingSection;
  char * TransferBuffer = new char[4096];
  size_t BytesCopied;

  if(!TransferBuffer) return(0);

  NewSection = _fsopen(FileName, "rb", SH_DENYNO);
  if(!NewSection) return(0);

  if(!CheckFile(Existing)) ExistingSection = _fsopen(Existing, "wb", SH_DENYWR);
  else ExistingSection = _fsopen(Existing, "r+b", SH_DENYWR);
  if(!ExistingSection){
    perror("Can't open file");
    return(0);
  }

  fseek(ExistingSection, 0, SEEK_END);

  printf("  - Incorporating %s\n", FileName);

  fprintf(ExistingSection, "%s", Header);

  while(!feof(NewSection)){
    BytesCopied = fread(TransferBuffer, 1, 4096, NewSection);
    if(BytesCopied){
      if(TransferBuffer[BytesCopied - 1] == 0x1A) BytesCopied--;
      fwrite(TransferBuffer, 1, BytesCopied, ExistingSection);
    }
  }

  fprintf(ExistingSection, "\r\n");

  fclose(NewSection);
  fclose(ExistingSection);

  delete TransferBuffer;
  return(1);
}


/*
**    CheckFile
**
** Function to check for the existance of a file. This function is
** Netware friendly (it finishes all initiated searches).
**
**    Parameters
**
**    filename  The filename to check for.
**
**    Returns
**
**    A pointer to a find_t structure on a find, or NULL on failure.
*/
struct find_t *CheckFile(char * filename)
{
  static struct find_t found;
  struct find_t dummy;

  if(!_dos_findfirst(filename, 0, &dummy)){
    // We found at least one matching filename, preserve the real details
    memcpy(&found, &dummy, sizeof(struct find_t));
    while(!_dos_findnext(&dummy));
    return(&found);
  }
  else return(NULL);
}
