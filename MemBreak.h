// MemBreak.h: interface for the CMemBreak class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMBREAK_H__2DEF8D24_E109_41A7_B20E_C7115C261922__INCLUDED_)
#define AFX_MEMBREAK_H__2DEF8D24_E109_41A7_B20E_C7115C261922__INCLUDED_

/*******************************************************************************
*  
*  Copyright (c) all 2010 ���� All rights reserved
*  FileName : MemBreak.h
*  D a t e  : 2010.3.26
*  ��   ��  : ��¼�ڴ�ϵ���Ϣ
*  ˵   ��  : 
*
*******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h>
#include <windows.h>
// ��¼�ڴ�ϵ�
typedef struct _MemBreakNode
{
        _MemBreakNode   *pNext ;         // ָ����һ���ڴ�ϵ���Ϣ�ĵ�ַ
        int             nID ;
        LPVOID          lpBreakBase ;    // �ڴ�ϵ�������ַ
        SIZE_T          BreakLen    ;    // �ڴ�ϵ�ĳ���
        DWORD           Protect     ;    // Ȩ��(1:�� 2:д)
        int             isActive    ;    // �Ƿ񼤻�
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
