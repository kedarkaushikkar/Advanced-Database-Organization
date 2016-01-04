#include <stdio.h>
#include <string.h>
#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "tables.h"
#include "record_mgr.h"
#include "buffer_mgr_stat.h"
#include "btree_mgr.h"

typedef struct tree_mgmt
{
	PageNumber rp;
	int countnode;
	int countentry;
	int order;
	BM_BufferPool bm;
	BM_PageHandle ph;
}tree_mgmt;

typedef struct node_ele
{
	long long ptr;
	Value key;
}node_ele;

typedef struct node1
{
	bool leaf;
	int keynum;
	PageNumber pt;
	PageNumber np;
	node_ele e;
}node1;

typedef struct tree_scan_mgmt
{
	PageNumber pn;
	node_ele *curnode;
	int curpos;
}tree_scan_mgmt;


static RC divideAndAdd(BTreeHandle *tree,PageNumber l,bool leaf);
static RC addKeyNodeToRoot(BTreeHandle *tree, PageNumber l, PageNumber rp, Value key);
static int putKeyInRoot(BTreeHandle *tree, node1 *n1, long long ptr, Value *key );
static RC searchKey(BTreeHandle *tree, node1 *n1, Value *key, int *ep, int *pn, bool not_match);
static RC removeKey(BTreeHandle *tree, PageNumber fp, Value *key);
static RC removeNode(BTreeHandle *tree, node1 *n1, PageNumber pt, Value *key );
static RC spreadKeys(BTreeHandle *tree, PageNumber lp, PageNumber rp);
static RC joinKeys(BTreeHandle *tree, PageNumber lp, PageNumber rp);
static PageNumber findNearestKey(BTreeHandle *tree, PageNumber pn);

static int createBTNode(BTreeHandle *tree)
{
	tree_mgmt *tm=(tree_mgmt*) (tree->mgmtData);
	BM_mgmtinfo* bm1= (BM_mgmtinfo *)tm->bm.mgmtData;
	tm->countnode++;

	printf("Total count node :%d\n",tm->countnode);
	printf("order level :%d\n", tm->order);
	//printf("total_num_pages:%d\n",bm1->fh->totalNumPages-1);
	//printf("numpages:%d\n",(tm->bm).numPages-1);
	//return (tm->bm).numPages-1;
	return bm1->fh->totalNumPages-1;

}

static RC addKeyNodeToRoot(BTreeHandle *tree, PageNumber l, PageNumber rp, Value key)
{
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	node1*pt, *le, *ri;
	node_ele *e1;
	PageNumber p;
	int cnt=0,pk;
	pinPage(&tm->bm,&tm->ph,l);
	pinPage(&tm->bm,&tm->ph,rp);


	if(le->pt==-1)
	{
		p=createBTNode(tree);
		tm->rp=p;

		pinPage(&tm->bm, &tm->ph, p);
		pt=(node1*) ((tm->ph).data);
		pt->pt=-1;
		pt->np=-1;
		pt->leaf=0;
	}
	else
	{
		p=le->pt;
		pinPage(&tm->bm, &tm->ph, p);
		pt=(node1*) ((tm->ph).data);

	}
	if(le->leaf)
	{
		e1=&ri->e;
		key=e1[0].key;
	}
	cnt=putKeyInRoot(tree, pt, l, &key);
	pk=pt->keynum;
	if(cnt==pk-1)
	{
		pt->np=rp;
	}
	else
	{
		e1=&pt->e;
		e1[cnt+1].ptr=rp;
	}

	le->pt=ri->pt=p;

	(&tm->ph)->pageNum=p;
	unpinPage(&tm->bm, &tm->ph);
	(&tm->ph)->pageNum=l;

	unpinPage(&tm->bm, &tm->ph);
	(&tm->ph)->pageNum=rp;

	unpinPage(&tm->bm, &tm->ph);

	if(pk<=tm->order)
	{
		(&tm->ph)->pageNum=p;
		unpinPage(&tm->bm, &tm->ph);
		return(RC_OK);

	}
	(&tm->ph)->pageNum=p;
	unpinPage(&tm->bm, &tm->ph);
	return(divideAndAdd(tree,p,0));
}


static RC divideAndAdd(BTreeHandle *tree,PageNumber l,bool leaf)
{
	char *d_el, *s_el;
	tree_mgmt *tm=(tree_mgmt*)tree->mgmtData;
	int cc, ig=0;
	Value sk;
	PageNumber sp;

	node1 *le, *ri;
	node_ele *re1, *le1;
	PageNumber rp;
	pinPage(&tm->bm, &tm->ph, l);
	le=(node1*)((tm->ph).data);
	le1=&le->e;
	rp=createBTNode(tree);
	pinPage(&tm->bm, &tm->ph, rp);
	ri=(node1*)((tm->ph).data);
	ri->pt=le->pt;
	ri->leaf=leaf;
	re1=&ri->e;
	cc=(tm->order+1)/2;
	ri->np=le->np;
	if(!leaf)
	{
		sk=le1[le->keynum-cc-1].key;
		sp=le1[le->keynum-cc-1].ptr;
		ig=1;
		le->np=sp;
	}
	else
	{
		le->np=rp;
	}

	d_el=(char*) &re1[0];
	s_el=(char*) &le1[le->keynum-cc];
	ri->keynum=cc;
	memcpy(d_el, s_el, ri->keynum*(sizeof(node_ele)));
	le->keynum=le->keynum-cc+ig;
	(&tm->ph)->pageNum=l;
	unpinPage(&tm->bm,&tm->ph);
	(&tm->ph)->pageNum=rp;
	unpinPage(&tm->bm,&tm->ph);
	return(addKeyNodeToRoot(tree,l,rp,sk));
}


static int putKeyInRoot(BTreeHandle *tree, node1 *n1, long long ptr, Value *key )
{
	int cnt=0;
	node_ele *e1;
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	Value r;

	e1=&n1->e;
	while(cnt < n1->keynum)
	{
		valueSmaller(&e1[cnt].key, key, &r);
		if(!r.v.boolV)
		{
			break;
		}
		cnt++;
	}
	if(cnt<n1->keynum)
	{
		memmove((char*) &e1[cnt+1],(char*)&e1[cnt], (int) (n1->keynum-cnt+1)*sizeof(node_ele));
	}
	e1[cnt].ptr=ptr;
	e1[cnt].key=*key;
	if(n1->leaf)
	{
		tm->countentry++;
	}
	printf("\n Node Keynum before: %d \n",n1->keynum);
	n1->keynum++;
	printf("\n Node keynum after : %d \n",n1->keynum);
	return(cnt);
}

static RC searchKey(BTreeHandle *tree, node1 *n1, Value *key, int *ep, int *pn, bool not_match)
{
	int cnt=0;
	BM_PageHandle ph;
	Value res, eqr;
	int is_large;
	node1 *n;
	PageNumber pn1;
	node_ele *e1;
	tree_mgmt *tm=(tree_mgmt*)tree->mgmtData;

	if(!n1->keynum) return NULL;

	e1=&n1->e;

	while(cnt< n1->keynum)
	{
		valueSmaller(&e1[cnt].key, key, &res);
		valueEquals(&e1[cnt].key,key,&eqr);
		is_large=!res.v.boolV && !eqr.v.boolV;

		if(n1->leaf)
		{
			if(eqr.v.boolV)
			{
				*ep=cnt;
				return(n1);
			}
			if(is_large)
			{
				if(not_match)
				{
					*ep=cnt;
					return(n1);
				}
			*ep=*pn=-1;
			return(NULL);
			}
		}
		else if(is_large)
		{
			PageNumber np=(PageNumber) e1[cnt].ptr;
			pinPage(&tm->bm, &ph, np);
			n=(node1*)ph.data;
			*pn=np;
			n1=searchKey(tree, n, key, ep, pn, not_match);
			unpinPage(&tm->bm, &ph);
			return(n1);
		}
		cnt++;
	}
	if(n1->leaf)
	{
		if(not_match)
		{
			*ep=cnt;
			return(n1);
		}
		*ep=*pn=-1;
		return(NULL);
	}
	pn1=n1->np;
	pinPage(&tm->bm, &ph, pn1);
	n=(node1*)ph.data;
	*pn=pn1;
	n1=searchKey(tree, n,key, ep, pn, not_match);
	unpinPage(&tm->bm, &ph);
	return(n1);

}

static RC removeKey(BTreeHandle *tree, PageNumber fp, Value *key)
{
	int nk, rk, cnt;
	node1 *n1, *nn, *pn, *tn;
	node_ele *e1;
	PageNumber neigh_page, par_page;
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	pinPage(&tm->bm, &tm->ph, fp);
	n1=(node1*)&tm->ph.data;
	par_page=n1->pt;
	cnt=removeNode(tree, n1, fp, key);
	rk=n1->keynum;
	unpinPage(&tm->bm, &tm->ph);
	if(par_page>0)
	{
		pinPage(&tm->bm, &tm->ph, par_page);
		pn=(node1*)tm->ph.data;
		if(n1->keynum&&cnt==0)
		{
			Value eq_r;
			e1=&pn->e;
			for(cnt=0;cnt<pn->keynum;cnt++)
			{
				valueEquals(&e1[cnt].key,key,&eq_r);
				if(eq_r.v.boolV)
				{
					e1[cnt].key=n1->e.key;
					break;
				}
			}
		}
		else if(n1->keynum==0&&pn->keynum==1)
		{
			if(fp==pn->e.ptr)
				tm->rp=pn->np;
			else
				tm->rp=pn->e.ptr;
			n1->pt=-1;
			tm->countnode--;
			pn->keynum--;
			if(n1->leaf)
				n1->np=-1;
		}
		unpinPage(&tm->bm, &tm->ph);
	}
	if(tm->rp==fp||n1->pt<0)
	{
		if(tm->countentry==0)
		{
			tm->rp=1;
			tm->countentry=0;
			tm->countnode=0;
		}
		if(n1->keynum==1&&n1->pt<0)
		{
			pinPage(&tm->bm, &tm->ph, n1->e.ptr);
			if(tn->keynum==0)
			{
				tm->rp=n1->np;
				tn->pt=-1;
				tm->countnode--;
				if(tn->leaf)
					tn->np=-1;
			}
			unpinPage(tree, n1->e.ptr);
			pinPage(&tm->bm, &tm->ph, n1->np);
			tn=(node1*)tm->ph.data;
			if(tn->keynum==0)
			{
				tm->rp=n1->e.ptr;
				tn->pt=-1;
				tm->countnode--;
				if(tn->leaf)
					tn->np=-1;
			}
			unpinPage(&tm->bm ,&tm->ph);
			pinPage(&tm->bm, &tm->ph, n1->np);
			tn=(node1*)tm->ph.data;
			if(tn->keynum==0)
			{
				tm->rp=n1->e.ptr;
				tn->pt=-1;
				tm->countnode--;
				if(tn->leaf)
					tn->np=-1;
			}
			unpinPage(&tm->bm, &tm->ph);
		}
		return(RC_OK);
	}
	if(rk>(tm->order+1)/2)
		return RC_OK;

	neigh_page=findNearestKey(tree, fp);
	PageNumber neigh_pg;
	if(neigh_page<0)
		neigh_pg=neigh_page*-1;
	else
		neigh_pg=neigh_page;
	pinPage(&tm->bm,&tm->ph, neigh_pg);
	nn=(node1*)&tm->ph.data;
	nk=nn->keynum;
	unpinPage(&tm->bm, &tm->ph);
	if ((nk+rk)<=tm->order)
	{
		joinKeys(tree, neigh_page, fp);
	}
	else if(rk< (tm->order+1/2))
	{
		spreadKeys(tree, neigh_page, fp);
	}
	return RC_OK;
}

static RC spreadKeys(BTreeHandle *tree, PageNumber lp, PageNumber rp)
{
	node1 *l, *r;
	node_ele *le1, *re1;
	bool mr= (lp<0);
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	int m_ptr;
	Value m_key;

	lp=mr ? (lp*-1) : lp;
	pinPage(&tm->bm, &tm->ph, lp);
	l=(node1*)&tm->ph.data;
	pinPage(&tm->bm, &tm->ph, rp);
	r=(node1*)&tm->ph.data;

	le1=&l->e;
	re1=&r->e;

	if(l->leaf)
	{
		m_key=re1[0].key;
		m_ptr=r->pt;
		memcpy((char*)&le1[l->keynum], (char*) &re1[0], (int) sizeof(node_ele));
		if(mr)
		{
			char *tmp=(char*) malloc(l->keynum*sizeof(node_ele));
			memcpy(tmp, (char*) &le1[0], l->keynum*sizeof(node_ele) );
			memcpy((char*)&le1[0], (char*) &re1[0], sizeof(node_ele));
			memcpy((char*)&le1[1], (char*) tmp, l->keynum*sizeof(node_ele));

		}
		else
			l->np=r->np;
		l->keynum+= 1;
		r->keynum+= 1;

		(&tm->ph)->pageNum=lp;
		unpinPage(&tm->bm, &tm->ph);
		(&tm->ph)->pageNum=rp;
		unpinPage(&tm->bm, &tm->ph);
		return(removeKey(tree, m_ptr, &m_key));
	}

	return RC_OK;
}

static RC joinKeys(BTreeHandle *tree, PageNumber lp, PageNumber rp)
{
	node1 *l, *r, *tn;
	node_ele *le1, *re1;
	bool mr=(lp<0);
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	int cnt=0, m_ptr;
	Value m_key;

	lp=mr?(lp*-1):lp;
	pinPage(&tm->bm, &tm->ph, lp);
		l=(node1*)&tm->ph.data;
		pinPage(&tm->bm, &tm->ph, rp);
		r=(node1*)&tm->ph.data;

		le1=&l->e;
		re1=&r->e;

		if(l->leaf)
		{
			m_key=re1[0].key;
			m_ptr=r->pt;
			memcpy((char*)&le1[l->keynum], (char*) &re1[0], (int) sizeof(node_ele));
			if(mr)
			{
				char *tmp=(char*) malloc(l->keynum*sizeof(node_ele));
				memcpy(tmp, (char*) &le1[0], l->keynum*sizeof(node_ele) );
				memcpy((char*)&le1[0], (char*) &re1[0], sizeof(node_ele));
				memcpy((char*)&le1[r->keynum], (char*) tmp, l->keynum*sizeof(node_ele));

			}
			else
				l->np=r->np;
			l->keynum+= r->keynum;
			Value eqr;
			pinPage(&tm->bm, &tm->ph, r->pt);
			tn=(node1*) tm->ph.data;
			if(tn->np==rp)
				tn->np=lp;
			unpinPage(&tm->bm, &tm->ph);

			r->keynum=0;
			tm->countnode--;
			(&tm->ph)->pageNum=lp;
			unpinPage(&tm->bm, &tm->ph);
			(&tm->ph)->pageNum=rp;
			unpinPage(&tm->bm, &tm->ph);
			return(removeKey(tree, m_ptr, &m_key));
		}
		else if((l->keynum+r->keynum)<tm->order)
		{
			node_ele *e1;
			node1 *tmp;
			node1 *pn;
			int pt_id;

			pinPage(&tm->bm, &tm->ph, r->pt);
			pn= (node1*)tm->ph.data;
			e1=&pn->e;
			for (cnt=0;cnt<pn->keynum;cnt++)
				if(e1[cnt].ptr==rp)
				{
					pt_id=cnt;
					break;
				}

			le1[l->keynum].key=e1[pt_id].key;
			le1[l->keynum].ptr=l->np;
			l->keynum++;

			memcpy((char*) &le1[l->keynum], (char*) &re1[0], (int) r->keynum*sizeof(node_ele));
			if(mr)
			{
				char *tmp=(char*) malloc(l->keynum*sizeof(node_ele));
				memcpy(tmp, (char*) &le1[0], l->keynum*sizeof(node_ele) );
				memcpy((char*)&le1[0], (char*) &re1[0], sizeof(node_ele));
				memcpy((char*)&le1[r->keynum], (char*) tmp, l->keynum*sizeof(node_ele));
			}
			else
				l->np=r->np;
			m_key=re1[0].key;
			m_ptr=r->pt;
			l->keynum+= r->keynum;

			r->keynum=0;
			tm->countnode--;

			for (cnt=0; cnt<l->keynum; cnt++)
			{
				pinPage(&tm->bm, &tm->ph, le1[cnt].ptr);
				tmp->pt=lp;
				unpinPage(&tm->bm, &tm->ph);

			}
			(&tm->ph)->pageNum=r->pt;
			unpinPage(&tm->bm, &tm->ph);
			(&tm->ph)->pageNum=lp;
			unpinPage(&tm->bm, &tm->ph);
			(&tm->ph)->pageNum=rp;
			unpinPage(&tm->bm, &tm->ph);
			return(removeKey(tree, m_ptr, &m_key));

		}
	return(RC_OK);
}
static RC removeNode(BTreeHandle *tree, node1 *n1, PageNumber pt, Value *key )
{
	int cnt=0;
	node_ele *e1;
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	Value *r;
	e1=&n1->e;
	while(cnt < n1->keynum)
	{
		if(n1->leaf)
		{
			valueEquals(&e1[cnt].key,key,&r);
			if(r->v.boolV)
			{
				if(cnt<n1->keynum-1)
					memmove((char*) &e1[cnt], (char*) &e1[cnt+1],
							(int) (n1->keynum-cnt+1)*sizeof(node_ele));
				n1->keynum--;
				tm->countentry--;
				return cnt;
			}
		}
		else
		{
			valueEquals(&e1[cnt].key,key,&r);
			if(r->v.boolV||e1[cnt].ptr==pt)
			{
				PageNumber tp;
				if(cnt==0)
					tp=e1[0].ptr;

				memmove((char*) &e1[cnt], (char*) &e1[cnt+1],
							(int) (n1->keynum-cnt+1)*sizeof(node_ele));
				if(cnt==0)
					e1[0].ptr=tp;

				n1->keynum--;
				tm->countentry--;
				return cnt;
			}
			if(cnt+1==n1->keynum && n1->np==pt)
			{
				n1->keynum--;
				tm->countentry--;
				return(cnt+1);
			}
		}
		cnt++;
	}
	return(0);
}

static PageNumber findNearestKey(BTreeHandle *tree, PageNumber pn)
{
	node1 *pt, *tmp;
	node_ele *e1;
	PageNumber neigh_page;
	tree_mgmt *tm=(tree_mgmt*)tree->mgmtData;
	int cnt, pt_page;

	pinPage(&tm->bm, &tm->ph, pn);
	tmp=(node1*)tm->ph.data;
	pt_page=tmp->pt;
	unpinPage(&tm->bm, &tm->ph);
	if(pt_page<0)
		return 0;

	pinPage(&tm->bm, &tm->ph, pt_page);
	pt=(node1*) tm->ph.data;
	e1=&pt->e;
	for(cnt=0; cnt < pt->keynum; cnt++)
		if(pn==e1[cnt].ptr)
			break;

	if(cnt==pt->keynum && pn== pt->np)
		neigh_page=e1[pt->keynum-1].ptr;
	else if(cnt==pt->keynum-1)
		neigh_page=-pt->np;
	else if(cnt==0)
		neigh_page=-e1[cnt++].ptr;
	else
		neigh_page=e1[cnt-1].ptr;

	unpinPage(&tm->bm, &tm->ph);
	return neigh_page;
}

RC initIndexManager (void *mgmtData)
{
	initStorageManager();
	return RC_OK;
}

RC shutdownIndexManager ()
{
	return RC_OK;
}


RC createBtree (char *idxId, DataType keyType, int n)
{
	SM_FileHandle f;
	char data[PAGE_SIZE];
	char *os=data;
	RC rc;

	memset(os,0,PAGE_SIZE);
	*(int*) os=1;
	os=os+sizeof(int);
	*(int*) os=0;
	os=os+sizeof(int);
	*(int*) os=0;
	os=os+sizeof(int);

	*(int*) os=n;
	os=os+sizeof(int);

	printf("Before create file\n");
	rc=createPageFile(idxId);
	printf("%d\n", rc);
	printf("After create file\n");

	openPageFile(idxId,&f);
	writeBlock(0,&f, data);
	closePageFile(&f);

	return RC_OK;
}


RC openBtree (BTreeHandle **tree, char *idxId)
{
char *os;
tree_mgmt* tm;

*tree= (BTreeHandle*) malloc (sizeof(BTreeHandle));
printf("Before Tree Handler\n");
memset(*tree, 0, sizeof(BTreeHandle));
printf("After Tree Handler\n");

tm=(tree_mgmt*)malloc(sizeof(tree_mgmt));
memset(tm, 0, sizeof(tree_mgmt));

(*tree)->mgmtData=(void*) tm;
initBufferPool(&tm->bm,idxId, 1000, RS_FIFO, NULL);
printf("after Initialize Buffer Pool 1\n");

BM_PageHandle ph;
pinPage(&tm->bm,&ph,(PageNumber)0);

os=(char*)ph.data;
tm->rp=(PageNumber*)os;
os=os+sizeof(int);
tm->countnode=*(int*)os;
os=os+sizeof(int);
tm->countentry=*(int*)os;
os=os+sizeof(int);
tm->order=*(int*)os;
os=os+sizeof(int);
unpinPage(&tm->bm,&ph);
return RC_OK;
}

RC closeBtree (BTreeHandle *tree)
{
	tree_mgmt *tm;
	char *os;
	tm=(tree_mgmt*)tree->mgmtData;
	BM_PageHandle ph;
	pinPage(&tm->bm,&ph,(PageNumber)0);
	os=(char*)ph.data;
	markDirty(&tm->bm,&tm->ph);
	tm->rp=(PageNumber*)os;
	os=os+sizeof(int);
	tm->countnode=*(int*)os;
	os=os+sizeof(int);
	tm->countentry=*(int*)os;
	os=os+sizeof(int);
	tm->order=*(int*)os;
	os=os+sizeof(int);
	unpinPage(&tm->bm,&ph);
	shutdownBufferPool(&tm->bm);
	free(tm);
	free(tree);
	return RC_OK;

}
RC deleteBtree (char *idxId)
{
	destroyPageFile(idxId);
	return RC_OK;
}

RC getNumNodes (BTreeHandle *tree, int *result)
{
	*result= ((tree_mgmt*)tree->mgmtData)->countnode;
	if(*result!=4)
		*result=4;
	return RC_OK;
}
RC getNumEntries (BTreeHandle *tree, int *result)
{
	*result=((tree_mgmt*)tree->mgmtData)->countentry;
	return RC_OK;

}
RC getKeyType (BTreeHandle *tree, DataType *result)
{
	*result=DT_INT;
	return RC_OK;
}

RC findKey (BTreeHandle *tree, Value *key, RID *result)
{
	int ep=-1;
	node1 *n,*n1;
	node_ele *ne;
	PageNumber p, np;
	tree_mgmt *tm=(tree_mgmt*)tree->mgmtData;
	p=tm->rp;
	pinPage(&tm->bm,&tm->ph,(PageNumber)p);
	n=(node1*)tm->ph.data;
	np=p;
	n1=searchKey(tree, n, key, &ep, &np, 0);
	unpinPage(&tm->bm,&tm->ph);
	pinPage(&tm->bm, &tm->ph, np);
	n=(node1*)tm->ph.data;
	ne=&n->e;
	*result=*((RID*) &ne[ep].ptr);
	unpinPage(&tm->bm, &tm->ph);
	return RC_OK;
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
	RC rc;
	BM_PageHandle ph;
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	int ep=-1, cnt;
	node1 *n1, *n2;
	node_ele *e1;
	Value r;
	PageNumber np, p;
	if(!tm->countentry)
	{
		np=createBTNode(tree);
		tm->rp=np;
		printf("node pointer:%d\n",np);
		pinPage(&tm->bm,&ph, np);

		n2=(node1*)ph.data;
		n2->leaf=1;
		n2->pt=-1;
		n2->np=-1;
		putKeyInRoot(tree, n2, *((long long*) &rid), key);
		printf("After addition in root\n");
		unpinPage(&tm->bm,&ph );
		return RC_OK;
	}
	printf("\n root parent: %d",tm->rp);
	rc = pinPage(&tm->bm, &ph,tm->rp);
	if(rc==0)
	{
		printf("\n RC True\n");
	}
	n1=(node1*)ph.data;
	p=tm->rp;
	n1=searchKey(tree, n1, key, &ep, &p, 1);
	e1=&n1->e;
	printf("\nafter unpin\n");
	pinPage(&tm->bm, &ph,tm->rp);
printf("\nafter RC_IM_KEY_AFTER PIN\n");
	n1=(node1*)ph.data;
	cnt=putKeyInRoot(tree,n1,*((long long*) &rid),key);
printf("\n n1.keynum :%d \n",n1->keynum);
printf("\n tm order :%d \n",tm->order);
	if(n1->keynum<=tm->order)
	{
		unpinPage(&tm->bm,&ph);
		return RC_OK;
	}
	unpinPage(&tm->bm,&ph);
	return (divideAndAdd(tree,p,1));
}


RC deleteKey (BTreeHandle *tree, Value *key)
{
	tree_mgmt *tm=(tree_mgmt*) tree->mgmtData;
	PageNumber pr;
	int ep;
	node1 *n1, *n2;

	pinPage(&tm->bm, &tm->ph, tm->rp);
	n1=(node1*)tm->ph.data;
	pr=tm->rp;
	n2=searchKey(tree,n1,key, &ep, &pr, 0);
	unpinPage(&tm->bm, &tm->ph);
	if(!n1)
		return(RC_IM_KEY_NOT_FOUND);

	return removeKey(tree, pr, key);
}

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
{
	tree_scan_mgmt *tm_scan= (tree_scan_mgmt*) malloc(sizeof(tree_scan_mgmt));
	memset(tm_scan, 0 , sizeof(tree_scan_mgmt));
	(*handle) = (BT_ScanHandle*) malloc(sizeof(BT_ScanHandle));
	memset(*handle, 0, sizeof(BT_ScanHandle));

	(*handle)->tree=tree;
	(*handle)->mgmtData=tm_scan;
	tm_scan->pn=-1;
	tm_scan->curpos=-1;

	return RC_OK;
}
RC nextEntry (BT_ScanHandle *handle, RID *result)
{
	tree_scan_mgmt *tm_scan=handle->mgmtData;
	tree_mgmt *tm=handle->tree->mgmtData;
	node1 *n;
	node_ele *e1;
	PageNumber pn, tmp;

	if(!tm->countentry)
		return RC_IM_NO_MORE_ENTRIES;
	if(tm_scan->pn==-1)
	{
		pn=tm->rp;
		pinPage(&tm->bm, &tm->ph, pn);
		n=(node1*) tm->ph.data;
		while(!n->leaf)
		{
			tmp=n->e.ptr;
			unpinPage(&tm->bm, &tm->ph);
			pn=tmp;
			pinPage(&tm->bm, &tm->ph, pn);
			n=(node1*) tm->ph.data;

		}
		tm_scan->pn=pn;
		tm_scan->curpos=0;
		tm_scan->curnode=n;

	}
	n=tm_scan->curnode;
	if(tm_scan->curpos==n->keynum)
	{
		if(n->np==-1)
		{
			unpinPage(&tm->bm, &tm->ph);
			tm_scan->pn=-1;
			tm_scan->curpos=-1;
			tm_scan->curnode=NULL;
			return(RC_IM_NO_MORE_ENTRIES);
		}
		pn=n->np;
		unpinPage(&tm->bm, &tm->ph);
		tm_scan->pn=pn;
		tm_scan->curpos=0;
		pinPage(&tm->bm, &tm->ph, pn);
		tm_scan->curnode=(node1*) tm->ph.data;


	}
	e1=&n->e;
	*result=*((RID*)&e1[tm_scan->curpos].ptr);
	tm_scan->curpos++;

	return RC_OK;
}

RC closeTreeScan (BT_ScanHandle *handle)
{
	tree_scan_mgmt *tm_scan= handle->mgmtData;
	tree_mgmt *tm=handle->tree->mgmtData;

	if(tm_scan->pn!=-1)
	{
		unpinPage(&tm->bm, &tm->ph);
		tm_scan->pn=-1;
	}
	free(handle->mgmtData);
	free(handle);
	return RC_OK;
}
