#include "fdwcache.h"   
#ifdef g_RunDebug
#include <g_log.h>
#endif // g_RunDebug


/*
**    FDWCachedNode
**
** Basic plain vanilla constructor
**
**/
FDWCachedNode::FDWCachedNode() : FrontDoorWNode()
{
  Flags = Flags | WFDNodeOverWrite;
  Counter = 0;
  ConfigureDefaults();
  ConstructCache();
}


FDWCachedNode::FDWCachedNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags) : FrontDoorWNode()
{
  Flags = flags;
  Counter = 0;
  SetNLDir(nldir);
  SetNLExt(nlext);
  SetCountry(cc);
  ConfigureDefaults();
  ConstructCache();
  if(!(Flags & WFDNodeCreateFrozen)) Thaw();
}


FDWCachedNode::FDWCachedNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags, unsigned int nfdxCacheSize, unsigned int ufdxCacheSize) : FrontDoorWNode()
{
  Flags = flags;
  Counter = 0;
  SetNLDir(nldir);
  SetNLExt(nlext);
  SetCountry(cc);
  NFDXCacheSize = nfdxCacheSize;
  UFDXCacheSize = ufdxCacheSize;
  ConstructCache();
  if(!(Flags & WFDNodeCreateFrozen)) Thaw();
}


/*
**    ~FDWCachedNode
**
**
**
**/
FDWCachedNode::~FDWCachedNode()
{
  Freeze ();
  DestroyCache();
}


/*
**    ConstructCache
**
** Attempts to allocate memory for the Cache(s). If either allocation fails,
** the cache(s) will be disabled.
**
*/
void FDWCachedNode::ConstructCache()
{
  unsigned int loop;

  if(NFDXCacheSize){
    NFDXCache    = new NFDXPage[(const unsigned int) NFDXCacheSize];
    NFDXPageNo   = new long[(const unsigned int) NFDXCacheSize];
    NFDXHitNo    = new unsigned long[(const unsigned int) NFDXCacheSize];
    NFDXDirtyMap = new unsigned char[(NFDXCacheSize / 8) + 1];
    if(!NFDXCache || !NFDXPageNo || !NFDXHitNo || !NFDXDirtyMap){
      NFDXCacheSize = 0;
      if(NFDXCache)    delete [] NFDXCache;
      if(NFDXPageNo)   delete [] NFDXPageNo;
      if(NFDXHitNo)    delete [] NFDXHitNo;
      if(NFDXDirtyMap) delete [] NFDXDirtyMap;

      NFDXCache    = NULL;
      NFDXPageNo   = NULL;
      NFDXHitNo    = NULL;
      NFDXDirtyMap = NULL;

      SignalError(-1);
    }
  }

  if(UFDXCacheSize){
    UFDXCache    = new UFDXPage[(const unsigned int) UFDXCacheSize];
    UFDXPageNo   = new long[(const unsigned int) UFDXCacheSize];
    UFDXHitNo    = new unsigned long[(const unsigned int) UFDXCacheSize];
    UFDXDirtyMap = new unsigned char[(UFDXCacheSize / 8) + 1];
    if(!UFDXCache || !UFDXPageNo || !UFDXHitNo || !UFDXDirtyMap){
      UFDXCacheSize = 0;
      if(UFDXCache)    delete [] UFDXCache;
      if(UFDXPageNo)   delete [] UFDXPageNo;
      if(UFDXHitNo)    delete [] UFDXHitNo;
      if(UFDXDirtyMap) delete [] UFDXDirtyMap;

      UFDXCache    = NULL;
      UFDXPageNo   = NULL;
      UFDXHitNo    = NULL;
      UFDXDirtyMap = NULL;

      SignalError(-1);
    }
  }
  
  // Zero some bits and bobs. Should be done in OnThaw() anyway
  for(loop = 0; loop < NFDXCacheSize; loop++){
    NFDXPageNo[loop] = NFDXHitNo[loop] = 0;
  }
  for(loop = 0; loop < UFDXCacheSize; loop++) {
    UFDXPageNo[loop] = UFDXHitNo[loop] = 0;
  }

  if(NFDXCacheSize) memset(NFDXDirtyMap, 0, (NFDXCacheSize / 8) + 1);
  if(UFDXCacheSize) memset(UFDXDirtyMap, 0, (UFDXCacheSize / 8) + 1);

  // Debug logging
  #ifdef g_RunDebug
  Trace.SetName("CachedNode");
  Trace.SetVersion("1.01");
  Trace.SetFile("CACHED.LOG");
  //Trace.SetFlags(_G_LOG_FlushEvery);
  Trace.Open();
  #endif

}


/*
**    DestroyCache
**
** Deallocates memory set aside for the Cache pages.
*/
void FDWCachedNode::DestroyCache()
{
  if(NFDXCache)  delete [] NFDXCache;
  if(NFDXPageNo) delete [] NFDXPageNo;
  if(NFDXHitNo)  delete [] NFDXHitNo;
  if(UFDXCache)  delete [] UFDXCache;
  if(UFDXPageNo) delete [] UFDXPageNo;
  if(UFDXHitNo)  delete [] UFDXHitNo;

  if(NFDXDirtyMap) delete [] NFDXDirtyMap;
  if(UFDXDirtyMap) delete [] UFDXDirtyMap;

  NFDXCache    = NULL;
  UFDXCache    = NULL;
  NFDXPageNo   = UFDXPageNo = NULL;
  NFDXHitNo    = UFDXHitNo  = NULL;
  NFDXDirtyMap = UFDXDirtyMap = NULL;
}  


/*
**    OnThaw
**
** An override for the virtual function in the base class. Note that
** Constructors and Destructors of the base class will call the plain vanilla
** version, and /not/ this function. Therefore, it's essential that you call
** Thaw() in the derived class, and not the base if you want this called.
**
*/
void FDWCachedNode::OnThaw()
{
  unsigned int loop;

  #ifdef g_RunDebug
  Trace.Printf("%tOnThaw()\n");
  #endif

  for(loop = 0; loop < NFDXCacheSize; loop++){
    NFDXPageNo[loop] = NFDXHitNo[loop] = 0;
  }
  for(loop = 0; loop < UFDXCacheSize; loop++){
    UFDXPageNo[loop] = UFDXHitNo[loop] = 0;
  }
}


/*
**    OnFreeze
**
** Similar to the above, this is an override for a function in the base class,
** which ensures that any pages not committed to disk when a Freeze (probably
** due to class destruction) occurs, are flushed from the Cache(s).
**
*/
void FDWCachedNode::OnFreeze()
{
  unsigned int loop;

  #ifdef g_RunDebug
  Trace.Printf("%tOnFreeze()\n");
  #endif
  for(loop = 0; loop < NFDXCacheSize; loop++){
    if(NFDXPageNo[loop]) RawWritePage(NFDXCache[loop], NFDXPageNo[loop]);
  }
  for(loop = 0; loop < UFDXCacheSize; loop++){
    if(UFDXPageNo[loop]) RawWritePage(UFDXCache[loop], UFDXPageNo[loop]);
  }
}


/*
**    CheckCache
**
** Override of virtual function in base.
**
** This function simply checks if the requested page is in the Cache, and if so
** it copies it to the tofill structure and updates the HitCounter for that
** page.
**
**    Parameters
**
**    tofill    Where to copy the data (if we have it)
**    page      The page number being requested from the cache
**
**    Returns
**
**    0 on failure (NoCacheHit)
**    1 on success (CacheHit)
**
*/
int FDWCachedNode::CheckCache(NFDXPage & tofill, long page)
{
  unsigned int loop;
  int found = 0;

  if(!NFDXCacheSize) return(0);

  #ifdef g_RunDebug
  Trace.Printf("%tCheckCache - Page number %lu\n", page);
  #endif
  for(loop = 0; (loop < NFDXCacheSize) && !found; loop++){
   if(NFDXPageNo[loop] == page){
     #ifdef g_RunDebug
     Trace.Printf("%tCheckCache - CacheHit\n");
     #endif
     memcpy(&tofill, &(NFDXCache[loop]), sizeof(NFDXPage));
     NFDXHitNo[loop] = Counter++;
     found = 1;
   }
  }
  return(found);
}


/*
**    CommitCache
**
** Override of virtual function in base.
**
** The base class hands us the page and essentially asks if we are prepared to
** take care of it. In this simple cache system, we /always/ take care of the
** page. So we insert it in the Cache, and return 1.
**
**    Parameters
**
**    tofill    Where to get the data to commit to the cache
**    page      The page number being given to the cache
**
**    Returns
**
**    0 indicates the Cache system will not handle the page (only if disabled)
**    1 indicates the Cache system will take care of the saving of the page
**
*/
int FDWCachedNode::CommitCache(NFDXPage & tocache, long page)
{
  int InsertPoint;

  if(!NFDXCacheSize) return(0);
   
  InsertPoint = GetNFDXCacheEntryNo(page);

  #ifdef g_RunDebug
  Trace.Printf("%tCommitCache - Page number %lu\n", page);
  Trace.Printf("%tCommitCache - Insert page %u\n", InsertPoint);
  #endif
  
  if(NFDXPageNo[InsertPoint]){
    // The InsertPoint is currently occupied
    if(NFDXPageNo[InsertPoint] == page){
      // By the page we plan to insert
      // It will now be dirty
      SetNFDXBit(InsertPoint);
    }
    else{
      // By some other page
      // If it's dirty we need to put it to disk before we lose it
      if(GetNFDXBit(InsertPoint)){
        #ifdef g_RunDebug
        Trace.Printf("%tCommitCache - Send cache page to secondary\n");
        #endif
        RawWritePage(NFDXCache[InsertPoint], NFDXPageNo[InsertPoint]);
      }
      // The new page we load will be clean
      ClearNFDXBit(InsertPoint);
    }
  }
     
  memcpy(&(NFDXCache[InsertPoint]), &tocache, sizeof(NFDXPage));
  NFDXHitNo[InsertPoint] = Counter++;
  NFDXPageNo[InsertPoint] = page;
  return(1);
}


/*
**    See overloaded function above for details.
*/
int FDWCachedNode::CheckCache(UFDXPage & tofill, long page)
{
  unsigned int loop;
  int found = 0;

  if(!UFDXCacheSize) return(0);

  #ifdef g_RunDebug
  Trace.Printf("%tCheckCache - Page number %lu\n", page);
  #endif
  for(loop = 0; (loop < UFDXCacheSize) && !found; loop++){
   if(UFDXPageNo[loop] == page){
     #ifdef g_RunDebug
     Trace.Printf("%tCheckCache - CacheHit\n");
     #endif
     memcpy(&tofill, &(UFDXCache[loop]), sizeof(UFDXPage));
     UFDXHitNo[loop] = Counter++;
     found = 1;
   }
  }
  return(found);
}


/*
**    See overloaded function above for details.
*/
int FDWCachedNode::CommitCache(UFDXPage & tocache, long page)
{
  int InsertPoint;
   
  if(!UFDXCacheSize) return(0);

  InsertPoint = GetUFDXCacheEntryNo(page);

  #ifdef g_RunDebug
  Trace.Printf("%tCommitCache - Page number %lu\n", page);
  Trace.Printf("%tCommitCache - Insert page %u\n", InsertPoint);
  #endif
  
  if(UFDXPageNo[InsertPoint]){
    // The InsertPoint is currently occupied
    if(UFDXPageNo[InsertPoint] == page){
      // By the page we plan to insert
      // It will now be dirty
      SetUFDXBit(InsertPoint);
    }
    else{
      // By some other page
      // If it's dirty we need to put it to disk before we lose it
      if(GetUFDXBit(InsertPoint)){
        #ifdef g_RunDebug
        Trace.Printf("%tCommitCache - Send cache page to secondary\n");
        #endif
        RawWritePage(UFDXCache[InsertPoint], UFDXPageNo[InsertPoint]);
      }
      // The new page we load will be clean
      ClearUFDXBit(InsertPoint);
    }
  }

  memcpy(&(UFDXCache[InsertPoint]), &tocache, sizeof(UFDXPage));
  UFDXHitNo[InsertPoint] = Counter++;
  UFDXPageNo[InsertPoint] = page;
  return(1);
}


/*
**    GetNFDXCacheEntryNo
**
** Finds the best location for inserting a page in the cache, based on whether
** the page is already in the Cache, and what Hits the existing pages have
** received.
**
**    Parameters
**    
**    page    The number of the page we have to store
**
**    Returns
**
**    The subscript in the Cache array into which we copy the data
*/
int FDWCachedNode::GetNFDXCacheEntryNo(long page)
{
  unsigned int loop;
  int InsertPoint = NFDXCacheSize - 1;
  unsigned long MinimalHits = NFDXHitNo[NFDXCacheSize - 1];

  for(loop = 0; loop < NFDXCacheSize; loop++){
    // The correct page
    if(NFDXPageNo[loop] == page) return(loop);
  }
    
  // Go from left to right, looking for holes
  for(loop = 0; loop < NFDXCacheSize; loop++){
    // An empty record, fine
    if(!NFDXPageNo[loop]) return(loop);
  }
    
  for(loop = NFDXCacheSize; loop != 0; loop--){
    if(NFDXHitNo[loop - 1] < MinimalHits){
      MinimalHits = NFDXHitNo[loop - 1];
      InsertPoint = loop - 1;
    }
  }
  return(InsertPoint);
}


/*
**    See analogous function above for more details
*/
int FDWCachedNode::GetUFDXCacheEntryNo(long page)
{
  unsigned int loop;
  int InsertPoint = UFDXCacheSize - 1;
  unsigned long MinimalHits = UFDXHitNo[UFDXCacheSize - 1];

  for(loop = 0; loop < UFDXCacheSize; loop++){
    // The correct page
    if(UFDXPageNo[loop] == page) return(loop);
  }
    
  // Go from left to right, looking for holes
  for(loop = 0; loop < UFDXCacheSize; loop++){
    // An empty record, fine
    if(!UFDXPageNo[loop]) return(loop);
  }
    
  for(loop = UFDXCacheSize; loop != 0; loop--){
    //if(!UFDXPageNo[loop]) return(loop); // An empty record, fine
    if(UFDXHitNo[loop - 1] < MinimalHits){
      MinimalHits = UFDXHitNo[loop - 1];
      InsertPoint = loop - 1;
    }
  }
  return(InsertPoint);
}


/*
**    ConfigureDefaults
**
** A function which sets up some default values for items in the class.
**
**/
void FDWCachedNode::ConfigureDefaults()
{
  #ifdef __DOS__
    #ifdef __386__
    NFDXCacheSize = 500;
    UFDXCacheSize = 500;    
    #else
    NFDXCacheSize = 60;
    UFDXCacheSize = 60;  
    #endif    
  #elif defined(__NT__) || defined(__OS2__)
  NFDXCacheSize = 1000;
  UFDXCacheSize = 1000;
  #else
  NFDXCacheSize = 60;
  UFDXCacheSize = 60;  
  #endif
}


/*
**    SetCacheSize
**
** This function can be used to alter the size of the cache(s). It will flush
** data from any existant caches, deallocate the memory and create and
** initialise the new caches.
**
** A cache size of zero effectively disables a cache. For a Cache to be useful
** I would suggest using at least double the height of the index (10 pages).
**
**    Parameters
**
**    nfdxCacheSize   The number of pages for caching NODELIST.FDX
**    ufdxCacheSize   The number of pages for caching USERLIST.FDX
**
**/
void FDWCachedNode::SetCacheSize(unsigned int nfdxCacheSize, unsigned int ufdxCacheSize)
{
  // First we need to flush the Cache
  OnFreeze();
  // Then we destroy the current Cache
  DestroyCache();
  // Then we set the new values
  NFDXCacheSize = nfdxCacheSize;
  UFDXCacheSize = ufdxCacheSize;
  // And then we construct the cache
  ConstructCache();

  OnThaw();
}


void  FDWCachedNode::SetNFDXBit(long bit)
{
  NFDXDirtyMap[bit / 8] |= (char) (1 << (bit % 8));
}

void  FDWCachedNode::ClearNFDXBit(long bit)
{
  NFDXDirtyMap[bit / 8] &= (char) ~(1 << (bit % 8));
}

int   FDWCachedNode::GetNFDXBit(long bit)
{
  return(NFDXDirtyMap[bit / 8] & (char) (1 << (bit % 8)));
}

void  FDWCachedNode::SetUFDXBit(long bit)
{
  UFDXDirtyMap[bit / 8] |= (char) (1 << (bit % 8));
}

void  FDWCachedNode::ClearUFDXBit(long bit)
{
  UFDXDirtyMap[bit / 8] &= (char) ~(1 << (bit % 8));
}

int   FDWCachedNode::GetUFDXBit(long bit)
{
  //return(1);
  return(UFDXDirtyMap[bit / 8] & (char) (1 << (bit % 8)));
}

