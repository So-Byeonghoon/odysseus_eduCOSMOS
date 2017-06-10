/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2013-2015, Kyu-Young Whang, KAIST                         */
/*    All rights reserved.                                                    */
/*                                                                            */
/*    Redistribution and use in source and binary forms, with or without      */
/*    modification, are permitted provided that the following conditions      */
/*    are met:                                                                */
/*                                                                            */
/*    1. Redistributions of source code must retain the above copyright       */
/*       notice, this list of conditions and the following disclaimer.        */
/*                                                                            */
/*    2. Redistributions in binary form must reproduce the above copyright    */
/*       notice, this list of conditions and the following disclaimer in      */
/*       the documentation and/or other materials provided with the           */
/*       distribution.                                                        */
/*                                                                            */
/*    3. Neither the name of the copyright holder nor the names of its        */
/*       contributors may be used to endorse or promote products derived      */
/*       from this software without specific prior written permission.        */
/*                                                                            */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       */
/*    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          */
/*    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,    */
/*    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        */
/*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER        */
/*    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT      */
/*    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN       */
/*    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         */
/*    POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational Purpose Object Storage System            */
/*    (Version 1.0)                                                           */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: odysseus.educosmos@gmail.com                                    */
/*                                                                            */
/******************************************************************************/
/*
 * Module: EduBtM_FetchNext.c
 *
 * Description:
 *  Find the next ObjectID satisfying the given condition. The current ObjectID
 *  is specified by the 'current'.
 *
 * Exports:
 *  Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*, Four, BtreeCursor*, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_FetchNext(KeyDesc*, KeyValue*, Four, BtreeCursor*, BtreeCursor*);



/*@================================
 * EduBtM_FetchNext()
 *================================*/
/*
 * Function: Four EduBtM_FetchNext(PageID*, KeyDesc*, KeyValue*,
 *                              Four, BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Fetch the next ObjectID satisfying the given condition.
 * By the B+ tree structure modification resulted from the splitting or merging
 * the current cursor may point to the invalid position. So we should adjust
 * the B+ tree cursor before using the cursor.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    eBADCURSOR
 *    some errors caused by function calls
 */
Four EduBtM_FetchNext(
    PageID                      *root,          /* IN root page's PageID */
    KeyDesc                     *kdesc,         /* IN key descriptor */
    KeyValue                    *kval,          /* IN key value of stop condition */
    Four                        compOp,         /* IN comparison operator of stop condition */
    BtreeCursor                 *current,       /* IN current B+ tree cursor */
    BtreeCursor                 *next)          /* OUT next B+ tree cursor */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    int							i;
    Four                        e;              /* error number */
    Four                        cmp;            /* comparison result */
    Two                         slotNo;         /* slot no. of a leaf page */
    Two                         oidArrayElemNo; /* element no. of the array of ObjectIDs */
    Two                         alignedKlen;    /* aligned length of key length */
    PageID                      overflow;       /* temporary PageID of an overflow page */
    Boolean                     found;          /* search result */
    ObjectID                    *oidArray;      /* array of ObjectIDs */
    BtreeLeaf                   *apage;         /* pointer to a buffer holding a leaf page */
    BtreeOverflow               *opage;         /* pointer to a buffer holding an overflow page */
    btm_LeafEntry               *entry;         /* pointer to a leaf entry */
    BtreeCursor                 tCursor;        /* a temporary Btree cursor */
  
    
    /*@ check parameter */
    if (root == NULL || kdesc == NULL || kval == NULL || current == NULL || next == NULL)
	ERR(eBADPARAMETER_BTM);
    
    /* Is the current cursor valid? */
    if (current->flag != CURSOR_ON && current->flag != CURSOR_EOS)
		ERR(eBADCURSOR);
    
    if (current->flag == CURSOR_EOS) return(eNOERROR);
    
    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

    if (compOp == SM_BOF) {
        compOp = SM_GE;
        e = edubtm_FirstObject(root, kdesc, kval, compOp, &tCursor);
        if (e < 0) ERR(e);
        *kval = tCursor.key;
    }
    else if (compOp == SM_EOF) {
        compOp = SM_LE;
        e = edubtm_LastObject(root, kdesc, kval, compOp, &tCursor);
        if (e < 0) ERR(e);
        *kval = tCursor.key;
    }

    e = edubtm_FetchNext(kdesc, kval, compOp, current, next);
    if (e < 0) ERR(e);
    
    return(eNOERROR);
    
} /* EduBtM_FetchNext() */



/*@================================
 * edubtm_FetchNext()
 *================================*/
/*
 * Function: Four edubtm_FetchNext(KeyDesc*, KeyValue*, Four,
 *                              BtreeCursor*, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Get the next item. We assume that the current cursor is valid; that is.
 *  'current' rightly points to an existing ObjectID.
 *
 * Returns:
 *  Error code
 *    eBADCOMPOP_BTM
 *    some errors caused by function calls
 */
Four edubtm_FetchNext(
    KeyDesc  		*kdesc,		/* IN key descriptor */
    KeyValue 		*kval,		/* IN key value of stop condition */
    Four     		compOp,		/* IN comparison operator of stop condition */
    BtreeCursor 	*current,	/* IN current cursor */
    BtreeCursor 	*next)		/* OUT next cursor */
{
	/* These local variables are used in the solution code. However, you don¡¯t have to use all these variables in your code, and you may also declare and use additional local variables if needed. */
    Four 		e;		/* error number */
    Four 		cmp;		/* comparison result */
    Two 		alignedKlen;	/* aligned length of a key length */
    PageID 		leaf;		/* temporary PageID of a leaf page */
    // PageID 		overflow;	/* temporary PageID of an overflow page */
    ObjectID 		*oidArray;	/* array of ObjectIDs */
    BtreeLeaf 		*apage;		/* pointer to a buffer holding a leaf page */
    // BtreeOverflow 	*opage;		/* pointer to a buffer holding an overflow page */
    btm_LeafEntry 	*entry;		/* pointer to a leaf entry */    
    
    
    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }


    leaf = current->leaf;
    e = BfM_GetTrain((TrainID*)&leaf, (char**)&apage, PAGE_BUF);
    if (e < 0) ERR(e);

    *next = *current;
    // check objects of same key
    if (!(kdesc->flag & KEYFLAG_UNIQUE)) {
        entry = (btm_LeafEntry*)&(apage->data[apage->slot[-next->slotNo]]);
        alignedKlen = (entry->klen+3)/4*4;
        oidArray = (ObjectID*)&(entry->kval[alignedKlen]);
        // printf("not UNIQUE: n=%d\n", entry->nObjects);
        for (i=1; i < entry->nObjects; i++) {
            if (oidArray[i-1].unique == current->oid.unique) {
                next->oid = oidArray[i];
                e = BfM_FreeTrain((TrainID*)&leaf, PAGE_BUF);
                if (e < 0) ERR(e);
                return(eNOERROR);
            }
        }
        if (oidArray[entry->nObjects-1].unique != current->oid.unique) 
            ERR(eBADCURSOR);
    }

    // index increase order
    if (compOp & SM_LT) {
        // check next whether slot exist
        if (++(next->slotNo) >= apage->hdr.nSlots) {
            // Is last leaf page of index?
            if (apage->hdr.nextPage == NIL) {
                // printf("next page is NIL\n");
                next->flag = CURSOR_EOS;
                e = BfM_FreeTrain((TrainID*)&leaf, PAGE_BUF);
                if (e < 0) ERR(e);
                return(eNOERROR);
            }
            // go to next page (leaf)
            // printf("next page is %d\n", apage->hdr.nextPage);
            next->leaf.pageNo = apage->hdr.nextPage;
            next->slotNo = 0;
            e = BfM_FreeTrain((TrainID*)&leaf, PAGE_BUF);
            if (e < 0) ERR(e);
            leaf.pageNo = next->leaf.pageNo;
            e = BfM_GetTrain((TrainID*)&leaf, (char**)&apage, PAGE_BUF);
            if (e < 0) ERR(e);
        }
    }
    // index decrease order
    else {
        // check prev whether slot exist
        if (--(next->slotNo) < 0) {
            // Is first leaf page of index?
            if (apage->hdr.prevPage == NIL) {
                // printf("prev page is NIL\n");
                next->flag = CURSOR_EOS;
                e = BfM_FreeTrain((TrainID*)&leaf, PAGE_BUF);
                if (e < 0) ERR(e);
                return(eNOERROR);
            }
            // go to prev page (leaf)
            // printf("prev page is %d\n", apage->hdr.prevPage);
            next->leaf.pageNo = apage->hdr.prevPage;
            e = BfM_FreeTrain((TrainID*)&leaf, PAGE_BUF);
            if (e < 0) ERR(e);
            leaf.pageNo = next->leaf.pageNo;
            e = BfM_GetTrain((TrainID*)&leaf, (char**)&apage, PAGE_BUF);
            if (e < 0) ERR(e);
            next->slotNo = apage->hdr.nSlots-1;
        }
    }

    //update next
    entry = &apage->data[apage->slot[-next->slotNo]];
    alignedKlen = (entry->klen+3)/4*4;
    oidArray = (ObjectID*)&(entry->kval[alignedKlen]);
    next->oid = oidArray[0];
    next->key = *(KeyValue*)&entry->klen;

    // printf("LEN: kval=%d, next=%d\n", kval->len, next->key.len);
    // printf("STR: kval=%s, next=%s\n", kval->val, next->key.val);
    // printf("STR: kval=%d, next=%d\n", kval->val[0], next->key.val[0]);
    cmp = edubtm_KeyCompare(kdesc, kval, &next->key);
    if (1 << cmp & compOp)
        next->flag = CURSOR_ON;
    else
        next->flag = CURSOR_EOS;

    e = BfM_FreeTrain((TrainID*)&leaf, PAGE_BUF);
    if (e < 0) ERR(e);
    
    return(eNOERROR);
    
} /* edubtm_FetchNext() */
