#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testCreateOpenClose(void);
static void testSinglePageContent(void);
static void testPageContent(void);
static void testreadnonexistentPage(void);
static void testPagecreateread(void);
static void testPagewritenonexisting(void);


/* main function running all tests */
int
main (void)
{
  testName = "";
  
  initStorageManager();

  testCreateOpenClose();
  testSinglePageContent();
  testPageContent();
  testreadnonexistentPage();
  testPagecreateread();
  testPagewritenonexisting();

  return 0;
}


/* check a return code. If it is not RC_OK then output a message, error description, and exit */
/* Try to create, open, and close a page file */
void
testCreateOpenClose(void)
{
  SM_FileHandle fh;

  SM_PageHandle ph;
  int i;
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  printf("\n**********First Test Case*****************\n");
  testName = "test create open and close methods";

  TEST_CHECK(createPageFile (TESTPF));
  
  TEST_CHECK(openPageFile (TESTPF, &fh));
  ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
  ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");
  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));

  // after destruction trying to open the file should cause an error
  ASSERT_TRUE((openPageFile(TESTPF, &fh) != RC_OK), "opening non-existing file should return an error.");

  TEST_DONE();
}

/* Try to read and write data to a single page file */
void
testSinglePageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;
  printf("\n**********Second Test Case*****************\n");
  testName = "test single page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);


  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("\ncreated and opened file\n");
  
  // read first page into handle
  TEST_CHECK(readFirstBlock (&fh, ph));
  // the page should be empty (zero bytes)
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
  printf("\nfirst block was empty\n");
    
  // change ph to be a string and write that one to disk
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (0, &fh, ph));
  printf("writing first block\n");

  // read back the page containing the string and check that it is correct
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("\nreading first block\n");

  // destroy new page file
  ASSERT_TRUE(destroyPageFile (TESTPF)==RC_OK, "File has been destroyed");


  TEST_DONE();
}

/* Try to read , write , append empty block and create empty pages in the file */
void
testPageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;
  printf("\n**********Third Test Case*****************\n");
  testName = "test page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);


  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("\ncreated and opened file\n");

  // read first page into handle
  TEST_CHECK(readFirstBlock (&fh, ph));
  // the page should be empty (zero bytes)
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
  printf("first block was empty\n");

  // change ph to be a string and write that one to disk
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (0, &fh, ph));
  printf("writing first block\n");

  // read back the page containing the string and check that it is correct
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("\nreading first block\n");

  appendEmptyBlock(&fh);// created an empty page (File size = 8 KB)
  ensureCapacity(4, &fh); // created total 4 pages in the file (File size - 16 KB)

  fseek(fh.mgmtInfo, 0, SEEK_END);
  printf("\nFile Size after appending 1 page (appendEmptyBlock) and adding 4 pages (ensureCapacity):%d", (int) (ftell(fh.mgmtInfo)));
  fseek(fh.mgmtInfo, 0, SEEK_SET);

  for (i=0; i < PAGE_SIZE; i++)
      ph[i] = (i % 26)+'A';
    TEST_CHECK(writeBlock (2, &fh, ph));
    printf("\nwriting the third block\n");

    TEST_CHECK(readBlock (2, &fh, ph));
     for (i=0; i < 26; i++)
       ASSERT_TRUE((ph[i] >= 'A' && ph[i]<='Z'), "The third block contains all the letters from A to Z.");
     printf("\nreading third block\n");

     // destroy new page file
     ASSERT_TRUE(destroyPageFile (TESTPF)==RC_OK, "File has been destroyed");


  TEST_DONE();
}

/* Try to read non existing page to file */
void
testreadnonexistentPage(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  printf("\n**********Forth Test Case*****************\n");
  testName = "test read non existing page";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("\ncreated and opened file\n");

  ASSERT_TRUE((readBlock (fh.totalNumPages, &fh, ph) == RC_READ_NON_EXISTING_PAGE), "Trying to read non-existent page");

    // destroy new page file
  ASSERT_TRUE(destroyPageFile (TESTPF)==RC_OK, "File has been destroyed");


  TEST_DONE();
}

/* Try to read, write previous and next blocks from the file*/
void
testPagecreateread(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;
  printf("\n**********Fifth Test Case*****************\n");
  int currentpage;
  testName = "test page create read";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("\ncreated and opened file\n");

  // read first page into handle
  TEST_CHECK(readFirstBlock (&fh, ph));
  // the page should be empty (zero bytes)
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
  printf("\nfirst block was empty\n");

  ensureCapacity(10, &fh); // created total 10 pages in the file (File size - 40 KB)

  fseek(fh.mgmtInfo, 0, SEEK_END);
  printf("\nFile Size after adding 10 pages (ensureCapacity):%d", (int) (ftell(fh.mgmtInfo)));
  fseek(fh.mgmtInfo, 0, SEEK_SET);

  currentpage=getBlockPos(&fh);

  printf("\nCurrent Page: %d", currentpage);

  readPreviousBlock(&fh, ph);

  printf("\nCurrent Page after reading previous block: %d",getBlockPos(&fh) );

  readNextBlock( &fh, ph);

  printf("\nCurrent Page after reading next block: %d", getBlockPos(&fh));

  printf("\nTotal number of pages: %d\n", fh.totalNumPages);

  for (i=0; i < PAGE_SIZE; i++)
        ph[i] = (i % 100)+'0';
      TEST_CHECK(writeCurrentBlock ( &fh, ph));
      printf("\nwriting the current block\n");

      TEST_CHECK(readCurrentBlock (&fh, ph));
      printf("\nReading characters in the current block that were just written: ");
           for (i=0; i < PAGE_SIZE; i++)
             printf("%c", ph[i]);
           printf("\n");

   // destroy new page file
           ASSERT_TRUE(destroyPageFile (TESTPF)==RC_OK, "File has been destroyed");

  TEST_DONE();
}

void
testPagewritenonexisting(void)
{
 SM_FileHandle fh;
 SM_PageHandle ph;
 int i;
 printf("\n**********Sixth Test Case*****************\n");
 int currentpage;
 testName = "test write to non existing page";

 ph = (SM_PageHandle) malloc(PAGE_SIZE);

 // create a new page file
 TEST_CHECK(createPageFile (TESTPF));
 TEST_CHECK(openPageFile (TESTPF, &fh));
 printf("\ncreated and opened file\n");

 // read first page into handle
 TEST_CHECK(readFirstBlock (&fh, ph));
 // the page should be empty (zero bytes)
 for (i=0; i < PAGE_SIZE; i++)
   ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
 printf("\nfirst block was empty\n");

 for (i=0; i < PAGE_SIZE; i++)
         ph[i] = (i % 200)+'A';
 writeBlock(12, &fh, ph);

 fseek(fh.mgmtInfo, 0, SEEK_END);
 printf("\nFile Size after writing to the 12th page:%d", (int) (ftell(fh.mgmtInfo)));
 fseek(fh.mgmtInfo, 0, SEEK_SET);

 currentpage=getBlockPos(&fh);

 printf("\nTotal number of pages: %d\n", fh.totalNumPages);

 printf("\nCurrent Page: %d", currentpage);

     TEST_CHECK(readCurrentBlock (&fh, ph));
     printf("\nReading characters in the current block that were just written: ");
          for (i=0; i < PAGE_SIZE; i++)
            printf("%c", ph[i]);
          printf("\n");

          // destroy new page file
		  ASSERT_TRUE(destroyPageFile (TESTPF)==RC_OK, "File has been destroyed");
 TEST_DONE();
}
