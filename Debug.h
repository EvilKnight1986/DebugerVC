/*******************************************************************************
*
*  Copyright (c) all 2010 ���� All rights reserved
*  FileName : debug.h
*  D a t e  : 2010.3.4
*  ��   ��  : ʵ�ֵ��Թ���ͷ�ļ�
*  ˵   ��  :
*
*******************************************************************************/

#if !defined(AFX_DEBUG_H__1FDCD6B4_ADE1_4103_9765_1A4394B7982A__INCLUDED_)
#define AFX_DEBUG_H__1FDCD6B4_ADE1_4103_9765_1A4394B7982A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MemBreak.h"
#include "MemList.h"
#include "Adjacency.h"
#include "List.h"
#include "Tools.h"
#include "DllExportFun.h"
#include "PEAnalyse.h"
#include "Disasm/disasm.h"
#include "AberranceTree.h"
#include <stdio.h>
#include <iostream.h>
#include <windows.h>
#include <string.h>

class Debug ;
const int ShellCodeLen = 14 ;

typedef int(Debug::*DebugPtr)(void) ;

// ���ж�Ӧ������Ϣ����
typedef struct _TagCommand
{
        char * pCommand ;
        DebugPtr pFun ;
} Command ;

typedef union _Tag_DR7
{
        struct __DRFlag
        {
                unsigned int L0:  1 ;
                unsigned int G0:  1 ;
                unsigned int L1:  1 ;
                unsigned int G1:  1 ;
                unsigned int L2:  1 ;
                unsigned int G2:  1 ;
                unsigned int L3:  1 ;
                unsigned int G3:  1 ;
                unsigned int Le:  1 ;
                unsigned int Ge:  1 ;
                unsigned int b:   3 ;
                unsigned int gd:  1 ;
                unsigned int a:   2 ;
                unsigned int rw0: 2 ;
                unsigned int len0:2 ;
                unsigned int rw1: 2 ;
                unsigned int len1:2 ;
                unsigned int rw2: 2 ;
                unsigned int len2:2 ;
                unsigned int rw3: 2 ;
                unsigned int len3:2 ;
        } DRFlag;
        DWORD dwDr7 ;
}DR7 ;

class Debug  
{
public:
        Debug() ;
        virtual ~Debug() ;

public:
        int     BreakPointIsHave(IN unsigned int nAddr);
        int     RemoveAllBreakPoint(IN void);
        // ����һ����Ч���ڴ��ҳ
        int     InsertEffectiveMemoryPage(IN LPVOID lpAddr, OUT PMEMORY_BASIC_INFORMATION pMbi) ;
        // �жϵ�ַ�Ƿ�����Ч��ҳ֮��
        int     IsEffectiveAddress(IN LPVOID lpAddr, OUT MEMORY_BASIC_INFORMATION *pMbi) ;
        // ��������
        int     ReDebug(void);
        // ��ʾ�Ѿ����ص�DLL����
        int     DisplayLoadDllName(void);
        // ɱ����ǰ�����Գ���
        int     Kill(void) ;
        // ���µ���
        int     Restart(void);
        // ������չ����
        int     ParseExternCommand(IN char *szBuffer);
        // ����¼��ָ��д���ļ�
        int     WriteRecordToFile(void) ;
        // ָ���¼
        int     InstructionRecord(void) ;
        // ����S����
        int     ParseSCommand(IN char * pszCmd) ;
        // �༭�Ĵ�����ֵ
        int     EditRegisterValue(IN char *pszCmd) ;
        // ����Ӳ���ϵ��������ϵ����Ϣת������ϵ�洢
        int     OnceDrBreakToSoftBreak(void) ;
        int     ParseEditMemory(IN LPVOID pAddr) ;
        // �������
        void    PrintCommandHelp(IN char ch) ;
        // �жϼĴ�����ֵ�Ƿ���api��ַ,ͬʱȡ��dll���ͺ�����
        int     ParseFunNameOfRegisterValue(IN t_disasm &da, OUT char *pszDllName, OUT char *pszFunName) ;
        // ��ʾĿ������ڴ�����
        int     DisplayDestProcessMemory(IN LPVOID pAddr, IN int nLen) ;
        // ����D����
        int     ParseDCommand(IN char *pszCmd) ;
        // �Ƴ�Ӳ���ϵ�
        int     RemoveDrRegister(IN int nDrID) ;
        // ȡ��ָ��DR�Ĵ�����ֵ
        int     GetDrNumberOfAddress(IN LPVOID pAddr);
        // ����ϵ����ڴ�ҳ�໥�Ĺ�ϵ
        int     DisplayAdjacencyInformation(void);
        // ����һ����ַ���ж��Ƿ��ڶϵ���
        int     HaveInBreakArea(IN int nAddrm,IN PMemBreakNode pBreakNode) ;
        // �Ƴ��ڴ�ҳ����
        int     RemoveMemoryProtect(IN int nProtect, IN int Attrib) ;
        // �޸��ڴ�ҳ����
        int     RepairMemoryProtect(IN int nPageID) ;
        // �Ƴ��ڴ�ϵ�
        int     RemoveMemoryBreak(IN LPVOID pAddr) ;
        // ���ڴ�ҳ������Ϊ�ɶ���д��ִ��
        int     SetMemoryPageProtectEx(IN LPVOID pAddr, IN int nLen) ;
        // �ж��ڴ��������Ƿ���ָ��������
        int     IsHavememoryProtect(IN int nProtect, IN int Attrib) ;
        // ����ڴ�ϵ�
        int     AppendMemoryBreak(IN LPVOID nAddr, IN SIZE_T nLen, IN DWORD dwPurview) ;
        // �������쳣
        int     ParseSingleSetp(void) ;
        BOOL    SetCurrentThreadContext(IN  CONTEXT *pContext) ;
        BOOL    GetCurrentThreadContext(OUT CONTEXT *pContext) ;
        // ��ʾӲ���ϵ���Ϣ
        void    DispDrBreakPoint(void) ;
        // ����DRx�ϵ�
        int     SetDrBreakPoint(IN int nDrID, IN unsigned int nAddr, IN int nLen, IN int nPurview);
        int     SetFreeDrRegister(IN int nNum);
        int     GetFreeDrRegister(void);
        int     GrayBreakPoint(IN unsigned int nAddr, IN BOOL isTmp);
        int     ActiveBreakPoint(IN unsigned int nAddr);
        int     AddSoftBreakPoint(IN unsigned int nAddr);
        int     RevertAllBreakPoint(void);
        int     SetAllBreakPoint(void);
        int     ParseBCommand(IN char *pszCmd);
        int     DispCopyright(void);
        int     ParseGCommand(IN char *pszBuffer);
        int     ParseGoTo(IN unsigned int nAddr);
        void    PrintCurrentRegister(void);
        int     ParseUCommand(IN char *pszCmd, IN BOOL bISContinue);
        void    DebugHelp(void);
        int     SetTFRegister(void);
        int     isMyBreakPoint();
        int     ParseLoadDllExportTable(IN DEBUG_EVENT &debugEvent);     // �������dll�󵼳���ķ���
        int     SetSingleStep();                                         // �õ���
        int     GetInstructionLen(IN DWORD dwThreadId, OUT int *pLen);   // ȡ��ָ���
        int     GetCurrentEip(IN DWORD dwThreadId);                      // ȡ�õ�ǰEIP
        int     PrintInstruction(IN int Eip, IN BOOL bIsContimue, IN int nItem);   // ���ָ��
        void    PrintContext(void);                                     // ����Ĵ�������Ϣ
        int     ParseRequest(void);                                      // �����û�����
        int     ParseBreakPoint(void);                                   // ����ϵ�
        DWORD   ParseAccessVioltion(void) ;                              // ��������쳣
        BOOL    SetExePathName(IN char *_pExePatchName) ;                  // ���ñ����Գ���·��
        int     StartDebug(IN char *pExeName = NULL) ;               

private:
        AberranceTree           m_InstructionRecord ;             // ָ���¼
        BOOL                    m_bIsRecord ;                     // �Ƿ�ʼָ���¼
        char *                  m_pRecordFileName ;               // ָ���¼�ļ���
        BOOL                    m_bIsIntoAPI ;                    // �Ƿ����API
        BOOL                    m_nIsTCommand ;                   // ֮ǰ�Ƿ���T����
        unsigned int            m_uSoftBreakResume ;              // ������ϵ���Ҫ�ָ�
        unsigned int            m_uDRBreakResume ;                // �ĸ�Ӳ���ϵ���Ҫ�ָ�                     
        int                     m_nBreakCount ;                   // ��ǰ�����ϵ����
        void                  * m_lpBaseOfImage ;                 // �������
        DWORD                   m_lpStartAddress ;                // OEP
        DWORD                   m_dwThreadID ;                  
        DWORD                   m_dwProcessID ;
        HANDLE                  m_hThread ;
        HANDLE                  m_hProcess ;
        PROCESS_INFORMATION     m_pi ;
        STARTUPINFO             m_StartupInfo ;
        char                  * m_pExePathName ;
        List                    m_BreakPointList ;                // ���Դ洢��ϵ���Ϣ
        Node                    m_TmpBreakPoint ;                 // ���Դ洢һ���Ե�����ϵ㣬���絥��
        DEBUG_EVENT             m_DebugEvent ;
        PEAnalyse               m_PEInformation ;    
        DllExportFun            m_DllExportFun ;
        char                    m_UseDrRegister ;                 // ������¼DR0-3�Ĵ�����ʹ�����
        int                     m_nIsTmpDrBreak ;                 // ������¼�ǲ�����ʱ�ϵ�

        // ��������ڴ�ϵ�
        PVOID                   m_nPageAddress ;                  // ��¼Ҫ�ָ����Ե��ڴ��ַ
        CMemBreak               m_MemBreak ;                      // ��¼�û����ڴ�ϵ���Ϣ
        CMemList                m_MemList ;                       // ��¼Ŀ�������Ч���ڴ��ҳ��Ϣ
        Adjacency               m_BreakMapPage ;                  // ��¼�ϵ��Ӧ�ڴ�ҳ��Ϣ
        Adjacency               m_PageMapBreak ;                  // ��¼�ڴ�ҳ��Ӧ�ϵ���Ϣ
        static Command          c_ExternCommand[] ;
        DWORD                   m_dwProtect ;                     // ���ڱ���ԭ�����ڴ�����
        
        // ������
        char                    m_szInstrBuf[ShellCodeLen] ;
        BOOL                    m_bIsReDebugger ;

        
};

#endif // !defined(AFX_DEBUG_H__1FDCD6B4_ADE1_4103_9765_1A4394B7982A__INCLUDED_)
