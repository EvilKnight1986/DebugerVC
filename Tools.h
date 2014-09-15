/*******************************************************************************
*  
*  Copyright (c) all 2009 ���� All rights reserved
*  FileName : Tools.h
*  D a t e  : 2009.9.20
*  ��   ��  : һЩʵ�õĺ���
*  ˵   ��  : 
*
*
*******************************************************************************/

#if !defined(AFX_TOOLS_H__EC816A18_2F06_4553_A7B7_9A2F1711348F__INCLUDED_)
#define AFX_TOOLS_H__EC816A18_2F06_4553_A7B7_9A2F1711348F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <io.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <iostream.h>
#define IN
#define OUT

typedef struct __tagDate
{
        int year ;
        int month ;
        int day ;
}date ;

// �ж�ƽ������
int isLeap(IN int m) ;

// �ж��Ƿ���һ����ȷ������
int isSuccDate(IN date &pd) ;

// �ж�����ַ����Ƿ�ȫ������
int isSuccPhoto(IN const char *szBuf) ;

int isXNumString(IN const char *szbuf) ;

// ��ȫ���룬�����
int SafeInput(IN char *szBuffer, IN int nSize) ;

int SafeHexInput(IN char *szBuffer, IN int nSize) ;

int GetLine(IN FILE *fp , IN char *szBuffer, IN int nSize) ;

// �Ƿ���ʮ������
int IsDecNum(IN char szSrc[]) ;

// ���˿հ׷�
int SkipSpace(IN char *szBuf, IN int nSize) ;

void init_crc32_table() ;
unsigned long crc32(IN const char* str) ;

// ����crcֵ
unsigned int cal_crc(IN const char *str) ;

// RSHash ����
unsigned int RSHash(IN const char *str) ;

// using this hash function
// JS Hash Function 
unsigned int JSHash(IN const char *str) ;

// P. J. Weinberger Hash Function
unsigned int PJWHash(IN const char *str) ;

// ELF Hash Function
unsigned int ELFHash(IN const char *str) ;

// BKDR Hash Function
unsigned int BKDRHash(IN const char *str) ;

// SDBM Hash Function
unsigned int SDBMHash(IN const char *str) ;

// DJB Hash Function
unsigned int DJBHash(IN const char *str) ;

// AP Hash Function
unsigned int APHash(IN const char *str) ;

// ȡ�ò����ĸ���
int GetParamCount(IN char *pszCmd) ;


#endif // !defined(AFX_TOOLS_H__EC816A18_2F06_4553_A7B7_9A2F1711348F__INCLUDED_)
