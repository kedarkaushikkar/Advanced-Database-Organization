#include "dberror.h"
#include "expr.h"
#include "tables.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

//variable to check whether the record manager is initialized.
int init_rm=0;

//Structure of bookkeeping data for table.
typedef struct tablemgmt
{
	int tuples;
	int schema_len;
	int record_start;
	int record_last;
	int slot_len;
	int maxnumslot;
	BM_BufferPool *bm;
}tablemgmt;

//Structure of bookkeeping data for scanning records.
typedef struct recmgmt
{
	Expr *search_cond;
	int page_cur;
	int slot_cur;
	int page_num;
	int slot_num;
}recmgmt;

//Function to return the offset value of the attribute to be inserted
RC
attrOffset (Schema *schema, int attrNum, int *result)
{
  int offset = 0;
  int attrPos = 0;

  for(attrPos = 0; attrPos < attrNum; attrPos++)
    switch (schema->dataTypes[attrPos])
    {
      	  case DT_STRING:
		offset += schema->typeLength[attrPos];
		break;
		  case DT_INT:
		offset += sizeof(int);
		break;
		  case DT_FLOAT:
		offset += sizeof(float);
		break;
		  case DT_BOOL:
		offset += sizeof(bool);
		break;
    }

  *result = offset;
  return RC_OK;
}

//returns the size of slot where the record would be inserted
int getsizeofslot(Schema *schema)
{
	int slot_size=0, i=0, tempSize;
	    slot_size += 15;

	for(i=0; i<schema->numAttr; ++i){
		switch (schema->dataTypes[i]){
			case DT_STRING:
				tempSize = schema->typeLength[i];
				break;
			case DT_INT:
				tempSize = 5;
				break;
			case DT_FLOAT:
				tempSize = 10;
				break;
			case DT_BOOL:
				tempSize = 5;
				break;
			default:
				break;
		}
		slot_size += (tempSize + strlen(schema->attrNames[i]) + 2);
	}
	return slot_size;
}

//convert the table data into a string
char *tablemgmtstr(tablemgmt *tabmgmt)
{
	VarString *res_string;
	char *result;
	MAKE_VARSTRING(res_string);
	APPEND(res_string, "Schemalen {%i} FirstPage {%i} LastPage {%i} Tuples {%i} Slotlen {%i} Maxnumslots {%i} \n", tabmgmt->schema_len, tabmgmt->record_start, tabmgmt->record_last, tabmgmt->tuples, tabmgmt->slot_len, tabmgmt->maxnumslot);
	GET_STRING(result, res_string);
	return (result);
}

//convert string table data into table management structure using string tokenize functions
tablemgmt *tabmgmt(char *tabmgmtstr)
{
	tablemgmt *tabmgmtdata = (tablemgmt*) malloc (sizeof(tablemgmt));
	char tabmgmtdatastr[strlen(tabmgmtstr)];
	strcpy(tabmgmtdatastr, tabmgmtstr);
	char *str1, *str2;

	str1=strtok(tabmgmtdatastr,"{");
	str1=strtok(NULL,"}");
	tabmgmtdata->schema_len=strtol(str1, &str2, 10);

	str1=strtok(NULL,"{");
	str1=strtok(NULL,"}");
	tabmgmtdata->record_start=strtol(str1, &str2, 10);

	str1=strtok(NULL,"{");
	str1=strtok(NULL,"}");
	tabmgmtdata->record_last=strtol(str1, &str2, 10);

	str1=strtok(NULL,"{");
	str1=strtok(NULL,"}");
	tabmgmtdata->tuples=strtol(str1, &str2, 10);

	str1=strtok(NULL,"{");
	str1=strtok(NULL,"}");
	tabmgmtdata->slot_len=strtol(str1, &str2, 10);

	str1=strtok(NULL,"{");
	str1=strtok(NULL,"}");
	tabmgmtdata->maxnumslot=strtol(str1, &str2, 10);

	return tabmgmtdata;
}

//return the schema length of the table based on the attribute type and length
int getschemalen(Schema *schema)
{
	int i, schema_size;
	schema_size = sizeof(int)+sizeof(int)*schema->numAttr+sizeof(int)*schema->numAttr+sizeof(int)+sizeof(int)*schema->numAttr;
	for(i=0;i<schema->numAttr;i++)
	{
		schema_size=schema_size+strlen(schema->attrNames[i]);
	}
	return schema_size;
}

//convert the record data into string
Record *strtorecord(char *rec_str, RM_TableData *RM)
{
	Record *rec=(Record *) malloc(sizeof(Record));
	tablemgmt *tabmgmtdata = (tablemgmt*) (RM->mgmtData);
	Value *val;
	int i;
	rec->data = (char *) malloc (sizeof(char)* tabmgmtdata->slot_len );
	char rec_data[strlen(rec_str)];
	strcpy(rec_data, rec_str);
	char *str1,*str2;
	str1=strtok(rec_data, "-");
	str1=strtok(NULL, "]");
	str1=strtok(NULL, "(");

	for(i=0;i<RM->schema->numAttr;i++)
	{
		str1=strtok(NULL,":");
		if(i==RM->schema->numAttr-1)
		{
			str1=strtok(NULL,")");
		}
		else
		{
			str1=strtok(NULL,",");
		}
		if(RM->schema->dataTypes[i]==DT_INT)
		{
			int data_val;
			data_val=strtol(str1, &str2, 10);
			MAKE_VALUE(val, DT_INT, data_val);
			setAttr(rec,RM->schema, i, val);
			freeVal(val);
		}
		else if(RM->schema->dataTypes[i]==DT_STRING)
		{
			MAKE_STRING_VALUE(val, str1);
			setAttr(rec,RM->schema, i, val);
			freeVal(val);
		}
		else if(RM->schema->dataTypes[i]==DT_BOOL)
		{
			bool data_val;
			if(str1[0]=='t')
			{
				data_val= TRUE;
			}
			else
			{
				data_val= FALSE;
			}

			MAKE_VALUE(val, DT_BOOL, data_val);
			setAttr(rec,RM->schema, i, val);
			freeVal(val);
		}
		else if(RM->schema->dataTypes[i]==DT_FLOAT)
		{
			float data_val;
			data_val=strtof(str1, NULL);
			MAKE_VALUE(val, DT_FLOAT, data_val);
			setAttr(rec,RM->schema, i, val);
			freeVal(val);
		}
	}
	free(rec_str);
	return rec;
}

//convert schema data into string
Schema *strtoschema(char *schemarec)
{
	Schema *schema = (Schema*) malloc(sizeof(Schema));
	int i,j, numAttr;
	char data_schema[strlen(schemarec)];
	strcpy(data_schema, schemarec);
	char *str1, *str2, *str3;

	str1= strtok(data_schema, "<");
	str1= strtok(NULL, ">");
	numAttr=strtol(str1, &str2, 10);
	schema->numAttr=numAttr;
	schema->attrNames=(char **)malloc(sizeof(char*)*numAttr);
	schema->dataTypes=(DataType *)malloc(sizeof(DataType)*numAttr);
	schema->typeLength=(int*) malloc(sizeof(int)*numAttr);
	char* ref_str[numAttr];
	str1=strtok(NULL, "(");
	for(i=0;i<numAttr;i++)
	{
		str1=strtok(NULL, ": ");
		schema->attrNames[i]=(char*) calloc(strlen(str1), sizeof(char));
		strcpy(schema->attrNames[i], str1);

		if(i==numAttr-1)
		{
			str1=strtok(NULL, ") ");
		}
		else
		{
			str1=strtok(NULL, ", ");
		}

		ref_str[i]=(char *)calloc(strlen(str1), sizeof(char));

		if(strcmp(str1, "INT")==0)
		{
			schema->dataTypes[i]=DT_INT;
			schema->typeLength[i]=0;
		}
		else if(strcmp(str1, "FLOAT")==0)
			{
				schema->dataTypes[i]=DT_FLOAT;
				schema->typeLength[i]=0;
			}
		else if(strcmp(str1, "BOOL")==0)
			{
				schema->dataTypes[i]=DT_BOOL;
				schema->typeLength[i]=0;
			}
		else
			strcpy(ref_str[i], str1);

	}
	int flag=0, size=0;
	char* keyattr[numAttr];

	if((str1=strtok(NULL, "(")) != NULL)
	{
		str1=strtok(NULL, ")");
		char *keystr = strtok(str1, ", ");

		while(keystr!=NULL)
		{
			keyattr[size]=(char*)malloc(strlen(keystr)*sizeof(char));
			strcpy(keyattr[size], keystr);
			size++;
			keystr=strtok(NULL, ", ");

		}
		flag=1;
	}
	for(i=0; i<numAttr; i++)
	{
		if(strlen(ref_str[i])>0)
		{
			str3=(char*)malloc(sizeof(char)*strlen(ref_str[i]));
			memcpy(str3, ref_str[i], strlen(ref_str[i]));
			schema->dataTypes[i]= DT_STRING;
			str1=strtok(str3, "[");
			str1=strtok(NULL,"]");
			schema->typeLength[i]=strtol(str1, &str2, 10);
			free(str3);
			free(ref_str[i]);
		}
	}
	if(flag==1)
	{
		schema->keyAttrs=(int*)malloc(sizeof(int)*size);
		schema->keySize=size;
		for(i=0;i<size;i++)
		{
			for(j=0;j<numAttr;j++)
			{
				if(strcmp(keyattr[i], schema->attrNames[j])==0)
				{
					schema->keyAttrs[i]=j;
					free(keyattr[i]);
				}
			}
		}
	}

	return schema;
}

//write the table maangement data to the first page of the file
RC tabmgmttofile(char *name, tablemgmt *tabmgmtdata)
{
	if(exists(name)==0)
	{
		return RC_TABLE_NOT_FOUND;
	}

	SM_FileHandle fh;

	openPageFile(name, &fh);


	char *mgmt_str=tablemgmtstr(tabmgmtdata);
	writeBlock(0, &fh, mgmt_str);
	closePageFile(&fh);
	free(mgmt_str);
	return RC_OK;

}

//check if the record manager is initialized
int checkinit_rm (void)
{
    if (init_rm == 1)
    	return RC_OK;
    else
    	return -1;
}

//initialize the record manager using the above function
extern RC initRecordManager (void *mgmtData)
{
	if (checkinit_rm() != RC_OK)
	{
	   init_rm=1;
	}
	else
	{
		printf("Record manager is already initialized");
	}
	return RC_OK;
}

//close the record manager by reinitializing the global variable
extern RC shutdownRecordManager ()
{
	init_rm=0;
	return RC_OK;
}

//create table based on the schema created
extern RC createTable (char *name, Schema *schema)
{
	initStorageManager();
	int schemalen, slotlen, filelen, maxnumslots;
	char *tab_str, *schema_str;
	tablemgmt *tabmgmtdata = (tablemgmt*) malloc(sizeof(tablemgmt));
	int rc;

	if(exists(name)==1)
	{
		return RC_TABLE_ALREADY_EXISTS;
	}
	SM_FileHandle fh;
	createPageFile(name);

	schemalen=getschemalen(schema);
	slotlen=getsizeofslot(schema);
	filelen=((int) ((float)schemalen/PAGE_SIZE))+1;
	maxnumslots=((int) ((float)PAGE_SIZE/(float)slotlen))-1;
	rc=openPageFile(name, &fh);
	ensureCapacity(filelen+1, &fh);

	tabmgmtdata->tuples =0;
	tabmgmtdata->schema_len=schemalen;
	tabmgmtdata->record_start=filelen+1;
	tabmgmtdata->record_last=filelen+1;
	tabmgmtdata->slot_len=slotlen;
	tabmgmtdata->maxnumslot=maxnumslots;

	tab_str=tablemgmtstr(tabmgmtdata);
	schema_str=serializeSchema(schema);
	writeBlock(0, &fh, tab_str);
	writeBlock(1, &fh, schema_str);
	closePageFile(&fh);
	return RC_OK;
}

//open the created table
extern RC openTable (RM_TableData *rel, char *name)
{
	if(exists(name)==0)
	{
		return RC_TABLE_NOT_FOUND;
	}
	BM_BufferPool *bm = (BM_BufferPool*)malloc(sizeof(BM_BufferPool));
	BM_PageHandle *page = (BM_PageHandle*)malloc(sizeof(BM_PageHandle));
	initBufferPool(bm, name, 3, RS_FIFO, NULL);
	pinPage(bm, page, 0);
	tablemgmt *tabmgmtdata=tabmgmt(page->data);
	if(tabmgmtdata->schema_len<PAGE_SIZE)
	{
		pinPage(bm, page, 1);
	}
	rel->schema=strtoschema(page->data);
	rel->name=name;
	tabmgmtdata->bm=bm;
	rel->mgmtData=tabmgmtdata;
	free(page);
	return RC_OK;
}

//close the table and free all the variables
extern RC closeTable (RM_TableData *rel)
{
	shutdownBufferPool( ((tablemgmt*) rel->mgmtData)->bm );
	free(rel->mgmtData);
	free(rel->schema->attrNames);
	free(rel->schema->dataTypes);
	free(rel->schema->keyAttrs);
	free(rel->schema->typeLength);
	free(rel->schema);
	return RC_OK;
}

//delete the table file created
extern RC deleteTable (char *name)
{
	if(exists(name)==0)
	{
		return RC_TABLE_NOT_FOUND;
	}
	delete(name);
	return RC_OK;
}

//return the number of tuples from the bookkeeping data
extern int getNumTuples (RM_TableData *rel)
{
	return(((tablemgmt*)rel->mgmtData)->tuples);
}

//insert the record in the table file
extern RC insertRecord (RM_TableData *rel, Record *record)
{
	BM_PageHandle *page = (BM_PageHandle*)malloc(sizeof(BM_PageHandle));
	tablemgmt *tabmgmtdata=(tablemgmt *)(rel->mgmtData);
	int page_num, slot;
	char *rec_str;
	page_num=tabmgmtdata->record_last;
	slot=tabmgmtdata->tuples-((page_num-tabmgmtdata->record_start)*tabmgmtdata->maxnumslot);
	if(slot==tabmgmtdata->maxnumslot)
	{
		slot=0;
		page_num++;
	}
	tabmgmtdata->record_last=page_num;
	record->id.page=page_num;
	record->id.slot=slot;
	rec_str=serializeRecord(record,rel->schema);
	pinPage(tabmgmtdata->bm, page, page_num);
	memcpy(page->data+(slot*tabmgmtdata->slot_len), rec_str, strlen(rec_str) );
	free(rec_str);
	markDirty(tabmgmtdata->bm, page);
	unpinPage(tabmgmtdata->bm, page);
	forcePage(tabmgmtdata->bm, page);
	tabmgmtdata->tuples++;
	tabmgmttofile(rel->name, tabmgmtdata);
	free(page);
	return RC_OK;
}

//delete the record from the table
extern RC deleteRecord (RM_TableData *rel, RID id)
{
	BM_PageHandle *page = (BM_PageHandle *)malloc(sizeof(BM_PageHandle));
	    tablemgmt *tabmgmtdata = (tablemgmt *) (rel->mgmtData);
	    int page_num, slot;

	    page_num = id.page;
	    slot = id.slot;
	    pinPage(tabmgmtdata->bm, page, page_num);
	    memcpy(page->data + (slot*tabmgmtdata->slot_len), "\0",strlen(page->data + (slot*tabmgmtdata->slot_len)) );

	    markDirty(tabmgmtdata->bm , page);
	    unpinPage(tabmgmtdata->bm , page);
	    forcePage(tabmgmtdata->bm , page);

	    (tabmgmtdata->tuples)--;
	    tabmgmttofile(rel->name, tabmgmtdata);
	    return RC_OK;
}

//update the record table based on the record
extern RC updateRecord (RM_TableData *rel, Record *record)
{
	BM_PageHandle *page = (BM_PageHandle *)malloc(sizeof(BM_PageHandle));
	tablemgmt *tabmgmtdata = (tablemgmt *) (rel->mgmtData);
	int page_num, slot;
	char *rec_str;
	page_num = record->id.page;
	slot = record->id.slot;
	rec_str=serializeRecord(record, rel->schema);

	pinPage(tabmgmtdata->bm, page, page_num);
	memcpy(page->data + (slot*tabmgmtdata->slot_len), rec_str,strlen(rec_str) );

	free(rec_str);

	markDirty(tabmgmtdata->bm , page);
	unpinPage(tabmgmtdata->bm , page);
	forcePage(tabmgmtdata->bm , page);

	tabmgmttofile(rel->name, tabmgmtdata);
	return RC_OK;

}

//return the record from the table
extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
	BM_PageHandle *page = (BM_PageHandle *)malloc(sizeof(BM_PageHandle));
	tablemgmt *tabmgmtdata = (tablemgmt *) (rel->mgmtData);
	int page_num, slot, tuplenum;
	char *rec_str;
	Record *temp_rec;
	page_num = id.page;
	slot = id.slot;
	record->id.page=page_num;
	record->id.slot=slot;
	tuplenum=(page_num-tabmgmtdata->record_start)*(tabmgmtdata->maxnumslot)+slot+1;
	if(tuplenum>tabmgmtdata->tuples)
	{
		free(page);
		return RC_NO_TUPLES;
	}
	pinPage(tabmgmtdata->bm, page, page_num);
	rec_str=(char*) malloc(sizeof(char)*tabmgmtdata->slot_len);
	memcpy(rec_str, page->data+((slot)*tabmgmtdata->slot_len), sizeof(char)*tabmgmtdata->slot_len);
	unpinPage(tabmgmtdata->bm,page);
	temp_rec=strtorecord(rec_str, rel);
	record->data=temp_rec->data;
	free(temp_rec);
	free(page);
	return RC_OK;
}

//start the scan by initializing the record management structure
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	recmgmt *recordmgmt=(recmgmt*)malloc(sizeof(recmgmt));
	recordmgmt->page_cur=((tablemgmt*) rel->mgmtData)->record_start;
	recordmgmt->slot_cur=0;
	recordmgmt->search_cond=cond;
	recordmgmt->slot_num=((tablemgmt*) rel->mgmtData)->maxnumslot;
	recordmgmt->page_num=((tablemgmt*) rel->mgmtData)->record_last;
	scan->rel=rel;
	scan->mgmtData=(void*)recordmgmt;
	return RC_OK;
}

//run the scan based on the initialized record management structure
extern RC next (RM_ScanHandle *scan, Record *record)
{
	recmgmt *recordmgmt;
	Value *rec_val;
	RC return_code;
	recordmgmt=scan->mgmtData;
	record->id.page=recordmgmt->page_cur;
	record->id.slot=recordmgmt->slot_cur;
	return_code=getRecord(scan->rel, record->id,record);
	if(return_code== RC_NO_TUPLES)
	{
		return RC_NO_TUPLES;
	}
	else
	{
		evalExpr(record, scan->rel->schema, recordmgmt->search_cond, &rec_val);
		if(recordmgmt->slot_cur== recordmgmt->slot_num-1)
		{
			(recordmgmt->slot_cur)=0;
			recordmgmt->page_cur++;
		}
		else
		{
			(recordmgmt->slot_cur)++;
		}
		scan->mgmtData=recordmgmt;
		if(rec_val->v.boolV!=1)
		{
			return next(scan, record);
		}
		else
			return RC_OK;
	}
}

//finish the scan
extern RC closeScan (RM_ScanHandle *scan)
{
	return RC_OK;
}

// dealing with schemas
extern int getRecordSize (Schema *schema)
{
	int i, schema_size =0, size =0;
	for(i=0;i<schema->numAttr;i++)
	{
		if(schema->dataTypes[i]==DT_STRING)
		{
			size=schema->typeLength[i];
		}
		else if(schema->dataTypes[i]==DT_INT)
				{
					size=sizeof(int);
				}
		else if(schema->dataTypes[i]==DT_FLOAT)
				{
					size=sizeof(float);
				}
		else if(schema->dataTypes[i]==DT_BOOL)
				{
					size=sizeof(bool);
				}
		schema_size+=size;
	}
	return schema_size;
}

//create the schema based on the attributes passed
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	Schema *schema=(Schema*)malloc(sizeof(Schema));
	schema->attrNames=attrNames;
	schema->dataTypes=dataTypes;
	schema->keyAttrs=keys;
	schema->keySize=keySize;
	schema->typeLength=typeLength;
	schema->numAttr=numAttr;
	return schema;
}

//free the created schema
extern RC freeSchema (Schema *schema)
{
	free(schema);
	return RC_OK;
}

// dealing with records and attribute values
extern RC createRecord (Record **record, Schema *schema)
{
	*record=(Record*)malloc(sizeof(Record));
	(*record)->data=(char*)malloc((getRecordSize(schema)));
	return RC_OK;
}

//free the data of the record
extern RC freeRecord (Record *record)
{
	free(record->data);
	free(record);
	return RC_OK;
}

//get attribute of the value passed
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	*value=(Value*)malloc(sizeof(value));
	int off;
	char *data;
	attrOffset(schema, attrNum, &off);
	data=record->data+off;
	(*value)->dt=schema->dataTypes[attrNum];
	if(schema->dataTypes[attrNum]==DT_INT)
	{
		memcpy(&((*value)->v.intV), data, sizeof(int));
	}
	else if(schema->dataTypes[attrNum]==DT_STRING)
	{
		int length;
		char *str;
		length=schema->typeLength[attrNum];
		str=(char*)malloc(length+1);
		strncpy(str, data, length);
		str[length]='\0';
		(*value)->v.stringV=str;

	}
	else if(schema->dataTypes[attrNum]==DT_FLOAT)
	{
		memcpy(&((*value)->v.floatV), data, sizeof(float));
	}
	else if(schema->dataTypes[attrNum]==DT_BOOL)
	{
		memcpy(&((*value)->v.boolV), data, sizeof(bool));
	}
	return RC_OK;
}

//set the attribute
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	int off;
	char *data;

	attrOffset(schema, attrNum, &off);
	data=record->data+off;
	if(schema->dataTypes[attrNum]==DT_INT)
	{
		memcpy(data, &(value->v.intV), sizeof(int));
	}
	else if(schema->dataTypes[attrNum]==DT_STRING)
	{
		int length;
		char *str;
		length=schema->typeLength[attrNum];
		str=(char*)malloc(length);
		str=value->v.stringV;
		memcpy(data, (str), length);

	}
	else if(schema->dataTypes[attrNum]==DT_FLOAT)
	{
		memcpy(data, &((value->v.floatV)), sizeof(float));
	}
	else if(schema->dataTypes[attrNum]==DT_BOOL)
	{
		memcpy(data, &((value->v.boolV)), sizeof(bool));
	}
	return RC_OK;
}
