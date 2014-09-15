// MemList.h: interface for the CMemList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMLIST_H__4D18D113_9F4C_4330_979F_5257FFF15DA9__INCLUDED_)
#define AFX_MEMLIST_H__4D18D113_9F4C_4330_979F_5257FFF15DA9__INCLUDED_

/*******************************************************************************
*  
*  Copyright (c) all 2010 ���� All rights reserved
*  FileName : MemList.h
*  D a t e  : 2010.3.26
*  ��   ��  : ��¼�ڴ��ҳ��Ϣ
*  ˵   ��  : 
*
*******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h>
#include <windows.h>

typedef struct _MemNode
{
public:
        _MemNode       *pNext ;                 // ָ����һ���ڴ��ҳ��ַ
        int             nID    ;                // Ψһ��ʶ
        PVOID           BaseAddress ;           // ��ҳ��ʼ����ַ
        PVOID           AllocationBase ;        // ��ʼ��ַ���������Ľ��
        DWORD           AllocationProtect ;
        SIZE_T          RegionSize ;
        DWORD           State ;                 // ״̬
        DWORD           Protect ;               // ����
        DWORD           newProtect ;            // ������
        DWORD           Type ;
} MemNode, *PMemNode;

class CMemList  
{
public:
        int             HaveIn(LPVOID nAddr, int &nPage);
        PMemNode        FindById(int id) ;
	PMemNode        FindPrevById(int id);
	int             RemoveById(int id);
	void            Display(void);
	PMemNode        Find(PMEMORY_BASIC_INFORMATION pMemBaseInfo) ;
	PMemNode        FindPrev(PMEMORY_BASIC_INFORMATION pMemBaseInfo) ;
	int             Remove(PMEMORY_BASIC_INFORMATION pMemBaseInfo) ;
	int             IsHaving(PMEMORY_BASIC_INFORMATION pMemBaseInfo) ;
	int             Insert(PMEMORY_BASIC_INFORMATION pMemBaseInfo) ;
	int             RemoveAll(void) ;
public:
	int             RevertMemoryProtectAll(HANDLE hProcess);
	int             EnumDestProcessEffectivePage(HANDLE hProcess) ;

public:
	CMemList() ;
	virtual ~CMemList() ;

private:
        PMemNode        m_pHead ;
        int             m_nCount;
        int             m_nID ;
};

#endif // !defined(AFX_MEMLIST_H__4D18D113_9F4C_4330_979F_5257FFF15DA9__INCLUDED_)
