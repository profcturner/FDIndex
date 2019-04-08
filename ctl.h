/*
**  ctl.h
**
**  Structures for SETUP.FD
**
**  Copyright 1989-1995 Joaquim Homrighausen; All rights reserved.
**
**  Last revised: 95-08-24                         FrontDoor 2.25+
**
**  -------------------------------------------------------------------------
**  This information is not necessarily final and is subject to change at any
**  given time without further notice
**
**  Please note that some new global and mailer-specific data has been added
**  (94-05-31) to some rather inappropriate structures. This was done in
**  attempt to add capabilities without breaking current software that may
**  depend on the format of SETUP.FD. NOTE: The format of this file will
**  be completely revised for 3.00.
**  -------------------------------------------------------------------------
*/
#ifndef __SETUPFD__
#define __SETUPFD__

#ifdef __cplusplus
extern "C" {
#endif

#define CURRPRGREV  0x94053100UL

#pragma pack(1)

/*
**  Mailer ------------------------------------------------------------------
*/

    /* Loglevels */

#define LOGFATAL    0x0001                 /*'!' Fatal errors*/
#define LOGERROR    0x0002                 /*'?' Errors*/
#define LOGBRIEF    0x0004                 /*'+' Major changes in action*/
#define LOGACCT     0x0008                 /*'$' Accounting information*/
#define LOGXFER     0x0010                 /*'*' Sent/Received files*/
#define LOGTRIVIAL  0x0020                 /*'-' Minor changes in action*/
#define LOGXFERMSG  0x0040                 /*'%' Transfer messages*/
#define LOGMODEM    0x0080                 /*'=' Modem activities/responses*/
#define LOGTRX      0x0100                 /*':' Session transaction numbers*/
#define LOGSESSION  0x0200                 /*'~' Session stuff, SysOp, etc.*/
#define LOGRMTAKAS  0x0400                 /*'~' Remote system's AKAs*/
#define LOGRMTINFO  0x0800                 /*'~' Remote system's information*/
#define LOGUNEXPECT 0x1000                 /*'#' Unexpected password*/
#define LOGDEBUG    0x8000                 /*    All loglevels enabled*/

    /* Miscellaneous flags (FLAGS) */

#define NOUNLISTED      0x00000001L           /*Don't allow unlisted systems*/
#define CREATE_BATCH    0x00000002L       /*Create batchfile when BBS caller*/
#define TERMONLY        0x00000004L               /*Running as Terminal-Only*/
#define DTR_HANGUP      0x00000008L                      /*Use DTR to hangup*/
#define DTR_DIAL        0x00000010L            /*Toggle DTR prior to dialing*/
#define DTR_BUSY        0x00000020L                 /*Use DTR to signal BUSY*/
#define MANUAL_ANSWER   0x00000040L                      /*Use manual answer*/
#define CONNECT_RESET   0x00000080L           /*Reset modem to connect speed*/
#define MODEM_FIXED     0x00000100L       /*Use constant speed against modem*/
#define NOMAIL_NOPWD    0x00000200L         /*No mail from unsecured systems*/
#define xxxxDELAYFIRST  0x00000400L                /*Delay before first call*/
#define CLOCK           0x00000800L                          /*Display clock*/
#define KILLNULL        0x00001000L           /*Remove messages with no text*/
#define MAILONLY        0x00002000L              /*Don't allow human callers*/
#define xxxxLIMITEDANSW 0x00004000L                /*Limited hours to answer*/
#define EXIT_NETMAIL    0x00008000L             /*Exit when netmail received*/
#define ZONE_ADAPTION   0x00010000L              /*Adopt called/calling zone*/
#define PRINTNEWMSGS    0x00020000L                /*Print received messages*/
#define NOUNKNOWNPOINTS 0x00040000L           /*Don't accept unlisted points*/
#define FORCEDCARRIER   0x00080000L       /*Carrier is forced. Ring required*/
#define NULLMODEM       0x00100000L          /*Initiate session when CD high*/
#define MAILER43LINES   0x00200000L          /*Use 43/50 line mode in mailer*/
#define SWAPTOEMSDISK   0x00400000L      /*Swap file to LIM/EMS/DISK w/shell*/
#define MAILEREMSOK     0x00800000L        /*If EMS can be used for swapping*/
#define PRESENTAKAS     0x01000000L      /*Present AKAs during EMSI sessions*/
#define NOBLINKMAIL     0x02000000L      /*Don't flash the waiting mail sign*/
#define EXITONANYFILE   0x04000000L      /*Exit after ANY data has been rcvd*/
#define MAILERAUTOLINES 0x08000000L      /*Use whatever screen mode is there*/
#define HONOR_RRQ       0x10000000L                              /*Honor RRQ*/
#define NO_TIMESTAMP    0x20000000L       /*Don't show timestamp in window-1*/
#define MAILERCUSTOMCRT 0x40000000L               /*User-defined screen size*/

    /* Miscellaneous flags (FLAGS2) */

#define FAXAUTORECEIVE  0x00000001L               /*Handle inbound fax calls*/
#define MAILERLOGDFRS   0x00000002L         /*Log data following RING signal*/
#define MEXTFREQS       0x00000004L            /*Use external file ReqServer*/
#define MEXTFREQNOSWAP  0x00000008L      /*Don't swap when calling ReqServer*/
#define FAXAUTOPRINT    0x00000010L       /*Auto print faxes (internal only)*/

    /* Audio flags */

#define CLOCKNOISE      0x0001                     /*Tic-tac-tic-tac-tic-tac*/
#define INMAILNOISE     0x0002                           /*Unpacked any mail*/
#define INCRASHNOISE    0x0004            /*Unpacked crash or immediate mail*/
#define INCONNECTNOISE  0x0008                /*Incoming MAIL call (connect)*/
#define INCALLERNOISE   0x0010           /*Incoming human caller passed >BBS*/
#define MAILWAITING     0x0020                             /*Mail is waiting*/
#define OUTMAILNOISE    0x0040                   /*Sent mail (after session)*/
#define OUTCONNECTNOISE 0x0080                /*Outgoing MAIL call (connect)*/
#define ERRORNOISE      0x0100                                      /*S.O.S.*/

    /* Request types */

#define REQALL          0x01                           /*Anybody can request*/
#define REQNONE         0x02                            /*No one can request*/
#define REQLISTED       0x04                           /*Only listed systems*/
#define REQLIMITED      0x08                                 /*Limited hours*/
#define REQONLYONEWCARD 0x40               /*Stop after one match (wildcard)*/
#define REQONLYONEMATCH 0x80               /*Stop after one match (explicit)*/


typedef struct
    {
    char                log[71];                              /*Log filename*/
    unsigned short int  loglevel;                            /*See Loglevels*/

    char                prefix[31];                           /*Always added*/
    char                hidden[10][31];       /*Strip these if they are in #*/
    char                postfix[31];                       /*Always appended*/

    long                flags;                     /*See Miscellaneous flags*/
    long                flags2;                    /*See Miscellaneous flags*/
    unsigned short int  audio;                             /*See Audio flags*/
    unsigned char       synchtimer;   /*Number of seconds before drop to BBS*/

    unsigned char       crashexit;                               /*Mail exit*/
    unsigned char       bbs300;                    /*Human caller    300 BPS*/
    unsigned char       bbs1200;                   /*Human caller   1200 BPS*/
    unsigned char       bbs1275;                   /*Human caller  12/75 BPS*/
    unsigned char       bbs2400;                   /*Human caller   2400 BPS*/
    unsigned char       bbs4800;                   /*Human caller   4800 BPS*/
    unsigned char       bbs9600;                   /*Human caller   9600 BPS*/
    unsigned char       bbs19200;                  /*Human caller  19200 BPS*/
    unsigned char       bbs38400;                  /*Human caller  38400 BPS*/

    unsigned short int  modembaud;                   /*30=300, 24=2400, etc.*/
    unsigned char       modemport;            /*1-255 (COM1=1, COM2=2, etc.)*/
    unsigned char       modemdelay;         /*1/10 seconds delay / line sent*/

    char                b300msg[16];                           /*CONNECT 300*/
    char                b1200msg[16];                         /*CONNECT 1200*/
    char                b1275msg[16];                         /*CONNECT 1275*/
    char                b2400msg[16];                         /*CONNECT 2400*/
    char                b4800msg[16];                         /*CONNECT 4800*/
    char                b9600msg[16];                         /*CONNECT 9600*/
    char                b19200msg[16];                       /*CONNECT 19200*/
    char                b38400msg[16];                       /*CONNECT 38400*/
    char                errormsg[16];                                /*ERROR*/
    char                busymsg[16];                                  /*BUSY*/
    char                carriermsg[16];                         /*NO CARRIER*/
    char                okmsg[16];                                      /*OK*/
    char                ringmsg[16];                                  /*RING*/
    char                nodialmsg[16];                         /*NO DIALTONE*/
    char                noanswmsg[16];                           /*NO ANSWER*/
    char                voicemsg[16];                                /*VOICE*/

    char                escapestr[11];              /*Return to command mode*/
    char                offhookstr[11];                          /*Busy line*/
    char                reconnectstr[11];              /*Return to data mode*/
    char                init1[50];                        /*Initialization-1*/
    char                init2[50];                        /*Initialization-2*/
    char                init3[50];                        /*Initialization-3*/
    char                resetstr[50];                  /*Force "OK" response*/
    char                downstr[50];                    /*Sent upon shutdown*/
    char                hangupstr[11];                             /*On hook*/
    char                dialstr[11];                                  /*Dial*/

    char                oldmodemanswer[11];                   /*!!Not used!!*/
    unsigned char       answerdelay;          /*1/10s delay before answering*/

    unsigned char       begin_hour;                           /*!!Not used!!*/
    unsigned char       begin_minute;                         /*!!Not used!!*/
    unsigned char       end_hour;                             /*!!Not used!!*/
    unsigned char       end_minute;                           /*!!Not used!!*/

    unsigned char       retrybusy;                /*Retry attempts when BUSY*/
    unsigned char       retryresend;              /*Retry attemps on failure*/
    unsigned char       retrydelay;                  /*Seconds between calls*/

    char                reqlist[71];         /*List to scan for reqable dirs*/
    char                reqalias[71];                      /*Magic filenames*/
    char                reqmessage[71]; /*Appended to FAILED REQUEST message*/
    unsigned char       reqtype;                                 /*See below*/
    unsigned char       reqmaxfiles;  /*Max number of files to send on 1 req*/
    unsigned short int  reqmaxtime;      /*Maximum number of minutes for req*/
    unsigned short int  reqmaxsize;           /*Maximum size (in KB) for req*/
    unsigned short int  OLDreqminbaud;                        /*!!Not used!!*/
    unsigned char       reqstarthr;   /*Start time for file requests, can be*/
    unsigned char       reqstartmin;     /*..combined with the reqdays field*/
    unsigned char       reqendhr;               /*End time for file requests*/
    unsigned char       reqendmin;
    unsigned char       reqdays;      /* 7 6 5 4 3 2 1         0x80=All days
                                         _ _ _ _ _ _ _
                                         : : : : : : :
                                         : : : : : : +--- Saturday
                                         : : : : : +----- Friday
                                         : : : : +------- Thursday
                                         : : : +--------- Wednesday
                                         : : +----------- Tuesday
                                         : +------------- Monday
                                         +--------------- Sunday            */

    char                bbsname[11];             /*Press <Esc> for "bbsname"*/
    char                beforebbsbanner[71];  /*File sent before drop to BBS*/

    struct                                            /*Mailer function keys*/
        {
        char            cmd[61];                        /*Program to execute*/
        char            title[26];               /*Title to appear on screen*/
        unsigned char   behavior;     /*0x01-Pause, 0x02-Rescan, 0x04-NoSwap*/
        }
    key[24];                                          /*F1-F12, Shift F1-F12*/

    unsigned char       color[11];                        /*Header
                                                            Highlight
                                                            Clock
                                                            Data entry
                                                            Error
                                                            Normal text
                                                            Frame
                                                            Window text
                                                            Window frame
                                                            Window select
                                                            Window highlight*/

    unsigned char       keep_history; /*Days to keep entries in mail history*/

    char                slavepwd[21];    /*FDServer password, empty=inactive*/

    char                ineventfile[71];  /*Displayed to users when no users*/
    char                mailonlyfile[71];/*Displayed to users when mail-only*/

    struct                                         /*External mail interface*/
        {
        char            wakeupstr[39];              /*String to trigger exit*/
        unsigned char   flags;                       /*0x01=Leave FOSSIL hot*/
        unsigned char   errorlevel;                /*Errorlevel to exit with*/
        }
    externmail[10];

    unsigned char       audio_begin_hour;   /*Start and end of audio allowed*/
    unsigned char       audio_begin_minute;   /*If all four fields are zero,*/
    unsigned char       audio_end_hour;    /*..audio is allowed at all times*/
    unsigned char       audio_end_minute;

    unsigned short int  min_undial_cost;           /*Minimum undialable cost*/

    unsigned char       CDMASK;                    /*Carrier detect mask (C)*/

    unsigned long       SETUPFDTimestamp;    /*When file was last written to*/

    char                reqseclist[71];      /*List to scan for reqable dirs*/
                                             /*...during SECURE sessions (C)*/

    unsigned char       bbs7200;                   /*Human caller   7200 BPS*/
    unsigned char       bbs12000;                  /*Human caller  12000 BPS*/
    unsigned char       bbs14400;                  /*Human caller  14400 BPS*/

    unsigned short int  SETcustomcrt_AX,        /*INT 10H register values to*/
                        SETcustomcrt_BX,        /*set CUSTOM screen mode (C)*/
                        SETcustomcrt_CX,
                        SETcustomcrt_DX;

    unsigned short int  RESETcustomcrt_AX,      /*INT 10H register values to*/
                        RESETcustomcrt_BX,    /*reset CUSTOM screen mode (C)*/
                        RESETcustomcrt_CX,
                        RESETcustomcrt_DX;

    char                reqsecalias[71];        /*Magic filenames for SECURE*/
                                                              /*sessions (C)*/

    char                modemanswer[41];          /*Instruct modem to answer*/

    char                b7200msg[16];                         /*CONNECT 7200*/
    char                b12000msg[16];                       /*CONNECT 12000*/
    char                b14400msg[16];                       /*CONNECT 14400*/
    char                b16800msg[16];                       /*CONNECT 16800*/
    char                bfaxmsg[16];                           /*CONNECT FAX*/
    unsigned char       bbs16800;                  /*Human caller  16800 BPS*/
    unsigned char       bbsfax;                        /*FAX call errorlevel*/

    char                InboundFaxPath[71];           /*Inbound fax path (C)*/

    char                FReqServer[71];       /*External file request server*/

    char                b57600msg[16];                       /*CONNECT 57600*/
    char                b64000msg[16];                       /*CONNECT 64000*/
    char                b115200msg[16];                     /*CONNECT 115200*/
    unsigned char       bbs57600;                  /*Human caller  57600 BPS*/
    unsigned char       bbs64000;                  /*Human caller  64000 BPS*/
    unsigned char       bbs115200;                 /*Human caller 115200 BPS*/

    char                ModemCustomMsg[16];     /*Custom CONNECT message (C)*/
    long                ModemCustomBaud;      /*Actual baudrate of above (C)*/
    unsigned char       bbsCustom;             /*Human caller Custom BPS (C)*/

    unsigned long       reqminbaud;      /*Minimum baudrate for file request*/

    char                MailExitSemaphore[71];   /*Touched/created when mail*/
                                                     /*has been received (C)*/
    char                FaxPrinter[71];         /*Program to invoke to print*/
                                                        /*a received fax (C)*/

    char                b21600msg[16];                       /*CONNECT 21600*/
    char                b24000msg[16];                       /*CONNECT 24000*/
    char                b26400msg[16];                       /*CONNECT 26400*/
    char                b28800msg[16];                       /*CONNECT 28800*/
    unsigned char       bbs21600;                   /*Human caller 21600 BPS*/
    unsigned char       bbs24000;                   /*Human caller 24000 BPS*/
    unsigned char       bbs26400;                   /*Human caller 26400 BPS*/
    unsigned char       bbs28800;                   /*Human caller 28800 BPS*/

    unsigned char       RingingCount;  /*Number of RINGING before dial abort*/

    char                b31200msg[16];                       /*CONNECT 31200*/
    char                b33600msg[16];                       /*CONNECT 33600*/
    unsigned char       bbs31200;                   /*Human caller 31200 BPS*/
    unsigned char       bbs33600;                   /*Human caller 33600 BPS*/

    char                RESERVERAT[266];                          /*RESERVED*/
    } 
    _mailer;

/*
**  Editor ------------------------------------------------------------------
*/

    /* Miscellaneous flags (FLAGS) */

#define EDITOR43LINES   0x00000001L                    /*Use 43/50-line mode*/
#define SHOWHARDCRS     0x00000002L       /*Display paragraph (hard CR) char*/
#define EDITORSWAP      0x00000004L        /*Swap to LIM/EMS/DISK when shell*/
#define EDITOREMSOK     0x00000008L                  /*EMS OK for swap image*/
#define EDITORAUTOLINES 0x00000010L      /*Use whatever screen mode is there*/
#define HONOR_CFM       0x00000020L                              /*Honor CFM*/
#define EDITORCUSTOMCRT 0x00000040L               /*User-defined screen size*/
#define RKILL_NEVER_E   0x00000080L         /*Never ask "Delete Original?"-E*/
#define RKILL_NEVER_L   0x00000100L         /*Never ask "Delete Original?"-L*/
#define RKILL_NEVER_N   0x00000200L         /*Never ask "Delete Original?"-N*/
#define USEZONEGATE_YES 0x00000400L             /*Always use zone gate if OK*/
#define USEZONEGATE_NO  0x00000800L        /*Never use zone gate, even if OK*/
#define NEWMSGSEM       0x00001000L   /*Semaphores when new msgs are created*/
#define FAXAUTOVIEW     0x00002000L       /*View fax documents automatically*/
#define FAXAUTOKILL     0x00004000L /*Remove fax document with cover message*/

    /* Netmail folder behavior */

#define RESTRICTED      0x00000001L                           /*See FOLDER.H*/
#define EXPORTOK        0x00000004L
#define USEXLATTABLES   0x00000008L
#define EDREADONLY      0x00000020L

typedef struct
    {
    char                macrokey[24][61];             /*F1-F12, Shift F1-F12*/

    unsigned char       margin;                /*Wordwrap margin, default=60*/

    unsigned short int  msgbits;            /*Default NetMail message status*/

    long                flags;                                   /*See above*/

    char                origin[20][61];                       /*Origin lines*/

    unsigned char       color[15];                     /*Top line
                                                         Status line
                                                         Error
                                                         Message text
                                                         Quoted message text
                                                         Reverse message text
                                                         Hard <CR>s
                                                         Message header
                                                         Message header data
                                                         Header data highlight
                                                         Input fields
                                                         Window frame
                                                         Window text
                                                         Window select
                                                         Window highlight*/

    long                netfolderflags;               /*NetMail folder flags*/

    unsigned char       translate_in[256];     /*Translation table (reading)*/
    unsigned char       translate_out[256];    /*Translation table (writing)*/

    char                qbase[71];       /*Path to Hudson Message Base (HMB)*/

    unsigned short int  SETcustomcrt_AX,        /*INT 10H register values to*/
                        SETcustomcrt_BX,        /*set CUSTOM screen mode (C)*/
                        SETcustomcrt_CX,
                        SETcustomcrt_DX;

    unsigned short int  RESETcustomcrt_AX,      /*INT 10H register values to*/
                        RESETcustomcrt_BX,    /*reset CUSTOM screen mode (C)*/
                        RESETcustomcrt_CX,
                        RESETcustomcrt_DX;

    char                FaxViewer[71];    /*Program invoked by <Ctrl-F3> (C)*/

    char                RESERVERAT[937];                          /*RESERVED*/
    } 
    _editor;

/*
**  Shared data -------------------------------------------------------------
*/

    /* Miscellaneous flags (FLAGS) */

#define FASTKEY         0x00000001L               /*Set/reset typematic rate*/
#define FLICKER         0x00000002L        /*Prevent flicker on CGA monitors*/
#define BLACKOUT        0x00000004L                  /*Screen blanker active*/
#define HAVEEXTKBD      0x00000008L    /*Use extended INT 16H keyboard calls*/
#define xxFORCE24HOUR   0x00000010L                               /*Not used*/
#define MONOMODE        0x00000020L            /*Monochrome mode for FDSETUP*/
#define DATEFMTUSA      0x00000040L                               /* m/dd/yy*/
#define DATEFMTEUR      0x00000080L                               /* d/mm/yy*/
#define DATEFMTMIL      0x00000100L                               /*yy-mm-dd*/
#define TIMEFMT12H      0x00000200L                               /*hh:mma/p*/
#define TIMEFMT24H      0x00000400L                                  /*hh:mm*/
#define INHIBITBLINK    0x00000800L   /*Inhibit blink attribute upon startup*/
#define RESETBLINK      0x00001000L               /*Reset blinking upon exit*/


    /* User flags */

#define SUPERUSER       0x00000001L
#define ADMINUSER       0x00000002L
#define USER            0x00000004L
#define BYPASSRO        0x00010000L           /*Bypass Read-only restriction*/
#define BYPASSEXP       0x00020000L              /*Bypass Export restriction*/

    /* Protection flags */

#define PROTECT_MEXIT      0x00000001L            /*Mailer Protect Alt-Q (C)*/
#define PROTECT_MSHELL     0x00000002L            /*Mailer Protect Alt-Z (C)*/
#define PROTECT_MKEYS      0x00000004L    /*Mailer Protect function keys (C)*/
#define PROTECT_MFREQ      0x00000008L    /*Mailer Protect file requests (C)*/
#define PROTECT_MXMIT      0x00000010L         /*Mailer Protect transmit (C)*/
#define PROTECT_MSEND      0x00000020L        /*Mailer Protect send mail (C)*/
#define PROTECT_MPOLL      0x00000040L             /*Mailer Protect poll (C)*/
#define PROTECT_MNCOMP     0x00000080L             /*Mailer Protect FDNC (C)*/
#define PROTECT_MQUEUE     0x00000100L       /*Mailer Protect mail queue (C)*/
#define PROTECT_MPRN       0x00000200L   /*Mailer Protect printer toggle (C)*/
#define PROTECT_MEDITOR    0x00000400L           /*Mailer Protect editor (C)*/
#define PROTECT_MSETUP     0x00000800L          /*Mailer Protect FDSETUP (C)*/
#define PROTECT_MUTILITIES 0x00001000L   /*Mailer Protect Utilities menu (C)*/
#define PROTECT_MTERMINAL  0x00002000L         /*Mailer Protect Terminal (C)*/

    /* FTN-Domain structure */

typedef struct
    {
    unsigned short int  zone,                             /* Zone for domain*/
                        RESERVED;
    char                name[28];                           /*Name of domain*/
    }
    _DOMAIN;

    /* AKA matching structure */

typedef struct
    {
    unsigned short int  zone,                                /*Zone to match*/
                        net,                                  /*Net to match*/
                        akanum;                  /*AKA to use, 0=Primary AKA*/
    }
    _ZMATCH;

typedef struct
    {
    char                systempath[71];                        /*SYSTEM path*/
    char                mailpath[71];                /*System NetMail folder*/
    char                swap_path[71];                 /*Path for swap files*/
    char                rescanpath[71];                /*Semaphore files (C)*/
    char                infilesecpath[71];    /*Rcvd files, SECURE sessions*/
    char                infilepath[71];                         /*Rcvd files*/
    char                packetpath[71];       /*Path for outbound .PKT files*/
    char                nodelistpath[71];       /*Path for nodelist database*/

    unsigned short int  countrycode;  /*Country code for tel.no. translation*/

    struct                                /*One primary address and ten AKAs*/
        {
        unsigned short int
                        zone,
                        net,
                        node,
                        point;
        }
    aka[11];

    long                flags;                                   /*See above*/

    unsigned char       blackout_time;     /*Screen blanker timer in seconds*/

    /* --- User record */

    struct                                                           /*Users*/
        {
        char            name[31];                                 /*Username*/
        short int       defAKA;                       /*Default AKA for user*/
        long            userID;           /*UserID for JAM lastread handling*/
        long            pwdcrc;        /*CRC-32 of user password, -1L No pwd*/
        unsigned long   flags;                       /*User flags, see above*/
        }
    user[10];

    unsigned long       exitpwdcrc;/*Password for DOS shell, exits, etc. (C)*/
    unsigned long       exitflags;     /*Which flags should be protected (C)*/

    _ZMATCH             zmatch[20];                     /*AKA matching table*/

    _DOMAIN             domain[20];                       /*FTN domain table*/

    struct                                                /*Site information*/
        {                       
        char            name[50];                           /*Name of system*/
        char            location[40];                   /*Location of system*/
        char            phone[26];         /*Phone number or '-Unpublished-'*/
        unsigned long   baud;                       /*Baud rate (300-115200)*/
        char            flags[50];              /*Capability flags of system*/
        }
    siteinfo;

    short int           FaxRecipient;          /*Fax recipient (0-based) (C)*/

    char                RESERVED[80];                             /*RESERVED*/
    unsigned long       CurrPrgRevLev;                          /*CURRPRGREV*/
    } 
    _shared;

/*
**    Terminal ----------------------------------------------------------------
*/

    /* Miscellaneous flags (FLAGS) */

#define CONNECT_NOISE   0x00000001L                  /*Make noise on connect*/
#define TRANSFER_NOISE  0x00000002L              /*Make noise after transfer*/
#define USE_TRANSLATE   0x00000004L                 /*Use translation tables*/
#define USE_43LINES     0x00000008L                    /*Use 43/50-line mode*/
#define AUTOZMODEM      0x00000010L            /*Allow auto-Zmodem downloads*/
#define NOWRAPAROUND    0x00000020L             /*Disable automatic wrapping*/
#define LOCALECHO       0x00000040L                /*Echo local input to CRT*/
#define NOAVATAR        0x00000080L              /*Disable AVATAR/0+ support*/
#define USE_AUTOLINES   0x00000100L     /*Use whatever screen mode is active*/
#define NOCLRONFF       0x00000200L       /*Don't clear screen on form feeds*/
#define IEMSIACTIVE     0x00000400L               /*Interactive EMSI Support*/
#define TERMCUSTOMCRT   0x00000800L           /*User-defined screen size (C)*/
#define TERMLOG         0x00001000L                /*Logging in Terminal (C)*/
#define TERMAUTODWMODE  0x00002000L                      /*Auto Doorway Mode*/
#define TERMTIMERON     0x00004000L   /*Inactivity timer to return to Mailer*/
#define TERMBSIBS       0x00008000L    /*Backspace sends BS (instead of DEL)*/

typedef struct
    {
    char                initstring[41];               /*Modem initialization*/

    unsigned short int  scrollsize;  /*Maximum memory (kb) for scroll buffer*/

    unsigned char       emulation;          /*0=TTY, 1=ANSI, 2=VT52, 3=VT100*/

    unsigned char       protocol;           /*Default file transfer protocol*/

    char                shiftkey[12][31];                     /*Shift F1-F12*/
    char                ctrlkey[12][31];                       /*Ctrl F1-F12*/

    char                downloadpath[60];            /*Default download path*/
    char                uploadpath[60];                /*Default upload path*/

    unsigned char       translate_in[256];       /*Inbound translation table*/
    unsigned char       translate_out[256];     /*Outbound translation table*/

    unsigned char       retrywait;        /*Seconds to wait before next dial*/

    unsigned long       flags;                                   /*See above*/

    unsigned long       directorypwd;/*CRC-32 of password to enter phone dir*/

    char                editor[60];           /*Program to invoke on <Alt-I>*/

    char                profile[94];       /*Default profile, see TERMINAL.H*/

    unsigned short int  SETcustomcrt_AX,        /*INT 10H register values to*/
                        SETcustomcrt_BX,        /*set CUSTOM screen mode (C)*/
                        SETcustomcrt_CX,
                        SETcustomcrt_DX;

    unsigned short int  RESETcustomcrt_AX,      /*INT 10H register values to*/
                        RESETcustomcrt_BX,    /*reset CUSTOM screen mode (C)*/
                        RESETcustomcrt_CX,
                        RESETcustomcrt_DX;

    unsigned long       MailerTimer;      /*Inactivity timer (1/10s) setting*/

    char                RESERVERAT[633];                          /*RESERVED*/
    unsigned char       VT100Colors[8];              /*VT-100 CRT attributes*/
    char                InitAddon[19];         /*Add-on modem initialization*/
    char                NewBBSName[30];               /*Name of BBS software*/

    struct                                                  /*Some more AKAs*/
        {
        unsigned short int
                        zone,
                        net,
                        node,
                        point;
        }
    aka[20];
    } 
    _terminal;

/*
**    Printer (C) -------------------------------------------------------------
*/

    /* Miscellaneous flags (BEHAVIOR) */

#define PAGE_FORMAT     0x00000001L
#define PAGE_FFEED      0x00000002L
#define EJECT           0x00000004L
#define MANUAL_PAPER    0x00000008L
#define HIDE_KLUDGE     0x00000010L
#define CONTINOUS       0x00000020L

typedef struct
    {
    char                port;       /*0=LPT1, 1=LPT2, 2=LPT3, 3=COM1, 4=COM2*/
    char                baud;
                          /*0=38400, 1=19200, 2=9600, 3=4800, 4=2400, 5=1200*/
    unsigned char       stopbits;                                 /*0=1, 1=2*/
    unsigned char       wordlength;                               /*0=7, 1=8*/
    unsigned char       parity;                      /*0=Even, 1=Odd, 2=None*/
    unsigned char       pagelen;               /*Number of lines on one page*/
    long                behavior;                                /*See above*/
    char                init[71];                           /*Initialization*/
    char                reset[71];                        /*Deinitialization*/
    char                bold_on[31];
    char                bold_off[31];
    char                under_on[31];
    char                under_off[31];
    char                italic_on[31];
    char                italic_off[31];
    unsigned char       pagewidth;              /*Width in columns of a page*/
    unsigned char       leftmargin;                            /*Left margin*/
    unsigned char       footer;          /*Footer margin, ie. leave nn lines*/
    unsigned char       header;           /*Header margin, ie. skip nn lines*/
    unsigned char       translate_out[256];              /*Translation table*/
    char                RESERVED[100];                            /*RESERVED*/
    } 
    _printer;


/*
**    SETUP.FD ----------------------------------------------------------------
**
** Don't use ANY of the data in the file if the CRC values are NOT correct.
**
** Check the fingerprint[] field before assuming 1.99b and higher. Your
** software should also check the sysrev field to make sure it's compatible
** with the data listed in SETUP.FD
*/

    /* Current system file revision level */

#define FD_THISREV      0x0100

struct _ctl
    {
    char                fingerprint[5];           /*Must contain "JoHo<NUL>"*/
    unsigned short int  sysrev;                 /*Must contain THISREV above*/
    unsigned long       ctlcrc;      /*CRC-32 of _ctl excluding 1st 11 bytes*/
    _mailer             m;
    _editor             e;
    _shared             s;
    _terminal           t;
    _printer            p;
    unsigned long       ctlcrc2;                   /*CRC-32 of all the above*/
    };

#pragma pack()

#ifdef __cplusplus
}
#endif
#endif

/* end of file "ctl.h" */
