
Assignment 1 - Storage Manager
Submitted by Group No. 19 (kkaushik@hawk.iit.edu , pmankani@hawk.iit.edu , sguhatha@hawk.iit.edu)

***********************************************************
The assignment consists of three C files and three header files , a Makefile and a README: 
1. storage_mgr.c
2. storage_mgr.h
3. test_assign1.c
4. test_helper.h
5. dberror.c
6. dberror.h
7. Makefile
8. README

***********************Makefile***************************
Files on the fourier server are present in the path: 
/home/class/fall-15/cs525/kkaushik/Assignment1/

The files are also checked in the Bitbucket repository: (https://kedarkaushikkar@bitbucket.org/bglavic/cs525-f15-kkaushik.git)

In order to run the files, you can execute any one of the below commands:
1. make 
2. make all

The above commands create an output file "test_assign1". This can be then executed using "./test_assign1"

To clean the output files created use the command:
make clean

***************************storage_mgr.c**************************************
The interface declared in storage_mgr.h is defined in storage_mgr.c. The implementation of each of the functions in storage_mgr.c is defined as follows:

All the interface functions check if the storage manager is initialized and check if the file exists using the below functions:
int checkinit (void): Checks the value of the global variable "init"
int exists(const char *fname): Checks if the file exists using the built-in function "access"

void initStorageManager (void): 
1. Checks the value of a global variable "init"
2. Storage manager is initialized if init==1 
3. Else assign init = 1 and initialize the storage manager

RC createPageFile (char *fileName): 
1. Open the file in write mode and create it with size 4096 
2. Write an emptyblock to the file 

RC openPageFile (char *fileName, SM_FileHandle *fHandle):
1. Open the file in read+write mode
2. Store the totalNumPages, curPagePos, fileName
3. Store the POSIX file pointer in mgmtInfo

RC closePageFile (SM_FileHandle *fHandle):
1. Close the file using fclose
2. Deinitialize the file handler

RC destroyPageFile (char *fileName):
1. Delete the file using the built-in function "remove"

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage):
1. Checks if the page exists using the totalNumPages
2. In case the page exists, seek the file pointer until that page using built-in function "fseek"
3. Read the file into PageHandler from the seeked location for the next 4096 bytes using built-in function "fread"
4. In case the page does not exists return "NON_EXISTING_PAGE" error

int getBlockPos (SM_FileHandle *fHandle):
1. returns the current block position using curPagePos variable of the file handler

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
Calls the readBlock function with pageNum=0

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
Calls the readBlock function with pageNum=curPagePos-1

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
Calls the readBlock function with pageNum=curPagePos

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
Calls the readBlock function with pageNum=curPagePos+1

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
Calls the readBlock function with pageNum=totalNumPages-1

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage):
1. Checks if the page number is greater / equal to 0
2. In case the above condition is true, seek the file pointer until that page using built-in function "fseek"
3. In case the page to be written in greater than totalNumPages, increase the pages of the file as totalNumPages=pageNum+1
4. Write the data from the PageHandler to the seeked location for the next 4096 bytes using built-in function "fwite"
5. In case the page does not exists return "INVALID_PAGE_NUMBER" error

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
Calls the writeBlock function with pageNum=curPagePos

RC appendEmptyBlock (SM_FileHandle *fHandle):
Calls the writeBlock fucntion with pageNum=totalNumPages and memPage=emptyblock

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle):
Calls the writeBlock function with pageNum=numberOfPages-1 and memPage=emptyblock

***************************dberror.h**************************************
Created additional error codes in header files as follows:

#define RC_STORAGE_MGR_NOT_INIT 5
#define RC_FILE_PRESENT 6
#define RC_INVALID_PAGE_NUMBER 7
#define RC_READ_FAILED 600
#define RC_DELETE_FAILED 601

***************************test_assign1_1.c**************************************
Four additional test cases are created in the C file. Each of the test cases process the file "test_pagefile.bin"

static void testPageContent(void):
1. Create and Open the file
2. Read the first block which is a empty block
3. Write 0-9 numbers to the first block
4. Append empty block to the file (File size = 8KB)
5. Add 4 pages to the file (File size = 16KB)
6. Write A-Z alphabets to the 3rd block of the file
7. Read the 3rd block to ensure A-Z is written
8. Destroy the file

static void testreadnonexistentPage(void):
1. Create and Open the file
2. Read the block as pageNum=totalNumPages 
3. Since this block does not exists case return "Reading non-existent page"
4. Destroy the file

static void testPagecreateread(void):
1. Create and Open the file
2. Read the first block which is a empty block
3. Add 10 pages to the file (File size = 40KB)
4. Get the current page of the file using getBlockPos
5. Read previous block of the file and print the previous block number 
6. Read the next block of the file and print the next block number 
7. Print total number of pages of the file
8. Write characters ranging from numbers, alphabets and special characters to the current block of the file 
9. Read characters from the current block to ensure if the data is written to the file correctly 
10. Destroy the file

static void testPagewritenonexisting(void):
1. Create and Open the file
2. Read the first block which is a empty block
3. Write character ":" to a page "12" which is a non-existent page (File size = KB)
4. Read characters from the current block to ensure if the data is written to the file correctly 
5. Destroy the file