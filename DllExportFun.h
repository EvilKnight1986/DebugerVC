/*******************************************************************************
*  
*  Copyright (c) all 2009 黄奇 All rights reserved
*  FileName : DllExportFun.h
*  D a t e  : 2010.3.22
*  功   能  : 用来管理Dll的导出函数
*  说   明  : 
*
*
*******************************************************************************/

#if !defined(AFX_DLLEXPORTFUN_H__D5C0DBF9_5F2C_4C56_B3DD_5DEF4EFCD71F__INCLUDED_)
#define AFX_DLLEXPORTFUN_H__D5C0DBF9_5F2C_4C56_B3DD_5DEF4EFCD71F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MyList.h"
#include "Disasm/disasm.h"
#include "RedBlackTree.h"
#include <stdio.h>
#include <windows.h>

#define IN
#define OUT

class DllExportFun  
{
public:
        int     InsertDll(HMODULE  hModule) ;
        int     RemoveDll(HMODULE  hModule) ;
        void    SetProcessValue(HANDLE handle) ;
        int     DisplayLoadDllName(void);       // 显示已加载dll名称
        int     AddressToFunName(t_disasm &da, char *pszDllName, char *pszFunName) ;
        int     GetFunName(IN unsigned int _nAddr, OUT char *&pDllName, OUT char *&pFunName) ;
public:
        DllExportFun();
	virtual ~DllExportFun() ;
public:

        int             m_nDllCount ;               // dll数量
        MyList          m_DllNameList ;             // 保存dll名字的链表
        RedBlackTree    m_rbt ;                     // 保存api函数的地址以为函数名
        HANDLE          m_hProcess ;
};

#endif // !defined(AFX_DLLEXPORTFUN_H__D5C0DBF9_5F2C_4C56_B3DD_5DEF4EFCD71F__INCLUDED_)
