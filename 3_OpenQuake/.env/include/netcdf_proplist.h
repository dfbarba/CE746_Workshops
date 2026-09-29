/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/

#ifndef NETCDF_PROPLIST_H
#define NETCDF_PROPLIST_H

#ifndef OPTEXPORT
#ifdef NETCDF_PROPLIST_H
#define OPTEXPORT static
#else /*!NETCDF_PROPLIST_H*/
#ifdef _WIN32
#define OPTEXPORT __declspec(dllexport)
#else
#define OPTEXPORT extern
#endif
#endif /*NETCDF_PROPLIST_H*/
#endif /*OPTEXPORT*/

/**************************************************/
/*
This is used to store a property list mapping a small number of
keys to objects. The uintptr_t type is used to ensure that the value can be a pointer or a
small string upto sizeof(uintptr_t) - 1 (for trailing nul) or an integer constant.

There are two operations that may be defined for the property:
1. reclaiming the value when proplist is free'd and property value points to allocated data of arbitrary complexity.
2. coping the value (for cloning) if it points to allocated data of arbitrary complexity.

The fact that the number of keys is small makes it feasible to use
linear search.  This is currently only used for plugins, but may be
extended to other uses.
*/

/*! Proplist-related structs.
  NOTES:
  1. 'value' is the an arbitrary uintptr_t integer or void* pointer.

  WARNINGS:
  1. It is critical that |uintptr_t| == |void*|
*/

#define NCPROPSMAXKEY 31 /* characters; assert (NCPROPSMAXKEY+1)/8 == 0*/

/* Opaque forward */
struct NCPpair;

/* This function performs all of the following operations on a complex type */
typedef enum NCPtypeop {NCP_RECLAIM=1,NCP_COPY=2} NCPtypeop;

/* There are three possible types for a property value */
typedef enum NCPtype {
	NCP_CONST=0,  /* Value is a simple uintptr_t constant */
	NCP_BYTES=2,  /* Value points to a counted sequence of bytes; If a string,
			then it includes the nul term character */
	NCP_COMPLEX=3 /* Value points to an arbitrarily complex structure */
} NCPtype;

/* (Returns < 0 => error) (>= 0 => success) */
typedef int (*NCPtypefcn)(NCPtypeop op, struct NCPpair* input, struct NCPpair* output);

/* Expose this prefix of NCProperty; used in clone and lookup */
/* Hold just the key+value pair */
typedef struct NCPpair {
    char key[NCPROPSMAXKEY+1]; /* copy of the key string; +1 for trailing nul */
    NCPtype sort;
    uintptr_t value;
    uintptr_t size;        /* size = |value| as ptr to memory, if string, then include trailing nul */
} NCPpair;

/* The property list proper is a sequence of these objects */
typedef struct NCPproperty {
    NCPpair pair;        /* Allowed by C language standard */
    uintptr_t userdata;  /* extra data for the type function */
    NCPtypefcn typefcn;  /* Process type operations */
} NCPproperty;

/*
The property list object.
*/
typedef struct NCproplist {
  size_t alloc; /* allocated space to hold properties */
  size_t count; /* # of defined properties */
  NCPproperty* properties;
} NCproplist;

/**************************************************/
/* Extended API */

#if defined(_cplusplus_) || defined(__cplusplus__)
extern "C" {
#endif

/* All int valued functions return < 0 if error; >= 0 otherwise */


/* Create, free, etc. */
OPTEXPORT NCproplist* ncproplistnew(void);
OPTEXPORT int ncproplistfree(NCproplist*);

/* Insert properties */
OPTEXPORT int ncproplistadd(NCproplist* plist,const char* key, uintptr_t value); /* use when reclaim not needed */
OPTEXPORT int ncproplistaddstring(NCproplist* plist, const char* key, const char* str); /* use when value is simple string (char*) */

/* Insert an instance of type NCP_BYTES */
OPTEXPORT int ncproplistaddbytes(NCproplist* plist, const char* key, void* value, uintptr_t size);

/* Add instance of a complex type */
OPTEXPORT int ncproplistaddx(NCproplist* plist, const char* key, void* value, uintptr_t size, uintptr_t userdata, NCPtypefcn typefcn);

/* clone; keys are copies and values are copied using the NCPtypefcn */
OPTEXPORT int ncproplistclone(const NCproplist* src, NCproplist* clone);

/* 
Lookup key and return value.
@return ::NC_NOERR if found ::NC_EINVAL otherwise; returns the data in datap if !null
*/
OPTEXPORT int ncproplistget(const NCproplist*, const char* key, uintptr_t* datap, uintptr_t* sizep);

/* Iteration support */

/* Return the number of properties in the property list */
#define ncproplistlen(plist) (((NCproplist)(plist))->count)

/* get the ith key+value */
OPTEXPORT int ncproplistith(const NCproplist*, size_t i, char* const * keyp, uintptr_t const * valuep, uintptr_t* sizep);

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
}
#endif

#endif /*!NETCDF_PROPLIST_H*/ /* WARNING: Do not remove the !; used in building netcdf_proplist.h  */
#ifdef NETCDF_PROPLIST_H
/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "ncdispatch.h"
#include "nccrc.h"


#undef DEBUG
#define ASSERTIONS

#ifdef ASSERTIONS
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

/**************************************************/
/* Hide everything for plugins */
#ifdef NETCDF_PROPLIST_H
#define OPTSTATIC static
#else /*!NETCDF_PROPLIST_H*/
#define OPTSTATIC
#endif /*NETCDF_PROPLIST_H*/

/**************************************************/

#define MINPROPS 2
#define EXPANDFACTOR 1

#define hasspace(plist,nelems) ((plist)->alloc >= ((plist)->count + (nelems)))

#define emptyprop {"                               ",0,0,0,NULL}

/**************************************************/

/* Forward */
static int ncproplistinit(NCproplist* plist);
static int extendplist(NCproplist* plist, size_t nprops);

/* Static'ize everything for plugins */
#ifdef NETCDF_PROPLIST_H
#define OPTSTATIC static
static NCproplist* ncproplistnew(void);
static int ncproplistfree(NCproplist* plist);
static int ncproplistadd(NCproplist* plist, const char* key, uintptr_t value);
static int ncproplistaddbytes(NCproplist* plist, const char* key, void* value, uintptr_t size);
static int ncproplistaddstring(NCproplist* plist, const char* key, const char* str);
static int ncproplistaddx(NCproplist* plist, const char* key, void* value, uintptr_t size, uintptr_t userdata, NCPtypefcn fcn);
static int ncproplistclone(const NCproplist* src, NCproplist* clone);
static int ncproplistget(const NCproplist* plist, const char* key, uintptr_t* valuep, uintptr_t* sizep);
static int ncproplistith(const NCproplist* plist, size_t i, char* const * keyp, uintptr_t const * valuep, uintptr_t* sizep);
#else /*!NETCDF_PROPLIST_H*/
#define OPTSTATIC
#endif /*NETCDF_PROPLIST_H*/


/**
 * Create new empty property list
 * @return pointer to the created property list.
 */
OPTSTATIC NCproplist*
ncproplistnew(void)
{
   NCproplist* plist = NULL;
   plist = calloc(1,sizeof(NCproplist));
   if(ncproplistinit(plist) != NC_NOERR)
       {ncproplistfree(plist); plist = NULL;}
   return plist;
}

/**
 * Reclaim property pairs used by a property list
 * @param plist to clear
 * @return >= 0 if succeed, < 0 if fail
 */
static int
ncproplistclear(NCproplist* plist)
{
    int stat = 0;
    size_t i;
    if(plist == NULL) goto done;
    if(plist->properties != NULL) {
        for(i=0;i<plist->count;i++) {
            NCPproperty* prop = &plist->properties[i];
	    void* ptr = (void*)prop->pair.value; /* convert to ptr */
	    switch (prop->pair.sort) {
	    case NCP_CONST: /* value need not be free'd */
		break;
	    case NCP_BYTES: /* simple free of the value */
		if(ptr != NULL) free(ptr);
		break;
	    case NCP_COMPLEX: /* Need the typeop fcn */
		assert(prop->typefcn != NULL);
		stat = prop->typefcn(NCP_RECLAIM,&prop->pair,NULL);
		if(stat < 0) goto done;
		break;
	    }	
	}
    }
    plist->count = 0;
done:
    return stat;
}

/**
 * Reclaim memory used by a property list
 * @param plist to reclaim
 * @return >= 0 if succeed, < 0 if fail
 */
OPTSTATIC int
ncproplistfree(NCproplist* plist)
{
    int stat = 0;
    if(plist == NULL) goto done;
    if((stat = ncproplistclear(plist))<0) goto done;
    free(plist->properties);
    free(plist);
done:
    return stat;
}

/**
 * Add an NCP_CONST  entry to the property list
 * @param plist into which the value is be inserted.
 * @param key
 * @param value
 * @return >= 0 if succeed, < 0 if fail
 */
OPTSTATIC int
ncproplistadd(NCproplist* plist, const char* key, uintptr_t value)
{
    int stat = NC_NOERR;
    NCPproperty* prop = NULL;
    size_t keylen;
    if(plist == NULL) goto done;
    if(!hasspace(plist,1)) {if((stat = extendplist(plist,(plist->count+1)*EXPANDFACTOR))) goto done;} /* extra space */
    prop = &plist->properties[plist->count];
    keylen = strlen(key);
    if(keylen > NCPROPSMAXKEY) keylen = NCPROPSMAXKEY; /* truncate */
    memcpy(prop->pair.key,key,keylen);
    prop->pair.key[keylen] = '\0';
    prop->pair.value = value;
    prop->pair.sort = NCP_CONST;
    plist->count++;
done:
    return stat;
}
	
/**
 * Add a byte string to the property list.
 * The proplist takes control of the value => do not free.
 * @param plist into which the value is be inserted.
 * @param key
 * @param value ptr to memory chunk
 * @param size |value|
 * @return >= 0 if succeed, < 0 if fail
 */
OPTSTATIC int
ncproplistaddbytes(NCproplist* plist, const char* key, void* value, uintptr_t size)
{
    int stat = NC_NOERR;
    NCPproperty* prop = NULL;
    size_t keylen;

    NC_UNUSED(size);
    if(plist == NULL) goto done;
    if(!hasspace(plist,1)) {if((stat = extendplist(plist,(plist->count+1)*EXPANDFACTOR))) goto done;} /* extra space */
    prop = &plist->properties[plist->count];
    keylen = strlen(key);
    if(keylen > NCPROPSMAXKEY) keylen = NCPROPSMAXKEY; /* truncate */
    memcpy(prop->pair.key,key,keylen);
    prop->pair.key[keylen] = '\0';
    prop->pair.value = (uintptr_t)value;
    prop->pair.sort = NCP_BYTES;
    plist->count++;
done:
    return stat;
}

/**
 * Add a  nul terminated string to the property list.
 * Wraps ncproplistaddbytes.
 * The proplist takes control of the value => do not free.
 * @param plist into which the value is be inserted.
 * @param key
 * @param value ptr to char* string
 * @param size strlen(value)+1
 * @return >= 0 if succeed, < 0 if fail.
 */
OPTSTATIC int
ncproplistaddstring(NCproplist* plist, const char* key, const char* str)
{
    uintptr_t size = 0;
    if(str) size = (uintptr_t)(strlen(str)+1);
    return ncproplistaddbytes(plist,key,(void*)str,size);
}

/**
 * Most general case for adding a property.
 * The value is always a ptr to some arbitrary complex structure.
 * The proplist takes control of the value => do not free.
 * @param plist into which the value is be inserted.
 * @param key
 * @param value
 * @param size
 * @param userdata extra environment data for the reclaim function.
 * @param fcn the type operations function
 * @return >= 0 if succeed, < 0 otherwise.
 */
OPTSTATIC int
ncproplistaddx(NCproplist* plist, const char* key, void* value, uintptr_t size, uintptr_t userdata, NCPtypefcn fcn)
{
    int stat = NC_NOERR;
    NCPproperty* prop = NULL;
    size_t keylen;
    if(plist == NULL) goto done;
    if(!hasspace(plist,1)) {if((stat = extendplist(plist,(plist->count+1)*EXPANDFACTOR))) goto done;} /* extra space */
    prop = &plist->properties[plist->count];
    keylen = strlen(key);
    if(keylen > NCPROPSMAXKEY) keylen = NCPROPSMAXKEY; /* truncate */
    memcpy(prop->pair.key,key,keylen);
    prop->pair.key[keylen] = '\0';
    prop->pair.value = (uintptr_t)value;
    prop->pair.size = size;
    prop->typefcn = fcn;
    prop->userdata = userdata;
    prop->pair.sort = NCP_COMPLEX;
    plist->count++;
done:
    return stat;
}

/* Clone using the NCtypefcn to copy values */
OPTSTATIC int
ncproplistclone(const NCproplist* src, NCproplist* clone)
{
    int stat = NC_NOERR;
    size_t i;
    NCPproperty* srcprops;
    NCPproperty* cloneprops;

    if(src == NULL || clone == NULL) {stat = NC_EINVAL; goto done;}
    if((stat=ncproplistinit(clone))) goto done;
    if((stat=extendplist(clone,src->count))) goto done;
    srcprops = src->properties;
    cloneprops = clone->properties;
    for(i=0;i<src->count;i++) {
	NCPproperty* sp = &srcprops[i];
	NCPproperty* cp = &cloneprops[i];
	void* p = NULL;
	*cp = *sp; /* Do a mass copy of the property and then fixup as needed */
        switch (sp->pair.sort) {
	case NCP_CONST:
	    break;
	case NCP_BYTES:
	    p = malloc(cp->pair.size);
	    memcpy(p,(void*)sp->pair.value,sp->pair.size);
	    cp->pair.value = (uintptr_t)p;
	    break;
	case NCP_COMPLEX: /* Need the typeop fcn */
	    stat = sp->typefcn(NCP_COPY,&sp->pair,&cp->pair);
	    if(stat < 0) goto done;
	    break;
	}	
    }
    clone->count = src->count;
done:
    return stat;
}

/* Increase size of a plist to be at lease nprops properties */
static int
extendplist(NCproplist* plist, size_t nprops)
{
    int stat = NC_NOERR;
    size_t newsize = plist->count + nprops;
    NCPproperty* newlist = NULL;
    if((plist->alloc >= newsize) || (nprops == 0))
	goto done; /* Already enough space */
    newlist = realloc(plist->properties,newsize*sizeof(NCPproperty));
    if(newlist == NULL) {stat = NC_ENOMEM; goto done;}
    plist->properties = newlist; newlist = NULL;    
    plist->alloc = newsize;
done:
    return stat;
}

/**
 * Lookup key and return value and size
 * @param plist to search
 * @param key for which to search
 * @param valuep returned value
 * @param sizep returned size
 * @return NC_NOERR if key found, NC_ENOOBJECT if key not found; NC_EXXX otherwise
 */
OPTSTATIC int
ncproplistget(const NCproplist* plist, const char* key, uintptr_t* valuep, uintptr_t* sizep)
{
    int stat = NC_ENOOBJECT; /* assume not found til proven otherwise */
    size_t i;
    NCPproperty* props;
    uintptr_t value = 0;
    uintptr_t size = 0;
    if(plist == NULL || key == NULL) goto done;
    for(i=0,props=plist->properties;i<plist->count;i++,props++) {
	if(strcmp(props->pair.key,key)==0) {
	    value = props->pair.value;
	    size = props->pair.size;	    
	    stat = NC_NOERR; /* found */
	    break;
	}
    }
    if(valuep) *valuep = value;
    if(sizep) *sizep = size;
done:
    return stat;
}

/* Iteration support */

/**
 * Get the ith key+value.a
 * @param plist to search
 * @param i which property to get.
 * @param keyp return i'th key
 * @param valuep return i'th value
 * @param valuep return i'th size
 * @return NC_NOERR if success, NC_EINVAL otherwise
 */
OPTSTATIC int
ncproplistith(const NCproplist* plist, size_t i, char* const * keyp, uintptr_t const * valuep, uintptr_t* sizep)
{
    int stat = NC_NOERR;
    NCPproperty* prop = NULL;    
    if(plist == NULL) goto done;
    if(i >= plist->count) {stat = NC_EINVAL; goto done;}
    prop = &plist->properties[i];
    if(keyp) *((char**)keyp) = (char*)prop->pair.key;
    if(valuep) *((uintptr_t*)valuep) = (uintptr_t)prop->pair.value;
    if(sizep) *sizep = prop->pair.size;
done:
    return stat;
}

/**************************************************/
/* Support Functions */

/**
 * Initialize a new property list 
 */
static int
ncproplistinit(NCproplist* plist)
{
    int stat = 0;
    /* Assume property list will hold at lease MINPROPS properties */
    if(plist->alloc == 0) {
	plist->alloc = MINPROPS;
	plist->properties = (NCPproperty*)calloc(plist->alloc,sizeof(NCPproperty));
        plist->count = 0;
    } else {
	if((stat = ncproplistclear(plist))<0) goto done;
    }
done:
    return stat;
}

/* Suppress unused statics warning */
static void
ncproplist_unused(void)
{
    void* unused = ncproplist_unused;
    unused = ncproplistnew;
    unused = ncproplistfree;
    unused = ncproplistadd;
    unused = ncproplistaddbytes;
    unused = ncproplistaddstring;
    unused = ncproplistaddx;
    unused = ncproplistclone;
    unused = ncproplistget;
    unused = ncproplistith;
    unused = ncproplistinit;
    unused = (void*)ncproplistith;
    unused = unused;
}
#endif /*NETCDF_PROPLIST_H*/
