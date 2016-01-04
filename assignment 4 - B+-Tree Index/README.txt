Assignment 4 - B+ Tree Index
Submitted by Group No. 19 (kkaushik@hawk.iit.edu, pmankani@hawk.iit.edu, sguhatha@hawk.iit.edu)

***********************************************************
The assignment consists of ten C files and nine header files, a Makefile and a README: 

1. btree_mgr.c
2. btrre_mgr.h
1. buffer_mgr_stat.c
2. buffer_mgr_stat.h
3. buffer_mgr.c
4. buffer_mgr.h
5. dberror.c
6. dberror.h
7. dt.h
8. storage_mgr.c
9. storage_mgr.h
10. test_assign4_1.c
11. test_helper.h
12. rm_serializer.c
13. test_expr.c
14. expr.c
15. expr.h
16. tables.h
17. record_mgr.c
18. record_mgr.h
19. Makefile
20. README

***********************Makefile***************************
Files on the fourier server are present in the path: 
/home/class/fall-15/cs525/kkaushik/Assignment4/

The files are also checked in the Bitbucket repository: (https://kedarkaushikkar@bitbucket.org/bglavic/cs525-f15-kkaushik.git)

In order to run the files, you can execute any one of the below commands:
1. make 

The above commands create an output file " test_assign4_1". This can be then executed using "./ test_assign4_1"

To clean the output files created use the command:
make clean

NOTE:
There are few changes that made in the test case file(test_assign4_1.c) to make sure the records match with the output in the compare_records test case to make sure that the expected records match with the test_case. 


*********************************btree_mgr.c************************************
We have implemented unique key constraint and the a interactive user interface. 

Data Structure Used:
In addition to the provided data structures we have we have defined following data structures:

tree_mgmt: 

A structure to store book-keeping information of the binary tree. It consists of the below entities:
a. PageNumber of the root element
b. Number of nodes in the tree
c. Number of entries in the node of the tree
d. Order of the tree
e. Buffer manager pool that keeps the management information of the buffer
f. Page handler of the buffer manager.

node_ele: 

A structure to store the node element value of the binary tree node. It consists of the following entities:
a. Pointer to the node
b. Value of the pointer

node1:
This structure is used to store the individual node information. It consists of the following entities:
a. Boolean variable to identify if the node is a bool or not
b. Number of keys in the binary tree
c. Pagenumber of the parent key
d. Page number of the corresponding node
e. Node element which consists of the value of the node (a instance of the node_ele structure)

tree_scan_mgmt: 
A structure to store the book keeping information of the scan handler. It consists of the below entities:
a. PageNumber of the page where the node is stored. 
b. Current node which is retrieved on the matching condition
c. Current position of the node that is retrieved. 

Functions:
Functions created to implement the btree manager are as below:

static RC divideAndAdd(BTreeHandle *tree,PageNumber l,bool leaf)
----------------------------------------------------------------------------------
1. This function is used to during insert key. 
2. While inserting a key to the node incase the keys overflows we need to divide the node into two nodes and then add the new node to the left element. 
3. Thus the function helps to divide the node and then insert the new node. 

static RC addKeyNodeToRoot(BTreeHandle *tree, PageNumber l, PageNumber rp, Value key)
----------------------------------------------------------------------------------
1. This function helps to add a key to the root node. 
2. In case the new node needs to be added and due to this the roots in the tree are merged then this function is called where the key needs to be added to the root node. 

static int putKeyInRoot(BTreeHandle *tree, node1 *n1, long long ptr, Value *key );
----------------------------------------------------------------------------------
1. This function help to add the key to the root node once the node is added to the root. 

static RC searchKey(BTreeHandle *tree, node1 *n1, Value *key, int *ep, int *pn, bool not_match);
----------------------------------------------------------------------------------
1. This function searches a key in the node and returns in case it matches.
2. This function is called in insertkey such that in case the key is already present in the tree it will not insert it again in the tree

static RC removeKey(BTreeHandle *tree, PageNumber fp, Value *key);
----------------------------------------------------------------------------------
1. This function removes the corresponding key once the node is deleted. 
2. Thus the value of the node is deleted. 

static RC removeNode(BTreeHandle *tree, node1 *n1, PageNumber pt, Value *key );
----------------------------------------------------------------------------------
1. This function remove the node from the binary tree. 
2. This is called in the delete node 

static RC spreadKeys(BTreeHandle *tree, PageNumber lp, PageNumber rp);
----------------------------------------------------------------------------------
1. This function is called from the insert node function which spread keys in case there is an overflow in the node and then inserts the corresponding key to the node.

static RC joinKeys(BTreeHandle *tree, PageNumber lp, PageNumber rp);
----------------------------------------------------------------------------------
1. This function is called from the insert node function which helps to join keys in case any node is underflowing due to a split

static PageNumber findNearestKey(BTreeHandle *tree, PageNumber pn);
----------------------------------------------------------------------------------
1. This function is used to find the nearest key of the node based on the deleted node. The nearest key of the deleted node is identified and then the corresponding node is deleted.

static int createBTNode(BTreeHandle *tree)
-----------------------------------------
1. This function creates a binary tree node. 
2. It increments the node count of the binary tree and also increments the entry count that are further used for manipulation of the tree. 


RC initIndexManager (void *mgmtData)
-----------------------------------------------------------------------
1. This function initializes the storage manager, buffer manager and index manager. 

RC shutdownIndexManager ()
-----------------------------------------------------------------------
1. This funciton frees all the memory blocks assigned to the index manager. 


RC createBtree (char *idxId, DataType keyType, int n)
-----------------------------------------------------------------------
1. This function creates a page file and initializes the buffer manager. 
2. It creates a new binary tree with the node as the root node and inserts data to the node. 
3. It increments the node count and the entry count of the binary tree

RC openBtree (BTreeHandle **tree, char *idxId)
-----------------------------------------------------------------------
1. This function opens already created binary tree from the page file. 

RC closeBtree (BTreeHandle *tree)
-----------------------------------------------------------------------
1. This function closes the binary tree and frees the memory structure allocated to it. 

RC deleteBtree (char *idxId)
-----------------------------------------------------------------------
1. This function deletes binary tree file created. 

RC getNumNodes (BTreeHandle *tree, int *result)
-----------------------------------------------------------------------
1. This function returns the number of nodes created for the binary tree. 

RC getNumEntries (BTreeHandle *tree, int *result)
-----------------------------------------------------------------------
1. This function returns the number of entries created for the binary tree. 
 
RC getKeyType (BTreeHandle *tree, DataType *result)
-----------------------------------------------------------------------
1. This function returns the type of the key inserted in the binary tree.
2. However, we created a tree with only Integer types hence this return DT_INT

RC findKey (BTreeHandle *tree, Value *key, RID *result)
-----------------------------------------------------------------------
1. This function finds the key of the binary tree based on the key passed as a parameter. 

RC insertKey (BTreeHandle *tree, Value *key, RID rid)
-----------------------------------------------------------------------
1. This function inserts the corresponding key passed as the parameter to the binary tree
2. It inserts the key by sorting the values of the keys already present in the binary tree

RC deleteKey (BTreeHandle *tree, Value *key)
-----------------------------------------------------------------------
1. This function deletes the key from the binary tree. It calls the removeKey and removeNode functions

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
-----------------------------------------------------------------------
1. This function initialzes the scan structure with the appropriate values.

RC nextEntry (BT_ScanHandle *handle, RID *result)
-----------------------------------------------------------------------
1. This function searches in the scan handle the appropriate key and returns it as a result. 

RC closeTreeScan (BT_ScanHandle *handle)
-----------------------------------------------------------------------
1. This function frees the scan handle and all the memory blocks associated to it. 

***************************dberror.h**************************************
Created additional error codes in header files as follows:

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4

#define RC_FILE_PRESENT 6
#define RC_STORAGE_MGR_NOT_INIT 5
#define RC_INVALID_PAGE_NUMBER 7

#define RC_READ_FAILED 600
#define RC_DELETE_FAILED 601

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_NO_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 0

#define RC_NO_MORE_SPACE_IN_BUFFER 401
#define RC_UNKNOWN_STRATEGY 402
#define RC_INVALID_BM 403
#define RC_NON_EXISTING_PAGE_IN_FRAME 404
#define RC_TABLE_ALREADY_EXISTS 405
#define RC_TABLE_NOT_FOUND 406

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 0

#define RC_NO_MORE_SPACE_IN_BUFFER 401
#define RC_UNKNOWN_STRATEGY 402
#define RC_INVALID_BM 403
#define RC_NON_EXISTING_PAGE_IN_FRAME 404
#define RC_TABLE_ALREADY_EXISTS 405
#define RC_TABLE_NOT_FOUND 406



