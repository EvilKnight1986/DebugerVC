/*******************************************************************************
*  
*  Copyright (c) all 2009 黄奇 All rights reserved
*  FileName : MyList.h
*  D a t e  : 2010.3.21
*  功   能  : 链表定义文件
*  说   明  : 
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
         // 取得DllName的CRC值，将去strcmp的成本
        unsigned int    GetCrc(void) ;
        int             SetImageBase(unsigned int _ImageBase) ;
        int             GetImageBase(void) ;
        int             SetDllName(const char *_pDllName) ;      // 设置dll的名字
        int             SetLoadState(BOOL _bStat) ;              // 设置载入状态(当前是否加载)
        int             SetLoad(void) ;                          // 将状态设置为当前载入
        int             SetUnLoad(void) ;                        // 将状态设置为当前没有载入
        char *          GetDllName(void) ;                       // 取得dll的名字  
        BOOL            GetState(void) ;                         // 取得dll当前的状态
        void            DispNode(void) ;
public:
        // 因为new可能会出错，所以能不放在构造函数中,单独用SetDllName来代替
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
        char   * m_pDllName ;            // 指向Dll的名字
        MyNode   * next;
        unsigned int m_DllNameCrc ;      // Dll名字的CRC值
        BOOL     m_bIsLoad ;             // 此dll当前是否已经载入
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
