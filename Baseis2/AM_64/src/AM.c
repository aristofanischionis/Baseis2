#include "AM.h"
#include "bf.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

int AM_errno = AME_OK;

typedef struct {
	int fileDesc;
	int blocknum;
	int recordnum;
} scan_file;

typedef struct {
	char *fileName;
	char attrType1;
	int attrLength1;
	char attrType2;
	int attrLength2;
} open_file;

scan_file *scan_arr;
open_file* open_arr;


void AM_Init() {
	scan_arr = (scan_file*)calloc(20,sizeof(scan_file));
	open_arr = (open_file*)calloc(20,sizeof(open_file));
	return;
}


int AM_CreateIndex(char *fileName,
	               char attrType1,
	               int attrLength1,
	               char attrType2,
	               int attrLength2) {
	 int fd;
   BF_Block *block;
   char* data;
   BF_CreateFile(fileName); //create
   BF_Block_Init(&block);
   BF_OpenFile(fileName, &fd);
   BF_AllocateBlock(fd, block);
   data = BF_Block_GetData(block);
   int root_num = 1;
   memcpy(data, "AM", 3);
   char* new_str = (char*)calloc(19,sizeof(char));
   sprintf(new_str,";%c;%d;%c;%d;%d;",attrType1,attrLength1,attrType2,attrLength2,root_num);
   strncat(*data,new_str,strlen(*data)+20);
   BF_Block_SetDirty(block);
   BF_UnpinBlock(block);
   BF_CloseFile(fd);
   return AME_OK;
}


int AM_DestroyIndex(char *fileName) {
	int i;
	for (i = 0; i < 20; i++) {
		
	}
  return AME_OK;
}


int AM_OpenIndex (char *fileName) {
  return AME_OK;
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
