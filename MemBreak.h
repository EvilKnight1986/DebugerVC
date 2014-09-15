// MemBreak.h: interface for the CMemBreak class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMBREAK_H__2DEF8D24_E109_41A7_B20E_C7115C261922__INCLUDED_)
#define AFX_MEMBREAK_H__2DEF8D24_E109_41A7_B20E_C7115C261922__INCLUDED_

/*******************************************************************************
*  
*  Copyright (c) all 2010 黄奇 All rights reserved
*  FileName : MemBreak.h
*  D a t e  : 2010.3.26
*  功   能  : 记录内存断点信息
*  说   明  : 
*
*******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h>
#include <windows.h>
// 记录内存断点
typedef struct _MemBreakNode
{
        _MemBreakNode   *pNext ;         // 指向下一个内存断点信息的地址
        int             nID ;
        LPVOID          lpBreakBase ;    // 内存断点的起码地址
        SIZE_T          BreakLen    ;    // 内存断点的长度
        DWORD           Protect     ;    // 权限(1:读 2:写)
        int             isActive    ;    // 是否激活
} MemBreakNode, *PMemBreakNode ;

class CMemBreak  
{
public:
        int             HaveIn(LPVOID nAddr, int &nPage);
        void            Display(void);
        int             RemoveById(int id) ;
        PMemBreakNode   FindPrevById(int id);
        PMemBreakNode   FindById(int id);
        int             Insert(LPVOID lpBreakBase, SIZE_T BreakLen, DWORD Protect);
        int             IsHaving(LPVOID nAddr);
        PMemBreakNode   Find(LPVOID nAddr);
        PMemBreakNode   FindPrev(LPVOID);
        int             Remove(LPVOID nAddr);
        int             RemoveAll(void);

public:
	int GetCount(void);
        CMemBreak();
        virtual ~CMemBreak();

private:
        PMemBreakNode m_pHead ;
        int           m_nCount ;
        int           m_ID ;
} ;

#endif // !defined(AFX_MEMBREAK_H__2DEF8D24_E109_41A7_B20E_C7115C261922__INCLUDED_)
