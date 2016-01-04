Assignment 2 - Buffer Manager
Submitted by Group No. 19 (kkaushik@hawk.iit.edu, pmankani@hawk.iit.edu, sguhatha@hawk.iit.edu)

***********************************************************
The assignment consists of five C files and six header files, a Makefile and a README: 

1. buffer_mgr_stat.c
2. buffer_mgr_stat.h
3. buffer_mgr.c
4. buffer_mgr.h
5. dberror.c
6. dberror.h
7. dt.h
8. storage_mgr.c
9. storage_mgr.h
10. test_assign2_1.c
11. test_helper.h
12. Makefile
13. README

***********************Extra Credit***********************
Implemented the below points required for extra credit:
1. Multithreading: 
a. Implemented the buffer manager program such that the program supports multithreading. 
b. In order to implement Multithreading we used pthread.h which is a threading header used in linux systems. 
c. Used the BM_LOCK and BM_UNLOCK functions to lock and unlock the buffer in operation
d. Once the buffer is locked by the user pthread.h will help to manage the multithreads 

2. Additional replacement strategy: 
a. LRU-K is implemented as a extra replacement stratedy. 
b. For LRU-K we have implemented LRU-3 wherein the 3rd least recently used page is replaced with the new page when the buffer is full. 
c. The code uses LRU strategy and finds the 3rd least recently used page from the buffer 
d. Once the page is recognized, in case the page has fixcount is greater than zero the next least recently used page is replaced with the new page in the buffer.

***********************Makefile***************************
Files on the fourier server are present in the path: 
/home/class/fall-15/cs525/kkaushik/Assignment2/

The files are also checked in the Bitbucket repository: (https://kedarkaushikkar@bitbucket.org/bglavic/cs525-f15-kkaushik.git)

In order to run the files, you can execute any one of the below commands:
1. make 
2. make all

The above commands create an output file " test_assign2_1". This can be then executed using "./ test_assign2_1"

To clean the output files created use the command:
make clean

NOTE:
The program has been executed using the concept of Doubly Link list. Thus there are few changes that were made in the test case file(test_assign2_1.c) to make sure that the expected pool content is in the order to match with doubly link list.
The values are still the same and complying with the logic. Only it has been rearranged to satisfy doubly link list.


***************************buffer_mgr.c************************************
We have implemented 3 replacement strategies FIFO,LRU, LRU-K and the program is thread safe.

Data Structure Used:
In addition to BM_BufferPool and BM_PageHandle we have defined following data structures:
Node_DLL: A double linked list is created to store the page level information of the buffer. It consists of following entities:
1. Pointer to the pagehandle
2. fixcount: integer variable to check if the page is in use (read/ write)
3. pagenumber of the respective storage page number
4. markdirty: boolean variable to check if the page is modified
5. *prev, *next: Pointer to previous and next nodes. 

BM_mgmtinfo: A structure to store the buffer level information: 
1. Pointer to the filehandle. This points to the file that is being initialized via the storage manager
2. read_io, write_io: The integer variables that store the read_io and write_io information if the buffer is read or written
3. node_dll *head: pointer to the double linked list

Functions:
Functions created to implement the buffer manager are as below:

node_dll *GetNewNode(int x)
-----------------------------------------
1. This function is used to create a new node in case the page is not available in buffer. 
2. The node is created such that the prev and next nodes are pointed to NULL
3. The is_dirty, fixcount, pagehandle are all initialized to NULL. 
4. The page number of the pagehandle is initialized with the page number that needs to be created. 

node_dll *InsertAtTail(node_dll *head, int pageNum)
-----------------------------------------------------------------------
1. This function helps to insert a double linked list node at the tail. 
2. The logic implemented for buffer manager is that the recently accessed page is added at the tail. 
3. This ensures that the most recent page is at the tail and the node should not be removed from the tail. 
4. This function calls the GetNewNode to create a new node and insert at tail

RC strategy(BM_BufferPool *const bm, int pageNum)
-----------------------------------------------------------------------
1. This function is created to implement the strategy (FIFO, LRU, LRU-K)
2. This function is called from the pinPage when the page is accessed by the user
3. In case the page needs to be replaced the "strategy" function is called. 
4. In case the strategy is "FIFO" or "LRU" simply the head is removed from the buffer and the new node is added at the tail by calling "InsertAtTail"
5. In case the strategy is LRU-K the 3rd least recently used node is moved to the head and then the page is removed from the buffer
6. In both the cases, if the page is marked as used, that is the fixcount is greater than 0, in this case the next available least recently used node is marked as head and removed from the buffer. 

RC initBufferPool(BM_BufferPool *bm, char *pageFileName,
		  int numPages, ReplacementStrategy strategy,
		  void *stratData)
---------------------------------------------------------------------------------
1. This function is initializes the buffer manager. 
2. It also initializes the node count of the buffer to 1 which marks that no pages are available in the buffer. 

RC shutdownBufferPool(BM_BufferPool *const bm)
-----------------------------------------------------------------------
1. This function is used to shutdown or in other words delete the buffer pool and initialize all its values to null.

RC forceFlushPool(BM_BufferPool *const bm)
-------------------------------------------------------------
This function is used in case if there are few nodes which are dirty or still under used by other user and not written back to the file. In such situation this function is called to make sure those respective nodes are saved back to the file before any other functionality can take place. This is required to make sure we don’t loose the latest updated data.

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
------------------------------------------------------------------------------------------------
This function is just used to mark a certain page as dirty when the new updated content is still not written back to the file.

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
------------------------------------------------------------------------------------------------
This function unpins the page if found in the buffer which was currently pinned and then reduces the fix count by one. Else it throws an error.

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
-----------------------------------------------------------------------------------------------
This function basically writes a page back to the file. Even if it is dirty it will forcibly make it save to the file and make the necessary changes to the parameters.

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
	    const PageNumber pageNum)
---------------------------------------------------------------------------------------------
This functionality carries out basic two functions. Return the requested page to the user if available in the buffer. Otherwise search the page from the file and then add to the buffer or replace it with any other node if full using strategy and return it to the user.

int getNumReadIO (BM_BufferPool *const bm)
--------------------------------------------------------------
Returns the number of read count.

int getNumWriteIO (BM_BufferPool *const bm)
---------------------------------------------------------------
Returns the number of write count.

PageNumber *getFrameContents (BM_BufferPool *const bm)
----------------------------------------------------------------------------------
Return the list of all the contents of the pages stored in the buffer pool.


bool *getDirtyFlags (BM_BufferPool *const bm)
----------------------------------------------------------------
Return the list of all the dirty pages stored in the buffer pool.

int *getFixCounts (BM_BufferPool *const bm)
-------------------------------------------------------------
Return the fix count of all the pages stored in the buffer pool.

***************************dberror.h**************************************
Created additional error codes in header files as follows:

#define RC_STORAGE_MGR_NOT_INIT 5
#define RC_FILE_PRESENT 6
#define RC_INVALID_PAGE_NUMBER 7
#define RC_READ_FAILED 600
#define RC_DELETE_FAILED 601

*************************** test_assign2_1.c **********************************
One additional test cases are created in the C file for LRU-K.

static void testLRUK (void):
1. Create the poolContent based on the expected output for verification.
2. Make a orderRequests for the list of pages requested to be read to the buffer.
3. Create the page file, and 100 Dummy pages and initialize the buffer pool.
4. Read the first five pages linearly with direct unpin and no modifications.
5. Read the pages to change LRU-K order.
6. Replace pages and check that it happens in LRU-K order.
7. Check the number of reads and write.
8. Shutdown the buffer and destroy the file.

