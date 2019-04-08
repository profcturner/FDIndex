#include "fdnode.h"

/****************************************************************************/
/*                                                                          */
/* Class: FDNFile                                                           */
/* Purpose: Used to perform file io on the nodelist indices and data files  */
/*                                                                          */
/* Related Classes: FrontDoorWNode                                          */
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
  Status=Error=Flags=0;
  *FileName=0;
  Data = NULL;
}

#elif defined(FDN_USEIOS)

FDNFile::FDNFile()
{
  Status=Error=Flags=0;
  *FileName=0;
}

#elif defined(FDN_USEHAND)

FDNFile::FDNFile()
{
  Status=Error=Flags=0;
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

  int mode = 0;
  if(Flags & FDNFileDestroy) mode = 1;
  if(Flags & FDNFileUpdate)  mode = 2;

  switch(mode){
    case 0 : // open for read
      Data = _fsopen(pFileName, "rb", SH_DENYWR);
      break;
    case 1 : // open for destructive write
      Data = _fsopen(pFileName, "w+b", SH_DENYWR);
      break;
    case 2 : // open for update write
      Data = _fsopen(pFileName, "r+b", SH_DENYWR);
      #ifdef __WATCOMC__
      if(!Data && errno == ENOENT) Data = _fsopen(pFileName, "w+b", SH_DENYWR);
      #else
      if(!Data && errno == ENOFILE) Data = _fsopen(pFileName, "w+b", SH_DENYWR);
      #endif
      break;
  }

  if(!Data){
    SignalError(errno);
    return(0);
  }
  fseek(Data, 0, SEEK_SET);
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
  int mode = 0;
  if(Flags & FDNFileDestroy) mode = 1;
  if(Flags & FDNFileUpdate)  mode = 2;

  switch(mode){
    case 0 : // open for read
      Data.open(pFileName, ios::binary | ios::in, SH_DENYWR);      
      break;
    case 1 : // open for destructive write
      Data.open(pFileName, ios::binary | ios::in | ios::out | ios::truncate , SH_DENYWR);
      break;
    case 2 : // open for update write
      Data.open(pFileName, ios::binary | ios::in | ios::out , SH_DENYWR);
      break;
  }
      
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
  int mode = 0;
  if(Flags & FDNFileDestroy) mode = 1;
  if(Flags & FDNFileUpdate)  mode = 2;

  switch(mode){
    case 0 : // open for read
      flag = sopen(pFileName, O_RDONLY | O_BINARY, SH_DENYWR);
      break;
    case 1 : // open for destructive write
      flag = sopen(pFileName, O_TRUNC | O_CREAT | O_RDWR | O_BINARY, SH_DENYWR);
      break;
    case 2 : // open for update write
      flag = sopen(pFileName, O_RDWR | O_BINARY, SH_DENYWR);
      break;
  }
  
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


// long int FDNFile::Size()
// This functions determines the size of the opened file

#ifdef FDN_USESTD

FDNPREF long int FDNFUNC FDNFile::Size()
{
  long Extent, Current;
  Current = ftell(Data);
  fseek(Data, 0, SEEK_END);
  Extent  = ftell(Data);
  fseek(Data, Current, SEEK_SET);

  return(Extent);
}

#elif defined FDN_USEIOS

FDNPREF long int FDNFUNC FDNFile::Size()
{
  streampos Extent, Current;
  Current = Data.tellp();
  Data.seekg(0, ios::end);
  Extent  = Data.tellp();
  Data.seekg(Current, ios::cur);

  return((long int) Extent);
}

#elif defined FDN_USEHAND

FDNPREF long int FDNFUNC FDNFile::Size()
{
  long Extent, Current;
  Current = tell(Data);
  lseek(Data, 0, SEEK_END);
  Extent  = tell(Data);
  lseek(Data, Current, SEEK_SET);

  return(Extent);
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
  Data.clear();
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


// int FDNFile::Write(void * address, size_t size, size_t items, int ErrSense)
// This function attempts to write "items" objects of size "size" into the memory
// location pointed to be address.
// The function should return 0 on failure, non zero on success.

#ifdef FDN_USESTD

FDNPREF int  FDNFUNC FDNFile::Write(void * address, size_t size, size_t items, int ErrSensitive)
{
  size_t nowritten;
  nowritten = fwrite(address, size, items, Data);
  if(ErrSensitive && (nowritten!=items)){
    SignalError(EZERO);
    // Hmm, nothing to call with SignalError really :-|.
    return(0);
  }
  else return(1);
}

#elif defined(FDN_USEIOS)

FDNPREF int  FDNFUNC FDNFile::Write(void * address, size_t size, size_t items, int ErrSensitive)
{
  Data.write((char *) address, (int) (size * items));
  if(ErrSensitive && Data.rdstate()){
    SignalError(errno);
    Data.clear();
    return(0);
  }
  //if(Data.rdstart())
  Data.clear();
  return(1);
}

#elif defined(FDN_USEHAND)

FDNPREF int  FDNFUNC FDNFile::Write(void * address, size_t size, size_t items, int ErrSensitive)
{
  int flag;
  flag = write(Data, address, (unsigned int) (size * items));
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


