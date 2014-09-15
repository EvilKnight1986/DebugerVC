// List.h: interface for the List class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIST_H__83A8CC72_06D3_46E6_BF57_49D173B68763__INCLUDED_)
#define AFX_LIST_H__83A8CC72_06D3_46E6_BF57_49D173B68763__INCLUDED_

/*******************************************************************************
*  
*  Copyright (c) all 2010 黄奇 All rights reserved
*  FileName : List.h
*  D a t e  : 2010.3.25
*  功   能  : 记录int3断点信息
*  说   明  : 
*
*******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

class Node
{
public:
        Node():nAddr(0),byOldValue(0),bIsActive(0)
        {
        }
        virtual ~Node()
        {
        }
public:
        void    ClsState(void);
        void    StdState(void);
        void    SetState(char state);
        Node    * pNext ;          // 保存下一个结点的位置
        unsigned int nAddr ;       // 地址
        char    byOldValue ;       // 保存原来的值
        char    bIsActive  ;       // 当前是否有激活
} ;

class List  
{
public:
        int     Clean(void);
        Node *  GetListHead(void);
        int     GetNodeCount(void);
        Node *  FindPrev(unsigned int _nAddr);
        Node *  Find(unsigned int _nAddr);
        int     Delete(unsigned int _nAddr);
        int     Insert(unsigned int _nAddr,char _byOldValue);
        int     SetState(unsigned int _nAddr, char state);
        int     InsertTail(unsigned int _nAddr, char _byOldValue);
	int     InsertPrev(unsigned int _nAddr, char _byOldValue);

public:
	int     IsActive(unsigned int _nAddr);
	void    Display(void);
	int     GetValue(unsigned int _nAddr, char *pValue) ;
	int     ClsState(unsigned int _nAddr);
	int     StdState(unsigned int _nAddr);

public:
        List():pHead(NULL),nSum(0)
        {}
	virtual ~List();
private:
        Node    * pHead ;               // 指向链表头
        int nSum;                       // 节点总数
};

#endif // !defined(AFX_LIST_H__83A8CC72_06D3_46E6_BF57_49D173B68763__INCLUDED_)
