#include "AM.h"
#include "bf.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"


int AM_errno = AME_OK;

#define CALL_OR_DIE(call)     \
{                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
        BF_PrintError(code);    \
        exit(code);             \
    }                         \
}
//removes created Files automatically
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
    int fileDesc;
} open_file;

scan_file* scan_arr;
open_file* open_arr;


//finds a spot to put next element in open_arr
int find_open(){
    for(int i =0;i<20;i++){
        if (open_arr[i].isInitialized == 0) {
            return i;
        }
    }
    return -1;
}


//finds a spot to put next element in scan_arr
int find_scan(){
    for(int i =0;i<20;i++){
        if (scan_arr[i].isInitialized == 0) {
            return i;
        }
    }
    return -1;
}


//finds position of already opened file with filedesc (can be used to extract struct members such as attribute lenght)
int search_open_arr(int fileDesc)
{
    for(int i = 0; i < 20; i++){
        if (open_arr[i].isInitialized == 1 && open_arr[i].fileDesc == fileDesc)
        {
            return i;
        }
    }
    return -1;
}


//initializes struct members in open_arr
void put_to_open(int i,int pos,char* token){
    switch (i) {
        case 0:
            open_arr[pos].attrType1 = token[0];
            break;
        case 1:
            open_arr[pos].attrLength1 = atoi(token);
            break;
        case 2:
            open_arr[pos].attrType2 = token[0];
            break;
        case 3:
            open_arr[pos].attrLength2 = atoi(token);
            break;
        default:
            break;
    }
}


//initializes struct members of open_arr[pos]
void get_open_arr(int pos,char* data,char* fileName, int fileDesc){
    open_arr[pos].isInitialized = 1;
    open_arr[pos].fileName = fileName;
    open_arr[pos].fileDesc = fileDesc;
    const char s[2] = ";";
    char *token;
    token = strtok(data, s);
    int i=0;
    while( token != NULL ) {
        token = strtok(NULL, s);
        put_to_open(i,pos,token);
        i++;
        if(i == 4){
            break;
        }
    }
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
}

int AM_CreateIndex(char *fileName,
    char attrType1,
    int attrLength1,
    char attrType2,
    int attrLength2) {

        int fd;
        BF_Block *block;
        BF_Block_Init(&block);
        char* data;
        CALL_OR_DIE(BF_CreateFile(fileName));
        CALL_OR_DIE(BF_OpenFile(fileName, &fd));
        CALL_OR_DIE(BF_AllocateBlock(fd, block));
        data = BF_Block_GetData(block);
        int root_num = 1;     //initially root is in block number 1
        memcpy(data, "AM", 3);    //to recognise if am file
        char* new_str = (char*)calloc(21,sizeof(char));
        sprintf(new_str,";%c;%d;%c;%d;%d;",attrType1,attrLength1,attrType2,attrLength2,root_num);
        strncat(data,new_str,strlen(data)+22);
        BF_Block_SetDirty(block);
        CALL_OR_DIE(BF_UnpinBlock(block));
        BF_CloseFile(fd);
        return AME_OK;
    }


int AM_DestroyIndex(char *fileName) {
    int i;
    for (i = 0; i < 20; i++) {
        if(open_arr[i].isInitialized == 1 && (strcmp(open_arr[i].fileName,fileName))==0){
            return AME_EOF;
        }
    }
    remove(fileName);
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

    int pos = find_open();
    get_open_arr(pos,data,fileName,fileDesc);
    CALL_OR_DIE(BF_UnpinBlock(block));

    return pos;
}


int AM_CloseIndex (int fileDesc) {
    int i;
    for (i = 0; i < 20; i++) {
        if(scan_arr[i].isInitialized == 1 && scan_arr[i].fileDesc==fileDesc)
        return AME_EOF;
    }
    for (i = 0; i < 20; i++) {
        if (open_arr[i].isInitialized == 1 && open_arr[i].fileDesc==fileDesc) {
            open_arr[i].isInitialized = 0;
        }
    }
    BF_CloseFile(fileDesc);
    return AME_OK;
}

//Insert for the first time
void Insert_First(int fileDesc, void *value1, void *value2){
    BF_Block* block1;
    BF_Block* block2;
    BF_Block_Init(&block1);
    BF_Block_Init(&block2);
    char* data1;
    char* data2;
    int pos = search_open_arr(fileDesc);

    CALL_OR_DIE(BF_AllocateBlock(fileDesc, block1)); //root block
    data1 = BF_Block_GetData(block1);
    data1[0] = 'e'; //eurethriou
    data1[1] = '0'; //null value for pointer that points to index block with values less than root
    data1[2] = '2'; //to root = 1, to 2 einai to prwto mplok dedomenwn, ara deixnoume sto prwto mplok dedomenwn
    data1[3] = '\0';
    data1 += 3;
    memcpy(data1, value1, open_arr[pos].attrLength1);

    data1 += open_arr[pos].attrLength1;

    BF_Block_SetDirty(block1);
    CALL_OR_DIE(BF_UnpinBlock(block1));

    //insert record in data block
    CALL_OR_DIE(BF_AllocateBlock(fileDesc, block2));  //block of root's record
    data2 = BF_Block_GetData(block2);
    data1 = BF_Block_GetData(block1); // get data pointer in the beginning
    data2[0] = 'd'; //data block
    data2[1] = '0'; //number of currents keys in block
    data2[2] = '0'; //pointer to next leaf(initialized with 0 = null)
    data2[3] = '\0';
    data2 += 3;
    memcpy(data2,value2,sizeof(value2));

    BF_Block_SetDirty(block2);
    CALL_OR_DIE(BF_UnpinBlock(block2));
}


//searches tree untill leafs to find where the record should be placed
void Search(int fileDesc, void* value1, void *value2, int totalnumberofkeys){

}


int AM_InsertEntry(int fileDesc, void *value1, void *value2) {
    int blocks_num;
    int totalnumberofkeys;
    int totalnumberofkeysLeafs;
    BF_GetBlockCounter(fileDesc, &blocks_num);

    int pos = search_open_arr(fileDesc);
    //BF_BLOCK_SIZE -8 because 4 bytes for extra pointer + 4 bytes for ids (e02\0)
    totalnumberofkeys = (BF_BLOCK_SIZE - 8) / (open_arr[pos].attrLength1 + 4);
    totalnumberofkeysLeafs = (BF_BLOCK_SIZE - 4) / (open_arr[pos].attrLength1 + 4);
    if (blocks_num == 1)      //if the only block is metadata and we dont have a root
    {
        Insert_First(fileDesc,value1,value2);
        return AME_OK;
    }
    else
    {
        Search(fileDesc, value1, value2, totalnumberofkeys);
    }

    return AME_OK;
}


int AM_OpenIndexScan(int fileDesc, int op, void *value) {
    int pos = find_scan();
    scan_arr[pos].isInitialized = 1;
    scan_arr[pos].fileDesc = fileDesc;

    return pos;
}


void *AM_FindNextEntry(int scanDesc) {

}


int AM_CloseIndexScan(int scanDesc) {
    scan_arr[scanDesc].isInitialized = 0;
    return AME_OK;
}


void AM_PrintError(char *errString) {
     fprintf(stderr, errString, AM_errno);
}


void AM_Close() {
    free(open_arr);
    free(scan_arr);
}
