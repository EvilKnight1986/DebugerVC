// PEAnalyse.h: interface for the PEAnalyse class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PEANALYSE_H__4EF34F97_8D6A_4DBD_8F4E_2751D062B21F__INCLUDED_)
#define AFX_PEANALYSE_H__4EF34F97_8D6A_4DBD_8F4E_2751D062B21F__INCLUDED_

/*******************************************************************************
*  
*  Copyright (c) all 2010 ���� All rights reserved
*  FileName : PEAnalyse.h
*  D a t e  : 
*  ��   ��  : ����PE�ļ�
*  ˵   ��  : 
*
*
*******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

class PEAnalyse  
{
public:
	void                    DispPEInformation(void);
	PVOID                   GetMapToFilePointer(void);
	IMAGE_SECTION_HEADER *  GetSectionHeaderPtr();
	int                     GetNumberOfRvaAndSizes(void);
	int                     GetSizeOfOptionalHeader(void);
	int                     GetCheckSum(void);
	int                     GetCharacteristics(void);
	int                     GetSizeOfHeaders(void);
	int                     GetTimeDataStamp(void);
	int                     GetNumberOfSection(void);
	int                     GetSubSystem(void);
	int                     GetMagic(void);
	int                     GetFileAlignment(void);
	int                     GetSectionAlignment(void) ;
	int                     GetBaseOfData(void) ;
	int                     GetBaseOfCode(void) ;
	int                     GetSizeOfImage(void) ;
	int                     IsPE(void) ;
	int                     GetImageBase(void) ;
	int                     GetEntryPoint(void) ;
	void                    CloseFileHandle(void) ;
	int                     Analyse(char *pPath) ;
public:
	PEAnalyse();
	virtual ~PEAnalyse();
private:
	int                     m_OptionHeaderOffset;   // ��¼ѡ��ͷ��ƫ����
	HANDLE                  m_hMap;
	LPVOID                  m_lpFile;
	HANDLE                  m_hFile;
	int                     m_isPE;                 // 
        IMAGE_DOS_HEADER        m_DosHeader ;           // ����DOSͷ
        IMAGE_NT_HEADERS        m_NtHeaders ;           // ����NTͷ
        IMAGE_SECTION_HEADER *  m_pSectionHeader;       // 
};

#endif // !defined(AFX_PEANALYSE_H__4EF34F97_8D6A_4DBD_8F4E_2751D062B21F__INCLUDED_)
