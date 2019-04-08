
/****************************************************************************/
/*                                                                          */
/* Definitions for C and C++ code to read the FrontDoor (TM) nodelist       */
/* indices.                                                                 */
/*                                                                          */
/****************************************************************************/
/* Please see FDNODE.DOC for more details on how to use the code.           */
/****************************************************************************/

/****************************************************************************/
/*  PLEASE READ !! PLEASE READ !! PLEASE READ !! PLEASE READ !! PLEASE READ */
/****************************************************************************/
/* This code is under active development by Colin Turner, 2:443/13.0, if    */
/* you feel a change should be made PLEASE ask me so that I can coordinate  */
/* changes and ensure the coherency of the code.                            */
/*                                                                          */
/* Apart from that, if you use the code in your application, I would like to*/
/* hear from you to see how far it gets, and I would appreciate a mention   */
/* in any relevant credit sections.                                         */
/****************************************************************************/

/****************************************************************************/
/*     Thanks are due to Bill Birrell, Tobias Burchhardt, John Ellin        */
/*              Joaquim Homrighausen and Mats Wallin                        */
/****************************************************************************/

/****************************************************************************/
/*           Copyright Colin Turner, 1995, 1996, 1997, 1998                 */
/*                        All Rights Reserved                               */
/****************************************************************************/


#ifndef __FDNODE_H
#define __FDNODE_H

#define FrontDoorNode_CRL       4.27  // Internal class revision level
#define FrontDoorWNode_CRL      1.03  // Write class level

// Load user related details on how to compile code
#include "fdnuser.h"

/************* NO USER SERVICABLE PARTS BELOW THIS LINE :-) *****************/

#define FDNDATA
#define FDNFUNC
#define FDNPREF


#ifdef FDN_DLL
  #define strcpy(a,b) lstrcpy(a,b)
  #define strcat(a, b) lstrcat(a, b)
  #undef FDNFUNC
  #undef FDNPREF
  #ifdef WIN32
         #define FDNFUNC
         #define FDNPREF _declspec( dllexport )
  #else
         #define FDNFUNC far pascal _export
         #define FDNPREF _declspec( dllexport )
  #endif
#endif


#if ((defined(FDN_USEIOS) && defined(FDN_USESTD)) || (defined(FDN_USEIOS) && defined(FDN_USEHAND)) || (defined(FDN_USESTD) && defined(FDN_USEHAND)) || (defined(FDN_USEUSER) && defined(FDN_USESTD)) || (defined(FDN_USEUSER) && defined(FDN_USEHAND)) || (defined(FDN_USEUSER) && defined(FDN_USEIOS)))
  #error Only one IO system may be defined
#endif

#ifndef FDN_USEIOS
  #ifndef FDN_USESTD
     #ifndef FDN_USEHAND
       #ifndef FDN_USEUSER
                #error You must define one IO system to compile under
       #endif         
     #endif
  #endif
#endif

#ifndef FDN_USEUSER
  #ifdef FDN_INST_USESTD
    #error You may only use a FDN_INST_USE define under USER IO
  #endif  
  #ifdef FDN_INST_USEIOS
    #error You may only use a FDN_INST_USE define under USER IO
  #endif  
  #ifdef FDN_INST_USEHAND
    #error You may only use a FDN_INST_USE define under USER IO
  #endif  
#endif

#if ( (defined(FDN_INST_USESTD) && defined(FDN_INST_USEHAND)) || (defined(FDN_INST_USESTD) && defined(FDN_INST_USEIOS)) || (defined(FDN_INST_USEHAND) && defined(FDN_INST_USEIOS)) )
  #error Only one IO system for semaphored may be defined
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#if defined(__MSDOS__) || defined(__DOS__) || defined(__NT__)
  #include <dos.h>
#endif  


#if   (defined(FDN_USEIOS) || defined(FDN_INST_USEIOS))
  #include <iostream.h>
  #include <fstream.h>
  #include <stdio.h>
#elif (defined(FDN_USESTD) || defined(FDN_INST_USESTD))
  #include <stdio.h>
#elif (defined(FDN_USEHAND) || defined(FDN_INST_USEHAND))
  #include <io.h>
  #include <fcntl.h>
  #include <dos.h>
#endif


/* Defines maximum height of index BTrees. According to NODELIST.H this can */
/* be as much as 5, but in practice I have not found it to exceed 4.        */

#define MAXHEIGHT 5

/* Maximum length of nodelist line to be allowed */

#define NODELINELENGTH  255

/* Number of characters to allow for full pathnames */

#define PATHLENGTH      72

/* Some flags used by C++ class, most of these give control over what files */
/* will be held open for the lifetime of the class, and which will be       */
/* opened only on requirement. The more you keep open the greater the speed */
/* but obviously the greater the number of file handles required.           */

const int FDNodeNFDX          =0x0001;  /* Reopen NODELIST.FDX each time */
const int FDNodeUFDX          =0x0002;  /* Reopen USERLIST.FDX each time */
const int FDNodeNode          =0x0004;  /* Reopen NODELIST.??? each time */
const int FDNodePvt           =0x0008;  /* Reopen FDNODE.PVT each time */
const int FDNodePoint         =0x0010;  /* Reopen FDPOINT.PVT each time */
const int FDNodeFDA           =0x0020;  /* Reopen FDNODE.FDA each time */
const int FDNodePhone         =0x0040;  /* Reopen PHONE.FDA each time */
const int FDNodePFDX          =0x0080;  /* Reopen PHONE.FDX each time */
const int FDNodeNoCacheN      =0x0100;  /* Don't keep NODELIST.FDX root in memory */
const int FDNodeNoCacheU      =0x0200;  /* Don't keep USERLIST.FDX root in memory */
const int FDNodeNoCacheP      =0x0400;  /* Don't keep PHONE.FDX root in memory */
const int FDNodeNoSem         =0x1000;  /* Don't support FDNODE*.* semaphores */
const int FDNodeCreateFrozen  =0x8000;  /* Initialise Class in "Frozen" form */

/* Useful combinations */
/* These don't prevent the lookups specified, but they optimize on the */
/* basis that such searches will be at best very rare                  */

/* No Cost / Dial translation required */

const int FDNodeNoPhone   = FDNodePhone | FDNodePFDX | FDNodeNoCacheP;

/* No User lookups required */

const int FDNodeNoUser    = FDNodeUFDX | FDNodeNoCacheU;

/* No Node lookups required */

const int FDNodeNoNode    = FDNodeNFDX | FDNodeNoCacheN;

/* Structure of Pages in NODELIST.FDX */
/* Each page is made of 32 records, as detailed below */

#ifdef FDN_PACK
#pragma pack(1)
#endif

struct NFDXRecord {
  union {
    unsigned long int loff;
    char              coff[4];
  } offset;                       /* Offset in Nodelist files */
  unsigned long int link;         /* Link to other pages (if any) */
  char key[1];                    /* Always 14 bytes */
  unsigned short int zone;
  unsigned short int net;
  unsigned short int node;
  unsigned short int point;
  unsigned short int rnet;        /* Net to route to (default)  */
  unsigned short int rnode;       /* Node to route to (default) */
  char nodetype;
  unsigned char esmark;
};

struct NFDXPage {
  char              records;               /* Number of valid records in this page */
  unsigned long int backref;               /* Pointer to previous page */
  struct            NFDXRecord nodes[32];  /* The Node keys and offsets */
};

#ifdef FDN_PACK
#pragma pack()
#endif

/* This is the format of the first, special page in NODELIST.FDX */
/* USERLIST.FDX does not contain most of this, but it does have  */
/* a first, special page                                         */

#ifdef FDN_PACK
#pragma pack(1)
#endif

#if 0
struct FirstPage {
  unsigned long flags;
  unsigned long start;
  unsigned long index;        /* Points to first index page */
  unsigned long pagelen;      /* Length of each page in index */
};
#endif

struct FirstPage {
  unsigned long flags;
  unsigned long start;
  long index;        /* Points to first index page */
  unsigned long pagelen;      /* Length of each page in index */
};

struct ExtendedPage
{
  struct         FirstPage stub;
  char           unknown[240];
  char           nodeext[4];            /* Current official nodelist extension */
  unsigned char  RevisionMaj;
  unsigned char  RevisionMin;
  char           swedish;
};

struct StubInfo
{
  char           NodeExt[4];
  unsigned char  RevisionMaj;
  unsigned char  RevisionMin;
  char           Swedish;
  char           Reserved[9];
  unsigned short ZeroWord;
  unsigned short CountryCode;
  unsigned long  CompileTime;
};

  

#ifdef FDN_PACK
#pragma pack()
#endif


/* Format of pages and record in USERLIST.FDX */

#ifdef FDN_PACK
#pragma pack(1)
#endif

struct UFDXRecord {
  unsigned long      offset;
  unsigned long      link;
  char               key[16];
  unsigned short int zone;
  unsigned short int net;
  unsigned short int node;
  unsigned short int point;
  char nodetype;
};

struct UFDXPage {
  char              records;
  unsigned long int backref;
  struct            UFDXRecord names[32];
};

#ifdef FDN_PACK
#pragma pack()
#endif

/* Format of pages and record in PHONE.FDX */

#ifdef FDN_PACK
#pragma pack(1)
#endif

struct PFDXRecord {
  unsigned long offset;
  unsigned long link;
  char key[20];         /* Pascal string for key */
  char unknown;
};

struct PFDXPage {
  char records;
  unsigned long backref;
  struct PFDXRecord phones[32];
};

#ifdef FDN_PACK
#pragma pack()
#endif

#ifdef FDN_PACK
#pragma pack(1)
#endif

/* This structure is found at offset 0x100 in NODELIST.FDX and PHONE.FDX */
/* But only in databases compiled for FD2.30 above or equivalent         */

struct NLinfoRec
{
  unsigned short  ZeroWord;
  unsigned short  CountryCode;
  unsigned long   CompileTime;
  char            RESERVED[26];
};
  

#ifdef FDN_PACK
#pragma pack()
#endif

/*
**  The private Nodelist Database (FDNODE.FDA) has the following record
**  format. Note that the char[] fields are in Pascal fashion. The first
**  byte is the length byte. The actual string starts at [1] and the
**  string is NOT NUL terminated.
*/

  /* Status */

#define ISZC        1
#define ISRC        2
#define ISNC        3
#define ISHUB       4
#define ISPVT       5
#define ISHOLD      6
#define ISDOWN      7
#define ISPOINT     9

  /* Capability flags */

#define OldNLflagCM      0x00000002L
#define OldNLflagMO      0x00000004L
#define OldNLflagHST     0x00000008L
#define OldNLflagH96     0x00000010L
#define OldNLflagPEP     0x00000020L
#define OldNLflagMAX     0x00000040L
#define OldNLflagXX      0x00000080L
#define OldNLflagXB      0x00000100L
#define OldNLflagXR      0x00000200L
#define OldNLflagXP      0x00000400L
#define OldNLflagXW      0x00000800L
#define OldNLflagMNP     0x00001000L
#define OldNLflagHST14   0x00002000L
#define OldNLflagV32     0x00004000L
#define OldNLflagV33     0x00008000L
#define OldNLflagV34     0x00010000L
#define OldNLflagV42     0x00020000L
#define OldNLflagXC      0x00040000L
#define OldNLflagXA      0x00080000L
#define OldNLflagV42B    0x00100000L
#define OldNLflagV32B    0x00200000L
#define OldNLflagHST16   0x00400000L
#define OldNLflagLO      0x00800000L
#define OldNLflagZYX     0x01000000L
#define OldNLflagUISDNA  0x02000000L
#define OldNLflagUISDNB  0x04000000L
#define OldNLflagUISDNC  0x08000000L
#define OldNLflagFAX     0x10000000L


#define  NLflagCM      0x00000001L
#define  NLflagMO      0x00000002L
#define  NLflagLO      0x00000004L
#define  NLflagMN      0x00000008L

#define  NLflagV32     0x00000010L
#define  NLflagV32B    0x00000020L
#define  NLflagV42     0x00000040L
#define  NLflagV42B    0x00000080L
#define  NLflagV34     0x00000100L

#define  NLflagZYX     0x00000400L
#define  NLflagHST     0x00000800L
#define  NLflagFAX     0x00001000L
#define  NLflagX2C     0x00002000L
#define  NLflagX2S     0x00004000L

#define  NLflagXA      0x00020000L
#define  NLflagXB      0x00040000L
#define  NLflagXC      0x00080000L
#define  NLflagXP      0x00100000L
#define  NLflagXR      0x00200000L
#define  NLflagXW      0x00400000L
#define  NLflagXX      0x00800000L

#define  NLflagX75     0x01000000L
#define  NLflagV110L   0x02000000L
#define  NLflagV110H   0x04000000L
#define  NLflagV120L   0x08000000L
#define  NLflagV120H   0x10000000L


  /* MaxBaud field */

#define ISBAUD300       2
#define ISBAUD1200      4
#define ISBAUD2400      5
#define ISBAUD4800      6
#define ISBAUD7200      10
#define ISBAUD9600      7
#define ISBAUD12000     11
#define ISBAUD14400     12
#define ISBAUD16800     13
#define ISBAUD19200     14
#define ISBAUD38400     15
#define ISBAUD57600     16
#define ISBAUD64000     17
#define ISBAUD76800     18
#define ISBAUD115200    19

  /* Record structure */

  /* Note that while the private database can only hold a fixed amount of
     information about a system's capabilities (nodelist flags), FrontDoor
     is capable of using the actual string present in FidoNet-style node-
     lists for routing and other lookup purposes. */

/* This structure is also used by the FrontDoorNode class to fill with */
/* fetched data. This allows a consistent interface on all later       */
/* handling                                                            */

#ifdef FDN_PACK
#pragma pack(1)
#endif

struct FDANodeRec
{
    long int            Erased;             /*Used to signal erased status*/
    unsigned char       Status;                    /*Zone, host, hub, etc.*/
    unsigned short int  NodeNo,                          /*Network address*/
                        NetNo,
                        Zone,
                        Point,
                        RoutNode,            /*Default routing within zone*/
                        RoutNet,
                        Cost;                 /*Cost per minute for system*/
    long int            Capability;                     /*Capability flags*/
    unsigned char       MaxBaud;                       /*Maximum baud rate*/
    char                Name[31];                         /*Name of system*/
    char                Telephone[41];              /*Raw telephone number*/
    char                Location[41];                 /*Location of system*/
    char                User[37];                             /*SysOp name*/
    char                SelectTag[4];                        /*Group field*/
};

#ifdef FDN_PACK
#pragma pack()
#endif

/*
**  The telephone number database (FDPHONE.FDA) has the following record
**  format. Note that the char[] fields are in Pascal fashion. The first
**  byte is the length byte. The actual string starts at [1] and the
**  string is NOT NUL terminated.
*/

#ifdef FDN_PACK
#pragma pack(1)
#endif

struct FDNPhoneRec
{
         long                  Erased;             /*Used to signal erased status*/
         char                  Telephone[41];          /*Phone number translation*/
         unsigned short int    Cost;                   /*Cost per minute of calls*/
         unsigned short int    Baudrate;             /*Max baudrate for this area*/
};

#ifdef FDN_PACK
#pragma pack()
#endif


#ifndef __cplusplus

/* See explanations for the C++ class equivalent below */

#ifdef FDN_FULL_PACK
#pragma pack(1)
#endif


struct FDNFind {
  char           key[16];
  int            searchtype;
  long           page[MAXHEIGHT];
  int            record[MAXHEIGHT];
  int            maxrec[MAXHEIGHT];
  int            level;
  long           offset;
  unsigned short zone;
  unsigned short net;
  unsigned short node;
  unsigned short point;
  unsigned short rnet;
  unsigned short rnode;
  int            status;
  int            finished;
  time_t         UnixStamp;
};

#ifdef FDN_FULL_PACK
#pragma pack()
#endif

#endif

#ifdef __cplusplus

class FrontDoorNode;
class FDNFind;
class FDN_FileObject;

// This is a generic file object, used by the class for file io, this helps
// clean the source code with respect to the three different file systems
// and also allows a derived class to trap errors in file io.

#ifdef FDN_FULL_PACK
#pragma pack(1)
#endif

class FDNFile {

  protected :

    char  FileName[PATHLENGTH];      // Filename.
    int   Status;                    // Whether the file is open, etc.
    int   Error;                     // The last error code. 0 = no error.
    int   Flags;                     // Flags data

  public :

    // Error support functions

    FDNPREF            int FDNFUNC GetError() { return(Error); }
    FDNPREF   virtual void FDNFUNC ClearError() { Error=0; }

  protected :
  
    FDNPREF   virtual void FDNFUNC SignalError(int errcode) {Error = errcode; }

  public :

    // File support functions
    FDNPREF           void FDNFUNC SetName(const char * name) {if(!Status) strcpy(FileName, name);} // Set filename
    FDNPREF            int FDNFUNC GetStatus() { return(Status); }                            // Is file open
    FDNPREF           void FDNFUNC SetFlags(int newFlags) {Flags = newFlags; }

    FDNPREF            int FDNFUNC Open();
    FDNPREF            int FDNFUNC Close();
    FDNPREF       long int FDNFUNC Size();
    FDNPREF            int FDNFUNC Seek(long offset, int whence);
    FDNPREF            int FDNFUNC Read(void * address, size_t size, size_t items, int ErrSensitive);
    FDNPREF            int FDNFUNC Write(void * address, size_t size, size_t items, int ErrSensitive);

    // Constructors etc.
    FDNFile();
    virtual ~FDNFile();
  
    // File system specific bit.

    protected :

    #if defined(FDN_USEIOS)
    fstream Data;
    #elif defined(FDN_USESTD)
    FILE * Data;
    #elif defined(FDN_USEHAND)
    int Data;
    #elif defined(FDN_USEUSER)
    FDN_UserFileObject Data;
    #endif
};



#include "fdnuser.h"


// This is as the above FDNFind structure, but if you're using C++ you may as well
// benefit from the extra security to prevent you corrupting the structure.

// Notes.

// 1. The class stores locations of matches in the BTree in order to find
// the next match. Entries in the page and record array beyond the level
// defined are not valid.

// 2. If the variable offset is zero an error has occured, use the GetError()
// funtion of the main FrontDoorNode class to get more information.

// 3. GetRNet() and GetRNode() are invalid in username searches, and will, in
// that case, return zero.

class FDNFind {

  protected :

    char            key[16];             /* Key being searched for */
    int             searchtype;          /* For Zone, net... searches */
    long            offset;              /* Offset of last match */
    unsigned short  zone;                /* Nodelist data of last match */
    unsigned short  net;
    unsigned short  node;
    unsigned short  point;
    unsigned short  rnet;                /* Routing info from NODELIST.FDX */
    unsigned short  rnode;

  private :

    long            page[MAXHEIGHT];     /* To allow speedy location of next key */
    int             record[MAXHEIGHT];
    int             maxrec[MAXHEIGHT];
    int             level;
    int             status;
    int             finished;
    time_t          UnixStamp;           /* Time of last update  */
    class FrontDoorNode FDNDATA *Parent; /* Class using this one */
                        
  protected :

    // A Filter which dictates whether an fblock is a valid return
    FDNPREF virtual int FDNFUNC Filter() { return(1); }

  public :

    FDNPREF long            FDNFUNC GetOffset() { return(offset); }
    FDNPREF unsigned short  FDNFUNC GetZone()   { return(zone); }
    FDNPREF unsigned short  FDNFUNC GetNet()    { return(net); }
    FDNPREF unsigned short  FDNFUNC GetNode()   { return(node); }
    FDNPREF unsigned short  FDNFUNC GetPoint()  { return(point); }
    FDNPREF unsigned short  FDNFUNC GetRNet()   { if(searchtype) return(rnet); else return(0); }
    FDNPREF unsigned short  FDNFUNC GetRNode()  { if(searchtype) return(rnode); else return(0); }
    FDNPREF int             FDNFUNC GetStatus() { return(status); }
    FDNPREF int             FDNFUNC IsFDA()     { return( ( (int) ( (offset & 0xFF000000L) >> 24))==0x01); }

    // The following functions merely call those in the parent class
    FDNPREF char FDNFUNC            *GetSysop();
    FDNPREF char FDNFUNC            *GetLocation();
    FDNPREF char FDNFUNC            *GetSysName();
    FDNPREF unsigned long FDNFUNC   GetSpeed();
    FDNPREF char FDNFUNC            *GetNumber();
    FDNPREF char FDNFUNC            *GetFlags();
    FDNPREF long FDNFUNC            GetFDAFlags();
    FDNPREF unsigned short FDNFUNC  GetPhoneData(char * buffer);

    // A few convenient Operator overloads
    FDNPREF FDNFind FDNFUNC         operator ++ ();
    FDNPREF FDNFUNC                 operator int() { return(!finished); }

    // Get and Set Index offsets
    FDNPREF long FDNFUNC            GetIndexOffset();
    FDNPREF int  FDNFUNC            SetIndexOffset(long newoffset);         
    FDNPREF int  FDNFUNC            SetIndexOffset(long newoffset, FrontDoorNode & newparent);
          
  friend class FrontDoorNode;

};

/****************************************************************************/
/* Please read FDNODE.DOC for documentation on the usage of this class      */
/****************************************************************************/

class FrontDoorNode {


  // Attributes
  private :
    
    // Internal data, file control data etc.
    int                Reopen[4];                                        // Which files to reopon on demand
    unsigned int       Instance;                                         // A serial number of the class on the
    int                Task;                                             // Task the machine is running on.
    int                NLDBRevision;                                     // What sort of database we are using (2.20 / 2.30)
    int                Frozen;
    char               NodelistLine[NODELINELENGTH];
    char               NodelistDir[PATHLENGTH], SemaphoreDir[PATHLENGTH];
    char               InstanceSemaphore[PATHLENGTH];
    long               NLCurOffset;
    long               INTLOffset, DOMOffset;
    NFDXPage FDNDATA   *nroot;
    UFDXPage FDNDATA   *uroot;
    PFDXPage FDNDATA   *proot;
    FDANodeRec FDNDATA *FDAStore;
    unsigned long      FDAStoreSpeed;
    NLinfoRec          NLInfo;
    FirstPage          first_n, first_u, first_p;
    time_t             UnixStamp;
    #ifndef            FDN_NoFlagBuild
    char               FlagBuild[100];
    #endif

  protected :                        

    FDN_FileObject     NFDX, UFDX, PFDX, PFDA;
    FDN_FileObject     DataFile[4];

    int                Flags, error, swedish;
    char               NodeExt[4];
    FDNPREF time_t FDNFUNC GetUnixStamp() { return(UnixStamp); }

  // Services
  public :
  
    // Nodelist Database info
    FDNPREF unsigned short FDNFUNC GetCountryCode()  {return(NLInfo.CountryCode); }
    FDNPREF            int FDNFUNC GetNLDBRevision() {return(NLDBRevision); }
    FDNPREF  unsigned long FDNFUNC GetCompileTime()  {return(NLInfo.CompileTime); }
    FDNPREF           char FDNFUNC *GetNodeExt();
  
    // Getting nodelist offsets for addresses and usernames
    FDNPREF            int FDNFUNC Find(FDNFind& fblock, unsigned short int zone, unsigned short int net, unsigned short int node, unsigned short int point);
    FDNPREF            int FDNFUNC Find(FDNFind& fblock, const char FDNDATA *username);
    FDNPREF            int FDNFUNC Find(FDNFind& fblock);
    FDNPREF            int FDNFUNC GetZones(FDNFind& fblock);
    FDNPREF            int FDNFUNC GetZones(FDNFind& fblock, unsigned short start);
    FDNPREF            int FDNFUNC GetNets(FDNFind& fblock, unsigned short zone);
    FDNPREF            int FDNFUNC GetNets(FDNFind& fblock, unsigned short zone, unsigned short start);
    FDNPREF            int FDNFUNC GetNodes(FDNFind& fblock, unsigned short zone, unsigned short net);
    FDNPREF            int FDNFUNC GetNodes(FDNFind& fblock, unsigned short zone, unsigned short net, unsigned short start);
    FDNPREF            int FDNFUNC GetPoints(FDNFind& fblock, unsigned short zone, unsigned short net, unsigned short node);
    FDNPREF            int FDNFUNC GetPoints(FDNFind& fblock, unsigned short zone, unsigned short net, unsigned short node, unsigned short start);

    // Getting the relevant nodelist line
    FDNPREF           char FDNFUNC *GetNLine(long offset);
    FDNPREF           char FDNFUNC *GetNLine(FDNFind& fblock);
    FDNPREF           void FDNFUNC ClearFDAStore(void);

    // Getting data from the nodelist line / FDNode.FDA structure
    FDNPREF           char FDNFUNC *GetSysop(FDNFind& fblock);
    FDNPREF           char FDNFUNC *GetLocation(FDNFind& fblock);
    FDNPREF           char FDNFUNC *GetSysName(FDNFind& fblock);
    FDNPREF  unsigned long FDNFUNC GetSpeed(FDNFind& fblock);
    FDNPREF           char FDNFUNC *GetNumber(FDNFind& fblock);
    FDNPREF           char FDNFUNC *GetFlags(FDNFind& fblock);
    #ifdef FDN_NoFlagBuild
    FDNPREF           long FDNFUNC GetFDAFlags(FDNFind& fblock);
    #endif
    FDNPREF unsigned short FDNFUNC GetPhoneData(FDNFind & fblock, char * buffer);

  private :
  
    FDNPREF           long FDNFUNC GetNFDXOffset(char FDNDATA *key, long page, FDNFind& fblock);
    FDNPREF           long FDNFUNC GetUFDXOffset(char FDNDATA *key, long page, FDNFind& fblock);
    FDNPREF unsigned short FDNFUNC GetPFDXData(char * SearchKey, char * buffer, long page);
    FDNPREF inline unsigned short FDNFUNC SwapBytes(unsigned short initial){ return((unsigned short) (((initial&0xFF00) >> 8) + ((initial&0x00FF) << 8)) ); };
    FDNPREF           int  FDNFUNC GetNextZone(FDNFind& fblock);
    FDNPREF           int  FDNFUNC GetNextNet(FDNFind& fblock);
    FDNPREF           int  FDNFUNC GetNextNode(FDNFind& fblock);
    FDNPREF           int  FDNFUNC GetNextPoint(FDNFind& fblock);
    FDNPREF           int  FDNFUNC UGetNextKey(FDNFind& fblock);
    FDNPREF           int  FDNFUNC NGetNextKey(FDNFind& fblock);
    FDNPREF           int  FDNFUNC NGetNextKey(FDNFind& fblock, NFDXPage FDNDATA *cnd);
    FDNPREF           char FDNFUNC *MakeKey(unsigned short zone, unsigned short net, unsigned short node, unsigned short point);
    FDNPREF           char FDNFUNC *MakeKey(unsigned short zone, unsigned short net, unsigned short node, unsigned short point, int);
    FDNPREF           int  FDNFUNC CompareKey(char FDNDATA *key1, char FDNDATA *key2);
    FDNPREF           int  FDNFUNC CompareKey(const char FDNDATA *key1, const char FDNDATA *key2, int MaxLen);
    FDNPREF           void FDNFUNC GetPrefixNumber(char FDNDATA * XLT, char FDNDATA * Buffer);
    FDNPREF           void FDNFUNC GetSuffixNumber(char FDNDATA * XLT, char FDNDATA * Buffer);
    FDNPREF           int  FDNFUNC GetNFDXPage(NFDXPage& nd, long pageno);
    FDNPREF           int  FDNFUNC GetUFDXPage(UFDXPage& ud, long pageno);
    FDNPREF           int  FDNFUNC GetPFDXPage(PFDXPage& pd, long pageno);
    FDNPREF           int  FDNFUNC GetPFDAPage(FDNPhoneRec& rd, long pageno);

    FDNPREF    virtual int FDNFUNC CheckNFDXCache(NFDXPage & , long ) { return(0); }
    FDNPREF   virtual void FDNFUNC CommitNFDXCache(NFDXPage & , long ) { return; }
    FDNPREF    virtual int FDNFUNC CheckUFDXCache(UFDXPage & , long ) {return(0); }
    FDNPREF   virtual void FDNFUNC CommitUFDXCache(UFDXPage & , long ) {return; }

    FDNPREF           char FDNFUNC *CSVField(char FDNDATA *Input, int field, char FDNDATA *tofill);
    FDNPREF           char FDNFUNC *CSVFieldStart(char FDNDATA *Input, int field);

    FDNPREF  unsigned long FDNFUNC GetSpeedFromFDA(unsigned char maxbaud);                        
    #ifndef FDN_NoFlagBuild
    FDNPREF           char FDNFUNC *GetFlagsFromFDA(unsigned long flags);
    #endif


  // Implementation
  public:
    // Contructors, Destructors
    FrontDoorNode();
    FrontDoorNode(const char FDNDATA *pathname, const char FDNDATA *path2, unsigned short NewTask);
    FrontDoorNode(const char FDNDATA *pathname, const char FDNDATA *path2, unsigned short setflags, unsigned short NewTask);
    virtual ~FrontDoorNode();

    // Control over nodelist dir and whether files are open
    FDNPREF           void FDNFUNC SetNLDir(const char FDNDATA *pathname);
    FDNPREF           void FDNFUNC SetSemDir(const char FDNDATA *pathname);
    FDNPREF           void FDNFUNC Freeze();
    FDNPREF            int FDNFUNC Thaw();
    FDNPREF            int FDNFUNC IsFrozen();
    FDNPREF            int FDNFUNC AutoFreezeThaw();
    FDNPREF           void FDNFUNC SetCountry(unsigned short cc) { NLInfo.CountryCode=cc; }
    FDNPREF           void FDNFUNC SetTask(int NewTask) { if(IsFrozen()) Task=NewTask; }

    FDNPREF   virtual void FDNFUNC OnFreeze() { return; }
    FDNPREF   virtual void FDNFUNC OnThaw(const char * ) { return; }
    
    // Internal error control and detection
    FDNPREF           int  FDNFUNC GetError();
    FDNPREF           void FDNFUNC ClearError();

  private:
    FDNPREF           void FDNFUNC Constructor(const char *pathname, const char *path2, unsigned short setflags, unsigned short NewTask);
    FDNPREF            int FDNFUNC InitClass();
    FDNPREF           void FDNFUNC ConvertToC(char FDNDATA *string);
    FDNPREF           char FDNFUNC *AddTrail(char *rawfile);
    FDNPREF            int FDNFUNC CreateInstance(void);
    FDNPREF            int FDNFUNC DeleteInstance(void);
    FDNPREF           char FDNFUNC *FCRGetS(char FDNDATA * buffer, int maxlength, FDN_FileObject & file);
    FDNPREF           char FDNFUNC ToUpper(char c);
    FDNPREF            int FDNFUNC CheckFile(char *filename);
  
  protected :

    FDNPREF   virtual void FDNFUNC SignalError(int newerr) { error=newerr; }

  friend class FDNFind;

};

#ifdef FDN_FULL_PACK
#pragma pack()
#endif


/**************************************************************************/
/*                                                                        */
/* FrontDoorNode Error Key                                                */
/* The error variable holds the last error condition. Note that in the    */
/* C++ Class implementation, there is a virtual function SignalError()    */
/* which you can use to tailor the reaction to the problem.               */
/*                                                                        */
/*  0    No error condition.                                              */
/*  1    Unable to open NODELIST.FDX, fatal error, class will be frozen.  */
/*  2    Unable to open USERLIST.FDX, fatal error, class will be frozen.  */
/*  3    Unable to open NODELIST.???, fatal error, class will be frozen   */
/*  4    Unable to open FDNODE.FDA, trivial error.                        */
/*  5    Unable to open FDNET.PVT, trivial error.                         */
/*  6    Unable to open FDPOINT.PVT, trivial error.                       */
/*  7    Memory allocation failure for NFDX mini cache.                   */
/*  8    Memory allocation failure for UFDX mini cache.                   */
/*  9    Memory allocation failure for FDA Store, fatal error.            */
/* 10    Memory allocation failure in search function.                    */
/* 11    Memory allocation failure for PFDX mini cache.                   */
/* 12    Unable to open PHONE.FDA, trivial error.                         */
/* 13    FDNFind out of date, or not owned, search terminated.            */
/* 14    Unable to open PHONE.FDX                                         */
/* 15    CountryCode not set, Dial translation impossible                 */
/* 16    Could not create Instance semaphore (no vacancies)               */
/* 17    Could not create Instance semaphore (no path)                    */
/* 18    Unable to open NODELIST.???, trivial error.                      */
/* 19    Index points to invalid record in FDNODE.FDA                     */
/* 20    Index points to invalid record in FDNET.PVT                      */
/* 21    Index points to invalid record in FDPOINT.PVT                    */
/* 22    Index points to record in an invalid file                        */
/* 23    Zero entries in NODELIST.FDX                                     */
/* 24    Zero entries in USERLIST.FDX                                     */
/* 25    Zero entries in PHONE.FDX                                        */
/* 26    Bad format of NODELIST.FDX, fatal error.                         */
/* 27    Bad format of USERLIST.FDX, fatal error.                         */
/* 28    Bad format of PHONE.FDX, fatal error.                            */
/* 29    Frozen - Cannot process                                          */
/* 30    Error reading PHONE.FDX                                          */
/* 31    Error reading PHONE.FDA                                          */
/*                                                                        */
/* The class should attempt to limit damage if it encounters an error     */
/* condition, by terminating a search, freezing itself, or otherwise      */
/* attempting to return safe data.                                        */
/*                                                                        */
/* WARNING! All positive error numbers are reserved. Any derived class    */
/*          should use only negative error numbers if it needs to issue   */
/*          its own error messages.                                       */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/* This new class is a prototype for writing FD Index files. Eventually I */
/* hope to support alteration and deletion of records, but not for now.   */
/**************************************************************************/

const int WFDNodeOverWrite = 0x0001;  // Implemented - but no overwrite not tested
const int WFDNodeSwedish   = 0x0002;  // Implemented (I think)
const int WFDNodeNoPhone   = 0x0004;
const int WFDNodeNoPFDA    = 0x0008;
const int WFDNodeNoNCache  = 0x0010;
const int WFDNodeNoUCache  = 0x0020;
const int WFDNodeNoPCache  = 0x0040;
const int WFDNodeUseDupes  = 0x0100;
//const int WFDNodeIsFrozen  = 0x8000;  // Implemented
const int WFDNodeCreateFrozen = 0x8000;

const long int WFDNOfficial     = 0x00000000UL;
const long int WFDNPrivate      = 0x10000000UL;
const long int WFDNPoint        = 0x20000000UL;
const long int WFDNInternal     = 0x01000000UL;

const      int FDNFileDestroy  = 0x0001U;     // Open file destructively
const      int FDNFileUpdate   = 0x0002U;     // Open file for update

const     char NFDXIndex = 1;
const     char UFDXIndex = 2;
const     char PFDXIndex = 3;

// Declare classes

class FDWNTreeInfo;
class FDWNInsert;
class FDNFile;
class FrontDoorWNode;

#ifdef FDN_FULL_PACK
#pragma pack(1)
#endif


class FDWNTreeInfo
{
  public :

  int  Level;         // The current level of the index (unreliable - only valid in index we are creating)
  long Root;          // The location of the root page (0 indicated empty index)
  long Pages;         // The number of Pages in the index
  long Records;       // The number of records in the index (unreliable, as above)
  long Flags;
  char PromoteRecord; // The record which should be promoted when a page splits (default is 16)

  // Constructor
  FDWNTreeInfo();

  
};



class FDWNInsert
{
  public :
  
  int  Level;                 // Level in which to insert record (greatest valid subscript for arrays)
  int  Status;                // 1 indicates a record with this key already exists, 0 otherwise
  long Page[MAXHEIGHT];       // Page in which to add record (and path to this record)
  int  Record[MAXHEIGHT];     // Record in which to add record (and path to this record)
  int  MaxRecord[MAXHEIGHT];  // Maximum valid record in the page of the same subscript
};


class FrontDoorWNode
{
  // Attributes

  protected :
    long           Flags;
    long           PFDARecords;
    int            error;
    unsigned short CountryCode;
    char           NodeExt[4];
    char           NodelistDir[PATHLENGTH];
    char           Frozen;
    FDN_FileObject NFDX, UFDX, PFDX;
    FDN_FileObject PFDA;
    NFDXPage       NRoot;
    UFDXPage       URoot;
    PFDXPage       PRoot;
    FirstPage      NFirst, UFirst, PFirst;
    FDWNTreeInfo   NInfo, UInfo, PInfo;
    FDWNInsert     InsertPoint;
    StubInfo       DefaultInfo;

  public :
    FDNPREF           void FDNFUNC SetNLDir(const char FDNDATA *dirname);
    FDNPREF           void FDNFUNC SetCountry(unsigned short int newCode)  {if(IsFrozen()) CountryCode = newCode;}
    FDNPREF           void FDNFUNC SetNLExt(const char FDNDATA *nlExt);
    FDNPREF           void FDNFUNC SetFlags(long newFlags)  {Flags = newFlags;}

    // Sophisticated tweaking
    FDNPREF           void FDNFUNC SetTreeFlags(char Index, long Flags, char PromoteRecord);
    FDNPREF   FDWNTreeInfo FDNFUNC *GetTreeFlags(char Index);

    FDNPREF           void FDNFUNC Freeze();
    FDNPREF            int FDNFUNC Thaw();
//    FDNPREF     inline int FDNFUNC IsFrozen() {return((int) (Flags & WFDNodeIsFrozen)); }    
    FDNPREF     inline int FDNFUNC IsFrozen() {return(Frozen); }    

    // Error detection & handling
    FDNPREF            int FDNFUNC GetError()    {return(error);}
    FDNPREF           void FDNFUNC ClearError()  {error=0;}
  protected :
    FDNPREF   virtual void FDNFUNC SignalError(int errorcon)  {error = errorcon;}

    FDNPREF   virtual void FDNFUNC OnFreeze() { return; }
    FDNPREF   virtual void FDNFUNC OnThaw() { return; }
    
  // Services
  public :

    FDNPREF           void FDNFUNC CreateRecord(NFDXRecord & ToFill, unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point, unsigned short RNet, unsigned short RNode, char Status, long int Whence, long int Offset);
    FDNPREF           void FDNFUNC CreateRecord(UFDXRecord & ToFill, unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point, const char * UserName, char Status, long int Whence, long int Offset);

    FDNPREF            int FDNFUNC AddRecord(NFDXRecord & NData);
    FDNPREF            int FDNFUNC AddRecord(unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point,
                                             unsigned short RNet, unsigned short RNode, char Status, long int Whence, long int Offset);
    FDNPREF            int FDNFUNC AddRecord(UFDXRecord & UData);
    FDNPREF            int FDNFUNC AddRecord(unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point,
                                             const char * UserName, char Status, long int Whence, long int Offset);    
    FDNPREF            int FDNFUNC AddRecord(PFDXRecord & PData);
    FDNPREF            int FDNFUNC AddRecord(const char * ToMatch, const char * XLT, unsigned short Cost);        
  
  protected :


    FDNPREF    virtual int FDNFUNC CommitCache(NFDXPage & , long ); //{ return(0); }
    FDNPREF    virtual int FDNFUNC CheckCache(UFDXPage & , long ); //{ return(0); }
    FDNPREF    virtual int FDNFUNC CommitCache(UFDXPage & , long ); //{ return(0); }
    FDNPREF    virtual int FDNFUNC CheckCache(NFDXPage & , long ); //{ return(0); }

    FDNPREF            int FDNFUNC RawReadPage(NFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC RawWritePage(NFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC RawReadPage(UFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC RawWritePage(UFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC RawReadPage(PFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC RawWritePage(PFDXPage & Page, long PageNo);

    FDNPREF            int FDNFUNC ReadPage(NFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC WritePage(NFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC ReadPage(UFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC WritePage(UFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC ReadPage(PFDXPage & Page, long PageNo);
    FDNPREF            int FDNFUNC WritePage(PFDXPage & Page, long PageNo);

    FDNPREF            int FDNFUNC WriteNFDXStub();
    FDNPREF            int FDNFUNC ReadNFDXStub();
    FDNPREF            int FDNFUNC WriteUFDXStub();
    FDNPREF            int FDNFUNC ReadUFDXStub();
    FDNPREF            int FDNFUNC WritePFDXStub();
    FDNPREF            int FDNFUNC ReadPFDXStub();
    FDNPREF            int FDNFUNC WritePFDAStub();

    FDNPREF            int FDNFUNC GetInsertPoint(NFDXRecord & NData);
    FDNPREF            int FDNFUNC AddRecord(NFDXRecord & NData, long LeftChild, long RightChild);
    FDNPREF            int FDNFUNC GetInsertPoint(UFDXRecord & UData);
    FDNPREF            int FDNFUNC AddRecord(UFDXRecord & UData, long LeftChild, long RightChild);
    FDNPREF            int FDNFUNC GetInsertPoint(PFDXRecord & PData);
    FDNPREF            int FDNFUNC AddRecord(PFDXRecord & PData, long LeftChild, long RightChild);

    FDNPREF            int FDNFUNC CompareKey(const char * key1, const char * key2, int MaxLen);
    FDNPREF           void FDNFUNC FormUserName(const char * In, char * Out);
    FDNPREF inline unsigned short FDNFUNC SwapBytes(unsigned short initial){ return((unsigned short) (((initial&0xFF00) >> 8) + ((initial&0x00FF) << 8)) ); };
    FDNPREF           char FDNFUNC ToUpper(char c);

    FDNPREF           char FDNFUNC *AddTrail(char *rawfile);
    FDNPREF  unsigned long FDNFUNC Time(unsigned long * Pointer);
  
  // Implementation

  public :
    // Constructors
    FrontDoorWNode();
    FrontDoorWNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc);
    FrontDoorWNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags);

    // Destructors
    virtual ~FrontDoorWNode();

  protected :

    FDNPREF void FDNFUNC Constructor(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags);

  private :
    
    FDNPREF  int FDNFUNC InitClass();

};

#ifdef FDN_FULL_PACK
#pragma pack()
#endif

#endif

#endif

/**************************************************************************/
/*                                                                        */
/* FrontDoorWNode Error Key                                               */
/* The error variable holds the last error condition. Note that in the    */
/* C++ Class implementation, there is a virtual function SignalError()    */
/* which you can use to tailor the reaction to the problem.               */
/*                                                                        */
/*  0    No error condition.                                              */
/*  1    Unable to open NODELIST.FDX, fatal error, class will be frozen.  */
/*  2    Unable to open USERLIST.FDX, fatal error, class will be frozen.  */
/*  7    Memory allocation failure for NFDX mini cache.                   */
/*  8    Memory allocation failure for UFDX mini cache.                   */
/* 10    Memory allocation failure in search or write function.           */
/* 11    Memory allocation failure for PFDX mini cache.                   */
/* 12    Unable to open PHONE.FDA                                         */
/* 14    Unable to open PHONE.FDX                                         */
/* 15    CountryCode or Nodelist Extension not set, Fatal error           */
/* 26    Bad format of NODELIST.FDX, fatal error.                         */
/* 27    Bad format of USERLIST.FDX, fatal error.                         */
/* 28    Bad format of PHONE.FDX, fatal error.                            */
/* 29    Frozen - cannot process                                          */
/* 32    Invalid page in NODELIST.FDX                                     */
/* 33    Invalid page in USERLIST.FDX                                     */
/* 34    Invalid page in PHONE.FDX                                        */
/* 100    Current nodelist extension invalid in OverWrite mode            */
/* 101    String too long                                                 */
/*                                                                        */
/* The class should attempt to limit damage if it encounters an error     */
/* condition, by terminating a search, freezing itself, or otherwise      */
/* attempting to return safe data.                                        */
/*                                                                        */
/* WARNING! All positive error numbers are reserved. Any derived class    */
/*          should use only negative error numbers if it needs to issue   */
/*          its own error messages.                                       */
/*                                                                        */
/**************************************************************************/

