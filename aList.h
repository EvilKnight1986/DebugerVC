// List.h: interface for the List class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIST_H__CFDEEA1C_9BA0_4FB9_B4C3_C87B4E3EC490__INCLUDED_)
#define AFX_LIST_H__CFDEEA1C_9BA0_4FB9_B4C3_C87B4E3EC490__INCLUDED_

/*******************************************************************************
*  
*  Copyright (c) all 2009 黄奇 All rights reserved
*  FileName : aList.h
*  D a t e  : 2009.9.16
*  功   能  : 链表
*  说   明  : 
*
*******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

#define IN
#define OUT 

class DistNode
{
public:
        DistNode(IN int Data,IN int Offset):data(Data),offset(Offset),next(NULL){}
        unsigned int GetData()   { return data;   }
        unsigned int GetOffset() { return offset; }
public:
        unsigned int data   ;
        unsigned int offset ;
        DistNode  *  next   ;
};

typedef DistNode * DistPosition ;
typedef DistPosition DistList ;

class aList  
{
public:
        DistPosition operator [] (IN unsigned int index) ;
        int             Clear();
        int             GetSize(void);
        DistNode *      Top(void);
        unsigned int    Pop(void);
        int             Push(IN unsigned int data, IN unsigned int offset);
        int             Delete(IN unsigned int data);
        int             InsertLast(IN unsigned int data, IN unsigned int offset);
        int             InsertPrev(IN unsigned int data, IN unsigned int offset);
        int             Insert(IN unsigned int data,  IN unsigned int offset,IN int nIndex);
        DistPosition    FindPrev(IN unsigned int data);
        DistPosition    Find(IN unsigned int data);
        int             IsEmpty(void);
public:
        aList();
        ~aList();
private:
        DistList pHead;
        DistList pLast;
        unsigned int nSize;
};

#endif // !defined(AFX_LIST_H__CFDEEA1C_9BA0_4FB9_B4C3_C87B4E3EC490__INCLUDED_)
