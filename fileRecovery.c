//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#include "fileRecovery.h"
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
//function that traverses through the FAT, gathers information about the file
void rootContents(unsigned long start, char* path) {

    //variables to hold all the info needed to recover the file
    char filename[9];
    char extension[4];
    uint8_t attribute;
    uint16_t firstCluster;
    uint32_t fileSize;
    int deleted = 0; //flag for if file has been deleted: 1 = true, 0 = false


    for(int i = 0; i<ROOTSECTORS; i++){
      //update the start
      start = start + SECTORSIZE*i;

      for (int j = 0; j < SECTORSIZE; j += ENTRYLEN) {

        //reset deleted to 0
        deleted = 0;

        //find the block of the fat
        unsigned long block = start + j;

        //conditional to exit recursion
        if (pic[block] == 0) {
          return;
        }

          if(pic[block] == DELETEDFLAG){ //if the file is deleted
            filename[0]='_'; //file name is _
            deleted=1;
          }
          else{
            filename[0] = pic[block];
          }

          for(int i = 1; i < NAMELOCATION; i++){//loop to put each character in the array
            filename[i] = pic[block+i];
          }

          //assign the extension
          int pos = 0;
          for(int i=NAMELOCATION; i<EXTLOCATION; i++){
            if(pic[block+i] != SPACES){
              extension[pos]=pic[block+i];
            }
            pos++;
          }

          //store the attribute bytes
          attribute = (uint8_t)pic[block+ATTRILOCATION];

          //add the null character to the end of the filename
          filename[8] = '\0';

          //add padding for any spaces in the name
          for (int c = 0; c < NAMELENGTH; c++) {
            if (filename[c] == SPACES) {
              filename[c] = '\0';
              break;
            }
          }

          //add null character to end of the extension string
          extension[3] = '\0';

          //store the first cluster
          firstCluster = (uint16_t)pic[block + CLUSTERLOCATION];

          //store file size
          memcpy(&fileSize,&pic[block+SIZELOCATION],SIZEBYTES);

          //determine if it is a subdirectory
          if(attribute & (1<<4)){ //is a sub
            //Code to handle subs
            if(*filename != '.'){ //no empty directories

              //string to hold the entire path
              char filePath[strlen(path)+PATHSIZE];

              createPath(filename, path, filePath);

              //move to the nxt cluster
              uint16_t location = (firstCluster + MOVECLUSTER) * SECTORSIZE;

              //keep traversing the directory
              rootContents(location,filePath);
            }
          }
          else{ //is not a sub
            //format and print the file names etc.
                if(deleted == 1){
                  fprintf(stdout,"FILE\tDELETED\t%s%s.%s\t%d\n", path,filename, extension, fileSize);
                }
                else if(deleted == 0){
                  fprintf(stdout,"FILE\tNORMAL\t%s%s.%s\t%d\n",path, filename, extension, fileSize);
                }

              //string to hold the file name
              char outpath[256];

              //string to hold the sumber of files
              char filenum[2];

              //name the file according to the number of files
              sprintf(filenum,"%d",totalFiles++);
              strcpy(outpath, out);
              strcat(outpath, "/");
              strcat(outpath, "file");
              strcat(outpath, filenum);
              strcat(outpath, ".");
              strcat(outpath, extension);

              //call function to locate the next cluster and begin writing out files
              findCluster(&firstCluster, deleted, &fileSize, outpath);


          }
      }
  }
}
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
//function used to locate the next cluster and write out the contents of each file
void findCluster(uint16_t* current, int deleted, uint32_t* filesize, char* filename) {
  //variable declaration and initializing the output file
  uint32_t temp;
  int outfile = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

  //iterate through clusters
  while(*current >= CLUSTERINUSE && (*current <= RESERVED || *current >= LASTCLUSTER)){
    //if file is deleted, increment to the next cluster
    //might need to move this down

    //if filesize exceeds 512, set it to 512
    if (*filesize >= SECTORSIZE){
      temp = SECTORSIZE;
    }
    //otherwise, leave it as filesize
    else{
      temp = *filesize;
    }

    //write the information to the file
    write(outfile, &pic[(*current + MOVECLUSTER)*SECTORSIZE], temp);
    if(deleted) {
      *current += 1;
    }

    //update the file size
    *filesize -= temp;

    //Variable declarations
    unsigned long block = SECTORSIZE + (*current/2)*3;
    unsigned long nibble = 0;
    uint16_t next = 0;

    //store the nibble
    memcpy(&nibble, &pic[block], NIBBLESIZE);


    //decode
    if (*current % 2 == 0) {
      next = (EVEN & nibble);
    }
    else {
      next = ((ODD & nibble) >> CLUSTERSIZE);
    }

    //update the cluster
    *current = next;

    //if the file is deleted and the next cluster is non-zero, return and truncate the file
    if (deleted && next != FREECLUSTER) {
      *current = FREECLUSTER;
      // ftruncate(outfile,temp);
      return;
    }
  }
  //close the file
  close(outfile);
}
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
//helper function to create the whole path
void createPath(char* name, char* path, char* filePath){

  //update the file path
  strcpy(filePath, path);

  //add the filename to the path

  strcat(filePath,name);

  //add the next
  strcat(filePath,"/");
}
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
int main (int argc, char *argv[]){

  //check that the correct number of argument were passed in
  if(argc != 3){
    fprintf(stderr,"Not enough arguments");
    exit(0);
  }

  //open the file
  int file = open(argv[1], O_RDWR);

  //check that the file was able to be opened
  if(file < 0){
    fprintf(stderr, "File was unable to be opened");
    exit(1);
  }

  //store the FAT
  pic = mmap(0, FLOPPYSIZE, PROT_READ, MAP_PRIVATE | MAP_FILE, file, 0);

  //get the file where the output should be written to
  out = argv[2];

  rootContents(ROOT, "/");
  return 0;
}
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
