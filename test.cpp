/***************************************************************************/
/*                                                                         */
/* test.CPP, a sample file for use with the FrontDoor Nodelist Code        */
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
#include "ctl.h"        // FrontDoor SETUP.FD structure
#include <stdio.h>
#include <conio.h>


// Prototypes

unsigned short getZone(char *string);
unsigned short getNet(char *string);
unsigned short getNode(char *string);
unsigned short getPoint(char *string);
void           PrintBanner();
void           main();
void           ReadFD(void);

char NAME[]="FDNode Test";
char VERSION[]="4.27";
char FDNodelistDir[72]="";
char FDSemaphoreDir[72]="";
unsigned short Country=0;
FrontDoorNode NodeList;    // Global nodelist class


void main()
{
  char teststring[100];
  FDNFind test;
  int quit=0, count=0;
  unsigned short cost;

  char menu;

  // Get the FrontDoor config
  ReadFD();

  //strcpy(FDNodelistDir, "C:\\GOLDDEV\\FDINDEX\\TEST2");

  // We will need to set the path in the class, and thaw() it.
  NodeList.SetNLDir(FDNodelistDir);
  NodeList.SetSemDir(FDSemaphoreDir);

  if(!NodeList.Thaw()){
    PrintBanner();
    printf("\nError opening nodelist indices.");
    printf("\nEither run this program in the nodelist directory, the FD system\nDirectory, or correctly set the FD environment variable.\n\n");
    exit(1);
  }
  printf("\nNodelist in path %s\n", FDNodelistDir);
  if(!NodeList.GetNLDBRevision()){
    printf("\nOld format database. We must set country code.");    
    NodeList.SetCountry(Country); // needed for dial translation
  } else {
    printf("\nNew format database. Country Code is %u\n", NodeList.GetCountryCode());
  }

  while(!quit){
    printf("\n\n");
    PrintBanner();
    if(NodeList.IsFrozen()) printf("\n!! NODELIST INDEX IS FROZEN !!\n");
    printf("\n\n1. Find Node\n2. Find User\n3. List Zones\n4. List Nets\n5. List Nodes\n6. List Points\n7. Test AutoFreezeThaw()\n8. Quit\nEnter your choice");
    menu = (char) getch();
    printf("\n");

    switch(menu-'0'){

      case 1 : // Search for a specific node
        printf("\nEnter node to find ([RETURN] quits).\n");
        gets(teststring);
        // Fetch data
        if(strlen(teststring)){
          if(!NodeList.Find(test, getZone(teststring), getNet(teststring), getNode(teststring), getPoint(teststring))){
            printf("\n\n[NodeFind] Sysop %s\n[NodeFind] System Name %s\n", test.GetSysop(), test.GetSysName());
            cost=test.GetPhoneData(teststring);
            printf("[NodeFind] Cost %5u, Dial %s\n", cost, teststring);
            if(test.IsFDA()) printf("[NodeFind] Is Private DataBase entry\n");
          } else {
            printf("\n\n[NodeFind] Unable to find node in nodelist database\n");
          }

        }
        break;

                case 2 : // Wildcard username
        printf("\nWildcard username search. Enter last name first, use the character _\nto represent '*' type wildcard.");
        printf("\nEnter a name to find ([RETURN] quits).\n");
        gets(teststring);
        if(strlen(teststring)){
          count=0;
          NodeList.Find(test, teststring);
          while(test){
            printf("\n[UserFind] %s;%s", test.GetSysop(), test.GetSysName());
            printf("\n[UserFind]   At address %u:%u/%u.%u", test.GetZone(), test.GetNet(), test.GetNode(), test.GetPoint());
            ++test;
            count++;
          }
          printf("\n\n[UserFind] %u matches", count);
        }
        break;

      case 3 : // Get Zones
        NodeList.GetZones(test);
        while(test){
          printf("\n[ZoneList] %-5u; %s",test.GetZone(), test.GetSysName());
          ++test;
        }
        break;

      case 4 : // Get Nets
        printf("\nEnter Zone to find nets in\n");
        gets(teststring);
        NodeList.GetNets(test, getZone(teststring));
        while(test){
          printf("\n[NetList] %-5u; %s",test.GetNet(), test.GetSysName());
          ++test;
        }
        break;

      case 5 : // Get Nodes
        printf("\nEnter net to find nodes in (Zone:Net)\n");
        gets(teststring);
        NodeList.GetNodes(test, getZone(teststring), getNet(teststring));
        while(test){
          printf("\n[NodeList] %-5u; Route via %u:%u/%u; %s",test.GetNode(), getZone(teststring), test.GetRNet(), test.GetRNode(), test.GetSysop());
          ++test;
        }
        break;

      case 6 : // Get Points
        printf("\nEnter node to find points of (Zone:Net/Node)\n");
        gets(teststring);
        NodeList.GetPoints(test, getZone(teststring), getNet(teststring), getNode(teststring));
        while(test){
          printf("\[PointList] %-5u; %s\n",test.GetPoint(), test.GetSysop());
          ++test;
        }
        break;

      case 7 :
        switch(NodeList.AutoFreezeThaw()){
          case 0 : printf("\nNo change - Index closed\n"); break;
          case 1 : printf("\nNo change - Index open\n"); break;
          case 2 : printf("\nClass has just been frozen\n"); break;
          case 3 : printf("\nClass has just been thawed\n"); break;
          case 4 : printf("\nClass just failed Thaw!!\n"); break;
        }
        break;
      case 8 :
        quit=1;
        break;
    }
    if(!quit){
      printf("\nPress any key...\n");
      getch();
    }
  }
}

unsigned short int getZone(char *string)
{
  return((unsigned short) atoi(string));
}

unsigned short int getNet(char *string)
{
  return((unsigned short) atoi(strpbrk(string, ":")+1));
}

unsigned short int getNode(char *string)
{
  return((unsigned short) atoi(strpbrk(string, "/")+1));
}

unsigned short int getPoint(char *string)
{
  char * pos = strpbrk(string, ".");
  if(!pos) return(0); else return((unsigned short) atoi(pos+1));
}

void PrintBanner()
{
  printf("\nFrontDoor (TM) Nodelist Index Tests, Version %s, Colin Turner, 2:443/13.0\nCompiled at %s on %s\n", VERSION, __TIME__, __DATE__);
}

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
