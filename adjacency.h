// Adjacency.h: interface for the Adjacency class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADJACENCY_H__5BF7404E_5C52_4628_B154_09ADECEE8B28__INCLUDED_)
#define AFX_ADJACENCY_H__5BF7404E_5C52_4628_B154_09ADECEE8B28__INCLUDED_
/*******************************************************************************
*  
*  Copyright (c) all 2009 ���� All rights reserved
*  FileName : adjacency.h
*  D a t e  : 2009.9.16
*  ��   ��  : ��̬�ڽӱ�
*  ˵   ��  : 
*
*******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define IN
#define OUT

#define RECORDMAX 0x1000

#include "aList.h"
#include <string.h>

class Adjacency  
{
public:
    // ȡ���ڽӱ��С
    unsigned int GetCapacity(void) ;
    // �������ļ�
    int ExportFile(IN const char * szFileName) ;
    // ���ļ�����
    int ImportFile(IN const char * szFileName) ;
    // ȡ���ڽӱ���ָ������ĳ���
    unsigned int GetSize(IN unsigned int index) ;
    // ����[]������
    aList & operator [](unsigned int index) ;

    int Clear(void);

public:
    Adjacency(int size = 10) ;
    ~Adjacency() ;

private:
    // �����ڴ�ռ�
    int Malloc(IN int size) ;
    // ���������ڴ�ռ�
    int Realloc(IN int size) ;

private:
    aList ** pList ;
    unsigned int nCapacity ;
} ;

#endif // !defined(AFX_ADJACENCY_H__5BF7404E_5C52_4628_B154_09ADECEE8B28__INCLUDED_)
