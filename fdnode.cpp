// A 'C++' class implementation for the FrontDoor nodelist indices.
// Written by Colin Turner; 2:443/13@fidonet
// Copyright 1995-1998

// Initial research by Bill Birrell and Colin Turner.

// FrontDoor is a registered trademark of Joaquim Homrighausen

/****************************************************************************/
/*                                                                          */
/* Class: FrontDoorNode                                                     */
/* Purpose: Reading FrontDoor(TM) nodelist indices and nodelist files       */
/*                                                                          */
/* Related Classes: FDNFind                                                 */
/*                                                                          */
/****************************************************************************/


#ifndef __FDNODE_H_
#include "fdnode.h"
#endif

#if defined(_WINDOWS) || defined(_Windows) || defined(__WINDOWS__)
#  define FDN_WINDOWS
#  include "windows.h"
#elif defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
#  define FDN_DOS
#  include <dos.h>
#  if defined(__WATCOMC__)
#    include <i86.h>
#  endif
#elif defined(__OS2__) || defined(OS2)
#  define FDN_OS2
#  define INCL_DOSNLS
#  include <os2.h>
#endif

#ifndef  FDN_CustomCheckFile
#include <dos.h>
#endif

static char *ListFileName[]={
  "NODELIST.XXX",
  "FDNODE.FDA",
  "FDNET.PVT",
  "FDPOINT.PVT"
};


/* Much of the configuration data, and history/todo list have been moved to */
/* The top and bottom of FDNODE.H, please see that file for more.           */

/****************************************************************************/
/*                                                                          */
/*                   P U B L I C   F U N C T I O N S                        */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                  S E R V I C E   F U N C T I O N S                       */
/****************************************************************************/

/*
**    GetNodeExt
**
** This function allows the application to determine the nodelist extension
** as stored in the Nodelist Database. This is returned as a string, because
** some values like "PVT" are possible.
**
**    Parameters
**
**    buffer    A region of memory to place the string (4 characters needed)
**
**    Returns
**
**    A pointer to the modified buffer
**/
FDNPREF char FDNFUNC *FrontDoorNode::GetNodeExt()
{
  return(NodeExt);
}


/*
**    Find  (Address variant)
**
** This function attempts to initialise the FDNFind block passed with the
** information relevant to the specified nodelist address.
**
**    Returns
**
**    1 on failure, 0 on success.
*/
FDNPREF int FDNFUNC FrontDoorNode::Find(FDNFind& fblock, unsigned short int zone, unsigned short int net, unsigned short int node, unsigned short point)
{
  char search_key[17];
  long dud;

  if(IsFrozen()){
    fblock.Parent = this;
    fblock.offset = 0;
    SignalError(29);
    return(1);
  }
  fblock.searchtype=1;
  strcpy(search_key, MakeKey(zone, net, node, point, 0));
  dud=GetNFDXOffset(search_key, first_n.index, fblock);
  if(dud && dud!=0xFFFFFFFFL && fblock.Filter()) return(0);
  else return(1);
}


/*
**    Find (Username variant)
**
** This function attempts to initialise the FDNFind block passed with the
** information relevant to the passed username. (First match).
**
**    Parameters
**
**    UserName should have surname first, then other names in normal order.
**    A '_' character may be appended to the name to match all users with
**    this start.
**
**    Returns
**
**    1 on failure, 0 on success.
*/
FDNPREF int FDNFUNC FrontDoorNode::Find(FDNFind& fblock, const char FDNDATA *UserName)
{
  int loop, length, stillleft=1;
  char search_key[20];

  if(IsFrozen()){
    fblock.Parent=this;
   fblock.offset=0;
   fblock.finished=1;
    SignalError(29);
    return(1);
  }
  fblock.Parent=this;
  // Let's clean the find class ready for use. This is deliberately not done
  // in the constructor so the same class can be used for repeated searches.
  fblock.searchtype=fblock.level=0;
  fblock.finished=0;

  // Now we generate our search key to the correct format.
  for(loop=0; loop<16; loop++) search_key[loop]=0x20;
  length=strlen(UserName);
  if(length>=15) memcpy(search_key+1, UserName, 15);
  else memcpy(search_key+1, UserName, length);
#ifdef FDN_WINDOWS
  AnsiUpperBuff(search_key+1,15);
  AnsiToOemBuff(search_key+1,search_key+1,15);
#else
  for(loop=0; loop<16; loop++) search_key[loop]=(char) ToUpper(search_key[loop]);
#endif
  search_key[0]=0x18;

  if(GetUFDXOffset(search_key, first_u.index, fblock) !=0){
    while(!fblock.Filter() && stillleft){
      stillleft=!UGetNextKey(fblock);
    }
    if(!fblock.Filter() || !stillleft){
      fblock.finished=1;
      return(1);
    }
    return(0);
  }
  else{
    fblock.finished=1;
    return(1);
  }
}


/*
**    Find  (Continued search variant)
**
** This function is used to continue the search already started
** with details in the FDNFind block. Searches that may be continued
** (to find the next match) are those started with:
**
** Find   (Username variant)
** GetZones, GetNets, GetNodes, GetPoints
**
**    Parameters
**
**    A FDNFind block already initialised by one of the above
**    functions.
**
**    Returns
**
**    1 on failure, 0 on success.
**
*/
FDNPREF int FDNFUNC FrontDoorNode::Find(FDNFind& fblock)
{
  int stillleft=1;
  if(IsFrozen()){
    fblock.Parent=this;
    fblock.offset=0;
    SignalError(29);
    return(1);
  }
  if((fblock.UnixStamp < UnixStamp) || (fblock.Parent !=this)){
    SignalError(13);
    return(1); // Class has been frozen since last search
  }
  switch(fblock.searchtype){
    case 0 : // This is a userfind search.
      stillleft=!UGetNextKey(fblock);
      if(!stillleft){
        fblock.finished=1;
        return(1);
      }
      else while(stillleft && !fblock.Filter()){
        stillleft=!UGetNextKey(fblock);
      }
      return(!stillleft);
    case 2 : // This is zone search.
      return(GetNextZone(fblock));
    case 3 : // This is net search.
      return(GetNextNet(fblock));
    case 4 : // This is a node search.
      return(GetNextNode(fblock));
    case 5 : // This is a point search.
      return(GetNextPoint(fblock));
    default : // Unknown search type, block is probably corrupt, terminate search
      fblock.finished=1;
      return(1);
  }
}


/*
**    GetZones
**
** Initiates a search for the Node entries for each Zone.
** Two versions exist, one which starts with the first Zone in
** the index, and the other which starts with the specified
** Zone.
**
**    Returns
**
**    1 on failure, 0 on success.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetZones(FDNFind& fblock)
{
  return(GetZones(fblock, 0));
}


FDNPREF int FDNFUNC FrontDoorNode::GetZones(FDNFind& fblock, unsigned short start)
{
  int found=0, quit=0;
  long dud;
  char search_key[17];

  if(IsFrozen()){
    fblock.Parent=this;
    fblock.offset=0;
    fblock.finished=1;
    SignalError(29);
    return(1);
  }
  fblock.searchtype=2;
  fblock.finished=0;
  while(!found && !quit){
    strcpy(search_key, MakeKey(start, (unsigned short) -1, (unsigned short) -1, (unsigned short) -1, 0));
    dud=GetNFDXOffset(search_key, first_n.index, fblock);
    if(dud){
      if(dud==0xFFFFFFFFL) quit=1; // off end of index
      else dud=NGetNextKey(fblock); // otherwise we need to move on one more
    }
    if(!quit){
      if(fblock.zone==fblock.net && !fblock.node && !fblock.point){
        // We've found a ZC, pass it through Filter
        if(fblock.Filter()) found=1;
        else start=fblock.zone; // failed filter
      }
      else{
        if(fblock.net >= fblock.zone) start=fblock.zone; // We must have passed any ZC, move to next zone
        else{
          strcpy(search_key, MakeKey(fblock.zone, fblock.zone, 0, 0, 0));
          dud=GetNFDXOffset(search_key, first_n.index, fblock);
          if(dud){
            if(dud==0xFFFFFFFFL) quit=1; // off index
            else{
              if(fblock.Filter()) found=1; // We found a ZC after an explicit search for one
              else start=fblock.zone; // failed filter, move on
            }
          }
        }
      }
    }
  }
  if(!found) fblock.finished=1;
  return(!found);
}


/*
**    GetNets
**
** Initiates a search for the Node entries for each Net in the
** specified Zone.
** Two versions exist, one which starts with the first Net in
** the Zone, and the other which starts with the specified
** Net.
**
**    Returns
**
**    1 on failure, 0 on success.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetNets(FDNFind& fblock, unsigned short zone)
{
  return(GetNets(fblock, zone, 0));
}


FDNPREF int FDNFUNC FrontDoorNode::GetNets(FDNFind& fblock, unsigned short zone, unsigned short start)
{
  int found=0, quit=0;
  long dud;
  char search_key[17];

  if(IsFrozen()){
    fblock.finished=1;
    fblock.Parent=this;
    fblock.offset=0;
    SignalError(29);
    return(1);
  }
  fblock.searchtype=3;
  fblock.finished=0;
  fblock.zone=zone;
  while(!found && !quit && fblock.zone==zone){
    strcpy(search_key, MakeKey(zone, start, (unsigned short) -1, (unsigned short) -1, 0));
    dud=GetNFDXOffset(search_key, first_n.index, fblock);
    if(dud){
      if(dud==0xFFFFFFFFL) quit=1;  // off end of index
      else dud=NGetNextKey(fblock); // otherwise we need to move on one more
    }
    if(!quit){
      if(fblock.zone==zone && !fblock.node && !fblock.point){
        // We've found an NC, pass it through Filter
        if(fblock.Filter()) found=1;
        else start=fblock.net; // failed filter
      }
      else{
        if(fblock.node) start=fblock.net; // We must have passed any NC, move to next zone
        if(fblock.zone!=zone) quit=1; // Moved pass the correct zone
      }
    }
  }
  if(!found) fblock.finished=1;
  return(!found);
}


/*
**    GetNodes
**
** Initiates a search for the Node entries for each Node in the
** specified Net.
** Two versions exist, one which starts with the first Node in
** the Net, and the other which starts with the specified
** Node.
**
**    Returns
**
**    1 on failure, 0 on success.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetNodes(FDNFind& fblock, unsigned short zone, unsigned short net)
{
  return(GetNodes(fblock, zone, net, 0));
}


FDNPREF int FDNFUNC FrontDoorNode::GetNodes(FDNFind& fblock, unsigned short zone, unsigned short net, unsigned short start)
{
  int found=0, quit=0;
  long dud;
  char search_key[17];

  if(IsFrozen()){
    fblock.finished=1;
    fblock.offset=0;
    fblock.Parent=this;
    SignalError(29);
    return(1);
  }
  fblock.searchtype=4;
  fblock.finished=0;
  fblock.zone=zone;
  fblock.net=net;
  while(!found && !quit && fblock.zone==zone && fblock.net==net){
    strcpy(search_key, MakeKey(zone, net, start, 0, 0));
    dud=GetNFDXOffset(search_key, first_n.index, fblock);
    if(dud){
      if(dud==0xFFFFFFFFL) quit=1; // off end of index
      else{
        if(fblock.Filter()) found=1;
        else start=(unsigned short) (fblock.node+1);
      }
    }
    else{
      if(fblock.point) start=(unsigned short) (fblock.node+1); // Moved pass the node, look for next one
      if(fblock.zone!=zone || fblock.net!=net) quit=1; // Moved pass the net
      if(!quit && !fblock.point){
        // We didn't find the node we were looking for, but this is the next one in the net
        if(fblock.Filter()) found=1;
        else start=(unsigned short) (fblock.node+1);
      }
    }
  }
  if(!found) fblock.finished=1;
  return(!found);
}


/*
**    GetPoints
**
** Initiates a search for the Node entries for each Point of the
** specified Node.
** Two versions exist, one which starts with the first Point of the
** the Node, and the other which starts with the specified
** Point.
**
**    Returns
**
**    1 on failure, 0 on success.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetPoints(FDNFind& fblock, unsigned short zone, unsigned short net, unsigned short node)
{
  return(GetPoints(fblock, zone, net, node, 0));
}


FDNPREF int FDNFUNC FrontDoorNode::GetPoints(FDNFind& fblock, unsigned short zone, unsigned short net, unsigned short node, unsigned short start)
{
  int found=0, quit=0;
  long dud;
  char search_key[17];

  if(IsFrozen()){
    fblock.finished=1;
    fblock.offset=0;
    fblock.Parent=this;
    SignalError(29);
    return(1);
  }
  fblock.searchtype=5;
  fblock.finished=0;
  fblock.zone=zone;
  fblock.net=net;
  fblock.node=node;
  while(!found && !quit && fblock.zone==zone && fblock.net==net && fblock.node==node){
    strcpy(search_key, MakeKey(zone, net, node, start, 0));
    dud=GetNFDXOffset(search_key, first_n.index, fblock);
    if(dud){
      if(dud==0xFFFFFFFFL) quit=1; // off end of index
      else{
        if(fblock.Filter()) found=1;
        else start= (unsigned short) (fblock.point+1);
      }
    }
    else{
      if(fblock.zone!=zone || fblock.net!=net || fblock.node!=node) quit=1; // Moved pass the node
      if(!quit){
        // We didn't find the point we were looking for, but this is the next one for this node
        if(fblock.Filter()) found=1;
        else start= (unsigned short) (fblock.point+1);
      }

    }
  }
  if(!found) fblock.finished=1;
  return(!found);
}


/*
**    ClearFDAStore
**
** This private function is used to blank the FDAStore, which the
** class uses to store and parse and store fetched information.
** This is done to ensure safe (ie. blank) returns for failed
** fetches. 
*/
FDNPREF void FDNFUNC FrontDoorNode::ClearFDAStore(void)
{
  NLCurOffset=0;
  NodelistLine[0]=0;
  FDAStore->Name[0]=FDAStore->Telephone[0]=FDAStore->Location[0]=FDAStore->User[0]=0;
  FDAStore->Zone=FDAStore->NetNo=FDAStore->NodeNo=FDAStore->Point=0;
  FDAStore->Capability=0;
  FDAStore->MaxBaud=0;
  FDAStore->Cost=0;
  FDAStore->Erased=0;
  FDAStoreSpeed=0;
}
  

/*
**    GetNLine
**
** This function accepts an index database offset (composed of
** file and offset parts), retrieves the data from the file (whether
** a text nodelist, or the internal binary database) and parses it into
** FDAStore.
**
** It is often called on the first data fetch for a node entry, and
** subsequently returns already parsed data.
**
**    Parameters
**
**    An database offset. First byte indicates file, other are offset.
**
**    Returns
**
**    A pointer to the actual raw nodelist line if applicable.
**
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetNLine(long offset)
{
  int file;
  char speed[30];
  long suboffset=0;

  if(IsFrozen()){
    SignalError(29);
    return(NULL);
  }
  if(offset==0L || offset==0xFFFFFFFFL){
    ClearFDAStore();
    return(NULL);
  }
  if(offset==NLCurOffset) return(NULL); // We're already on this record.

  // We have to load this record, Let's see what file it's in.
  switch((int) ((offset & 0xFF000000L) >> 24)){
    case 0x00 : file=0; break;
    case 0x10 : file=2; break;
    case 0x20 : file=3; break;
    case 0x01 : file=1; break;
    default   : file=9; break;
  }
  suboffset = offset & 0x00FFFFFFL;

  // Index points to a record in an invalid database file
  if(file==9){
    SignalError(22);
    ClearFDAStore();
    return(NodelistLine);
  }    
    

  if(Reopen[file]) DataFile[file].Open();
  if(!DataFile[file].GetStatus()){
    SignalError(file+18); // Unable to open specified file
    ClearFDAStore();
    return(NodelistLine);
  }
  if(file!=1){
    DataFile[file].Seek(suboffset, SEEK_SET);
    FCRGetS(NodelistLine, (NODELINELENGTH-1), DataFile[file]);
    CSVField(NodelistLine, 0, FDAStore->Name);
    CSVField(NodelistLine, 3, FDAStore->Telephone);
    CSVField(NodelistLine, 1, FDAStore->Location);
    CSVField(NodelistLine, 2, FDAStore->User);
    CSVField(NodelistLine, 4, speed);
    FDAStore->Name[30]=0;
    FDAStore->Telephone[40]=0;
    FDAStore->Location[40]=0;
    FDAStore->User[36]=0;
    FDAStoreSpeed = atol(speed);
  }
  else{
    DataFile[file].Seek(suboffset*sizeof(FDANodeRec), SEEK_SET);
    DataFile[file].Read(FDAStore, sizeof(FDANodeRec), 1, 1);
    NodelistLine[0]=0;
    ConvertToC(FDAStore->Name);
    ConvertToC(FDAStore->Location);
    ConvertToC(FDAStore->Telephone);
    ConvertToC(FDAStore->User);
#ifdef FDN_WINDOWS
    OemToAnsi(FDAStore->Name, FDAStore->Name);
    OemToAnsi(FDAStore->Location, FDAStore->Location);
    OemToAnsi(FDAStore->Telephone, FDAStore->Telephone);
    OemToAnsi(FDAStore->User, FDAStore->User);
#endif
    FDAStoreSpeed = GetSpeedFromFDA(FDAStore->MaxBaud);
  }
  NLCurOffset=offset;
  if(Reopen[file]) DataFile[file].Close();
  return(NodelistLine);
}


/*
**    GetNLine  (Find block variant)
**
** Calls the above function with the encapsulated
** offset.
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetNLine(FDNFind& fblock)
{
  return(GetNLine(fblock.offset));
}


/*
**    GetSysop
**
** Returns the Sysop name for the relevant entry with details
** in the find block. (Already initalised by a search function).
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetSysop(FDNFind& fblock)
{
  if(IsFrozen()){
    SignalError(29);
    return(NULL);
  }
  GetNLine(fblock.offset);
  return(FDAStore->User);
}


/*
**    GetLocation
**
** Returns the Location data for the relevant entry with details
** in the find block. (Already initalised by a search function).
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetLocation(FDNFind& fblock)
{
  if(IsFrozen()){
    SignalError(29);
    return(NULL);
  }
  GetNLine(fblock.offset);
  return(FDAStore->Location);
}


/*
**    GetSysName
**
** Returns the System name for the relevant entry with details
** in the find block. (Already initalised by a search function).
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetSysName(FDNFind& fblock)
{
  if(IsFrozen()){
    SignalError(29);
    return(NULL);
  }
  GetNLine(fblock.offset);
  return(FDAStore->Name);
}


/*
**    GetSpeed
**
** Returns the Speed field for the relevant entry with details
** in the find block. (Already initalised by a search function).
*/
FDNPREF unsigned long FDNFUNC FrontDoorNode::GetSpeed(FDNFind& fblock)
{
  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  GetNLine(fblock.offset);
  return(FDAStoreSpeed);
}



/*
**    GetSpeedFromFDA
**
** This private function is used to calculate the Speed field
** (as returned above) from the values stored in an Internal database
** entry.
**
**    Parameters
**
**    Takes the maxbaud field from FDNODE.FDA
*/
FDNPREF unsigned long FDNFUNC FrontDoorNode::GetSpeedFromFDA(unsigned char maxbaud)
{
  switch(NLDBRevision){

   case 0 : // FD 2.20 etc.
    switch((int) maxbaud){
      case 2  : return(300L);
      case 4  : return(1200L);
      case 5  : return(2400L);
      case 6  : return(4800L);
        case 10 : return(7200L);
        case 7  : return(9600L);
        case 11 : return(12000L);
        case 12 : return(14400L);
        case 13 : return(16800L);
        case 14 : return(19200L);
        case 15 : return(38400L);
        case 16 : return(57600L);
        case 17 : return(64000L);
        default : return(0L); // Something wrong here!
      }


    default : // FD 2.30 and above
      switch((int) maxbaud){
        case 1  : return(300L);
        case 2  : return(1200L);
        case 3  : return(2400L);
        case 4  : return(4800L);
        case 5  : return(9600L);
        case 6  : return(14400L);
        case 7  : return(16800L);
        case 8  : return(19200L);
        case 9  : return(21600L);
        case 10 : return(28800L);
        case 11 : return(33600L);
        case 12 : return(38400L);
        case 13 : return(57600L);
        case 14 : return(64000L);
      case 15 : return(76800L);
        case 16 : return(115200L);
        case 17 : return(128000L);
        case 18 : return(12000L);
        case 19 : return(24000L);
        case 20 : return(31200L);
        case 21 : return(256000L);
        default : return(14400L);
      }
  }
}

   
/*
**    GetNumber
**
** Returns the raw untranslated number for the relevant entry with
** details in the find block. (Already initalised by a search function).
**
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetNumber(FDNFind& fblock)
{
  if(IsFrozen()) return(NULL);
  GetNLine(fblock.offset);
  return(FDAStore->Telephone);
}


/*
**    GetFlags
**
** The behaviour of this function is partially dependant upon the
** whether FDN_NoFlagBuild has been #defined in FDNUSER.H
**
** In general this returns the Flags field of the relevant
** nodelist line (attached to details in the find block).
**
** If the entry is stored in FDNODE.FDA however, a NULL string
** will be returned if FDN_NoFlagBuild has been defined. Otherwise
** a flags string will be constructed and returned.
**
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetFlags(FDNFind& fblock)
{
  if(IsFrozen()){
    SignalError(29);
    return(NULL);
  }
  #ifdef FDN_NoFlagBuild
  if(fblock.IsFDA()) return(NULL);
  #endif
  GetNLine(fblock.offset);
  #ifndef FDN_NoFlagBuild
  if(fblock.IsFDA()) return(GetFlagsFromFDA(FDAStore->Capability));
  #endif
  return(CSVFieldStart(NodelistLine, 5));
}


#ifdef FDN_NoFlagBuild

/*
**    GetFDAFlags
**
** This function is only accessible if FDN_NoFlagBuild is defined.
** It returns the long int in the FDNODE.FDA record which defines
** the nodelist flags. If the #define is not made, a string will
** be constructed on GetFlags calls and this function is irrelevant.
*/
FDNPREF long FDNFUNC FrontDoorNode::GetFDAFlags(FDNFind& fblock)
{
  if(IsFrozen()){
    SignalError(29);
    return(0);
  }
  GetNLine(fblock.offset);
  return(FDAStore->Capability);
}

#else


/*
**    GetFlagsFromFDA
**
** This function is only included if FDN_NoFlagBuild is not defined.
** It creates a textual flags string for a record in FDNODE.FDA which
** has only a flags long int.
*/
FDNPREF char FDNFUNC *FrontDoorNode::GetFlagsFromFDA(unsigned long flags)
{
  *FlagBuild=0;
  switch(NLDBRevision){
   case 0 :
    if(flags & OldNLflagCM)   strcat(FlagBuild, "CM,");
    if(flags & OldNLflagMO)   strcat(FlagBuild, "MO,");
    if(flags & OldNLflagLO)   strcat(FlagBuild, "LO,");
    if(flags & OldNLflagMNP)  strcat(FlagBuild, "MNP,");

    if(flags & OldNLflagV32)  strcat(FlagBuild, "V32,");
    if(flags & OldNLflagV32B) strcat(FlagBuild, "V32B,");
    if(flags & OldNLflagV42)  strcat(FlagBuild, "V42,");
    if(flags & OldNLflagV42B) strcat(FlagBuild, "V42B,");
    if(flags & OldNLflagV33)  strcat(FlagBuild, "V33,");
    if(flags & OldNLflagV34)  strcat(FlagBuild, "V34,");

    if(flags & OldNLflagZYX)  strcat(FlagBuild, "ZYX,");
    if(flags & OldNLflagHST)  strcat(FlagBuild, "HST,");
    if(flags & OldNLflagH96)  strcat(FlagBuild, "H96,");
    if(flags & OldNLflagHST16)  strcat(FlagBuild, "H16,");
    if(flags & OldNLflagFAX)  strcat(FlagBuild, "FAX,");

    // HST14?

    if(flags & OldNLflagXA)   strcat(FlagBuild, "XA,");
    if(flags & OldNLflagXB)   strcat(FlagBuild, "XB,");
    if(flags & OldNLflagXC)   strcat(FlagBuild, "XC,");
    if(flags & OldNLflagXP)   strcat(FlagBuild, "XP,");
    if(flags & OldNLflagXR)   strcat(FlagBuild, "XR,");
    if(flags & OldNLflagXW)   strcat(FlagBuild, "XW,");
    if(flags & OldNLflagXX)   strcat(FlagBuild, "XX,");

    if(flags & OldNLflagUISDNA)  strcat(FlagBuild, "UISDNA,");
    if(flags & OldNLflagUISDNB)  strcat(FlagBuild, "UISDNB,");
      if(flags & OldNLflagUISDNC)  strcat(FlagBuild, "UISDNC,");

      if(flags & OldNLflagPEP)     strcat(FlagBuild, "PEP,");
      if(flags & OldNLflagMAX)     strcat(FlagBuild, "MAX,");
      break;

    default:
      if(flags & NLflagCM)   strcat(FlagBuild, "CM,");
      if(flags & NLflagMO)   strcat(FlagBuild, "MO,");
      if(flags & NLflagLO)   strcat(FlagBuild, "LO,");
      if(flags & NLflagMN)   strcat(FlagBuild, "MN,");

      if(flags & NLflagV32)  strcat(FlagBuild, "V32,");
      if(flags & NLflagV32B) strcat(FlagBuild, "V32B,");
      if(flags & NLflagV42)  strcat(FlagBuild, "V42,");
      if(flags & NLflagV42B) strcat(FlagBuild, "V42B,");
      if(flags & NLflagV34)  strcat(FlagBuild, "V34,");

      if(flags & NLflagZYX)  strcat(FlagBuild, "ZYX,");
      if(flags & NLflagHST)  strcat(FlagBuild, "HST,");
      if(flags & NLflagFAX)  strcat(FlagBuild, "FAX,");
      if(flags & NLflagX2C)  strcat(FlagBuild, "X2C,");
      if(flags & NLflagX2S)  strcat(FlagBuild, "X2S,");

      if(flags & NLflagXA)   strcat(FlagBuild, "XA,");
      if(flags & NLflagXB)   strcat(FlagBuild, "XB,");
      if(flags & NLflagXC)   strcat(FlagBuild, "XC,");
      if(flags & NLflagXP)   strcat(FlagBuild, "XP,");
      if(flags & NLflagXR)   strcat(FlagBuild, "XR,");
      if(flags & NLflagXW)   strcat(FlagBuild, "XW,");
      if(flags & NLflagXX)   strcat(FlagBuild, "XX,");

      if(flags & NLflagX75)   strcat(FlagBuild, "X75,");
      if(flags & NLflagV110L) strcat(FlagBuild, "V110L,");
      if(flags & NLflagV110H) strcat(FlagBuild, "V110H,");
      if(flags & NLflagV120L) strcat(FlagBuild, "V120L,");
      if(flags & NLflagV120H) strcat(FlagBuild, "V120H,");

      break;
  }

  // Strip trailing comma
  if(*FlagBuild) FlagBuild[strlen(FlagBuild)-1]=0;
  return(FlagBuild);

}

#endif


/*
**    GetPhoneData
**
** This function is used to retrive a translated phone number and the
** cost to dial a system.
**
**    Parameters
**
**    fblock    The FDNFind for the specified system.
**    buffer    For storing the translated number. NULL indicates no
**              translation will occur.
**
**    Returns
**
**    The cost to dial the system.
*/
FDNPREF unsigned short FDNFUNC FrontDoorNode::GetPhoneData(FDNFind & fblock, char * buffer)
{
  char * test = NULL;
  unsigned short TempCost=0;
  int Error = 0;

  if(IsFrozen()) Error = 29;
  if((fblock.UnixStamp < UnixStamp) || (fblock.Parent !=this)) Error = 13;

  if(Error){
   if(buffer) *buffer = 0;
   SignalError(Error);
   return(0xFFFF);
  }
  // If no CountryCode, Dial XLT impossible, we can continue though
  if(!NLInfo.CountryCode){
   if(buffer) *buffer = 0;
   SignalError(15);
  }
  else test = buffer;
  TempCost = GetPFDXData(fblock.GetNumber(), test, first_p.index);

  // Should we override the cost?
  if(NLDBRevision && fblock.IsFDA() && FDAStore->Cost!=0xFFFE) return(FDAStore->Cost);
  return(TempCost);
}


/****************************************************************************/
/*             I M P L E M E N T A T I O N  F U N C T I O N S               */
/****************************************************************************/

// FrontDoorNode()
// This is a constructor to keep the class safe if no path is passed to it.
// It initialises the class in a frozen state with an unknown path.
FDNPREF FrontDoorNode::FrontDoorNode()
{
  Constructor("", "", FDNodeCreateFrozen, 0);
}

// FrontDoorNode(directory name)
// This is the default constructor. It initialises the class ready for use
// NO checks are performed on the directory name, you must ensure it has a
// trailing backslash and that the directory is valid. This constructor
// assumes you want to keep ALL the files open for the lifetime of the class.
// This is fast, but uses many file handles.
FDNPREF FrontDoorNode::FrontDoorNode(const char FDNDATA *pathname, const char FDNDATA *path2, unsigned short NewTask)
{
  Constructor(pathname, path2, 0, NewTask);
}


// FrontDoorNode(directory name, flags)
// An overloaded constructor, this allows you to specify exactly what index
// and nodelist files should be held open for the duration of the class
// existence. Specify the flags given in FDNODE.H to signify that a file
// should be opened and closed after each use.
FDNPREF FrontDoorNode::FrontDoorNode(const char FDNDATA *pathname, const char *path2, unsigned short setflags, unsigned short NewTask)
{
  Constructor(pathname, path2, setflags, NewTask);
}


// ~FrontDoorNode()
// The destructor ensures that all active file handles are closed.
FDNPREF FrontDoorNode::~FrontDoorNode()
{
  Freeze(); // Close handles etc.
  delete FDAStore;
  if(!(Flags & FDNodeNoCacheP)){
    delete proot;
  }
  if(!(Flags & FDNodeNoCacheU)){
    delete uroot;
  }
  if(!(Flags & FDNodeNoCacheN)){
    delete nroot; // Remove cache allocation
  }
}


/*
**    SetNLDir
**
** Used to specify the Nodelist directory the class should use,
** typically only needed when a global instance is created and
** no directory could be passed to the constructor.
**
** This will only work if the Nodelist object is frozen.
*/
FDNPREF void FDNFUNC FrontDoorNode::SetNLDir(const char FDNDATA *pathname)
{
  if(!IsFrozen()) return;       // Not frozen!
  strcpy(NodelistDir, pathname);
  AddTrail(NodelistDir);
}


/*
**    SetSemDir
**
** Used to specify the Semaphore directory the class should use,
** typically only needed when a global instance is created and
** no directory could be passed to the constructor.
**
** This will only work if the Nodelist object is frozen.
*/
FDNPREF void FDNFUNC FrontDoorNode::SetSemDir(const char FDNDATA *pathname)
{
  if(!IsFrozen()) return;       // Not frozen!
  if(!*pathname) return;
  Flags = Flags & ~ FDNodeNoSem;
  strcpy(SemaphoreDir, pathname);
  AddTrail(SemaphoreDir);
}


/*
**    Freeze
**
** This function will instruct the class to relinquish any files it
** has opened (typically so the nodelist index can be accessed to
** write it).
*/
FDNPREF void FDNFUNC FrontDoorNode::Freeze()
{
  register int loop;

  if(Frozen) return;

  OnFreeze();

  NFDX.Close();
  UFDX.Close();
  PFDX.Close();
  PFDA.Close();
  for(loop=0; loop<4; loop++) DataFile[loop].Close();
  if(!(Flags & FDNodeNoSem) && ((unsigned) Instance != 0xFFFF)) DeleteInstance();

  Frozen = 1;
}           


/*
**    Thaw
**
** This function is used to restore the class from a frozen state
** possibly to reverse the effects of an earlier Freeze() function,
** or possibly because the object was created in a Frozen state.
** (Default constructor, or FDNodeCreateFrozen flag passed)
**
**    Returns
**
**    1 on success (ready for use), 0 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::Thaw()
{
  if(!IsFrozen()) return(1);

  if (InitClass()) {
    OnThaw (InstanceSemaphore [0] != '\0' ? InstanceSemaphore : NULL);
    Frozen = 0;
    return 1;
  }

  return 0;
}


/*
**    IsFrozen
**
** Allows the calling application to test if the Nodelist class is
** a frozen state or not.
**
**    Returns
**
**    1 on success, 0 on failure
*/
FDNPREF int FDNFUNC FrontDoorNode::IsFrozen()
{
  return(Frozen);
}


/*
**    GetError
**
** Returns the internal Error state of the class, which is set to
** a non zero value on certain error states (see the table in
** FDNODE.H). Also see SignalError
*/
FDNPREF int FDNFUNC FrontDoorNode::GetError()
{
  return(error);
}


/*
**    ClearError
**
** Allows the programmer to clear the internal Error state. It is
** never required to call this function. It is provided for
** convenience.
*/
FDNPREF void FDNFUNC FrontDoorNode::ClearError()
{
  error=0;
}


/*
**    AutoFreezeThaw
**
** A significant service function. This checks for the presence
** of specific semaphores which should trigger a Freeze or Thaw of
** the class. The application using the class should attempt to call
** this function periodically.
**
**    Returns
**
**    0   No change - index remains closed
**    1   No change - index remains open
**    2   Index has just been Frozen
**    3   Index has just been Thawed
**    4   Index should have been Thawed - but that failed
*/
FDNPREF int FDNFUNC FrontDoorNode::AutoFreezeThaw()
{
  int ShouldFreeze = 0;
  char filename[72];
  char buffer[17];

  itoa(Task, buffer, 10);

  strcpy(filename, SemaphoreDir);
  strcat(filename, "FDNC.NOW");
  ShouldFreeze |= CheckFile(filename);
  strcpy(filename, SemaphoreDir);
  strcat(filename, "FDNLFREZ.ALL");
  ShouldFreeze |= CheckFile(filename);
  strcpy(filename, SemaphoreDir);
  strcat(filename, "FDNLFREZ.");
  strcat(filename, buffer);
  ShouldFreeze |= CheckFile(filename);
  
  if(ShouldFreeze){
    if(IsFrozen()) return(0);
    else{
      Freeze();
      return(2);
    }
  }
  else{
    if(IsFrozen()){
      if(Thaw()) return(3); else return(4);
   }
    else{
      return(1);
    }
  }
}


//***************************************************************************
//*                 P R I V A T E   F U N C T I O N S                       *
//***************************************************************************


#ifndef  FDN_CustomCheckFile

/*
**    CheckFile
**
** This function checks for the existence of a specific file.
**
**    Returns
**
**    1 on success (found), 0 on failure
*/
FDNPREF int FDNFUNC FrontDoorNode::CheckFile(char *FileName)
{
  // Function to check for the existance of a file. Note that if it finds
  // something, it continues looking till the find_next terminates (in order
  // to solve a known NetWare "bug". Returns a structure pointer with file data
  struct find_t dummy;

  if(!_dos_findfirst(FileName, 0, &dummy)){
   // We found at least one matching filename, preserve the real details
   while(!_dos_findnext(&dummy));
   return(1);
  }
  else return(0);
}

#endif


/*
**    AddTrail
**
** This function is used to ensure that a trailing backslash
** is present on directory names where appropriate.
** It is destructive and returns the new string.
*/
FDNPREF char FDNFUNC *FrontDoorNode::AddTrail(char *rawfile)
{
  if(strlen(rawfile) && rawfile[strlen(rawfile)-1]!='\\' && rawfile[strlen(rawfile)-1]!=':') strcat(rawfile, "\\");
  return(rawfile);
}


/*
**    CreateInstance
**
** This function is used by the class to create a Busy semaphore
** (provided these have not been inhibited) to signal to other
** programs that the nodelist index is in use.
**
** The filename takes the form FDNODE<Instance>.<Task>, where
** <Task> is set to BSY if no Task was passed to the class.
**
** Instance is a unique value for this task, and may change
** during a Freeze/Thaw cycle.
**
** It is possible to define your own CreateInstance body, see
** FDNUSER.H and FDNODE.DOC for details.
*/

#if   (defined(FDN_USESTD) || defined(FDN_INST_USESTD))

FDNPREF int  FDNFUNC FrontDoorNode::CreateInstance(void)
{
  char Store[6]="BSY";
  char Buffer[20];
  int instance = 0, quit = 0, error = 0;
  FILE *Test;

  if(!strlen(SemaphoreDir)){
    SignalError(17);
    Flags = Flags | FDNodeNoSem;
    return(0);
  }

  // Get extension of busy semaphore
  if(Task) itoa(Task, Store, 10);

  while(!quit){
    // Form prospective Semaphore name
    strcpy(InstanceSemaphore, SemaphoreDir);
    strcat(InstanceSemaphore, "FDNODE");
    itoa(instance, Buffer, 16);
    strcat(InstanceSemaphore, Buffer);
    strcat(InstanceSemaphore, ".");
    strcat(InstanceSemaphore, Store);

    // Now attempt to create the semaphore
    Test=fopen(InstanceSemaphore, "rb");
    if(!Test){
      Test=fopen(InstanceSemaphore, "wb");
      fclose(Test);
      quit=1;
    }
    else fclose(Test);

    if(!quit && instance==0xFF){
      error=1;
      quit=1;
    }
   if(!quit) instance++;
  }
  if(error){
    instance = 0xFFFF;
    SignalError(16);
  }  
  return(instance);
}


/*
**    DeleteInstance
**
** Deletes the semaphore created by CreateInstance above.
** See that function for more details.
*/
FDNPREF int  FDNFUNC FrontDoorNode::DeleteInstance(void)
{
  return(remove(InstanceSemaphore));
}


#elif (defined(FDN_USEHAND) || defined(FDN_INST_USEHAND))


/*
** See the notes for the conditionally compiled variant above.
*/
FDNPREF int  FDNFUNC FrontDoorNode::CreateInstance(void)
{
  char Store[6]="BSY";
  char Buffer[20];
  int instance = 0, quit = 0, error = 0;
  int Test;

  if(!strlen(SemaphoreDir)){
    SignalError(17);
    Flags = Flags | FDNodeNoSem;
    return(0);
  }


  // Get extension of busy semaphore
  if(Task) itoa(Task, Store, 10);

  while(!quit){
    // Form prospective Semaphore name
    strcpy(InstanceSemaphore, SemaphoreDir);
    strcat(InstanceSemaphore, "FDNODE");
    itoa(instance, Buffer, 16);
    strcat(InstanceSemaphore, Buffer);
    strcat(InstanceSemaphore, ".");
    strcat(InstanceSemaphore, Store);

    // Now attempt to create the semaphore
    if (!_dos_creatnew(InstanceSemaphore, _A_NORMAL, &Test)){
      quit=1;
      close(Test);
    }

    if(!quit && instance==0xFF){
    error=1;
      quit=1;
    }
  if(!quit) instance++;
  }
  if(error){
    instance = 0xFFFF;
    SignalError(16);
  }
  return(instance);
}


/*
** See the notes for the conditionally compiled variant above.
*/
FDNPREF int  FDNFUNC FrontDoorNode::DeleteInstance(void)
{
  return(unlink(InstanceSemaphore));
}

#elif (defined(FDN_USEIOS) || defined(FDN_INST_USEIOS))


/*
** See the notes for the conditionally compiled variant above.
*/
FDNPREF int  FDNFUNC FrontDoorNode::CreateInstance(void)
{
  char Store[6]="BSY";
  char Buffer[20];
  int instance = 0, quit = 0, error = 0;
  ofstream Test;

  if(!strlen(SemaphoreDir)){
    SignalError(17);
    Flags = Flags | FDNodeNoSem;
    return(0);
  }


  // Get extension of busy semaphore
  if(Task) itoa(Task, Store, 10);

  while(!quit){
    // Form prospective Semaphore name
    strcpy(InstanceSemaphore, SemaphoreDir);
    strcat(InstanceSemaphore, "FDNODE");
    itoa(instance, Buffer, 16);
    strcat(InstanceSemaphore, Buffer);
    strcat(InstanceSemaphore, ".");
    strcat(InstanceSemaphore, Store);

    // Now attempt to create the semaphore
    Test.open(InstanceSemaphore, ios::noreplace | ios::out);
    if(Test){
      quit=1;
      Test.close();
     }
     else Test.clear();

   if(!quit && instance==0xFF){
      error=1;
      quit=1;
    }
    if(!quit) instance++;
  }
  if(error){
    instance = 0xFFFF;
     SignalError(16);
  }
 
  return(instance);
}


/*
** See the notes for the conditionally compiled variant above.
*/
FDNPREF int  FDNFUNC FrontDoorNode::DeleteInstance(void)
{
  return(remove(InstanceSemaphore));

}

#endif


/*
**    Constructor (private function)
**
** This function is called by all the "real" constructor functions.
** It sets up some variables and allocates some memory caches.
**
**    Parameters
**
**    NLPath    Path to the nodelist files;
**    SemPath   Path to the semaphore files;
**    setflags  Flags to set on the class (see FDNODE.H);
**    NewTask   TASK setting to pass to the class.
*/
FDNPREF void FDNFUNC FrontDoorNode::Constructor(const char FDNDATA *NLPath, const char *SemPath, unsigned short setflags, unsigned short NewTask)
{
  Frozen = 1;
  Flags=setflags;
  Task=NewTask;
  strcpy(NodelistDir, NLPath);
  AddTrail(NodelistDir);
  strcpy(SemaphoreDir, SemPath);
  AddTrail(SemaphoreDir);
  NLInfo.CountryCode = 0;
  error=0;
  InstanceSemaphore[0] = '\0';
  
  if(Flags & FDNodeNoCacheN) nroot=NULL; // Cache is disabled.
  else{
    nroot=new NFDXPage;
    if(!nroot){
      Flags = Flags | FDNodeNoCacheN; // Not enough memory, disable cache
      SignalError(7);
    }
  }
  if(Flags & FDNodeNoCacheU) uroot=NULL; // As above, but for user root cache
  else{
    uroot=new UFDXPage;
   if(!uroot){
      Flags = Flags | FDNodeNoCacheU;
      SignalError(8);
    }
  }
  if(Flags & FDNodeNoCacheP) proot=NULL; // As above, but for user root cache
  else{
    proot=new PFDXPage;
    if(!proot){
      Flags = Flags | FDNodeNoCacheP;
      SignalError(11);
    }
  }
  FDAStore=new FDANodeRec;
  if(!FDAStore){
    SignalError(9);
    Frozen = 1;
    return;
  }
  if(!(Flags & FDNodeCreateFrozen)) Thaw ();
}


/*
**    InitClass
**
** This function attempts to open the class, ready for use.
** It opens the various files as required, attempts to read
** and validate vital information about the various FDX files.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::InitClass()
{
  // returns 1 if class is ready for use
  // returns 0 otherwise
  unsigned short TempCountryCode = NLInfo.CountryCode;
  char filename[PATHLENGTH];
  ExtendedPage ExtPage;

  if(!(Flags & FDNodeNoSem)) Instance=CreateInstance();
  Reopen[0]=Reopen[1]=Reopen[2]=Reopen[3]=1;

  // NODELIST.FDX

  strcpy(filename, NodelistDir);
  strcat(filename, "NODELIST.FDX");
  NFDX.SetName(filename);
  if(!NFDX.Open()){
    SignalError(1);
    Freeze();
    return(0);
  }
  NFDX.Read(&ExtPage, sizeof(ExtendedPage), 1, 1);
  memcpy(&first_n, &ExtPage, sizeof(FirstPage));
  // Validation
  if(first_n.pagelen!=sizeof(NFDXPage) || ExtPage.RevisionMaj!=2){
    SignalError(26);
    Freeze();
    return(0);
  }
  // Read some header data from offset 0x100
  NFDX.Seek(0x110, SEEK_SET);
  NFDX.Read(&NLInfo, sizeof(NLinfoRec), 1, 1);
  if(NLInfo.ZeroWord){
    NLDBRevision = 0;
    NLInfo.CountryCode = TempCountryCode;
    NLInfo.CompileTime = 0;
  }
  else NLDBRevision=1;
  // mini cache load
  if(!(Flags & FDNodeNoCacheN)){
    NFDX.Seek(first_n.pagelen*first_n.index, SEEK_SET);
    NFDX.Read(nroot, (size_t) first_n.pagelen, 1, 1);
  // Possible error, no records in Index file
    if(!nroot->records || !first_n.index) SignalError(23);
  }
  ConvertToC(ExtPage.nodeext);
  strcpy(NodeExt, ExtPage.nodeext);
  swedish=(int) ExtPage.swedish;
  strcpy(ListFileName[0], "NODELIST.");
  strcat(ListFileName[0], NodeExt);
  
  if(Flags & FDNodeNFDX) NFDX.Close();

  // USERLIST.FDX
  strcpy(filename, NodelistDir);
  strcat(filename, "USERLIST.FDX");
  UFDX.SetName(filename);
  if(!UFDX.Open()){
    SignalError(2);
    Freeze();
    return(0);
  }
  UFDX.Read(&first_u, sizeof(FirstPage), 1, 1);
  // Validation
  if(first_u.pagelen!=sizeof(UFDXPage)){
    SignalError(27);
    Freeze();
    return(0);
  }    
  if(!(Flags & FDNodeNoCacheU)){
    UFDX.Seek(first_u.pagelen*first_u.index, SEEK_SET);
    UFDX.Read(uroot, (size_t) first_u.pagelen, 1, 1);
    // Possible error, no records in Index file
    if(!uroot->records || !first_u.index) SignalError(24);      
  }
  if(Flags & FDNodeUFDX) UFDX.Close();

  // PHONE.FDX
  strcpy(filename, NodelistDir);
  strcat(filename, "PHONE.FDX");
 PFDX.SetName(filename);
  if(!PFDX.Open()){
    SignalError(14);
    Freeze();
    return(0);
  }
  PFDX.Read(&first_p, sizeof(FirstPage), 1, 1);
  // Validation
  if(first_p.pagelen!=sizeof(PFDXPage)){
    SignalError(28);
    Freeze();
    return(0);
  }  
  if(!(Flags & FDNodeNoCacheP)){
    PFDX.Seek(first_p.pagelen*first_p.index, SEEK_SET);
    PFDX.Read(proot, (size_t) first_p.pagelen, 1, 1);
    // Possible error, no records in Index file
    if(!proot->records || !first_p.index) SignalError(25);
  }
  if(Flags & FDNodePFDX) PFDX.Close();

  // Now open the data files as required by the flags
  strcpy(filename, NodelistDir);
  strcat(filename, ListFileName[0]);
  DataFile[0].SetName(filename);
  if(!(Flags & FDNodeNode)){
    if(!DataFile[0].Open()){
      if(!(isdigit(NodeExt[0]) && isdigit(NodeExt[1]) && isdigit(NodeExt[2]))){
        SignalError(18);
      }
      else{
        SignalError(3); // The file /should/ be openable. Fatal error.
        Freeze();
        return(0);
      }
    }
    else Reopen[0]=0;
  }

  strcpy(filename, NodelistDir);
  strcat(filename, ListFileName[1]);
  DataFile[1].SetName(filename);
  if(!(Flags & FDNodeFDA)){
    if(!DataFile[1].Open()) SignalError(4);
    else Reopen[1]=0;
  }

  strcpy(filename, NodelistDir);
  strcat(filename, ListFileName[2]);
  DataFile[2].SetName(filename);
  if(!(Flags & FDNodePvt)){
    if(!DataFile[2].Open()) SignalError(5);
    else Reopen[2]=0;
  }

  strcpy(filename, NodelistDir);
  strcat(filename, ListFileName[3]);
  DataFile[3].SetName(filename);
  if(!(Flags & FDNodePoint)){
    if(!DataFile[3].Open()) SignalError(6);
    else Reopen[3]=0;
  }

  strcpy(filename, NodelistDir);
  strcat(filename, "PHONE.FDA");
  PFDA.SetName(filename);
  if(!(Flags & FDNodePhone)){
    if(!PFDA.Open()) SignalError(12);
  }

  // We need to fetch the location of default dial translations
  GetPFDXData("INTL", NULL, first_p.index);
  GetPFDXData("DOM", NULL, first_p.index);
  NLCurOffset=0;
  UnixStamp=time(NULL);

  return(1);
}


/*
**    GetPFDXData
**
** This function searches PHONE.FDX and PHONE.FDA for details
** on the number passed in.
**
**    Parameters
**
**    SearchKey   Phone number to examine;
**    buffer      Buffer to copy translated number to,
**                if NULL, no translation occurs;
**    page        The root page of PHONE.FDX
**
**    Returns
**
**    The cost to call the system with that number.
*/
FDNPREF unsigned short FDNFUNC FrontDoorNode::GetPFDXData(char * SearchKey, char * buffer, long Page)
{
  char * pSearchKey=SearchKey;
#ifdef FDN_WINDOWS
  char OemSearchKey[sizeof(FDAStore->Telephone)];
  AnsiToOem(SearchKey, OemSearchKey);
  pSearchKey=OemSearchKey;
#endif

  PFDXPage *      pd;            // Phone Index Data
  FDNPhoneRec *   PFDAData;      // Phone Data Page
  long            NextPage;
  int             found=0, quit=0, Test, loop, BestLevel=0;
  unsigned short  Cost = 0, SetCost = 0;
  char            TempKey[22];

  // Storing position in BTree of the Match we consider
  int  Level = 0;
  int  RecordM[MAXHEIGHT];
  long PageM[MAXHEIGHT];

  // Is this a valid page? (ie. Check for an empty index)
  if(!Page){
    SignalError(25);
    if(buffer) strcpy(buffer, SearchKey);
    return(0xFFFFU);
  }

  // We may need to open file pointers, and we will need to alloc some space
  if(Flags & FDNodePFDX){
    if(!PFDX.Open()){
      SignalError(14);
      return(0xFFFFU);
    }
  }
  if(Flags & FDNodePhone){
    if(!PFDA.Open()){
      SignalError(12);
      return(0xFFFFU);
    }
  }
  
  pd = new PFDXPage;
  if(!pd){
    SignalError(10);
    return(0);
  }

  // Unpublished numbers can't be translated, and have default international cost.
  if(!strnicmp(SearchKey, "-U", 2) || !*SearchKey){
    PFDAData = new FDNPhoneRec;
    if(!PFDAData){
      SignalError(10);
      return(0xFFFFU);
    }
   if(!GetPFDAPage(*PFDAData, INTLOffset)){
      SignalError(22);
      *buffer=0;
      delete pd;
      delete PFDAData;
      return(0xFFFFU);
    }
    if(buffer) strcpy(buffer, SearchKey);
    if(Flags & FDNodePFDX)  PFDX.Close();
    if(Flags & FDNodePhone) PFDA.Close();
    Cost = PFDAData->Cost;
    delete pd;
    delete PFDAData;
    
    return(Cost);
  }

  // We first find the rightmost match for our key in the PHONE.FDX file
  while(!quit && Page){
    
    NextPage = 0;
    Level++;
    if(!GetPFDXPage(*pd, Page)){
      SignalError(30);
      delete pd;
      return(0xFFFFU);
    }
    PageM[Level - 1] = Page;
    
    for(loop = pd->records - 1; (loop >= 0) && !NextPage && !quit; loop--){
      Test = CompareKey(SearchKey, pd->phones[loop].key, 21);
      if(Test == 0){
        // Exact Match found! mark and quit
        RecordM[Level - 1] = loop;
        BestLevel = Level;
        quit = 1;
      }
      if(Test > 0){
      if(pd->backref){
          NextPage = pd->phones[loop].link;
          RecordM[Level - 1] = loop + 1;
        }
        else{
          RecordM[Level - 1] = loop;
          BestLevel = Level;
        }
      }
      if(Test < 0){
        if(!loop){
          RecordM[Level - 1] = 0; // Less than all items on the page
          if(!pd->backref) quit = 1;
          else NextPage = pd->backref;
        }
      }
    }
    if(!quit) Page = NextPage;
  }
  if(BestLevel && (Level!=BestLevel)){
    // We went into a leaf, but it was a red herring ;-)
    Level = BestLevel;
    GetPFDXPage(*pd, PageM[Level - 1]);
  }
  quit = 0;

  // We have a couple of special cases, namely "DOM" and "INTL"
  if(!strcmp(SearchKey, "DOM")){
    if(!Test) DOMOffset = pd->phones[RecordM[Level - 1]].offset;
    else DOMOffset = 0;
    delete pd;
    return(0);
  }
  if(!strcmp(SearchKey, "INTL")){
    if(!Test) INTLOffset = pd->phones[RecordM[Level - 1]].offset;
    else INTLOffset = 0;
    delete pd;
    return(0);
  }

  PFDAData = new FDNPhoneRec;
  if(Test>=0){
   // Ok, we fetch what data we can from this entry, usually the cost
    while(!quit){
      // Fetch the PHONE.FDA information
      if(!GetPFDAPage(*PFDAData, pd->phones[RecordM[Level - 1]].offset)){
        SignalError(31);
        delete pd;
        delete PFDAData;
        return(0xFFFF);
      }
      ConvertToC(PFDAData->Telephone);
      
      if(!strcmp(PFDAData->Telephone, "=")){
        // This is a Cost only entry, get the cost if we have nothing better, then continue
        if(!SetCost){
          Cost = PFDAData->Cost; SetCost = 1;
        }
      }
      else{
        // We have a genuine Dial XLT entry - perfect
        quit = 1;
        found = 1;
      }
     
      if(!quit){
        // Ok, get the previous entry in the Tree, and check if it's still a match
        if(pd->backref){
          // Descend to bottom of the tree (node case)
          while(pd->backref){
            loop = RecordM[Level - 1];
            Level++;
            
            if(loop) PageM[Level - 1] = pd->phones[loop - 1].link;
            else PageM[Level - 1] = pd->backref;
  
            if(!GetPFDXPage(*pd, PageM[Level - 1])){
              SignalError(30);
              delete pd; delete PFDAData;
              return(0xFFFFU);
        }
            if(pd->backref) RecordM[Level - 1] = pd->records + 1;
            else RecordM[Level - 1] = pd->records;
          }
        }
        else{
          // We'll be moving to the left, and possibly ascending (leaf case)
          if(RecordM[Level - 1]) RecordM[Level - 1]--;
          else{
            do{
              Level--;
            }
            while(Level && !RecordM[Level - 1]);
            if(Level) RecordM[Level - 1]--;
            else quit = 1; // We have traversed to the start of the index
          }
        }
      }
      // Check if it's still a match, otherwise quit out.
      if(!quit)
        if(CompareKey(SearchKey, pd->phones[RecordM[Level - 1]].key, 21)) quit = 1;
    }
  }


  // Ok, get the entry with the right Dial XLT (only if we didn't find it above)
  if(!found){
    // Is the number International?
    if(NLInfo.CountryCode != (unsigned short) atoi(SearchKey)){
      if(INTLOffset){
        if(!GetPFDAPage(*PFDAData, INTLOffset)){
          SignalError(31);
          delete pd; delete PFDAData;
          return(0xFFFF);
        }
        ConvertToC(PFDAData->Telephone);
        if(!SetCost) Cost = PFDAData->Cost;
        *TempKey = 0;
    }
    }
    else
    {
      if(DOMOffset){
        if(!GetPFDAPage(*PFDAData, DOMOffset)){
          SignalError(31);
          delete pd; delete PFDAData;
          return(0xFFFF);
        }
        ConvertToC(PFDAData->Telephone);
        if(!SetCost) Cost = PFDAData->Cost;
        itoa(NLInfo.CountryCode, TempKey, 10);
        strcat(TempKey, "-");
      }
    }
  }

  if(found && !SetCost){
    Cost = PFDAData->Cost;
    SetCost = 1;
  }

  if(buffer){
    // Perform DialXLT
    if(found){                                
      ConvertToC(pd->phones[RecordM[Level - 1]].key);
      strcpy(TempKey, pd->phones[RecordM[Level - 1]].key);
    }
    GetPrefixNumber(PFDAData->Telephone, buffer);
    strcat(buffer, SearchKey + strlen(TempKey));
    GetSuffixNumber(PFDAData->Telephone, buffer);
    if(!strncmp(PFDAData->Telephone, "Internet", 7)){
      for(loop = 0; loop < (int) strlen(buffer); loop++) if(buffer[loop]=='-') buffer[loop]='.';
    }
  }
  
  // Temporary guess
  if(SetCost && Cost==0x8000U){
    // Default domestic cost
    if(DOMOffset){
      GetPFDAPage(*PFDAData, DOMOffset);
      Cost = PFDAData->Cost;
    }
    else Cost = 0xFFFFU;
  }
  if(SetCost && Cost==0xFFFFU){
    // Default international cost
    if(INTLOffset){
      GetPFDAPage(*PFDAData, INTLOffset);
      Cost = PFDAData->Cost;
    }
    else Cost = 0xFFFFU;
  }
    
  delete PFDAData;
  delete pd;

  if(Flags & FDNodePhone) PFDA.Close();
  if(Flags & FDNodePFDX) PFDX.Close();

#ifdef FDN_WINDOWS
  if(buffer!=NULL)
    OemToAnsi(buffer, buffer);
#endif
  
  return(Cost);
}


/*
**    GetUFDXOffset
**
** Searches for the first occurence of the key given in
** USERLIST.FDX. Loads the search block with the result.
**
**    Parameters
**
**    search_key  The key to look for, formed elsewhere.
**                Note '_' is a match all wildcard.
**    page        The root of USERLIST.FDX.
**    fblock      The find block to load with the result.
**
**    Returns
**
**    0x00000000  Not found
**    0xFFFFFFFF  Off index
**                Otherwise returns Index Database Offset.
*/
FDNPREF long FDNFUNC FrontDoorNode::GetUFDXOffset(char FDNDATA *search_key, long page, FDNFind& fblock)
{
  long next_page, offset=0;
  struct UFDXPage *ud;
  int found=0, quit=0, test, loop, bestyet=0;

  // We may need to open an index file pointer
  if(Flags & FDNodeUFDX){
    if(!UFDX.Open()){
      SignalError(2);
      return(0);
    }
  }
  ud = new UFDXPage;
  if(!ud){
    SignalError(10);
    return(0);
  }
  fblock.level=1;
  while(!found && !quit){
    next_page=0;
    GetUFDXPage(*ud, page);
    // Check for zero records
    if(!ud->records || !page){
      SignalError(24);
    delete ud;
      return(0);
    }

    fblock.page[fblock.level-1]=page;
    fblock.maxrec[fblock.level-1]=ud->records;
    // Check it's not to the right of the last element in the page
    if(CompareKey(search_key, ud->names[ud->records-1].key)>0){
      if(ud->backref){
        next_page=ud->names[ud->records-1].link;
        fblock.record[fblock.level-1]=ud->records;
        // No matches possible in this page, continue down.
      }
      else quit=1;      // We've reached the end of a leaf, with no match :-(
    }
    for(loop=0; loop<ud->records && !found && !next_page; loop++){
      fblock.record[fblock.level-1]=loop;
      test=CompareKey(search_key, ud->names[loop].key);
      if(test<=0){
        if(test==0){
          if(ud->backref){
            // Not at bottom, we must record this match, but descend for
            // others.
            bestyet=fblock.level;
          }
          else found=1;
        }
        if(ud->backref) if(loop) next_page=ud->names[loop-1].link; else next_page=ud->backref;
      }
    }
    if(!found && !next_page){
      // We're at the bottom, and we've found no match in the leaf, but
      // we may have found one on the way down.
      if(bestyet){
        found=1;
        fblock.level=bestyet;
      }
      else{
      quit=1; // Not found, and nowhere else to go...
        offset=0;
      }
    }
    if(next_page){
      // Look in the next page.
      fblock.level++;
      page=next_page;
    }
  }
  if(found){
    // Let's be sure we're on the right page
    if(page!=fblock.page[fblock.level-1]){
      page=fblock.page[fblock.level-1];
      GetUFDXPage(*ud, page);
    }
    strncpy(fblock.key, search_key, 16);
    fblock.zone  = SwapBytes(ud->names[fblock.record[fblock.level-1]].zone);
    fblock.net   = SwapBytes(ud->names[fblock.record[fblock.level-1]].net);
    fblock.node  = SwapBytes(ud->names[fblock.record[fblock.level-1]].node);
    fblock.point = SwapBytes(ud->names[fblock.record[fblock.level-1]].point);
    fblock.status=ud->names[fblock.record[fblock.level-1]].nodetype;
    offset=fblock.offset=ud->names[fblock.record[fblock.level-1]].offset;
  }
  delete ud;

  fblock.Parent=this;
  fblock.UnixStamp=time(NULL);
  if(Flags & FDNodeUFDX) UFDX.Close();
  return(offset);
}


/*
**    GetNFDXOffset
**
** Searches for the first occurence of the key given in
** NODELIST.FDX. Loads the search block with the result.
**
**    Parameters
**
**    search_key  The key to look for, formed elsewhere.
**    page        The root of NODELIST.FDX.
**    fblock      The find block to load with the result.
**
**    Returns
**
**    0x00000000  Not found
**    0xFFFFFFFF  Off index
**                Otherwise returns Index Database Offset.
*/
FDNPREF long FDNFUNC FrontDoorNode::GetNFDXOffset(char FDNDATA *search_key, long page, FDNFind& fblock)
{
  long next_page, offset=0;
  struct NFDXPage *nd;
  int found=0, quit=0, loop, test, bestrecord=0;

  // We may need to open an index file pointer
  if(Flags & FDNodeNFDX){
    if(!NFDX.Open()){
      SignalError(1);
      return(0);
    }
  }
  nd=new NFDXPage;
  if(!nd){
    SignalError(10);
    return(0);
  }

  fblock.level=1;
  while(!found && !quit){
    bestrecord=0;
    next_page=0;
    GetNFDXPage(*nd, page);
    // Check for zero records
   if(!nd->records || !page){
      SignalError(23);
      delete nd;
      return(0xFFFFFFFF);
    }
    fblock.page[fblock.level-1]=page;
    fblock.maxrec[fblock.level-1]=nd->records;
    // Check it's not to the right of the last element in the page
    if(CompareKey(search_key, MakeKey(nd->nodes[nd->records-1].zone, nd->nodes[nd->records-1].net, nd->nodes[nd->records-1].node, nd->nodes[nd->records-1].point))>0){
      if(nd->backref){
        next_page=nd->nodes[nd->records-1].link; // No matches possible in this page, continue down.
        bestrecord=fblock.record[fblock.level-1]=nd->records;
      }
      else{
        quit=1; // We've reached the end of a leaf, with no match :-(
        bestrecord=fblock.record[fblock.level-1]=nd->records-1;
      }
    }
    for(loop=0; loop<nd->records && !quit && !found && !next_page; loop++){
      fblock.record[fblock.level-1]=loop;
      test=CompareKey(search_key, MakeKey(nd->nodes[loop].zone, nd->nodes[loop].net, nd->nodes[loop].node, nd->nodes[loop].point));
      switch(test){
        case -1:
          if(nd->backref){
            if(loop) next_page=nd->nodes[loop-1].link; else next_page=nd->backref;
          }
          else{
            quit=1;
          }
          break;
        case 0 :
          found=1; break;
        case 1 :
          bestrecord=loop; break;
      }
    }
    if(!found && !next_page){
      quit=1; // Not found, and nowhere else to go...
    fblock.record[fblock.level-1]=bestrecord;
      offset=0;
    }
    if(next_page){
      // Look in the next page.
      fblock.level++;
      page=next_page;
    }
  }
  if(page!=fblock.page[fblock.level-1]){
    page=fblock.page[fblock.level-1];
  }
  strncpy(fblock.key, search_key, 16);
  fblock.zone   = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].zone);
  fblock.net    = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].net);
  fblock.node   = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].node);
  fblock.point  = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].point);
  fblock.rnode  = nd->nodes[fblock.record[fblock.level-1]].rnode;
  fblock.rnet   = nd->nodes[fblock.record[fblock.level-1]].rnet;
  fblock.status = nd->nodes[fblock.record[fblock.level-1]].nodetype;
  offset=fblock.offset=nd->nodes[fblock.record[fblock.level-1]].offset.loff;

  if(!found){
    if(CompareKey(search_key, MakeKey(fblock.zone, fblock.net, fblock.node, fblock.point, 0))>=0){
      if(NGetNextKey(fblock, nd)) offset=0xFFFFFFFFL;
      else offset=0;
         }
         else offset=0;
  }

  delete(nd);

  fblock.Parent=this;
  fblock.UnixStamp=time(NULL);
  if(Flags & FDNodeNFDX) NFDX.Close();
  return(offset);
}


/*
**    GetNextZone
**
** Used to continue a search started with GetZones.
**
**    Parameters
**
**    The find block returned last.
**
**    Returns
**
**    0 on success (a match found), 1 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetNextZone(FDNFind& fblock)
{
  return(GetZones(fblock, fblock.zone));
}


/*
**    GetNextNet
**
** Used to continue a search started with GetNets.
**
**    Parameters
**
**    The find block returned last.
**
**    Returns
**
**    0 on success (a match found), 1 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetNextNet(FDNFind& fblock)
{
  return(GetNets(fblock, fblock.zone, fblock.net));
}


/*
**    GetNextNode
**
** Used to continue a search started with GetNodes.
**
**    Parameters
**
**    The find block returned last.
**
**    Returns
**
**    0 on success (a match found), 1 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetNextNode(FDNFind& fblock)
{
  unsigned short int oldzone=fblock.zone, oldnet=fblock.net;
  int found=0, quit=0;

  while(!found && !quit){
    if(NGetNextKey(fblock)) quit=1; // off index
    else{
      if(fblock.point){
        // Oops, there are points here, and so our short cut does not work
        return(GetNodes(fblock, fblock.zone, fblock.net, (unsigned short) (fblock.node+1)));
      }
      if(fblock.zone==oldzone && fblock.net==oldnet){
        // We've found a node, test in on the Filter
        if(fblock.Filter()) found=1;
      }
      else quit=1; // We've wandered into a new net
    }
  }
  if(!found) fblock.finished=1;
  return(!found);
}


/*
**    GetNextPoint
**
** Used to continue a search started with GetPoints.
**
**    Parameters
**
**    The find block returned last.
**
**    Returns
**
**    0 on success (a match found), 1 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetNextPoint(FDNFind& fblock)
{
  unsigned short int oldzone=fblock.zone, oldnet=fblock.net, oldnode=fblock.node, oldpoint=fblock.point;
  int found=0, quit=0;

  while(!found && !quit){
    if(NGetNextKey(fblock)) quit=1;
    else{
      if(fblock.point!=oldpoint && fblock.node==oldnode && fblock.net==oldnet && fblock.zone==oldzone){
        if(fblock.Filter()) found=1;
      }
      else quit=1;
    }
  }
  if(!found) fblock.finished=1;
  return(!found);
}


/*
**    UGetNextKey
**
** This function fetches the data for the next key in
** USERLIST.FDX with respect to the current one in the
** find block.
**
**    Returns
**
**    0 on success (another match), 1 on failure (no more keys)
*/
FDNPREF int FDNFUNC FrontDoorNode::UGetNextKey(FDNFind& fblock)
{
  int found=0, test;
  UFDXPage *ud;

  if(Flags & FDNodeUFDX){
    if(!UFDX.Open()){
      SignalError(2);
      fblock.finished=1;
      return(1);
    }
  }
  ud=new UFDXPage;
  if(!ud){
    SignalError(10);
    return(1);
  }

  GetUFDXPage(*ud, fblock.page[fblock.level-1]);
  // If possible, check the next entry in the page. Otherwise come up a level
  if((fblock.record[fblock.level-1] < (ud->records-1)) || ((fblock.record[fblock.level-1]==ud->records-1) && ud->backref)){
    fblock.record[fblock.level-1]++;
    if((fblock.record[fblock.level-1]==ud->records) && ud->backref)
    test=-1;
    // OK, behaviour is slightly different if we are in a node or leaf.
    else test=CompareKey(fblock.key, ud->names[fblock.record[fblock.level-1]].key);
    if(ud->backref){
      // we're in a node.
      if(test<=0){
        fblock.level++;
        fblock.record[fblock.level-1]=0;
        fblock.page[fblock.level-1]=ud->names[fblock.record[fblock.level-2]-1].link;
        GetUFDXPage(*ud, fblock.page[fblock.level-1]);
      // we have to check next item, descend to bottom of tree.
        while(ud->backref){
          fblock.level++;
          fblock.record[fblock.level-1]=0;
          fblock.page[fblock.level-1]=ud->backref;
          GetUFDXPage(*ud, fblock.page[fblock.level-1]);
        }
        if(!CompareKey(fblock.key, ud->names[0].key)) found=1;
      }
    }
    else{
      // we're in a leaf. Either the next entry was a match or we're finished
      if(!test) found=1;
    }
  }
  else{
    // The BTree is traversed in such a way that nodes are visited on ascent
    // therefore we must check the node entry now.
    
    do{
      fblock.level--;
      if(fblock.level) GetUFDXPage(*ud, fblock.page[fblock.level-1]);
    }
    while((fblock.record[fblock.level-1] == ud->records) && fblock.level);
    if(fblock.level) if(!CompareKey(fblock.key, ud->names[fblock.record[fblock.level-1]].key)) found=1; 

    /*  
    if(--fblock.level){
      GetUFDXPage(*ud, fblock.page[fblock.level-1]);
      if(!CompareKey(fblock.key, ud->names[fblock.record[fblock.level-1]].key)) found=1; 
    }
    */
  }
  if(Flags & FDNodeUFDX) UFDX.Close();

  fblock.Parent=this;
  fblock.UnixStamp=time(NULL);
  if(!found){
   fblock.offset=0;
    fblock.finished=1;
    delete ud;
    return(1);
  }
  // We did find something, so let's prepare the block for usage.
  fblock.zone   = SwapBytes(ud->names[fblock.record[fblock.level-1]].zone);
  fblock.net    = SwapBytes(ud->names[fblock.record[fblock.level-1]].net);
  fblock.node   = SwapBytes(ud->names[fblock.record[fblock.level-1]].node);
  fblock.point  = SwapBytes(ud->names[fblock.record[fblock.level-1]].point);
  fblock.status = ud->names[fblock.record[fblock.level-1]].nodetype;
  fblock.offset = ud->names[fblock.record[fblock.level-1]].offset;
  delete ud;
  return(0);
}


// NGetNextKey(search block)
// Used in conjunction with other NODELIST.FDX searches. This moves the
// pointer on one.
FDNPREF int FDNFUNC FrontDoorNode::NGetNextKey(FDNFind& fblock)
{
  return(NGetNextKey(fblock, NULL));
}


/*
**    NGetNextKey
**
** This function fetches the data for the next key in
** NODELIST.FDX with respect to the current one in the
** find block.
**
**    Returns
**
**    0 on success (another match), 1 on failure (no more keys)
*/
FDNPREF int FDNFUNC FrontDoorNode::NGetNextKey(FDNFind& fblock, NFDXPage FDNDATA *cnd)
{
  int found=0, test;
  NFDXPage *nd;

  if((Flags & FDNodeNFDX) && !cnd){
    if(!NFDX.Open()){
      SignalError(1);
      return(1);
    }
  }
  nd = new NFDXPage;
  if(!nd){
    SignalError(10);
    return(1);
  }

  if(!cnd) GetNFDXPage(*nd, fblock.page[fblock.level-1]);
  else memcpy(nd, cnd, (int) first_n.pagelen);
  // If possible, check the next entry in the page. Otherwise come up a level
  if((fblock.record[fblock.level-1] < (nd->records-1)) || ((fblock.record[fblock.level-1]==nd->records-1) && nd->backref)){
    fblock.record[fblock.level-1]++;
    if((fblock.record[fblock.level-1]==nd->records) && nd->backref) test = -1;
    // OK, behaviour is slightly different if we are in a node or leaf.
    else test=CompareKey(fblock.key, MakeKey(nd->nodes[fblock.record[fblock.level-1]].zone, nd->nodes[fblock.record[fblock.level-1]].net, nd->nodes[fblock.record[fblock.level-1]].node ,nd->nodes[fblock.record[fblock.level-1]].point ));
    if(nd->backref){
      // we're in a node.
      if(test<=0){
        fblock.level++;
        fblock.record[fblock.level-1]=0;
        fblock.page[fblock.level-1]=nd->nodes[fblock.record[fblock.level-2]-1].link;
        GetNFDXPage(*nd, fblock.page[fblock.level-1]);

        // we have to check next item, descend to bottom of tree.
        while(nd->backref){
          fblock.level++;
          fblock.record[fblock.level-1]=0;
          fblock.page[fblock.level-1]=nd->backref;
          GetNFDXPage(*nd, fblock.page[fblock.level-1]);
      }
        found=1;
      }
    }
    else{
      // we're in a leaf. Either the next entry was a match or we're finished
      found=1;
    }
  }
  else{
    while(fblock.level && !found){
      if(--fblock.level){
        // The BTree is traversed in such a way that nodes are visited on ascent
        // therefore we must check the node entry now.
        GetNFDXPage(*nd, fblock.page[fblock.level-1]);
        if(fblock.record[fblock.level-1]>= nd->records) found=0; else found=1;
      }
    }
  }
  if(Flags & FDNodeNFDX) NFDX.Close();

  fblock.Parent=this;
  fblock.UnixStamp=time(NULL);
  if(!found){
    delete nd;
    return(1);
  }
  // We did find something, so let's prepare the block for usage.
  fblock.zone   = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].zone);
  fblock.net    = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].net);
  fblock.node   = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].node);
  fblock.point  = SwapBytes(nd->nodes[fblock.record[fblock.level-1]].point);
  fblock.rnet   = nd->nodes[fblock.record[fblock.level-1]].rnet;
  fblock.rnode  = nd->nodes[fblock.record[fblock.level-1]].rnode;
  fblock.status = nd->nodes[fblock.record[fblock.level-1]].nodetype;
  fblock.offset = nd->nodes[fblock.record[fblock.level-1]].offset.loff;
  delete nd;
  return(0);
}


/*
**    MakeKey
*/
// char *MakeKey(zone, net, node, point)
// Generates a text key suitable for comparision. Uses static storage space.
FDNPREF char FDNFUNC *FrontDoorNode::MakeKey(unsigned short int zone, unsigned short int net, unsigned short int node, unsigned short int point)
{
  static char key_space[20];
  char buffer[18];
  register int loop;

  for(loop=0; loop<16; loop++) key_space[loop]='0';
  itoa(SwapBytes(zone), buffer, 16);
  strncpy(key_space+(4-strlen(buffer)), buffer, strlen(buffer));
  itoa(SwapBytes(net), buffer, 16);
  strncpy(key_space+(8-strlen(buffer)), buffer, strlen(buffer));
  itoa(SwapBytes(node), buffer, 16);
  strncpy(key_space+(12-strlen(buffer)), buffer, strlen(buffer));
  itoa(SwapBytes(point), buffer, 16);
  strncpy(key_space+(16-strlen(buffer)), buffer, strlen(buffer));

  return(key_space);
}

// char *MakeKey(zone, net, node, point, int)
// Generates a text key suitable for comparision. Uses static storage space.
// Overloaded version, does not swap data (as required for stuff in the index)
FDNPREF char FDNFUNC *FrontDoorNode::MakeKey(unsigned short int zone, unsigned short int net, unsigned short int node, unsigned short int point, int )
{
  static char key_space[20];
  char buffer[18];
  register int loop;

  for(loop=0; loop<16; loop++) key_space[loop]='0';
  itoa(zone, buffer, 16);
  strncpy(key_space+(4-strlen(buffer)), buffer, strlen(buffer));
  itoa(net, buffer, 16);
  strncpy(key_space+(8-strlen(buffer)), buffer, strlen(buffer));
  itoa(node, buffer, 16);
  strncpy(key_space+(12-strlen(buffer)), buffer, strlen(buffer));
  itoa(point, buffer, 16);
  strncpy(key_space+(16-strlen(buffer)), buffer, strlen(buffer));

  return(key_space);
}

// CompareKey(key1, key2)
// This is used to compare keys in the index files.
FDNPREF int FDNFUNC FrontDoorNode::CompareKey(char FDNDATA *key1, char FDNDATA *key2)
{
  register int loop;
  for(loop=0; loop<16; loop++){
    if(key1[loop]=='_' || key2[loop]=='_') return(0);
    if(key1[loop]!=',' && key2[loop]!=','){
      if((unsigned char) key1[loop] > (unsigned char) key2[loop]) return(1);
      if((unsigned char) key1[loop] < (unsigned char) key2[loop]) return(-1);
    }
  }
  return(0);
}


/*
**    CompareKey
**
** This function compares two keys, key1 is a 'C' string, while
** key2 is a Pascal string. Comparison occurs to a specified maximum
** length.
**
**    Returns
**
**    -1 if string key1 lies alphabetically before key2
**     0 if string key1 and key2 are identical
**    +1 if string key1 lies alphabetically after key2
*/
FDNPREF int  FDNFUNC FrontDoorNode::CompareKey(const char FDNDATA * key1, const char FDNDATA * key2, int MaxLen)
{
  register int loop;

  for(loop = 1; loop < MaxLen && loop <= (int) strlen(key1) && loop <= key2[0]; loop++){
    if((unsigned char) key1[loop - 1] > (unsigned char) key2[loop]) return(1);
    if((unsigned char) key1[loop - 1] < (unsigned char) key2[loop]) return(-1);
  }
  return(0);
}


/*
**    GetPrefixNumber
**
** Starts the creation of a translated phone number by copying
** any prefix into the string, stopping at any '/' token.
**
**    Parameters
**
**    XLT     The translation token, format as in FDNODE.CTL.
**    Buffer  Buffer in which to form the translated number.
*/
FDNPREF void FDNFUNC FrontDoorNode::GetPrefixNumber(char FDNDATA * XLT, char FDNDATA * Buffer)
{
  char * pos;
  if(!strncmp(XLT, "Internet", 8)) strcpy(Buffer, XLT + 8);
  else strcpy(Buffer, XLT);
  pos = strpbrk(Buffer, "//");
  if(pos) *pos=0;
}


/*
**    GetSuffixNumber
**
** Ends the creation of a translated phone number by copying
** any suffix into the string, starting after any '/' token.
**
**    Parameters
**
**    XLT     The translation token, format as in FDNODE.CTL.
**    Buffer  Buffer in which to form the translated number.
*/
FDNPREF void FDNFUNC FrontDoorNode::GetSuffixNumber(char FDNDATA * XLT, char FDNDATA * Buffer)
{
  char * pos = strpbrk(XLT, "//");
  if(pos) strcat(Buffer, pos + 1);
  else return;
}


/*
**    ConvertToC
**
** Destructively converts the Pascal style string passed in
** to a 'C' format string.
*/
FDNPREF void FDNFUNC FrontDoorNode::ConvertToC(char FDNDATA *string)
{
  int length = (unsigned char) string[0];
  register int loop;

  if(length!=0){
    for(loop = 1; loop <= length; loop++) string[loop-1] = string[loop];
    string[length]=0;
  }
}


/*
**    fcrgets
**
** This function is similar to fgets except that it removes any
** trailing '\r' characters.
*/
FDNPREF char FDNFUNC *FrontDoorNode::FCRGetS(char FDNDATA *buffer, int maxlength, FDN_FileObject & file)
{
  char * p;
  if(file.Read (buffer, maxlength, 1, 0)){
    buffer [maxlength - 1] = '\0';
    if((p = strchr (buffer, '\r')) != NULL)
      *p = '\0';
    if((p = strchr (buffer, '\n')) != NULL)
      *p = '\0';
#ifdef FDN_WINDOWS
    OemToAnsi(buffer, buffer);  
#endif
  }
  else
    buffer[0] = '\0';
  return(buffer);

}


/*
**    CSVField
**
** Copies the specified field from a CSV string into a
** buffer.
**
**    Parameters
**
**    Input   CSV string.
**    field   Number of field to cut, numbered from 0.
**    tofill  Buffer for copying the results.
*/
FDNPREF char FDNFUNC *FrontDoorNode::CSVField(char FDNDATA *Input, int field, char FDNDATA *tofill)
{
  char FDNDATA *pos=Input, FDNDATA *end;
  int loop;

  pos=CSVFieldStart(pos, field);
  if(pos){
    end=strpbrk(pos, ",");
    if(!end) strcpy(tofill, pos);
    else{
      strncpy(tofill, pos, ((int) (end-pos)));
      tofill[((int) (end-pos))]=0;
      for(loop=0; loop<(int)strlen(tofill); loop++)
        if(tofill[loop]=='_') tofill[loop]=' ';
    }
  }
  return(tofill);
}


/*
**    CSVFieldStart
**
** Returns a pointer to the start of the specified
** field of a CSV string.
**
**    Parameters
**
**    Input   CSV String
**    field   Number of field to look at, numbered from 0
*/
FDNPREF char FDNFUNC *FrontDoorNode::CSVFieldStart(char FDNDATA *Input, int field)
{
  char FDNDATA *pos=Input;
  int loop;
  for(loop=0; loop<field && pos; loop++){
    pos=strpbrk(pos, ",");
    if(pos) pos++;
  }
  if(!pos) return(Input+strlen(Input));
  return(pos);
}


/*
**    GetNFDXPage
**
** This function reads a page of data from NODELIST.FDX
** into a buffer.
**
**    Parameters
**
**    nd      Passed by reference block to copy data into.
**    pageno  Page number in index to copy, numbered from 1.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetNFDXPage(NFDXPage & nd, long pageno)
{
  if(!(Flags & FDNodeNoCacheN) && pageno==(long)first_n.index){
    // Page is in cache, copy to nd
    memcpy(&nd, nroot, (int) first_n.pagelen);
    return(1);
  }
  // Try to get from virtual cache system
  if(CheckNFDXCache(nd, pageno)) return(1);

  // No luck, we must fetch directly
  NFDX.Seek(first_n.pagelen*pageno, SEEK_SET);
  NFDX.Read(&nd, (size_t) first_n.pagelen, 1, 1);
  CommitNFDXCache(nd, pageno);
  return(1);
};


/*
**    GetUFDXPage
**
** This function reads a page of data from USERLIST.FDX
** into a buffer.
**
**    Parameters
**
**    ud      Passed by reference block to copy data into.
**    pageno  Page number in index to copy, numbered from 1.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetUFDXPage(UFDXPage & ud, long pageno)
{
  if(!(Flags & FDNodeNoCacheU) && pageno== (long) first_u.index){
    // Page is in cache, copy to ud
    memcpy(&ud, uroot, (int) first_u.pagelen);
    return(1);
  }
  // Try to get from virtual cache system
  if(CheckUFDXCache(ud, pageno)) return(1);

  // No luck, we must fetch directly
  UFDX.Seek(first_u.pagelen*pageno, SEEK_SET);
  UFDX.Read(&ud, (size_t) first_u.pagelen, 1, 1);

  // Allow virtual cache system to see fetched page
  CommitUFDXCache(ud, pageno);
  return(1);
}


/*
**    GetPFDXPage
**
** This function reads a page of data from PHONE.FDX
** into a buffer.
**
**    Parameters
**
**    pd      Passed by reference block to copy data into.
**    pageno  Page number in index to copy, numbered from 1.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetPFDXPage(PFDXPage & pd, long pageno)
{
  if(!(Flags & FDNodeNoCacheP) && pageno== (long) first_p.index){
    // Page is in cache, copy to ud
    memcpy(&pd, proot, (int) first_p.pagelen);
    return(1);
  }
  if(!PFDX.Seek(first_p.pagelen*pageno, SEEK_SET)) return(0);
  if(!PFDX.Read(&pd, (size_t) first_p.pagelen, 1, 1)) return(0);
  // Check for zero records
  if(!pd.records){
    SignalError(25);
    return(0);
  }  
  return(1);
}


/*
**    GetPFDAPage
**
** This function reads a page of data from PHONE.FDA
** into a buffer.
**
**    Parameters
**
**    pd      Passed by reference block to copy data into.
**    pageno  Page number in index to copy, numbered from 1.
**
**    Returns
**
**    1 on success, 0 on failure.
*/
FDNPREF int FDNFUNC FrontDoorNode::GetPFDAPage(FDNPhoneRec & rd, long pageno)
{
  if(!PFDA.Seek(sizeof(FDNPhoneRec)*pageno, SEEK_SET)) return(0);
  if(!PFDA.Read(&rd, (size_t) sizeof(FDNPhoneRec), 1, 1)) return(0);
  return(1);
}


/*
**    ToUpper
**
** A more general replacement of the toupper() standard
** library function.
*/
FDNPREF char FDNFUNC FrontDoorNode::ToUpper(char c)
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


/****************************************************************************/
/*                                                                          */
/* Class: FDNFind                                                           */
/* Purpose: Used to store and retrieve information from the nodelist        */
/*                                                                          */
/* Related Classes: FrontDoorNode                                           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                   P U B L I C   F U N C T I O N S                        */
/*                                                                          */
/****************************************************************************/

FDNPREF char FDNFUNC *FDNFind::GetSysop()
{
  return(Parent->GetSysop(*this));
}

FDNPREF char FDNFUNC *FDNFind::GetLocation()
{
  return(Parent->GetLocation(*this));
}

FDNPREF char FDNFUNC *FDNFind::GetSysName()
{
  return(Parent->GetSysName(*this));
}

FDNPREF unsigned long FDNFUNC FDNFind::GetSpeed()
{
  return(Parent->GetSpeed(*this));
}

FDNPREF char FDNFUNC *FDNFind::GetNumber()
{
  return(Parent->GetNumber(*this));
}

FDNPREF char FDNFUNC *FDNFind::GetFlags()
{
  return(Parent->GetFlags(*this));
}

FDNPREF unsigned short FDNFUNC FDNFind::GetPhoneData(char * buffer)
{
  return(Parent->GetPhoneData(*this, buffer));
}

#ifdef FDN_NoFlagBuild

FDNPREF long FDNFUNC FDNFind::GetFDAFlags()
{
  return(Parent->GetFDAFlags(*this));
}

#endif

FDNPREF FDNFind FDNFUNC FDNFind::operator++()
{
  Parent->Find(*this);
  return(*this);
}


/*
**    GetIndexOffset
**
** Produces a specialised offset used only by the nodelist
** class to specify the location of an Index Record.
** Can be used to fetch data on that item in future without
** another nodelist lookup as such. See below.
*/
FDNPREF long FDNFUNC FDNFind::GetIndexOffset()
{
  long IndexValue = 0xFF000000L;

  if(!searchtype) IndexValue = 0xFE000000L;

  IndexValue += (page[level-1] << 8);
  IndexValue += record[level-1];

  return(IndexValue);
}


/*
**    SetIndexOffset
**
** Attempts to initialise the find block back to the state
** it was in when GetIndexOffset was called to produce
** the passed offset.
**
**    Parameters
**
**    newoffset   Obtained from GetIndexOffset above.
**
**    Returns
**
**    1 on success, 0 on failure (do not use find block).
*/
FDNPREF int FDNFUNC FDNFind::SetIndexOffset(long newoffset)
{
  return(SetIndexOffset(newoffset, *Parent));
}


/*
**    SetIndexOffset
**
** A version of the above to be called when the find block
** has not already been attached to a FrontDoorNode object
** (by a previous search for example).
** See above for more details.
*/
FDNPREF int FDNFUNC FDNFind::SetIndexOffset(long newoffset, FrontDoorNode & NewParent)
{
  unsigned char IndexFile = (unsigned char) ((newoffset & 0xFF000000L) >> 24);
  long Page      = (newoffset & 0x00FFFF00L) >> 8;
  int  Record    = (int) (newoffset & 0x000000FFL);

  UFDXPage * ud;
  NFDXPage * nd;


  this->Parent = &NewParent;
  this->UnixStamp = time(NULL);
  this->finished=1;

  if(Parent->IsFrozen()){
    Parent->SignalError(29);
    return(0);
  }

  switch(IndexFile){
    case 0xFF : // NODELIST.FDX
      nd = new NFDXPage;
      if(!nd){
        Parent->SignalError(10);
        return(0);
      }
      if(!Parent->GetNFDXPage(*nd, Page)){
        Parent->SignalError(1); // Perhaps not appropriate.
        Parent->NFDX.ClearError();
        delete(nd);
        return(0);
      }
      this->zone   = Parent->SwapBytes(nd->nodes[Record].zone);
      this->net    = Parent->SwapBytes(nd->nodes[Record].net);
      this->node   = Parent->SwapBytes(nd->nodes[Record].node);
      this->point  = Parent->SwapBytes(nd->nodes[Record].point);
      this->rnode  = nd->nodes[Record].rnode;
      this->rnet   = nd->nodes[Record].rnet;
      this->status = nd->nodes[Record].nodetype;
      this->offset = nd->nodes[Record].offset.loff;
      this->searchtype = 1;
      delete(nd);
      break;
    case 0xFE : // USERLIST.FDX
      ud = new UFDXPage;
      if(!ud){
        Parent->SignalError(10);
        return(0);
      }
      if(!Parent->GetUFDXPage(*ud, Page)){
        Parent->SignalError(1); // Perhaps not appropriate.
        Parent->UFDX.ClearError();
        delete(ud);
        return(0);
      }
      this->zone   = Parent->SwapBytes(ud->names[Record].zone);
      this->net    = Parent->SwapBytes(ud->names[Record].net);
      this->node   = Parent->SwapBytes(ud->names[Record].node);
      this->point  = Parent->SwapBytes(ud->names[Record].point);
      this->status = ud->names[Record].nodetype;
      this->offset = ud->names[Record].offset;
      this->searchtype = 0;
      delete(ud);
      break;
    default :   // In all likelyhood this is a nodelist database offset
      return(0);
  }
  return(1);
}


/****************************************************************************/
/*                                                                          */
/* Class: FDNFile                                                           */
/* Purpose: Used to perform file io on the nodelist indices and data files  */
/*                                                                          */
/* Related Classes: FrontDoorNode                                           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                   P U B L I C   F U N C T I O N S                        */
/*                                                                          */
/****************************************************************************/


// FDNFile::FDNFile()
// If it is required that you initialise data you may do so in this section
// You /should/ initialise Status = Error = 0, and ensure FileName is empty.

#ifdef FDN_USESTD

FDNFile::FDNFile()
{
  Status=Error=0;
  *FileName=0;
  Data = NULL;
}

#elif defined(FDN_USEIOS)

FDNFile::FDNFile()
{
  Status=Error=0;
  *FileName=0;
}

#elif defined(FDN_USEHAND)

FDNFile::FDNFile()
{
  Status=Error=0;
  *FileName=0;
  Data = 0;
}

#endif


// int FDNFile::Open()
// Attempt to open the file pointed to in filename and connects it to Data
// Status should be set to 1 on a successful open.
// Returns 0 on failure, non zero on success.

#ifdef FDN_USESTD

FDNPREF int  FDNFUNC FDNFile::Open()
{
  char * pFileName=FileName;
#ifdef FDN_WINDOWS
  char AnsiFileName[PATHLENGTH];
  AnsiToOem(FileName, AnsiFileName);
  pFileName=AnsiFileName;
#endif
  Data = _fsopen(pFileName, "rb", SH_DENYWR);
  if(!Data){
    SignalError(errno);
    return(0);
  }
  Status=1;
  return(1);
}

#elif defined(FDN_USEIOS)

FDNPREF int  FDNFUNC FDNFile::Open()
{
  char * pFileName=FileName;
#ifdef FDN_WINDOWS
  char AnsiFileName[PATHLENGTH];
  AnsiToOem(FileName, AnsiFileName);
  pFileName=AnsiFileName;
#endif
  Data.open(pFileName, ios::binary | ios::in | ios::nocreate , SH_DENYWR);
  if(!Data){
    // I don't know if we can do this next bit...
    SignalError(errno);
    return(0);
  }
  else return(1);
}

#elif defined(FDN_USEHAND)

FDNPREF int  FDNFUNC FDNFile::Open()
{
  int flag;
  char * pFileName=FileName;
#ifdef FDN_WINDOWS
  char AnsiFileName[PATHLENGTH];
  AnsiToOem(FileName, AnsiFileName);
  pFileName=AnsiFileName;
#endif
  flag = sopen(pFileName, O_RDONLY | O_BINARY, SH_DENYWR);
  if(flag==-1){
    SignalError(errno);
    return(0);
  }
  Data = flag;
  Status=1;
  return(1);
}

#endif


// int FDNFile::Close();
// Attempts to close the file in use
// Status should be set to zero on successful closure.
// returns 0 on failure, non zero otherwise.

#ifdef FDN_USESTD

FDNPREF int  FDNFUNC FDNFile::Close()
{
  int flag;
  if(Data) flag=fclose(Data); else return(0);
  if(!flag){
    Status=0;
    return(1);
  }
  SignalError(errno);
  return(0);
}

#elif defined(FDN_USEIOS)

FDNPREF int  FDNFUNC FDNFile::Close()
{
  Data.close();
  Status=0;
  return(1);
}

#elif defined(FDN_USEHAND)

FDNPREF int  FDNFUNC FDNFile::Close()
{
  int flag;
  if(Data) flag=close(Data); else return(0);
  if(!flag){
    Status=0;
    return(1);
  }
  SignalError(errno);
  return(0);
}

#endif


// int FDNFile::Seek(long offset, int whence)
// This function should seek to the appropriate offset in a file based on whence
// which follows the SEEK_SET, SEEK_CUR and SEEK_END example.
// Function shouyld return 0 on failure, 1 otherwise.

#ifdef FDN_USESTD

FDNPREF int  FDNFUNC FDNFile::Seek(long offset, int whence)
{
  int flag;
  flag = fseek(Data, offset, whence);
  if(flag){
    SignalError(errno);
    return(0);
  }
  else return(1);
}


#elif defined(FDN_USEIOS)

FDNPREF int  FDNFUNC FDNFile::Seek(long offset, int whence)
{
  ios::seek_dir s;
  switch(whence){
    case SEEK_SET : s = ios::beg; break;
    case SEEK_CUR : s = ios::cur; break;
    case SEEK_END : s = ios::end; break;
  }
  Data.seekg(offset, s);
  if(Data.rdstate()){
    SignalError(errno);
    Data.clear();
    return(0);
  }
  return(1);
}


#elif defined(FDN_USEHAND)

FDNPREF int  FDNFUNC FDNFile::Seek(long offset, int whence)
{
  long flag;
  flag = lseek(Data, offset, whence);
  if(flag==-1){
    SignalError(errno);
    return(0);
  }
  else return(1);
}

#endif



// int FDNFile::Read(void * address, size_t size, size_t items)
// This function attempts to read "items" objects of size "size" into the memory
// location pointed to be address.
// The function should return 0 on failure, non zero on success.


#ifdef FDN_USESTD

FDNPREF int  FDNFUNC FDNFile::Read(void * address, size_t size, size_t items, int ErrSensitive)
{
  size_t noread;
  noread = fread(address, size, items, Data);
  if(ErrSensitive && (noread!=items)){
    SignalError(EZERO);
    // Hmm, nothing to call with SignalError really :-|.
    return(0);
  }
  else return(1);
}

#elif defined(FDN_USEIOS)

FDNPREF int  FDNFUNC FDNFile::Read(void * address, size_t size, size_t items, int ErrSensitive)
{
  Data.read((char *) address, (int) (size * items));
  if(ErrSensitive && Data.rdstate()){
    SignalError(errno);
    Data.clear();
    return(0);
  }
  if(Data.rdstart()) Data.clear();
  return(1);
}

#elif defined(FDN_USEHAND)

FDNPREF int  FDNFUNC FDNFile::Read(void * address, size_t size, size_t items, int ErrSensitive)
{
  int flag;
  flag = read(Data, address, (unsigned int) (size * items));
  // Were we able to read in all values?
  if(ErrSensitive && (flag <= (int) (items * size))){
    SignalError(EZERO);
    return(0);
  }
  /*
  if(flag==-1){
    // A more serious error, always report    
    SignalError(errno);
    return(0);
  }*/
  return(1);
}

#endif

#if defined(FDN_USEHAND) || defined(FDN_USEIOS) || defined(FDN_USESTD)
FDNFile::~FDNFile()
{
  Close();
  Status=0;
}

#endif
