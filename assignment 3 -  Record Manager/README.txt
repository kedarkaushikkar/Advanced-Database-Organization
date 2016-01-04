Assignment 2 - Record Manager
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
12. rm_serializer.c
13. test_expr.c
14. expr.c
15. expr.h
16. tables.h
17. record_mgr.c
18. record_mgr.h
19. Makefile
20. README

***********************Extra Credit***********************
Implemented the below points required for extra credit:
1. Check primary key constraints:  
a. Implemented the record manager with primary key constraints.
b. Created a key with one of the columns for e.g. "a"
c. While trying to insert records in the table with same values in "a" the value in the column will be incremented such that duplicate records will not be inserted in the table. 
d. This is implemented in all the test cases more specifically in "testInsertmanyRecords"

2. Interactive interface:
a. An interactive interface is created which helps to create a table based on the column names inputed by the user. 
b. The interface asks for column name and its corresponding datatype. The correct values of the datatype are: "INT, STRING, FLOAT & BOOL"
c. In case the datatype is string, the length of the string is asked from the user
d. Based on the inputs from the user, a schema is created. 
e. Once the schema is created the records are inserted in the table and displayed to the user as required. 

3. Handled memory issues in the test case by freeing the record and the rids created in the test case using free(record) and free(rid)

***********************Makefile***************************
Files on the fourier server are present in the path: 
/home/class/fall-15/cs525/kkaushik/Assignment3/

The files are also checked in the Bitbucket repository: (https://kedarkaushikkar@bitbucket.org/bglavic/cs525-f15-kkaushik.git)

In order to run the files, you can execute any one of the below commands:
1. make 
2. make all

The above commands create an output file " test_assign3_1". This can be then executed using "./ test_assign3_1"

To clean the output files created use the command:
make clean

NOTE:
There are few changes that made in the test case file(test_assign2_3.c) to make sure the records match with the output in the compare_records test case to make sure that the expected records match with the test_case. 


***************************record_mgr.c************************************
We have implemented unique key constraint and the a interactive user interface. 

Data Structure Used:
In addition to the provided data structures we have we have defined following data structures:

tablemgmt: A structure to the store book-keeping information of the table. It consists of the below entities:
a. Number of tuples in the table
b. Lenght of the schema based on the columns and the corresponding datatypes.
c. Pointer pointing to the first record. 
d. Pointer pointing to the last record. 
e. Lenght of the slot where each record would be inserted. 
f. Maximum number slots that can be stored in a page. 
g. Pointer to buffer pool which is used to write data.

recordmgmt: A structure to store the book-keeping information of the record that will be used while scanning data in the records. It consists of the below entities:
a. An expression that stores the search_condition based on the condition provided by the user. 
b. Current page position of the page while searching the record.  
c. Current slot number of the page while searching the record.  
d. Page number where the record is found matching the search condition. 
e. Slot number of the page where the record is found matching the search condition. 

Functions:
Functions created to implement the record manager are as below:

int getsizeofslot(Schema *schema)
-----------------------------------------
1. This function is used to calcuate the size of the slot where the record would be stored in the page. 
2. The initial slot length of the record is 2*sizeof(int) + 5. 
3. The two intergers are for the page number and slot number.
4. The rest of the 5 bits are for "[", "]", "-" " ", "{" since this would be a part of each record during insertion 
5. The record size is then calculated using the attrOffset function taken from the rm_serializer.c. 
6. This function is recreated in the record_mgr.c file. 
7. The function returns the slot size calculated. 

char *tablemgmtstr(tablemgmt *tabmgmt)
-----------------------------------------------------------------------
1. This function converts the table mgmt structure to a string which will be written to the first page of the file.
2. The attributes written to the file are schema length , first page , last page , tuples, slot length , max number of slots.
3. Each of the values of these attributes are enclosed in {} brackets.


tablemgmt *tabmgmt(char *tabmgmtstr)
-----------------------------------------------------------------------
1. This function converts the table mgmt string data to table mgmt structure.
2. The string tokenize functions strTol() and strTok() are used to identify the {} brackets which help to get the values of table mgmt data.

int getschemalen(Schema *schema) 
-----------------------------------------------------------------------
1. This function returns the total length of the schema.
2. It is based on the size of the data types and the number of attributes used to create the schema.

Record *strtorecord(char *rec_str, RM_TableData *RM)
-----------------------------------------------------------------------
1. This function converts the record string into record structure.
2. The function uses strTok() and strTol() to get the required structure.
3. Each of the data types and values of a column is seperated by a colon and each of the columns are seperated by a comma.
4. The function uses these 2 chars to identify its attributes and values. Once identified ,it is written to the structure using the setAttr() function.

Schema *strtoschema(char *schemarec)
-----------------------------------------------------------------------
1. This function converts the string data of the schema into schema structure.
2. The function uses strTok() to tokenize the string.
3. The schema is enclosed within <> brackets and each of the data types are seperated by a colon and comma.
4. The end of the attributes is market by ) brackets. These characters are used to identify the valid attributes of the schema.
5. The keyAttr char pointer which stores the keys present in the table is also formed in this function.

RC tabmgmttofile(char *name, tablemgmt *tabmgmtdata)
-----------------------------------------------------------------------
1. The table mgmt data is converted into string using the above function and written on the 0th page of the file.
 
int checkinit_rm (void)
-----------------------------------------------------------------------
1. The record manager is initialized using the global variable init_rm.

extern RC initRecordManager (void *mgmtData)
-----------------------------------------------------------------------
1. initializes the record manager using the checkinit_rm() function.

extern RC shutdownRecordManager ()
-----------------------------------------------------------------------
1. Deinitialized the global variable init_rm() function.

extern RC createTable (char *name, Schema *schema)
-----------------------------------------------------------------------
1. Creates a page file with the table name.
2. Once the page file is created, the table mgmt structure is initialized using the above function.
3. This table mgmt data is then converted into string and returned in the 0th page of the file.
4. Also the schema is serialized using the serialize(). The output of this function is written in tht 1st page of the file.
5. Thus the first two pages of the file are maintained for bookkeeping information.

extern RC openTable (RM_TableData *rel, char *name)
-----------------------------------------------------------------------
1. This function opens the file created in createTable() and initializes the same.
2. Initilize the buffer pool wih 3 pages using FIFO strategy.
3. Since page 0 and page 1 are for bookkeeping, these are pinned. Also the buffer mgr pointer is stored to the table mgmt structure.


extern RC closeTable (RM_TableData *rel)
-----------------------------------------------------------------------
1. This function closes the table and all the variable memory is free.
2. The table is closed using shutdownBufferpool()

extern RC deleteTable (char *name)
-----------------------------------------------------------------------
1. This function deletes the table if exists using the delete().
2. Incase the table does not exists , it returs a TABLE_NOT_FOUND error.

extern int getNumTuples (RM_TableData *rel)
-----------------------------------------------------------------------
1. Returns the tuple's value from the table mgmt structure.

extern RC insertRecord (RM_TableData *rel, Record *record)
-----------------------------------------------------------------------
1. This function inserts the record in the appropriate page and slot number
2. In case the page is full the slot 0 of the next page is considered. 
3. Once the slot and page number are identified, the appropriate page is pinned using "pinPage" 
4. The buffer manager takes care of replacing the buffer pages based on the strategy selected. In our case it is FIFO
5. The page is then marked as dirty and unpinned. The marked page is then forced to the buffer manager. 
6. Once inserted the number of tuples in the structure are incremented and the updated tablemanagement data is written to the file. 

extern RC deleteRecord (RM_TableData *rel, RID id)
-----------------------------------------------------------------------
1. This function deletes the mentioned record.
2. The page number and slot number is identified by the RID passed to the function
3. The deleted page is pinned and empty string is written in the record data.
4. The page is then marked dirty, unpinned and forced to the buffer manager. 
5. The number of tuples are decremented and the updated table mgmt data is returned to the file. 

extern RC updateRecord (RM_TableData *rel, Record *record)
-----------------------------------------------------------------------
1. This function updates the mentioned record.
2. The page number and slot number is identified by the RID passed to the function
3. The updated page is pinned and updated string is written in the record data.
4. The page is then marked dirty, unpinned and forced to the buffer manager. 
 
extern RC getRecord (RM_TableData *rel, RID id, Record *record)
-----------------------------------------------------------------------
1. This function retirevs the mentioned record.
2. The page number and slot number is identified by the RID passed to the function
3. The page is pinned and the data of the record is copied to the string and page is then unpinned
4. This data is then stored in record of data.

extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
-----------------------------------------------------------------------
1. This function initializes the record mgmt structure.

extern RC next (RM_ScanHandle *scan, Record *record)
-----------------------------------------------------------------------
1. This function searches the record based on the condition in the scan handler.
2. Once the record is retireved the getRecord() is called to get the data of the record.
3. The evaluateExpr() evaluates the function given by the user which helps to find the record.

extern int getRecordSize (Schema *schema)
-----------------------------------------------------------------------
1. This function retrieves the size of the records based on the datatypes of the schema.

extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
-----------------------------------------------------------------------
1. This function initialzes the schema structure with the appropriate values.

extern RC freeSchema (Schema *schema)
-----------------------------------------------------------------------
1. This function frees the schema structure created.

extern RC createRecord (Record **record, Schema *schema)
-----------------------------------------------------------------------
1. This function allocates memory for the record that needs to be created.

extern RC freeRecord (Record *record)
-----------------------------------------------------------------------
1. This function frees the record data and structure.

extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
-----------------------------------------------------------------------
1. This function gets the value of the attribute of the record.
2. attrOffset() is called to find the offset of the attribute.
3. Based on the offset the value is retrieved.
 
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
-----------------------------------------------------------------------
1. This function sets the attribute to the values passed to the function.
2. attrOffset() is called to find the offset of the attribute.
3. The value passed is then set to the record.

***************************dberror.h**************************************
Created additional error codes in header files as follows:

RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
RC_NO_TUPLES 203
RC_RM_NO_PRINT_FOR_DATATYPE 204
RC_RM_UNKOWN_DATATYPE 205

RC_IM_KEY_NOT_FOUND 300
RC_IM_KEY_ALREADY_EXISTS 301
RC_IM_N_TO_LAGE 302
RC_IM_NO_MORE_ENTRIES 303

RC_NO_MORE_SPACE_IN_BUFFER 401
RC_UNKNOWN_STRATEGY 402
RC_INVALID_BM 403
RC_NON_EXISTING_PAGE_IN_FRAME 404
RC_TABLE_ALREADY_EXISTS 405
RC_TABLE_NOT_FOUND 406


*************************** test_assign3_1.c **********************************
One additional test case is created in the C file for interactive interface.

void testUserInterface()
1. The number of attributes are taken as input.
2. Based on the number of attributes the columns names and its data types are taken as input from the user.
3. The valid values for inputs are int , string , float and bool.
4. Incase the string is the inputted datatype, the length of the string is also asked from the user.
5. Based on this input values the schema is created using testSchema() function.
6. Once the schema is created the corresponding table is created and records are inserted in the table as required.
7. Incase if the user inputs as mentioned below values ,

	column name a : int
	column name b : string of length 4
	column name c : int

then the compare records function will be called as required.