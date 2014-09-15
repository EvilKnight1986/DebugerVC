/*******************************************************************************
*
*  Copyright (c) all 2010 黄奇 All rights reserved
*  FileName : debug.h
*  D a t e  : 2010.3.4
*  功   能  : 实现调试功能头文件
*  说   明  :
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

// 例行对应处理消息函数
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
        // 插入一个有效的内存分页
        int     InsertEffectiveMemoryPage(IN LPVOID lpAddr, OUT PMEMORY_BASIC_INFORMATION pMbi) ;
        // 判断地址是否处于有效分页之内
        int     IsEffectiveAddress(IN LPVOID lpAddr, OUT MEMORY_BASIC_INFORMATION *pMbi) ;
        // 反反调试
        int     ReDebug(void);
        // 显示已经加载的DLL名称
        int     DisplayLoadDllName(void);
        // 杀死当前被调试程序
        int     Kill(void) ;
        // 重新调试
        int     Restart(void);
        // 处理扩展命令
        int     ParseExternCommand(IN char *szBuffer);
        // 将记录的指令写进文件
        int     WriteRecordToFile(void) ;
        // 指令记录
        int     InstructionRecord(void) ;
        // 处理S命令
        int     ParseSCommand(IN char * pszCmd) ;
        // 编辑寄存器的值
        int     EditRegisterValue(IN char *pszCmd) ;
        // 将用硬件断点加速软件断点的信息转成软件断点存储
        int     OnceDrBreakToSoftBreak(void) ;
        int     ParseEditMemory(IN LPVOID pAddr) ;
        // 输出帮助
        void    PrintCommandHelp(IN char ch) ;
        // 判断寄存器的值是否是api地址,同时取得dll名和函数名
        int     ParseFunNameOfRegisterValue(IN t_disasm &da, OUT char *pszDllName, OUT char *pszFunName) ;
        // 显示目标进程内存数据
        int     DisplayDestProcessMemory(IN LPVOID pAddr, IN int nLen) ;
        // 处理D命令
        int     ParseDCommand(IN char *pszCmd) ;
        // 移除硬件断点
        int     RemoveDrRegister(IN int nDrID) ;
        // 取得指定DR寄存器的值
        int     GetDrNumberOfAddress(IN LPVOID pAddr);
        // 输出断点与内存页相互的关系
        int     DisplayAdjacencyInformation(void);
        // 给定一个地址，判断是否处在断点内
        int     HaveInBreakArea(IN int nAddrm,IN PMemBreakNode pBreakNode) ;
        // 移除内存页属性
        int     RemoveMemoryProtect(IN int nProtect, IN int Attrib) ;
        // 修复内存页属性
        int     RepairMemoryProtect(IN int nPageID) ;
        // 移除内存断点
        int     RemoveMemoryBreak(IN LPVOID pAddr) ;
        // 将内存页属性设为可读可写可执行
        int     SetMemoryPageProtectEx(IN LPVOID pAddr, IN int nLen) ;
        // 判断内存属性中是否有指定的属性
        int     IsHavememoryProtect(IN int nProtect, IN int Attrib) ;
        // 添加内存断点
        int     AppendMemoryBreak(IN LPVOID nAddr, IN SIZE_T nLen, IN DWORD dwPurview) ;
        // 处理单步异常
        int     ParseSingleSetp(void) ;
        BOOL    SetCurrentThreadContext(IN  CONTEXT *pContext) ;
        BOOL    GetCurrentThreadContext(OUT CONTEXT *pContext) ;
        // 显示硬件断点信息
        void    DispDrBreakPoint(void) ;
        // 设置DRx断点
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
        int     ParseLoadDllExportTable(IN DEBUG_EVENT &debugEvent);     // 处理加载dll后导出表的分析
        int     SetSingleStep();                                         // 置单步
        int     GetInstructionLen(IN DWORD dwThreadId, OUT int *pLen);   // 取得指令长度
        int     GetCurrentEip(IN DWORD dwThreadId);                      // 取得当前EIP
        int     PrintInstruction(IN int Eip, IN BOOL bIsContimue, IN int nItem);   // 输出指令
        void    PrintContext(void);                                     // 输出寄存器等信息
        int     ParseRequest(void);                                      // 处理用户请求
        int     ParseBreakPoint(void);                                   // 处理断点
        DWORD   ParseAccessVioltion(void) ;                              // 处理访问异常
        BOOL    SetExePathName(IN char *_pExePatchName) ;                  // 设置被调试程序路径
        int     StartDebug(IN char *pExeName = NULL) ;               

private:
        AberranceTree           m_InstructionRecord ;             // 指令记录
        BOOL                    m_bIsRecord ;                     // 是否开始指令记录
        char *                  m_pRecordFileName ;               // 指令记录文件名
        BOOL                    m_bIsIntoAPI ;                    // 是否跟进API
        BOOL                    m_nIsTCommand ;                   // 之前是否是T命令
        unsigned int            m_uSoftBreakResume ;              // 当个软断点需要恢复
        unsigned int            m_uDRBreakResume ;                // 哪个硬件断点需要恢复                     
        int                     m_nBreakCount ;                   // 当前设的软断点个数
        void                  * m_lpBaseOfImage ;                 // 镜像基础
        DWORD                   m_lpStartAddress ;                // OEP
        DWORD                   m_dwThreadID ;                  
        DWORD                   m_dwProcessID ;
        HANDLE                  m_hThread ;
        HANDLE                  m_hProcess ;
        PROCESS_INFORMATION     m_pi ;
        STARTUPINFO             m_StartupInfo ;
        char                  * m_pExePathName ;
        List                    m_BreakPointList ;                // 用以存储软断点信息
        Node                    m_TmpBreakPoint ;                 // 用以存储一次性的软件断点，比如单步
        DEBUG_EVENT             m_DebugEvent ;
        PEAnalyse               m_PEInformation ;    
        DllExportFun            m_DllExportFun ;
        char                    m_UseDrRegister ;                 // 用来记录DR0-3寄存器的使用情况
        int                     m_nIsTmpDrBreak ;                 // 用来记录是不是临时断点

        // 用于完成内存断点
        PVOID                   m_nPageAddress ;                  // 记录要恢复属性的内存地址
        CMemBreak               m_MemBreak ;                      // 记录用户的内存断点信息
        CMemList                m_MemList ;                       // 记录目标进程有效的内存分页信息
        Adjacency               m_BreakMapPage ;                  // 记录断点对应内存页信息
        Adjacency               m_PageMapBreak ;                  // 记录内存页对应断点信息
        static Command          c_ExternCommand[] ;
        DWORD                   m_dwProtect ;                     // 用于保存原来的内存属性
        
        // 反调试
        char                    m_szInstrBuf[ShellCodeLen] ;
        BOOL                    m_bIsReDebugger ;

        
};

#endif // !defined(AFX_DEBUG_H__1FDCD6B4_ADE1_4103_9765_1A4394B7982A__INCLUDED_)
