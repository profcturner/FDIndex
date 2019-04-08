/**************************************************************************/
/*                                                                        */
/* This file should be modified as per your personal requirements for the */
/*    nodelist class. A different copy may be used for each project by    */
/*            placing in the work directory of the project.               */
/*                                                                        */
/**************************************************************************/


#ifndef _FDN_FDNUSER
#define _FDN_FDNUSER


// The following dictates what system to use for io. These represent
// iostream.h, stdio.h, and io.h style systems respectively.

// !BE SURE TO ONLY UNCOMMENT ONE AT A TIME! Otherwise the code will NOT
// compile. If you are not using C++ you cannot select FDN_USEIOS.

//#define FDN_USEIOS
#define FDN_USESTD
//#define FDN_USEHAND

// If you must use your own IO system uncomment the following line for the
// io system and carefully read the guidelines in FDNODE.DOC concerning
// the building of your own object.

//#define FDN_USEUSER

// You may elect to use a standard set of functions to create and delete
// busy semaphores if you use your own IO system. Uncomment one of the
// following to do this. Otherwise you must create your own functions.

#ifdef FDN_USEUSER

//#define FDN_INST_USESTD
//#define FDN_INST_USEIOS
//#define FDN_INST_USEHAND

#endif

// The following should be uncommented if compiling the code into a DLL
#define FDN_DLL


// The following should be uncommented if the alignment for the structures
// should be forced to one byte packing
#define FDN_PACK

// If you are deriving a new class based on the FDNFile class for enhanced
// error detection you should specify the name of your derived object here.
// Otherwise, it should remain as FDNFile.

#define FDN_FileObject FDNFile

// If you wish to use your own IO system then you must supply code for all
// member functions listed in FDNODE.H in the FDNFile for which there is no
// function body so far.
// Specify the name of the data object which holds the file info (eg.
// file pointer, integer handle, or some other object) after the define here.

#ifdef FDN_USEUSER
#define FDN_UserFileObject CAbsBufFile
#endif



#else /* #ifndef _FDN_FDNUSER */



#endif /* #ifndef _FDN_FDNUSER / #else */

/* end of file fdnuser.h */
