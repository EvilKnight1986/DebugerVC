/*******************************************************************************
*  
*  Copyright (c) all 2009 ���� All rights reserved
*  FileName : MyList.h
*  D a t e  : 2010.3.21
*  ��   ��  : �������ļ�
*  ˵   ��  : 
*
*
*******************************************************************************/

#if !defined(AFX_MYLIST_H__C6B7955D_68C6_4D6D_9A9E_6587C741979D__INCLUDED_)
#define AFX_MYLIST_H__C6B7955D_68C6_4D6D_9A9E_6587C741979D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "tools.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream.h>

class MyNode
{
public:
         // ȡ��DllName��CRCֵ����ȥstrcmp�ĳɱ�
        unsigned int    GetCrc(void) ;
        int             SetImageBase(unsigned int _ImageBase) ;
        int             GetImageBase(void) ;
        int             SetDllName(const char *_pDllName) ;      // ����dll������
        int             SetLoadState(BOOL _bStat) ;              // ��������״̬(��ǰ�Ƿ����)
        int             SetLoad(void) ;                          // ��״̬����Ϊ��ǰ����
        int             SetUnLoad(void) ;                        // ��״̬����Ϊ��ǰû������
        char *          GetDllName(void) ;                       // ȡ��dll������  
        BOOL            GetState(void) ;                         // ȡ��dll��ǰ��״̬
        void            DispNode(void) ;
public:
        // ��Ϊnew���ܻ���������ܲ����ڹ��캯����,������SetDllName������
        MyNode():m_pDllName(NULL),m_DllNameCrc(NULL),m_bIsLoad(FALSE),next(NULL),m_ImageBase(0)
        {}
        ~MyNode()
        {
                if (NULL != m_pDllName)
                {
                        delete [] m_pDllName ;
                        m_pDllName = NULL ;
                }
        }
public:
        char   * m_pDllName ;            // ָ��Dll������
        MyNode   * next;
        unsigned int m_DllNameCrc ;      // Dll���ֵ�CRCֵ
        BOOL     m_bIsLoad ;             // ��dll��ǰ�Ƿ��Ѿ�����
        unsigned int m_ImageBase ;       // 
        
};

typedef MyNode * position ;
typedef position list ;


class MyList  
{
public:
        unsigned int & operator [] (unsigned int index) ;
        int             GetSize(void) ;
        int             Delete(const char *_pDllName) ;
        int             Insert(const char *_pDllName, BOOL _isLoad) ;
        position        FindPrev(const char *_pDllName) ;
        position        Find(const char *_pDllName) ;
        int             IsEmpty(void) ;
        int             DisplayLoadDllName(void) ;
        position        GetNode(unsigned int nIndex) ;
	void            DispNode(position p) ;

public:
        MyList();
        ~MyList();
private:
        list pHead;
        list pLast;
        unsigned int nSize;
};

#endif // !defined(AFX_MYLIST_H__C6B7955D_68C6_4D6D_9A9E_6587C741979D__INCLUDED_)
