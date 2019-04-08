/*
** Piglet Productions
**
** FileName       : FDWCACHE.H
**
** Defines        : FDWCachedNode <- FrontDoorWNode
**
** Description
**
** Simple cache system for FrontDoor Nodelist Writing code.
**
**
** Initial Coding : Colin Turner
**
** Date           : March 1998
**
**
** Copyright applies on this file, and distribution may be limited.
*/

/*
** Revision 1.04
**
** For Revision history see FDNODE.HIS
**
*/
 
/*
** With thanks to Mats Wallin.
*/


//#define g_RunDebug

#include "fdnode.h"

#ifdef g_RunDebug
#include <g_log.h>
#endif // g_RunDebug



class FDWCachedNode;


class FDWCachedNode : public FrontDoorWNode{

  // Data

  protected :

    unsigned int  NFDXCacheSize;
    unsigned int  UFDXCacheSize;

    long *        NFDXPageNo;
    long *        UFDXPageNo;

    unsigned long * NFDXHitNo;
    unsigned long * UFDXHitNo;

    unsigned long Counter;

    NFDXPage *    NFDXCache;
    UFDXPage *    UFDXCache;

    unsigned char * NFDXDirtyMap;
    unsigned char * UFDXDirtyMap;
  
  // Services

  // Implementation

  public :
    
    FDWCachedNode();
    FDWCachedNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags);
    FDWCachedNode(const char FDNDATA *nldir, const char FDNDATA *nlext, unsigned short cc, long flags,
                  unsigned int nfdxCacheSize, unsigned int ufdxCacheSize);
    
    virtual ~FDWCachedNode();

  protected :

               virtual void OnThaw();
               virtual void OnFreeze();
    FDNPREF    virtual int  FDNFUNC CheckCache(NFDXPage & tofill, long page);
    FDNPREF    virtual int  FDNFUNC CommitCache(NFDXPage & value, long page);
    FDNPREF    virtual int  FDNFUNC CheckCache(UFDXPage & tofill, long page);
    FDNPREF    virtual int  FDNFUNC CommitCache(UFDXPage & value, long page);

                      void  SetNFDXBit(long bit);
                      void  ClearNFDXBit(long bit);
                       int  GetNFDXBit(long bit);

                      void  SetUFDXBit(long bit);
                      void  ClearUFDXBit(long bit);
                       int  GetUFDXBit(long bit);

    
                       int  GetNFDXCacheEntryNo(long page);
                       int  GetUFDXCacheEntryNo(long page);
                      void  ConstructCache();
                      void  DestroyCache();

                      void  ConfigureDefaults();

  public:

  FDNPREF void FDNFUNC SetCacheSize(unsigned int nfdxCacheSize, unsigned int ufdxCacheSize);

// Debugging only
protected:
  #ifdef g_RunDebug
  g_Log Trace;
  #endif


};

