#include "AM.h"
#include "bf.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"

int AM_errno = AME_OK;

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
	}

void remove_datadbs(){
	  int removed;
	  char filename[] = "EMP-AGE";
		char filename1[] = "EMP-DNAME";
		char filename2[] = "EMP-FAULT";
		char filename3[] = "EMP-NAME";
		char filename4[] = "EMP-SAL";
	  removed = remove(filename);
	  removed = remove(filename1);
		removed = remove(filename2);
		removed = remove(filename3);
		removed = remove(filename4);
		/*
		if(removed == 0) {
      printf("File deleted successfully\n");
   } else {
      printf("Error: unable to delete the file\n");
   }
		*/
	}

typedef struct {
	int fileDesc;
	int blocknum;
	int recordnum;
	int isInitialized;        //shows if it is initialized with file
} scan_file;

typedef struct {
	char *fileName;
	char attrType1;
	int attrLength1;
	char attrType2;
	int attrLength2;
	int isInitialized;       //shows if it is initialized with file
} open_file;

scan_file* scan_arr;
open_file* open_arr;

int find_open(){  //finds a spot to put next element in open_arr
	for(int i =0;i<20;i++){
		if (open_arr[i].isInitialized != 0) {
			return i;
		}
	}
	return -1;
}

int find_scan(){  //finds a spot to put next element in scan_arr
	for(int i =0;i<20;i++){
		if (scan_arr[i].isInitialized != 0) {
			return i;
		}
	}
	return -1;
}

void AM_Init() {
	remove_datadbs();
	BF_Init(LRU);
	scan_arr = (scan_file*)malloc(20 * sizeof(scan_file));
	for (int i = 0; i < 20; i++) {
		scan_arr[i].isInitialized = 0;   //this means that cells are not initialized with file
	}
	open_arr = (open_file*)malloc(20 * sizeof(open_file));
	for (int i = 0; i < 20; i++) {
		open_arr[i].isInitialized = 0;  //this means that cells are not initialized with file
	}
	//return;
}



int AM_CreateIndex(char *fileName,
	               char attrType1,
	               int attrLength1,
	               char attrType2,
	               int attrLength2) {
	 int fd;
   BF_Block *block;
   char* data;
   CALL_OR_DIE(BF_CreateFile(fileName)); //create
   BF_Block_Init(&block);
   CALL_OR_DIE(BF_OpenFile(fileName, &fd));
   CALL_OR_DIE(BF_AllocateBlock(fd, block));
   data = BF_Block_GetData(block);
   int root_num = 1;
   memcpy(data, "AM", 3);
   char* new_str = (char*)calloc(21,sizeof(char));
   sprintf(new_str,";%c;%d;%c;%d;%d;",attrType1,attrLength1,attrType2,attrLength2,root_num);
   strncat(data,new_str,strlen(data)+22);
   BF_Block_SetDirty(block);
   CALL_OR_DIE(BF_UnpinBlock(block));
   BF_CloseFile(fd);  //recheck
   return AME_OK;
}


int AM_DestroyIndex(char *fileName) {
	int i;
	for (i = 0; i < 20; i++) {

	}
  return AME_OK;
}


int AM_OpenIndex (char *fileName) {
	BF_Block *block;
  BF_Block_Init(&block);
  char* data;
	int fileDesc;
	CALL_OR_DIE(BF_OpenFile(fileName, &fileDesc));
	CALL_OR_DIE(BF_GetBlock(fileDesc, 0, block));
  data = BF_Block_GetData(block);
	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
	char str[2];
	memcpy(str, data, 2);    //read first 3 bytes from first block and pass them to str
	if (str != "AM")       //check if a AM file
		return AME_EOF;      //error!!! mporei lathos (tha prepei na epistrefei kati arnitiko gia na vrei i main oti iparxei lathos px grammi 120 am_main1)
	int pos = find_open();
	open_arr[pos].isInitialized = 1;
	open_arr[pos].fileName = fileName;
	//read first block to get remaining struct members

	return pos;
}


int AM_CloseIndex (int fileDesc) {
  return AME_OK;
}


int AM_InsertEntry(int fileDesc, void *value1, void *value2) {
  return AME_OK;
}


int AM_OpenIndexScan(int fileDesc, int op, void *value) {
  return AME_OK;
}


void *AM_FindNextEntry(int scanDesc) {

}


int AM_CloseIndexScan(int scanDesc) {
  return AME_OK;
}


void AM_PrintError(char *errString) {

}

void AM_Close() {

}
