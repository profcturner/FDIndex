// A 'C++' class implementation for creating FrontDoor nodelist indices.
// Written by Colin Turner; 2:443/13@fidonet

// FrontDoor is a registered trademark of Joaquim Homrighausen

#include "fdnode.h"
                              
/****************************************************************************/
/*                                                                          */
/* Class: FrontDoorWNode                                                    */
/* Purpose: Writes FrontDoor style index files                              */
/*                                                                          */
/* Related Classes: FDNWFile                                                */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                   P U B L I C   F U N C T I O N S                        */
/*                                                                          */
/****************************************************************************/


/*
**    SetNLDir
**
** Sets internal nodelist directory to specified path if the class is
** frozen.
**
*/
FDNPREF void FDNFUNC FrontDoorWNode::SetNLDir(const char FDNDATA *DirName)
{
  if(IsFrozen()){
    strcpy(NodelistDir, DirName);
    AddTrail(NodelistDir);
  }
}


/*
**    SetNLExt
**
** Used to set the extension of the official nodelist file NODELIST.???
** if the class is frozen.
**
**    Parameters
**
**    Specifically a three character 'C' string, Zero padded at left.
**    Two special cases exist
**
**    PVT - Used to indicate the absence of the official nodelist
**    CUR - Use current extenion (not in OverWrite mode).
*/
FDNPREF void FDNFUNC FrontDoorWNode::SetNLExt(const char FDNDATA *nlExt)
{
  if(IsFrozen()){
    if(strlen(nlExt) != 3) return;
    else strcpy(NodeExt, nlExt);
  }
}


/*
**    Constructors
**
** All call the private function detailed below. See it for more
** more details.
**
*/
FrontDoorWNode::FrontDoorWNode()
{
  Constructor("", "", 0, WFDNodeCreateFrozen);
}


FrontDoorWNode::FrontDoorWNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc)
{
  Constructor(nldir, nlext, cc, 0);
}


FrontDoorWNode::FrontDoorWNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags)
{
  Constructor(nldir, nlext, cc, flags);
}


/*
**    Destructor
**
** Simply calls the Freeze function to tidy up.
**
*/
FrontDoorWNode::~FrontDoorWNode()
{
  Freeze();
}


/*
**    Thaw
**
** Attempts to initialise the class for use
**
**    Returns
**
**    0 on failure; 1 on success
**
*/
FDNPREF int FDNFUNC FrontDoorWNode::Thaw()
{
  int success;
  
  if(IsFrozen()){
    success = InitClass();
    if(success) OnThaw();
    return(success);
  }
  return(1);
}


/*
**    Freeze
**
** Renders class inactive. Writes current Stub information
** and closes relevant files.
**
*/
FDNPREF void FDNFUNC FrontDoorWNode::Freeze()
{
  if(IsFrozen()) return;

  OnFreeze();

  WriteNFDXStub();
  WriteUFDXStub();
  WritePFDXStub();

  NFDX.Close();
  UFDX.Close();
  PFDX.Close();
  PFDA.Close();

  Frozen = 1;
  return;
}



/****************************************************************************/
/*                                                                          */
/*                  P R I V A T E   F U N C T I O N S                       */
/*                                                                          */
/****************************************************************************/


/*
**    Constructor
**
** Generic constuction function called by all overloaded constructors.
**
**    Parameters
**
**    nldir   Path to nodelist files
**    nlext   Extension of nodelist. See SetNLExt for more details
**    cc      Country code (1 for USA, 44 for UK, 46 for Sweden etc)
**    flags   Flags with which to create class
**
*/
FDNPREF void FDNFUNC FrontDoorWNode::Constructor(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags)
{
  Frozen = 1;
  strcpy(NodelistDir, nldir);
  AddTrail(NodelistDir);
  Flags = flags;
  CountryCode = cc;
  if(strlen(nlext) != 3) *NodeExt=0;
  else strcpy(NodeExt, nlext);

  // Initialise - unless we're told not to
  if(!(Flags & WFDNodeCreateFrozen)) Thaw();
}


/*
**    InitClass
**
** Called by Thaw or construction when not created in a frozen
** state. Initialises key data, opens files and reads in
** statistics concerning the current index set.
**
**    Returns
**
**    1 on success; 0 on failure
*/
FDNPREF int FDNFUNC FrontDoorWNode::InitClass()
{
  int  FatalError = 0;
  char filename[PATHLENGTH];

  if(!CountryCode || *NodeExt==0){
    // Insufficient Data
    SignalError(15);
    return(0);
  }

  strcpy(filename, NodelistDir);
  strcat(filename, "NODELIST.FDX");
  NFDX.SetName(filename);
  
  strcpy(filename, NodelistDir);
  strcat(filename, "USERLIST.FDX");
  UFDX.SetName(filename);

  strcpy(filename, NodelistDir);
  strcat(filename, "PHONE.FDX");
  PFDX.SetName(filename);

  strcpy(filename, NodelistDir);
  strcat(filename, "PHONE.FDA");
  PFDA.SetName(filename);

  // Examine our Nodelist Extent
  if((!stricmp(NodeExt, "CUR")) && (Flags & WFDNodeOverWrite)){
    // We can't use the "Current" extent when we're in overwrite mode!
    SignalError(100);
    Frozen = 1;
    return(0);
  }

  if(Flags & WFDNodeOverWrite){
    NFDX.SetFlags(FDNFileDestroy);
    UFDX.SetFlags(FDNFileDestroy);
    PFDX.SetFlags(FDNFileDestroy);
    PFDA.SetFlags(FDNFileDestroy);    
  }
  else {
    NFDX.SetFlags(FDNFileUpdate);
    UFDX.SetFlags(FDNFileUpdate);
    PFDX.SetFlags(FDNFileUpdate);
    PFDA.SetFlags(FDNFileUpdate);    
  }

  // Open our file objects
  NFDX.Open();
  if(!NFDX.GetStatus()) FatalError=1;
  UFDX.Open();
  if(!UFDX.GetStatus()) FatalError=2;
  PFDX.Open();
  if(!PFDX.GetStatus()) FatalError=14;
  PFDA.Open();
  if(!PFDA.GetStatus()) FatalError=12;

  if(FatalError){
    SignalError(FatalError);
    Frozen = 1;
    NFDX.Close();
    UFDX.Close();
    PFDX.Close();
    PFDA.Close();
    return(0);
  }

  memset(&NFirst, 0, sizeof(FirstPage));
  memset(&UFirst, 0, sizeof(FirstPage));
  memset(&PFirst, 0, sizeof(FirstPage));

  memset(&DefaultInfo, 0, sizeof(StubInfo));

  // Find the lengths of the files - Calculate the Info block details
  PFDARecords = (PFDA.Size() / (long) sizeof(FDNPhoneRec) - 1L);
  NInfo.Pages = (NFDX.Size() / (long) sizeof(NFDXPage) - 1L);
  UInfo.Pages = (UFDX.Size() / (long) sizeof(UFDXPage) - 1L);
  PInfo.Pages = (PFDX.Size() / (long) sizeof(PFDXPage) - 1L);    

  if(stricmp(NodeExt, "CUR")){
    DefaultInfo.NodeExt[0]=3;
    strcpy(DefaultInfo.NodeExt+1, NodeExt);
    // This puts a zero beyond the structure, but this is corrected now
  }
  DefaultInfo.Swedish     = (char) (Flags & WFDNodeSwedish);
  DefaultInfo.RevisionMaj = 2;
  DefaultInfo.RevisionMin = 3;
  DefaultInfo.ZeroWord    = 0;
  DefaultInfo.CountryCode = CountryCode;

  if(PFDARecords == -1){
    WritePFDAStub();
    PFDARecords++;
  }
    
  if(NInfo.Pages == -1){
    WriteNFDXStub();
    NInfo.Pages++;
  }
  else{
    if(!ReadNFDXStub()){
      SignalError(26);
      return(0);
    }
  }

  if(UInfo.Pages == -1){
    WriteUFDXStub();
    UInfo.Pages++;
  }
  else{
    if(!ReadUFDXStub()){
      SignalError(27);
      return(0);
    }
  }

  if(PInfo.Pages == -1){
    WritePFDXStub();
    PInfo.Pages++;
  }
  else{
    if(!ReadPFDXStub()){
      SignalError(28);
      return(0);
    }
  }

  // Load in root pages - so that we can use ReadPage, we zero the records
  if(NFirst.index) RawReadPage(NRoot, NFirst.index);
  if(UFirst.index) RawReadPage(URoot, UFirst.index);
  if(PFirst.index) RawReadPage(PRoot, PFirst.index);

  Frozen = 0;
  return(1);
}


/*
**    FormUserName
**
** Forms a key suitable for USERLIST.FDX and places it in the buffer
** indicated by Out. The string passed into In should be one of
**
** A Pascal string from FDNODE.FDA
** A Comma terminated 'C' string from a Nodelist File
**
** Underscores are converted to spaces, the last name is placed first
** and all characters are uppercased.
**
*/
FDNPREF void FDNFUNC FrontDoorWNode::FormUserName(const char * In, char * Out)
{
  int loop, count = 1, StrLen = 0, LastSpace = -1;
  const char * Start = In;

  if((*In <= 31) && !strchr(In, ',')){
    // Looks like a Pascal string
    Start++;
    StrLen = *In;
  }
  else
  {
    StrLen = strcspn(Start, ",");
  }

  // Begin processing the string
  for(loop = 0; loop < StrLen; loop++){
    if(Start[loop]=='_' || Start[loop]==' ') LastSpace = loop;
  }

  for(loop = LastSpace + 1; (loop < StrLen) && (count < 16); loop++) Out[count++] = ToUpper(Start[loop]);

  if(count < 16) Out[count++] = ' ';

  for(loop = 0; (loop < LastSpace) && (count < 16); loop++){
    if(Start[loop]=='_') Out[(count++)] = ' ';
    else Out[(count++)] = ToUpper(Start[loop]);
  }
  
  for(loop = count; loop < 17; loop++) Out[loop] = 0;
  *Out = 24;
}


/*
**    ToUpper
**
** A more general replacement of the toupper() standard
** library function.
*/
FDNPREF char FDNFUNC FrontDoorWNode::ToUpper(char c)
{
  if (((unsigned char) c) >= 'a' && ((unsigned char) c) <= 'z')
    c-=32;

  else if (((unsigned char) c) > 127){
#ifdef FDN_WINDOWS
    c=LOBYTE(LOWORD(AnsiUpper((char __far *)MAKELONG((unsigned char) c, 0))));
#elif defined(FDN_DOS)
    static int First=1;
    static unsigned char uprbuf[256];
    if(First){
      First=0;
      for(int i=0;i<sizeof(uprbuf);i++)
        uprbuf[i]=(unsigned char)i;
      for(i='a';i<='z';i++)
        uprbuf[i]=(unsigned char)(i-32);

      #ifdef __WATCOMC__
      #pragma off (unreferenced);
      #endif
      
      union REGS r;
      SREGS s;
      struct cbuf_t {
        unsigned char id;
        void __far * ptr;
      } cbuf;
        
      #ifdef __WATCOMC__
      #pragma on (unreferenced);
      #endif
      
      r.x.ax=0x3000;
      intdos(&r, &r);
      if(r.h.al>3 || (r.h.al==3 && r.h.ah>=30)){
        r.x.ax=0x6502;
        r.x.bx=0xFFFF;
        r.x.dx=0xFFFF;
        r.x.cx=5;
        r.x.di=FP_OFF((void __far *)&cbuf);
        s.es=FP_SEG((void __far *)&cbuf);
        intdosx(&r, &r, &s);

        i=(int) *((unsigned short __far *)cbuf.ptr);
        for(i=0; i<*((unsigned short __far *)cbuf.ptr); i++)
          uprbuf[128+i]=*((unsigned char __far *)cbuf.ptr+2+i);
      }
    }

    c=(char)uprbuf[(unsigned char)c];
    
#elif defined(FDN_OS2)
    COUNTRYCODE cc;
    cc.country=0;
    cc.codepage=0;
    DosCaseMap(1, &cc, &c);

#else
    static char * pLower = "\x87" "\x84" "\x86" "\x82" "\x91" "\x94" "\x81" "\xA4",
                * pUpper = "\x80" "\x8E" "\x8F" "\x90" "\x92" "\x99" "\x9A" "\xA5";
    char * p;
    if((p=strchr(pLower, c))!=NULL)
    c = (char) (((unsigned char *) pUpper)[p-pLower]);
#endif
  }

  return(c);
}

    
/*
**    CompareKey
**
** This function compares two Pascal style strings up to a MaxLength
** specified.
**
**    Returns
**
**    -1 if string key1 lies alphabetically before key2
**     0 if string key1 and key2 are identical
**    +1 if string key1 lies alphabetically after key2
*/
FDNPREF int  FDNFUNC FrontDoorWNode::CompareKey(const char * key1, const char * key2, int MaxLen)
{
  register int loop;

  for(loop = 1; loop < MaxLen && loop <= key1[0] && loop <= key2[0]; loop++){
    if((unsigned char) key1[loop] > (unsigned char) key2[loop]) return(1);
    if((unsigned char) key1[loop] < (unsigned char) key2[loop]) return(-1);
  }
  if((loop == key1[0] + 1) || (loop == key2[0] + 1)){
    if(key1[0] > key2[0]) return(1);
    if(key1[0] < key2[0]) return(-1);
    return(0);
  }
  return(0);
}


/****************************************************************************/
/*                                                                          */
/*                   P U B L I C   F U N C T I O N S                        */
/*                                                                          */
/****************************************************************************/

/*
**    CreateRecord (NODELIST.FDX variant)
**
** Formats an NFDXRecord according to relevant parameters for it.
**
**    Parameters
**
**    ToFill    An NFDXRecord (not pointer) in which to place the information
**    See the similar AddRecord for information on the other parameters.
**
**/
FDNPREF void FDNFUNC FrontDoorWNode::CreateRecord(NFDXRecord & ToFill, unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point, unsigned short RNet, unsigned short RNode, char Status, long int Whence, long int Offset)
{
  ToFill.zone     = SwapBytes(Zone);
  ToFill.net      = SwapBytes(Net);
  ToFill.node     = SwapBytes(Node);
  ToFill.point    = SwapBytes(Point);
  ToFill.rnet     = RNet;
  ToFill.rnode    = RNode;
  ToFill.nodetype = Status;
  ToFill.esmark   = 0;
  ToFill.key[0]   = 14;
  ToFill.offset.loff = Whence + Offset;
  ToFill.link     = 0;
}


/*
**    CreateRecord (USERLIST.FDX variant)
**
** Formats an UFDXRecord according to relevant parameters for it.
**
**    Parameters
**
**    ToFill    A UFDXRecord (not pointer) in which to place the information
**    See the similar AddRecord for information on the other parameters.
**
**/
FDNPREF void FDNFUNC FrontDoorWNode::CreateRecord(UFDXRecord & ToFill, unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point, const char * UserName, char Status, long int Whence, long int Offset)
{
  FormUserName(UserName, ToFill.key);
  ToFill.zone     = SwapBytes(Zone);
  ToFill.net      = SwapBytes(Net);
  ToFill.node     = SwapBytes(Node);
  ToFill.point    = SwapBytes(Point);
  ToFill.nodetype = Status;
  ToFill.offset   = Whence + Offset;
  ToFill.link     = 0;
}


/*
**    AddRecord (NODELIST.FDX variant)
**
** Adds a record into the BTree index. This function forms the record and passes
** it to the generic AddRecord(Record) function class detailed below.
**
**    Parameters
**
**    Zone, Net, Node, Point as obvious
**    RNet, RNode are routing targets.
**      ie. Host for non hubbed nodes, Hub for hubbed notes, Region/0 for Hosts.
**    Status is the status as defined in the development toolkit.
**    Whence is the code which denotes the file which the referenced entry came from.
**      ie. one of
**      WFDNOfficial, WFDNPrivate, WFDNPoint, WFDNInternal
**    Offset is the offset to the above file.
**      ie. in FDNODE.FDA, the record number, in textual nodelist fields
**      use the offset to the first character in the system name
**
**    Returns
**
**    0 on failure
**    1 on success (new key added)
**    2 on success (key replaced)
**
*/
FDNPREF int FDNFUNC FrontDoorWNode::AddRecord(unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point, unsigned short RNet, unsigned short RNode, char Status, long int Whence, long int Offset)
{
  int success;
  NFDXRecord NewData;

  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  CreateRecord(NewData, Zone, Net, Node, Point, RNet, RNode, Status, Whence, Offset);

  success = AddRecord(NewData);
  return(success);
}


/*
**    AddRecord (USERLIST.FDX variant)
**
** Adds a record into the BTree index. This function forms the record and passes
** it to the generic AddRecord(Record) function class detailed below.
**
**    Parameters
**
**    Zone, Net, Node, Point as obvious
**    UserName is a the SysOp name in one of two formats
**      Pascal style string from FDNODE.FDA;
**      'C' style string, but ',' terminated. (Raw nodelist field)
**      underscores are treated as spaces, case irrelevant.
**    Status is the status as defined in the development toolkit.
**    Whence is the code which denotes the file which the referenced entry came from.
**      ie. one of
**      WFDNOfficial, WFDNPrivate, WFDNPoint, WFDNInternal
**    Offset is the offset to the above file.
**      ie. in FDNODE.FDA, the record number, in textual nodelist fields
**      use the offset to the first character in the system name
**
**    Returns
**
**    0 on failure
**    1 on success (new key added)
**    2 on success (key replaced)
**
*/
FDNPREF int FDNFUNC FrontDoorWNode::AddRecord(unsigned short Zone, unsigned short Net, unsigned short Node, unsigned short Point, const char * UserName, char Status, long int Whence, long int Offset)
{
  int success;
  UFDXRecord NewData;

  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  CreateRecord(NewData, Zone, Net, Node, Point, UserName, Status, Whence, Offset);

  success = AddRecord(NewData);
  return(success);
}


/*
**    AddRecord (PHONE.FDX variant)
**
** Adds a record into the BTree index AND in this case PHONE.FDA
** Offsets are handled automatically and hence not passed.
**
**    Parameters
**
**    ToMatch   The string to look for in the phone number.
**    XLT       The translation string (format defined in FDNODE.CTL).
**    Cost      The cost for the system.
**
**    Set XLT = "=" for Cost only entries.
**    You must add two entries, one with ToMatch = "DOM", the other "INTL"
**    which specify default Domestic and International handling respectively.
**
**    Returns
**
**    0 on failure
**    1 on success (new key added)
**    2 on success (key replaced)
**
*/
FDNPREF int FDNFUNC FrontDoorWNode::AddRecord(const char * ToMatch, const char * XLT, unsigned short Cost)
{
  int success = 1;
  PFDXRecord IDXData;
  FDNPhoneRec PFDAData;

  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  if((strlen(ToMatch) > 20) || (strlen(XLT) > 40)){
    SignalError(101);
    return(0);
  } // String is too long too record, probably safer to discard
  

  IDXData.key[0] = (char) strlen(ToMatch);
  strncpy(IDXData.key + 1, ToMatch, 20);

  PFDAData.Telephone[0] = (char) strlen(XLT);
  strncpy(PFDAData.Telephone + 1, XLT, 40);
  PFDAData.Erased = 0;
  PFDAData.Cost   = Cost;
  PFDAData.Baudrate = 0;

  PFDA.Write(&PFDAData, sizeof(FDNPhoneRec), 1, 1);
  IDXData.offset = ++PFDARecords;
  AddRecord(IDXData);
  
  return(success);
}


/*
**    AddRecord
**
** A set of functions for adding fully formed records into existing
** trees. It is recommended that the above functions are used in
** preference. Note that the link value in the item is irrelevant.
**
**    Returns
**
**    0 on failure
**    1 on success (new key added)
**    2 on success (key replaced)
**
*/
FDNPREF int  FDNFUNC FrontDoorWNode::AddRecord(NFDXRecord & NData)
{
  int success;
  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  if(GetInsertPoint(NData)){
    if(InsertPoint.Status && !(NInfo.Flags & WFDNodeUseDupes)) return(0); // Duplicate, simply kill
    success = AddRecord(NData, 0, 0);
    if(success) NInfo.Records++;
    return(success);
  }
  return(0);
}


/*
** See overloaded variant above for details.
*/
FDNPREF int  FDNFUNC FrontDoorWNode::AddRecord(UFDXRecord & UData)
{
  int success;
  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  if(GetInsertPoint(UData)){
    if(InsertPoint.Status && !(UInfo.Flags & WFDNodeUseDupes)) return(0); // Duplicate, simply kill
    success = AddRecord(UData, 0, 0);
    if(success) UInfo.Records++;
    return(success);
  }
  return(0);
}


/*
** See overloaded variant above for details.
**
** Note that this function does NOT handle PHONE.FDA at ALL.
**
*/
FDNPREF int  FDNFUNC FrontDoorWNode::AddRecord(PFDXRecord & PData)
{
  int success;
  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  if(GetInsertPoint(PData)){
    if(InsertPoint.Status && !(PInfo.Flags & WFDNodeUseDupes)) return(0); // Duplicate, simply kill
    success = AddRecord(PData, 0, 0);
    if(success) PInfo.Records++;
    return(success);
  }
  return(0);
}


/****************************************************************************/
/*                                                                          */
/*                  P R I V A T E   F U N C T I O N S                       */
/*                                                                          */
/****************************************************************************/

  
/*
**    GetInsertPoint
**
** A set of functions, one for each tree, which determines the insertion
** point for a new record in an existing index.
**
** The insertion point will be such that the current key in that point
** should be moved to the right.
**
**    Parameters
**
**    The relevant data item. The link field is ignored.
**
**    Returns
**
**    The InsertPoint object is filled in. Note that
**    InsertPoint.Level - 1 is the greatest subscript of correct data.
**    InsertPoint.Status = 0 for simple insertions, 1 denotes a duplicate
**
*/
FDNPREF int FDNFUNC FrontDoorWNode::GetInsertPoint(NFDXRecord & NData)
{
  InsertPoint.Level  = 0;
  InsertPoint.Status = 0;
  long Page          = NFirst.index;
  long NextPage      = 0;
  int  Test;
  int  loop;
  int  quit = 0;
  char * SearchKey = NData.key;
  NFDXPage * PageData;
  
  if(!Page) return(1); // There isn't an index yet!

  PageData = new NFDXPage;
  if(!PageData){
    SignalError(0);
    return(0);
  }

  while(!quit){
    NextPage = 0;
    InsertPoint.Level++;
    ReadPage(*PageData, Page);
    InsertPoint.Page[InsertPoint.Level - 1] = Page;
    InsertPoint.MaxRecord[InsertPoint.Level - 1] = PageData->records;

    for(loop = PageData->records - 1; (loop >= 0) && !NextPage && !quit; loop--){
      Test = CompareKey(SearchKey, PageData->nodes[loop].key, SearchKey[0]);
      if(Test == 0){
        // Duplicate key : Match found, mark for replacement and quit
        InsertPoint.Status = 1;
        InsertPoint.Record[InsertPoint.Level - 1] = loop;
        quit = 1;
      }
      else InsertPoint.Status = 0;
      if(Test > 0){
        
        InsertPoint.Record[InsertPoint.Level - 1] = loop + 1;
        if(!PageData->backref) quit = 1; // Not a link page, nowhere to go to refine search
        else NextPage = PageData->nodes[loop].link; // Look further
      }
      if(Test < 0){
        if(!loop){
          InsertPoint.Record[InsertPoint.Level - 1] = 0; // Less than all items on the page
          if(!PageData->backref) quit = 1;
          else NextPage = PageData->backref;
        }
      }
    }
    if(!quit) Page = NextPage;
  }
  delete PageData;
  return(1);
}


/*
**    See notes for overloaded variant above
*/
FDNPREF int FDNFUNC FrontDoorWNode::GetInsertPoint(UFDXRecord & UData)
{
  InsertPoint.Level  = 0;
  InsertPoint.Status = 0;
  long Page          = UFirst.index;
  long NextPage      = 0;
  int  Test;
  int  loop;
  int  quit = 0;
  char * SearchKey = UData.key;
  UFDXPage * PageData;
  
  if(!Page) return(1); // There isn't an index yet!

  PageData = new UFDXPage;
  if(!PageData){
    SignalError(0);
    return(0);
  }

  while(!quit){
    NextPage = 0;
    InsertPoint.Level++;
    ReadPage(*PageData, Page);
    InsertPoint.Page[InsertPoint.Level - 1] = Page;
    InsertPoint.MaxRecord[InsertPoint.Level - 1] = PageData->records;

    for(loop = PageData->records - 1; (loop >= 0) && !NextPage && !quit; loop--){
      Test = CompareKey(SearchKey, PageData->names[loop].key, 24);
      if(Test == 0){
        // Duplicate key : Match found, mark for replacement and quit
        InsertPoint.Status = 1;
        InsertPoint.Record[InsertPoint.Level - 1] = loop;
        quit = 1;
      }
      else InsertPoint.Status = 0;
      if(Test > 0){
        InsertPoint.Record[InsertPoint.Level - 1] = loop + 1;
        if(!PageData->backref) quit = 1; // Not a link page, nowhere to go to refine search
        else NextPage = PageData->names[loop].link; // Look further
      }
      if(Test < 0){
        if(!loop){
          InsertPoint.Record[InsertPoint.Level - 1] = 0; // Less than all items on the page
          if(!PageData->backref) quit = 1;
          else NextPage = PageData->backref;
        }
      }
    }
    if(!quit) Page = NextPage;
  }
  delete PageData;
  return(1);
}


/*
**    See notes for overloaded variant above
*/
FDNPREF int FDNFUNC FrontDoorWNode::GetInsertPoint(PFDXRecord & PData)
{
  InsertPoint.Level  = 0;
  InsertPoint.Status = 0;
  long Page          = PFirst.index;
  long NextPage      = 0;
  int  Test;
  int  loop;
  int  quit = 0;
  char * SearchKey = PData.key;
  PFDXPage * PageData;
  
  if(!Page) return(1); // There isn't an index yet!

  PageData = new PFDXPage;
  if(!PageData){
    SignalError(0);
    return(0);
  }

  while(!quit){
    NextPage = 0;
    InsertPoint.Level++;
    ReadPage(*PageData, Page);
    InsertPoint.Page[InsertPoint.Level - 1] = Page;
    InsertPoint.MaxRecord[InsertPoint.Level - 1] = PageData->records;

    for(loop = PageData->records - 1; (loop >= 0) && !NextPage && !quit; loop--){
      Test = CompareKey(SearchKey, PageData->phones[loop].key, 21);
      if(Test == 0){
        // Duplicate key : Match found, mark for replacement and quit
        InsertPoint.Status = 1;
        InsertPoint.Record[InsertPoint.Level - 1] = loop;
        quit = 1;
      }
      else InsertPoint.Status = 0;
      if(Test > 0){
        InsertPoint.Record[InsertPoint.Level - 1] = loop + 1;
        if(!PageData->backref) quit = 1; // Not a link page, nowhere to go to refine search
        else NextPage = PageData->phones[loop].link; // Look further
      }
      if(Test < 0){
        if(!loop){
          InsertPoint.Record[InsertPoint.Level - 1] = 0; // Less than all items on the page
          if(!PageData->backref) quit = 1;
          else NextPage = PageData->backref;
        }
      }
    }
    if(!quit) Page = NextPage;
  }
  delete PageData;
  return(1);
}


/*
**    AddRecord   (Private)
**
** This set of functions does the work of inserting records once
** an insertion point has been established. It is a recursive
** function that may split pages and force insertions into higher
** levels of the tree
**
**    Returns
**
**    0 on failure
**    1 on success (new key added)
**    2 on success (key replaced)
**
*/
FDNPREF int  FDNFUNC FrontDoorWNode::AddRecord(NFDXRecord & NData, long LeftChild, long RightChild)
{
  int loop;
  int InsertRoom;
  int InsertRecord = InsertPoint.Record[InsertPoint.Level - 1];

  if(!(InsertPoint.Level)){    
    // No Insert point, create new root
    NFDXPage New;

    // Write root data
    memcpy(&(New.nodes[0]), &NData, sizeof(NData));
    New.backref        = LeftChild;
    New.nodes[0].link  = RightChild;
    New.records        = 1;

    // Write and cache new root, and update first page data
    WritePage(New, ++NInfo.Pages);
    memcpy(&NRoot, &New, sizeof(NFDXPage));
    NFirst.index = NInfo.Pages;
    NInfo.Level++;

    return(1);
  }

  // A level of braces is added so that the scope of Original and/or New is
  // lost by the time of recursive call.
  {
    // Ok, we need to fetch the appropriate page
    NFDXPage Original;
    ReadPage(Original, InsertPoint.Page[InsertPoint.Level-1]);

    // We now code the case of insertion
    InsertRoom = 32 - Original.records;
  
    if(InsertRoom > 0 || InsertPoint.Status){
      if(InsertPoint.Status){
        // We have a duplicate, and we should be in KillDupe mode if we got this far
        memcpy(&(Original.nodes[InsertRecord]), &NData, sizeof(NData));
      }
      else{
        // This page has room for the insertion - make the gap.
    
        for(loop = Original.records - 1; loop >= InsertRecord; loop--){
          memcpy(&(Original.nodes[loop+1]), &(Original.nodes[loop]), sizeof(NFDXRecord));
        }
        memcpy(&(Original.nodes[InsertRecord]), &NData, sizeof(NData));
    
        // Update links and record numbers
        Original.records++;
        Original.nodes[InsertRecord].link = RightChild;
        if(!InsertRecord) Original.backref = LeftChild;
    
        WritePage(Original, InsertPoint.Page[InsertPoint.Level-1]);
        // Is the page the root? If so, "remember" it.
        if(InsertPoint.Page[InsertPoint.Level-1] == NFirst.index) memcpy(&NRoot, &Original, sizeof(NFDXPage));
      }
    }
    if(InsertRoom == 0){
      // This page has no room for the insertion
      NFDXPage New;
  
      // Copy the last 32/2 elements into the new page and remove from original, then write
      if(InsertRecord < NInfo.PromoteRecord){
        for(loop = NInfo.PromoteRecord; loop < 32; loop++) memcpy(&(New.nodes[loop - NInfo.PromoteRecord]), &(Original.nodes[loop]), sizeof(NFDXRecord));
        for(loop = NInfo.PromoteRecord - 1; loop >= InsertRecord; loop--) memcpy(&(Original.nodes[loop+1]), &(Original.nodes[loop]), sizeof(NFDXRecord));
        memcpy(&(Original.nodes[InsertRecord]), &NData, sizeof(NFDXRecord));
  
        // copy element (NInfo.PromoteRecord) (in Original) into NData (getting promoted)
        memcpy(&NData, &(Original.nodes[NInfo.PromoteRecord]), sizeof(NFDXRecord));
        
        // Fix linking
        New.backref = NData.link;
        NData.link   = NInfo.Pages + 1; // Ignored?
        Original.nodes[InsertRecord].link  = RightChild;
        if(!InsertRecord) Original.backref = LeftChild;
      }
      
      if(InsertRecord == NInfo.PromoteRecord){
        for(loop = NInfo.PromoteRecord; loop < 32; loop++) memcpy(&(New.nodes[loop - NInfo.PromoteRecord]), &(Original.nodes[loop]), sizeof(NData));
        New.backref = RightChild;
      }
      
      if(InsertRecord > NInfo.PromoteRecord){
        for(loop = InsertRecord; loop < 32; loop++) memcpy(&(New.nodes[loop-NInfo.PromoteRecord]), &(Original.nodes[loop]), sizeof(NFDXRecord));
        for(loop = NInfo.PromoteRecord + 1; loop < InsertRecord; loop++) memcpy(&(New.nodes[loop-NInfo.PromoteRecord-1]), &(Original.nodes[loop]), sizeof(NFDXRecord));
        memcpy(&(New.nodes[InsertRecord-NInfo.PromoteRecord-1]), &NData, sizeof(NFDXRecord));
  
        // copy element NInfo.PromoteRecord (in Original) into PData (getting promoted)
        memcpy(&NData, &(Original.nodes[NInfo.PromoteRecord]), sizeof(NFDXRecord));
  
        // Fix linking
        New.backref = NData.link;
        NData.link   = NInfo.Pages + 1; // Ignored?
        New.nodes[InsertRecord - NInfo.PromoteRecord - 1].link    = RightChild;
        if(!(InsertRecord - NInfo.PromoteRecord - 1)) New.backref = LeftChild;
        
      }
  
      Original.records = NInfo.PromoteRecord;
      New.records = (char) (32 - NInfo.PromoteRecord);
      
      WritePage(Original, InsertPoint.Page[InsertPoint.Level - 1]);
      WritePage(New, ++NInfo.Pages);
      
  
      // Add the middle element to the level above, with references to these pages
      InsertPoint.Level--;
      //return(AddRecord(NData, InsertPoint.Page[InsertPoint.Level], NInfo.Pages));
    }
  }
  if(InsertPoint.Status) return(2);
  if(InsertRoom > 0 )    return(1);
  if(InsertRoom == 0)    return(AddRecord(NData, InsertPoint.Page[InsertPoint.Level], NInfo.Pages));
  // Therefore InsertRoom < 0 which tell us we have an invalid index page
  SignalError(32);
  return(0);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::AddRecord(UFDXRecord & UData, long LeftChild, long RightChild)
{
  int loop;
  int InsertRoom;
  int InsertRecord = InsertPoint.Record[InsertPoint.Level - 1];
  
  if(!(InsertPoint.Level)){    
    // No Insert point, create new root
    UFDXPage New;

    // Write root data
    memcpy(&(New.names[0]), &UData, sizeof(UData));
    New.backref        = LeftChild;
    New.names[0].link  = RightChild;
    New.records        = 1;

    // Write and cache new root, and update first page data
    WritePage(New, ++UInfo.Pages);
    memcpy(&URoot, &New, sizeof(UFDXPage));
    UFirst.index = UInfo.Pages;
    UInfo.Level++;

    return(1);
  }

  // A level of braces is added so that the scope of Original and/or New is
  // lost by the time of recursive call.
  {
    // Ok, we need to fetch the appropriate page
    UFDXPage Original;
    ReadPage(Original, InsertPoint.Page[InsertPoint.Level-1]);

    // We now code the case of insertion
    InsertRoom = 32 - Original.records;
  
    if(InsertRoom > 0 || InsertPoint.Status){
      if(InsertPoint.Status){
        // We have a duplicate, and we should be in KillDupe mode if we got this far
        memcpy(&(Original.names[InsertRecord]), &UData, sizeof(UData));
      }
      else{
        // This page has room for the insertion - make the gap.
    
        for(loop = Original.records - 1; loop >= InsertRecord; loop--){
          memcpy(&(Original.names[loop+1]), &(Original.names[loop]), sizeof(UFDXRecord));
        }
        memcpy(&(Original.names[InsertRecord]), &UData, sizeof(UData));
    
        // Update links and record numbers
        Original.records++;
        Original.names[InsertRecord].link = RightChild;
        if(!InsertRecord) Original.backref = LeftChild;
    
        WritePage(Original, InsertPoint.Page[InsertPoint.Level-1]);
        // Is the page the root? If so, "remember" it.
        if(InsertPoint.Page[InsertPoint.Level-1] == UFirst.index) memcpy(&URoot, &Original, sizeof(UFDXPage));
      }
    }
    if(InsertRoom == 0){
      // This page has no room for the insertion
      UFDXPage New;
  
      // Copy the last 32/2 elements into the new page and remove from original, then write
      if(InsertRecord < UInfo.PromoteRecord){
        for(loop = UInfo.PromoteRecord; loop < 32; loop++) memcpy(&(New.names[loop - UInfo.PromoteRecord]), &(Original.names[loop]), sizeof(UFDXRecord));
        for(loop = UInfo.PromoteRecord - 1; loop >= InsertRecord; loop--) memcpy(&(Original.names[loop+1]), &(Original.names[loop]), sizeof(UFDXRecord));
        memcpy(&(Original.names[InsertRecord]), &UData, sizeof(UFDXRecord));
  
        // copy element (UInfo.PromoteRecord) (in Original) into UData (getting promoted)
        memcpy(&UData, &(Original.names[UInfo.PromoteRecord]), sizeof(UFDXRecord));
        
        // Fix linking
        New.backref = UData.link;
        UData.link   = UInfo.Pages + 1; // Ignored?
        Original.names[InsertRecord].link  = RightChild;
        if(!InsertRecord) Original.backref = LeftChild;
      }
      
      if(InsertRecord == UInfo.PromoteRecord){
        for(loop = UInfo.PromoteRecord; loop < 32; loop++) memcpy(&(New.names[loop - UInfo.PromoteRecord]), &(Original.names[loop]), sizeof(UData));
        New.backref = RightChild;
      }
      
      if(InsertRecord > UInfo.PromoteRecord){
        for(loop = InsertRecord; loop < 32; loop++) memcpy(&(New.names[loop-UInfo.PromoteRecord]), &(Original.names[loop]), sizeof(UFDXRecord));
        for(loop = UInfo.PromoteRecord + 1; loop < InsertRecord; loop++) memcpy(&(New.names[loop-UInfo.PromoteRecord-1]), &(Original.names[loop]), sizeof(UFDXRecord));
        memcpy(&(New.names[InsertRecord-UInfo.PromoteRecord-1]), &UData, sizeof(UFDXRecord));
  
        // copy element UInfo.PromoteRecord (in Original) into PData (getting promoted)
        memcpy(&UData, &(Original.names[UInfo.PromoteRecord]), sizeof(UFDXRecord));
  
        // Fix linking
        New.backref = UData.link;
        UData.link   = UInfo.Pages + 1; // Ignored?
        New.names[InsertRecord - UInfo.PromoteRecord - 1].link    = RightChild;
        if(!(InsertRecord - UInfo.PromoteRecord - 1)) New.backref = LeftChild;
        
      }
  
      Original.records = UInfo.PromoteRecord;
      New.records = (char) (32 - UInfo.PromoteRecord);
      
      WritePage(Original, InsertPoint.Page[InsertPoint.Level - 1]);
      WritePage(New, ++UInfo.Pages);
      
  
      // Add the middle element to the level above, with references to these pages
      InsertPoint.Level--;
      //return(AddRecord(UData, InsertPoint.Page[InsertPoint.Level], UInfo.Pages));
    }
  }
  if(InsertPoint.Status) return(2);
  if(InsertRoom > 0 )    return(1);
  if(InsertRoom == 0)    return(AddRecord(UData, InsertPoint.Page[InsertPoint.Level], UInfo.Pages));
  // Therefore InsertRoom < 0 which tell us we have an invalid index page
  SignalError(32);
  return(0);
} 
  

/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::AddRecord(PFDXRecord & PData, long LeftChild, long RightChild)
{
  int loop;
  int InsertRoom;
  int InsertRecord = InsertPoint.Record[InsertPoint.Level - 1];

  if(!(InsertPoint.Level)){    
    // No Insert point, create new root
    PFDXPage New;

    // Write root data
    memcpy(&(New.phones[0]), &PData, sizeof(PData));
    New.backref         = LeftChild;
    New.phones[0].link  = RightChild;
    New.records         = 1;

    // Write and cache new root, and update first page data
    WritePage(New, ++PInfo.Pages);
    memcpy(&PRoot, &New, sizeof(PFDXPage));
    PFirst.index = PInfo.Pages;
    PInfo.Level++;

    return(1);
  }

  // A level of braces is added so that the scope of Original and/or New is
  // lost by the time of recursive call.
  {
    // Ok, we need to fetch the appropriate page
    PFDXPage Original;
    ReadPage(Original, InsertPoint.Page[InsertPoint.Level-1]);

    // We now code the case of insertion
    InsertRoom = 32 - Original.records;
  
    if(InsertRoom > 0 || InsertPoint.Status){
      if(InsertPoint.Status){
        // We have a duplicate, and we should be in KillDupe mode if we got this far
        memcpy(&(Original.phones[InsertRecord]), &PData, sizeof(PData));
      }
      else{
        // This page has room for the insertion - make the gap.
    
        for(loop = Original.records - 1; loop >= InsertRecord; loop--){
          memcpy(&(Original.phones[loop+1]), &(Original.phones[loop]), sizeof(PFDXRecord));
        }
        memcpy(&(Original.phones[InsertRecord]), &PData, sizeof(PData));
    
        // Update links and record numbers
        Original.records++;
        Original.phones[InsertRecord].link = RightChild;
        if(!InsertRecord) Original.backref = LeftChild;
    
        WritePage(Original, InsertPoint.Page[InsertPoint.Level-1]);
        // Is the page the root? If so, "remember" it.
        if(InsertPoint.Page[InsertPoint.Level-1] == PFirst.index) memcpy(&PRoot, &Original, sizeof(PFDXPage));
      }
    }
    if(InsertRoom == 0){
      // This page has no room for the insertion
      PFDXPage New;
  
      // Copy the last 32/2 elements into the new page and remove from original, then write
      if(InsertRecord < PInfo.PromoteRecord){
        for(loop = PInfo.PromoteRecord; loop < 32; loop++) memcpy(&(New.phones[loop - PInfo.PromoteRecord]), &(Original.phones[loop]), sizeof(PFDXRecord));
        for(loop = PInfo.PromoteRecord - 1; loop >= InsertRecord; loop--) memcpy(&(Original.phones[loop+1]), &(Original.phones[loop]), sizeof(PFDXRecord));
        memcpy(&(Original.phones[InsertRecord]), &PData, sizeof(PFDXRecord));
  
        // copy element (PInfo.PromoteRecord) (in Original) into PData (getting promoted)
        memcpy(&PData, &(Original.phones[PInfo.PromoteRecord]), sizeof(PFDXRecord));
        
        // Fix linking
        New.backref = PData.link;
        PData.link   = PInfo.Pages + 1; // Ignored?
        Original.phones[InsertRecord].link  = RightChild;
        if(!InsertRecord) Original.backref = LeftChild;
      }
      
      if(InsertRecord == PInfo.PromoteRecord){
        for(loop = PInfo.PromoteRecord; loop < 32; loop++) memcpy(&(New.phones[loop - PInfo.PromoteRecord]), &(Original.phones[loop]), sizeof(PData));
        New.backref = RightChild;
      }
      
      if(InsertRecord > PInfo.PromoteRecord){
        for(loop = InsertRecord; loop < 32; loop++) memcpy(&(New.phones[loop-PInfo.PromoteRecord]), &(Original.phones[loop]), sizeof(PFDXRecord));
        for(loop = PInfo.PromoteRecord + 1; loop < InsertRecord; loop++) memcpy(&(New.phones[loop-PInfo.PromoteRecord-1]), &(Original.phones[loop]), sizeof(PFDXRecord));
        memcpy(&(New.phones[InsertRecord-PInfo.PromoteRecord-1]), &PData, sizeof(PFDXRecord));
  
        // copy element PInfo.PromoteRecord (in Original) into PData (getting promoted)
        memcpy(&PData, &(Original.phones[PInfo.PromoteRecord]), sizeof(PFDXRecord));
  
        // Fix linking
        New.backref = PData.link;
        PData.link   = PInfo.Pages + 1; // Ignored?
        New.phones[InsertRecord - PInfo.PromoteRecord - 1].link    = RightChild;
        if(!(InsertRecord - PInfo.PromoteRecord - 1)) New.backref = LeftChild;
        
      }
  
      Original.records = PInfo.PromoteRecord;
      New.records = (char) (32 - PInfo.PromoteRecord);
      
      WritePage(Original, InsertPoint.Page[InsertPoint.Level - 1]);
      WritePage(New, ++PInfo.Pages);
      
  
      // Add the middle element to the level above, with references to these pages
      InsertPoint.Level--;
      //return(AddRecord(PData, InsertPoint.Page[InsertPoint.Level], PInfo.Pages));
    }
  }
  if(InsertPoint.Status) return(2);
  if(InsertRoom > 0 )    return(1);
  if(InsertRoom == 0)    return(AddRecord(PData, InsertPoint.Page[InsertPoint.Level], PInfo.Pages));
  // Therefore InsertRoom < 0 which tell us we have an invalid index page
  SignalError(32);
  return(0);
}


/*
**    RawReadPage
**
** A set of functions to read pages from the various index files. This function
** doesn't attempt to be fancy, examine the mini-cache system or virtual cache
** system. It just fetches the page from secondary storage (and thus is used by
** the above mechanisms).
**
**    Parameters
**
**    Page    Object to copy page data to
**    PageNo  The page number to read
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int  FDNFUNC FrontDoorWNode::RawReadPage(NFDXPage & Page, long PageNo)
{
  int success = 1;
  
  success &= NFDX.Seek(PageNo * sizeof(Page), SEEK_SET);
  success &= NFDX.Read(&Page, sizeof(Page), 1, 1);

  return(success);
}


/*
** See overloaded version above
*/
FDNPREF int  FDNFUNC FrontDoorWNode::RawReadPage(UFDXPage & Page, long PageNo)
{
  int success = 1;
  
  success &= UFDX.Seek(PageNo * sizeof(Page), SEEK_SET);
  success &= UFDX.Read(&Page, sizeof(Page), 1, 1);

  return(success);
}


/*
** See overloaded version above
*/
FDNPREF int  FDNFUNC FrontDoorWNode::RawReadPage(PFDXPage & Page, long PageNo)
{
  int success = 1;
  
  success &= PFDX.Seek(PageNo * sizeof(Page), SEEK_SET);
  success &= PFDX.Read(&Page, sizeof(Page), 1, 1);

  return(success);
}


/*
**    RawWritePage
**
** A set of functions to write pages to the various index files. This function
** doesn't attempt to be fancy, examine the mini-cache system or virtual cache
** system. It just sends the page from secondary storage (and thus is used by
** the above mechanisms).
**
**    Parameters
**
**    Page    Object to copy page data from
**    PageNo  The page number to write
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int  FDNFUNC FrontDoorWNode::RawWritePage(NFDXPage & Page, long PageNo)
{
  int success = 1;
  
  success &= NFDX.Seek(PageNo * sizeof(Page), SEEK_SET);
  success &= NFDX.Write(&Page, sizeof(Page), 1, 1);
  return(success);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::RawWritePage(UFDXPage & Page, long PageNo)
{
  int success = 1;
  
  success &= UFDX.Seek(PageNo * sizeof(Page), SEEK_SET);
  success &= UFDX.Write(&Page, sizeof(Page), 1, 1);
  return(success);
}


/*
**    ReadPage / WritePage
**
** A set of functions to read and write pages into the various index files.
** These functions are more high level than their "Raw" equivalents and will
** attempt to access the mini-cache or virtual cache systems if possible.
**
**    Parameters
**
**    Page    Object to copy page data to or from
**    PageNo  The page number to read write
**
**
**    Special case
**
** Read  : If PageNo is the root page the root is copied from memory instead
**         unless the root records are records as zero, in which case the
**         root is actually copied to the cache.
**
** Write : If a page number one greater than the current maximum is specified
**         the page is appended to the index.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int  FDNFUNC FrontDoorWNode::ReadPage(NFDXPage & Page, long PageNo)
{
  int success;

  // If a root exists (ie. not empty index) we may have the record in memory
  if((NFirst.index) && (NFirst.index == PageNo) && NRoot.records){
    // Copy in mini cache contents, we can override this by setting proot->records=0 (to load root).
    memcpy(&Page, &NRoot, sizeof(Page));
    return(1);
  }

  // Check for the page in the virtual Cache
  if(CheckCache(Page, PageNo)) return(1);

  // Ok, we're really going to have to fetch it
  success = RawReadPage(Page, PageNo);

  // Experiment
  if(success) CommitCache(Page, PageNo);

  return(success);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::WritePage(NFDXPage & Page, long PageNo)
{
  int success;

  // Let's see if we can persuade the cache to deal with it
  if(CommitCache(Page, PageNo)) return(1);

  // Look's like we're going to have to do it
  success = RawWritePage(Page, PageNo);

  return(success);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::ReadPage(UFDXPage & Page, long PageNo)
{
  int success;

  // If a root exists (ie. not empty index) we may have the record in memory
  if((UFirst.index) && (UFirst.index == PageNo) && URoot.records){
    // Copy in mini cache contents, we can override this by setting proot->records=0 (to load root).
    memcpy(&Page, &URoot, sizeof(Page));
    return(1);
  }

  // Check for the page in the virtual Cache
  if(CheckCache(Page, PageNo)) return(1);

  // Ok, we're really going to have to fetch it
  success = RawReadPage(Page, PageNo);
  
  // Experiment
  if(success) CommitCache(Page, PageNo);

  return(success);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::WritePage(UFDXPage & Page, long PageNo)
{
  int success;

  // Let's see if we can persuade the cache to deal with it
  if(CommitCache(Page, PageNo)) return(1);

  // Look's like we're going to have to do it
  success = RawWritePage(Page, PageNo);

  return(success);
}
  

/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::ReadPage(PFDXPage & Page, long PageNo)
{
  int success;

  // If a root exists (ie. not empty index) we may have the record in memory
  if((PFirst.index) && (PFirst.index == PageNo) && PRoot.records){
    // Copy in mini cache contents, we can override this by setting proot->records=0 (to load root).
    memcpy(&Page, &PRoot, sizeof(Page));
    return(1);
  }

  success = RawReadPage(Page, PageNo);

  return(success);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::WritePage(PFDXPage & Page, long PageNo)
{
  int success = 1;
  
  success &= PFDX.Seek(PageNo * sizeof(Page), SEEK_SET);
  success &= PFDX.Write(&Page, sizeof(Page), 1, 1);
  return(success);
}


/*
**    WriteStub
**
** Write the first "fake" page of the index, the Stub. This contains
** information used for validation as well as the location of the
** root page in the index.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int  FDNFUNC FrontDoorWNode::WriteNFDXStub()
{
  int success = 0;
  
  NFDXPage * First = new NFDXPage;
  if(!First){
    SignalError(10);
    return(0);
  }
  memset(First, 0, sizeof(NFDXPage));
  NFirst.flags = 0xFFFFFFFFUL;
  NFirst.pagelen = sizeof(NFDXPage);
  memcpy(First, &NFirst, sizeof(NFirst));

  DefaultInfo.CompileTime = Time(NULL);

  success &= RawWritePage(*First, 0);    // Write the stub "page"
  success &= NFDX.Seek(256, SEEK_SET);
  success &= NFDX.Write(&DefaultInfo, sizeof(DefaultInfo), 1, 1);
  
  delete First;
  return(success);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::WriteUFDXStub()
{
  int success = 0;
  
  UFDXPage * First = new UFDXPage;
  if(!First){
    SignalError(10);
    return(0);
  }
  memset(First, 0, sizeof(UFDXPage));
  UFirst.flags = 0xFFFFFFFFUL;
  UFirst.pagelen = sizeof(UFDXPage);
  memcpy(First, &UFirst, sizeof(UFirst));

  DefaultInfo.CompileTime = Time(NULL);

  success &= RawWritePage(*First, 0);    // Write the stub "page"
  success &= UFDX.Seek(256, SEEK_SET);
  success &= UFDX.Write(&DefaultInfo, sizeof(DefaultInfo), 1, 1);
  
  delete First;
  return(success);
}


/*
** See overloaded version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::WritePFDXStub()
{
  int success = 0;
  
  PFDXPage * First = new PFDXPage;
  if(!First){
    SignalError(10);
    return(0);
  }
  memset(First, 0, sizeof(PFDXPage));
  PFirst.flags = 0xFFFFFFFFUL;
  PFirst.pagelen = sizeof(PFDXPage);
  memcpy(First, &PFirst, sizeof(PFirst));

  DefaultInfo.CompileTime = Time(NULL);

  success &= WritePage(*First, 0);    // Write the stub "page"
  success &= PFDX.Seek(256, SEEK_SET);
  success &= PFDX.Write(&DefaultInfo, sizeof(DefaultInfo), 1, 1);
  
  delete First;
  return(success);
}


/*
**    ReadStub
**
** Reads the first "fake" page of the index, the Stub. This contains
** information used for validation as well as the location of the
** root page in the index.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int  FDNFUNC FrontDoorWNode::ReadNFDXStub()
{
  int success;
  int loop;
  StubInfo * SInfo;
  NFDXPage * First;

  First = new NFDXPage;
  if(!First){
    SignalError(10);
    return(0);
  }
  SInfo = new StubInfo;
  if(!SInfo){
    SignalError(10);
    return(0);
  }

  memset(First, 0, sizeof(NFDXPage));

  success = RawReadPage(*First, 0);    // Fetch the stub "page"
  memcpy(SInfo, (char *) First + 256, sizeof(StubInfo));
  if(success){
    memcpy(&NFirst, First, sizeof(NFirst));
  }
  // Validate the elements
  if(SInfo->ZeroWord || NFirst.pagelen!=sizeof(PFDXPage) || SInfo->RevisionMaj!=2 || SInfo->RevisionMin!=3){
    SignalError(26);
    success = 0;
  }
  if(success && !(Flags & WFDNodeOverWrite) && strnicmp(DefaultInfo.NodeExt+1, "CUR", 3)){
    // We need to read the current nodelist extension
    for(loop=1; loop<=3; loop++) DefaultInfo.NodeExt[loop] = SInfo->NodeExt[loop];
  }
  delete SInfo;
  delete First;
  return(success);
}


/*
** See similar version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::ReadUFDXStub()
{
  int success;
  StubInfo * SInfo;
  UFDXPage * First;

  First = new UFDXPage;
  if(!First){
    SignalError(10);
    return(0);
  }
  SInfo = new StubInfo;
  if(!SInfo){
    SignalError(10);
    return(0);
  }

  memset(First, 0, sizeof(UFDXPage));

  success = RawReadPage(*First, 0);    // Fetch the stub "page"
  memcpy(SInfo, (char *) First + 256, sizeof(StubInfo));
  if(success){
    memcpy(&UFirst, First, sizeof(UFirst));
  }
  // Validate the elements
  if(UFirst.pagelen!=sizeof(UFDXPage)){
    success = 0;
    SignalError(27);
  }
  delete SInfo;
  delete First;
  return(success);
}


/*
** See similar version above for more details
*/
FDNPREF int  FDNFUNC FrontDoorWNode::ReadPFDXStub()
{
  int success;
  StubInfo * SInfo;
  PFDXPage * First;

  First = new PFDXPage;
  if(!First){
    SignalError(10);
    return(0);
  }
  SInfo = new StubInfo;
  if(!SInfo){
    SignalError(10);
    return(0);
  }

  memset(First, 0, sizeof(PFDXPage));

  success = RawReadPage(*First, 0);    // Fetch the stub "page"
  memcpy(SInfo, (char *) First + 256, sizeof(StubInfo));
  if(success){
    memcpy(&PFirst, First, sizeof(PFirst));
  }
  // Validate the elements
  if(PFirst.pagelen!=sizeof(PFDXPage)){
    SignalError(28);
    success = 0;
  }
  delete SInfo;
  delete First;
  return(success);
}


FDNPREF int  FDNFUNC FrontDoorWNode::WritePFDAStub()
{
  int success = 0;
  
  FirstPage * First = (FirstPage *) new char[sizeof(FDNPhoneRec)];
  if(!First){
    SignalError(10);
    return(0);
  }
  memset(First, 0, sizeof(FDNPhoneRec));
  First->flags = 0xFFFFFFFFUL;
  First->pagelen = sizeof(FDNPhoneRec);

  success &= PFDA.Seek(0, SEEK_SET);
  success &= PFDA.Write(First, sizeof(FDNPhoneRec), 1, 1);
  
  delete First;
  return(success);
}


/*
**    AddTrail
**
** Is a directory name passed in is not empty, and does not end in a ":"
** or a '\' character, this function adds '\' to the end.
**
*/
FDNPREF char FDNFUNC *FrontDoorWNode::AddTrail(char *rawfile)
{
  if(strlen(rawfile) && rawfile[strlen(rawfile)-1]!='\\' && rawfile[strlen(rawfile)-1]!=':') strcat(rawfile, "\\");
  return(rawfile);
}


/*
**
**    Time()
**
** This function returns a unix like timestamp - the number of seconds that
** have elapsed since January 1, 1970 until the current time, in local time
**
** This is merely a modified function for the JAMAPI function written by
** Mats Wallin.
**
**    Used with permission.
**
*/
FDNPREF unsigned long FDNFUNC FrontDoorWNode::Time(unsigned long * pTime)
{
  
#if defined(__MSDOS__) || defined(_WINDOWS) || defined(__DOS__) || defined(__NT__)
    #if defined(__ZTC__) || defined(_MSC_VER) || defined(_QC) || defined(__WATCOMC__)
        #if defined(__ZTC__)
            struct dos_date_t   d;
            struct dos_time_t   t;
        #elif defined (_MSC_VER) || defined(_QC)
            struct _dosdate_t   d;
            struct _dostime_t   t;
        #elif defined (__WATCOMC__)
            struct dosdate_t    d;
            struct dostime_t    t;    
        #endif

        struct tm m;
        unsigned long ti;

        #if defined(__ZTC__)
            dos_getdate(&d);
            dos_gettime(&t);
        #elif defined (_MSC_VER) || defined(_QC) || (__WATCOMC__)
            _dos_getdate(&d);
            _dos_gettime(&t);
        #endif

        m.tm_year = d.year - 1900;
        m.tm_mon  = d.month - 1;
        m.tm_mday = d.day;
        m.tm_hour = t.hour;
        m.tm_min  = t.minute;
        m.tm_sec  = t.second;
    #elif defined(__TURBOC__) || defined(__BORLANDC__) || defined(__TSC__)
        struct date   d;
        struct time   t;
        struct tm     m;
        unsigned long ti;

        getdate(&d);
        gettime(&t);

        m.tm_year = d.da_year - 1900;
        m.tm_mon  = d.da_mon - 1;
        m.tm_mday = d.da_day;
        m.tm_hour = t.ti_hour;
        m.tm_min  = t.ti_min;
        m.tm_sec  = t.ti_sec;
    #endif
#elif defined(__OS2__)
    DATETIME      DT;
    struct tm     m;
    unsigned long ti;

    DosGetDateTime(&DT);

    m.tm_year = DT.year - 1900;
    m.tm_mon  = DT.month - 1;
    m.tm_mday  = DT.day;
    m.tm_hour = DT.hours;
    m.tm_min  = DT.minutes;
    m.tm_sec  = DT.seconds;
#endif

#if defined(__MSDOS__) || defined(_WINDOWS) || defined(__OS2__) || \
    defined(__NT__) || defined(__DOS__)

    unsigned long Days;
    int Years;
    int _mdays [13] =
            {
/* Jan */   0,
/* Feb */   31,
/* Mar */   31+28,
/* Apr */   31+28+31,
/* May */   31+28+31+30,
/* Jun */   31+28+31+30+31,
/* Jul */   31+28+31+30+31+30,
/* Aug */   31+28+31+30+31+30+31,
/* Sep */   31+28+31+30+31+30+31+31,
/* Oct */   31+28+31+30+31+30+31+31+30,
/* Nov */   31+28+31+30+31+30+31+31+30+31,
/* Dec */   31+28+31+30+31+30+31+31+30+31+30,
/* Jan */   31+28+31+30+31+30+31+31+30+31+30+31
            };
    
    /*Get number of years since 1970*/
    Years = m.tm_year - 70;

    /*Calculate number of days during these years,*/
    /*including extra days for leap years         */
    Days = Years * 365 + ((Years + 1) / 4);

    /*Add the number of days during this year*/
    Days += _mdays [m.tm_mon] + m.tm_mday - 1;
    if((m.tm_year & 3) == 0 && m.tm_mon > 1)
        Days++;

    /*Convert to seconds, and add the number of seconds this day*/
    ti =   (((unsigned long) Days * 86400L) + ((unsigned long) m.tm_hour * 3600L) +
           ((unsigned long) m.tm_min * 60L) + (unsigned long) m.tm_sec);

    if(pTime)
        *pTime = ti;

    return(ti);
#else
    return(time(pTime));
#endif
}


FDNPREF void FDNFUNC FrontDoorWNode::SetTreeFlags(char Index, long Flags, char PromoteRecord)
{
  if((PromoteRecord < 1) || (PromoteRecord > 31)) PromoteRecord = 16;
  switch(Index){
    case NFDXIndex :
      NInfo.Flags = Flags;
      NInfo.PromoteRecord = PromoteRecord;
      break;
    case UFDXIndex :
      UInfo.Flags = Flags;
      UInfo.PromoteRecord = PromoteRecord;
      break;
    case PFDXIndex :
      PInfo.Flags = Flags;
      PInfo.PromoteRecord = PromoteRecord;
      break;    
  }
}


FDNPREF    int FDNFUNC FrontDoorWNode::CheckCache(NFDXPage & , long ) { return(0); }
FDNPREF    int FDNFUNC FrontDoorWNode::CommitCache(NFDXPage & , long ) { return(0); }
FDNPREF    int FDNFUNC FrontDoorWNode::CheckCache(UFDXPage & , long ) { return(0); }
FDNPREF    int FDNFUNC FrontDoorWNode::CommitCache(UFDXPage & , long ) { return(0); }



FDWNTreeInfo::FDWNTreeInfo(){
  memset(this, 0, sizeof(FDWNTreeInfo));
  PromoteRecord = 16;
}
