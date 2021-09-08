#include "dat.h"
#include "fns.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FNV1APRIME  16777619
#define FNV1AOFFSET 2166136261

#define RESIDENT 1
#define EMPTY	 0

#define HASH(HT, K) (HT)->hf((K)) % (HT)->capacity

unsigned int
robotmiller()
{
	return 0;
}

unsigned int
fnv1a(unsigned int k)
{
	char *b, *i;
	unsigned int hash;

	hash = FNV1AOFFSET;
	b = (char *)&k;

	for(i = b; i < b + sizeof(unsigned int); i++){
		hash *= FNV1APRIME;
		hash ^= *i;
	}

	return hash;
}

struct ht *
htinit(int hf)
{
	struct ht *ht;

	ht = calloc(1, sizeof(struct ht));
	FAILIF(!ht, "Failed to create hash table", fail);

	switch(hf){
	case FNV1AHF: ht->hf = fnv1a; break;
	case ROBOTMILLERHF: ht->hf = robotmiller; break;
	default: LOG(ERROR, "Invalid hash funct4ion specified"); goto htafail;
	}

	ht->capacity = INITCAP;

	ht->nents = 0;
	ht->keys = calloc(INITCAP, sizeof(unsigned int));
	FAILIF(!ht->keys, "Failed to calloc keys array", htafail);

	ht->reslist = calloc(INITCAP, sizeof(unsigned char));
	FAILIF(!ht->reslist, "Failed to calloc resident indices list", htkfail);

	ht->vals = calloc(INITCAP, sizeof(void *));
	FAILIF(!ht->vals, "Failed to calloc vals array", htrfail);

	return ht;

htrfail:
	free(ht->reslist);
htkfail:
	free(ht->keys);
htafail:
	free(ht);
fail:
	return NULL;
}

void
htdeinit(struct ht *ht)
{
	free(ht->keys);
	free(ht->vals);
	free(ht->reslist);
	free(ht);
}

int
doinsertion(struct ht *ht, unsigned int k, void *v)
{
	unsigned int slot, ogslot;

	ogslot = slot = HASH(ht, k);
	while(ht->reslist[slot] == RESIDENT){
		slot = (slot + 1) % ht->capacity;
		RETIF(slot == ogslot, "Hash table load is 1", -1);
	}

	ht->nents++;
	ht->keys[slot] = k;
	ht->reslist[slot] = RESIDENT;
	ht->vals[slot] = v;
	return 0;
}

int
dodeletion(struct ht *ht, unsigned int k)
{
	unsigned int slot, ogslot;

	ogslot = slot = HASH(ht, k);
	while(!(ht->reslist[slot] == RESIDENT && ht->keys[slot] == k)){
		slot = (slot + 1) % ht->capacity;
		RETIF(slot == ogslot, "Deleted key that was never inserted",
		      -1);
	}

	ht->nents--;
	ht->reslist[slot] = EMPTY;
	ht->keys[slot] = 0;
	ht->vals[slot] = NULL;
	return 0;
}

int
resizerehash(struct ht *ht, int delta)
{
	unsigned int i;
	double lf;
	unsigned char *newreslist;
	unsigned int newcap, *newkeys;
	void **newvals;

	lf = ((double)ht->nents + (double)delta) / (double)ht->capacity;

	if(lf < MAXLF && lf > MINLF) goto end;
	else if(lf >= MAXLF)
		newcap = ht->capacity * 2;
	else
		newcap = ht->capacity / 2;

	if(lf <= MINLF && newcap < 5) goto end;

	newkeys = calloc(newcap, sizeof(unsigned int));
	FAILIF(!newkeys, "Failed to calloc new keys array during resize", end);
	newvals = calloc(newcap, sizeof(void *));
	FAILIF(!newvals, "Failed to calloc new value array during resize",
	       nkfail);
	newreslist = calloc(newcap, sizeof(unsigned char));
	FAILIF(!newreslist,
	       "Failed to calloc new residence array during resize", nvfail);

	for(i = 0; i < ht->capacity; i++){
		unsigned int key, nslot, ognslot;
		void *val;

		if(ht->reslist[i] == EMPTY) continue;
		key = ht->keys[i];
		val = ht->vals[i];

		ognslot = nslot = ht->hf(key) % newcap;
		while(newreslist[nslot] == RESIDENT){
			nslot = (nslot + 1) % newcap;
			FAILIF(nslot == ognslot,
			       "New hash table reached full load during resize",
			       nrfail);
		}

		newkeys[nslot] = key;
		newreslist[nslot] = RESIDENT;
		newvals[nslot] = val;
	}

	free(ht->keys);
	free(ht->reslist);
	free(ht->vals);

	ht->keys = newkeys;
	ht->reslist = newreslist;
	ht->vals = newvals;
	ht->capacity = newcap;

	return 0;

nrfail:
	free(newreslist);
nvfail:
	free(newvals);
nkfail:
	free(newkeys);
end:
	return -1;
}

int
htinsert(struct ht *ht, unsigned int k, void *v)
{
	RETIF(resizerehash(ht, 1) < 0, "Resizing hash table failed", -1);
	RETIF(doinsertion(ht, k, v) < 0, "Insertion of new item failed", -1);
	return 0;
}

void *
htget(struct ht *ht, unsigned int k)
{
	void *out;
	unsigned int slot, ogslot;

	out = NULL;
	ogslot = slot = HASH(ht, k);

	while(!(ht->reslist[slot] == RESIDENT && ht->keys[slot] == k)){
		slot = (slot + 1) % ht->capacity;
		if(slot == ogslot) goto done;
	}

	out = ht->vals[slot];
done:
	return out;
}

void *
htdel(struct ht *ht, unsigned int k)
{
	void *out;

	out = NULL;
	FAILIF(!(out = htget(ht, k)), "Getting item to be deleted failed",
	       done);
	FAILIF(resizerehash(ht, -1) < 0, "Resizing hash table failed", done);
	FAILIF(dodeletion(ht, k) < 0, "Performing hash table deletion failed",
	       done);
done:
	return out;
}
