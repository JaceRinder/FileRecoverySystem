#ifndef FILERECOVERY_H
#define FILERECOVERY_H
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<math.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<stdbool.h>
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#define FLOPPYSIZE 1474560
#define SECTORSIZE 512
#define ROOT 0x2600
#define ROOTSECTORS 13
#define ENTRYLEN 32
#define DELETEDFLAG -27
#define NAMELOCATION 8
#define NAMELENGTH 9
#define EXTLOCATION 12
#define CLUSTERSIZE 12
#define ATTRILOCATION 11
#define CLUSTERLOCATION 26
#define SIZELOCATION 28
#define SIZEBYTES 4
#define MOVECLUSTER 31
#define PATHSIZE 14
#define SPACES 0x20
#define CLUSTERINUSE 0x002
#define RESERVED 0xFF0
#define LASTCLUSTER 0xFF8
#define FREECLUSTER 0x000
#define EVEN 0x000FFF
#define ODD 0xFFF000
#define NIBBLESIZE 3
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
// Global Variables
char*  pic;
char* out;
int totalFiles = 0; //keep track of files for naming
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void rootContents(unsigned long, char *);
void createPath(char* name, char* path, char* filePath);
void findCluster(uint16_t* current, int deleted, uint32_t* filesize, char* filename);
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#endif
