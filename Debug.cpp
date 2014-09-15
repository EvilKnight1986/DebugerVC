/*******************************************************************************
*
*  Copyright (c) all 2010 黄奇 All rights reserved
*  FileName : debug.cpp
*  D a t e  : 2010.3.4
*  功   能  : 调试功能实现源文件
*  说   明  :
*
*******************************************************************************/

#include "Debug.h"

char chSoftBreak = '\xcc' ;

#define BUFFER_MAX      128
#define MEMPAGE_LEN     4

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Debug::Debug():m_pExePathName(NULL),m_hProcess(INVALID_HANDLE_VALUE),
               m_hThread(INVALID_HANDLE_VALUE),m_dwProcessID(0),m_dwThreadID(0),
               m_lpStartAddress(0),m_lpBaseOfImage(0),m_nBreakCount(0),
               m_uSoftBreakResume(0),m_UseDrRegister(0),m_uDRBreakResume(0),
               m_nIsTCommand(0),m_nPageAddress(0),m_nIsTmpDrBreak(FALSE),
               m_bIsIntoAPI(FALSE),m_pRecordFileName(NULL),m_bIsRecord(FALSE),
               m_dwProtect(0)
{
}

Debug::~Debug()
{
        if (NULL != m_pExePathName)
        {
                delete [] m_pExePathName ;
                m_pExePathName = NULL ;
        }

        if (NULL != m_pRecordFileName)
        {
                delete [] m_pRecordFileName ;
                m_pRecordFileName = NULL ;
        }
}

// 扩展命令
Command Debug::c_ExternCommand[] = {".restart", Restart,
                                    ".show",    DisplayLoadDllName,
                                    ".kill",    Kill} ;

/*******************************************************************************
*
*  函 数 名 : SetExePathName
*  功能描述 : 设置被调试可执行程序的名字
*  参数列表 : IN _pExePatchName    --  被调试程序的名称
*  说    明 : 
*  返回结果 : 如果返回FALSE的话出错,成功返回TRUE
*
*******************************************************************************/
BOOL Debug::SetExePathName(IN char *_pExePatchName)
{
        if (NULL == _pExePatchName)
        {
                return FALSE ;
        }

        if (NULL != m_pExePathName)
        {
                delete [] m_pExePathName ;
                m_pExePathName = NULL ;
        }

        int nLen = strlen(_pExePatchName) ;
        if (nLen >= BUFFER_MAX)
        {
                printf("大哥，别乱来?\r\n") ;
                return FALSE ;
        }

        if (nLen < 5)
        {
                printf("文件名太短了吧!\r\n") ;
                return FALSE ;
        }
        
        if (NULL != m_pExePathName)
        {
                delete [] m_pExePathName ;
                m_pExePathName = NULL ;
        }

        ++nLen ;
        m_pExePathName = new char[nLen] ;

        if (NULL == m_pExePathName)
        {
                return FALSE ;
        }

        memset(m_pExePathName, 0, sizeof(char) * nLen) ;
        strcpy(m_pExePathName, _pExePatchName) ;

        return TRUE ;

}

/*******************************************************************************
*
*  函 数 名 : StartDebug
*  功能描述 : 开启调试功能入口函数
*  参数列表 : IN _pExePatchName    --  被调试程序的名称
*  说    明 : 
*  返回结果 : 如果返回0的话是文件名出错,创建进程出错则返回-1，
*
*******************************************************************************/
int Debug::StartDebug(IN char *_pExePatchName)
{
        if (NULL == m_pExePathName && NULL == _pExePatchName)
        {
                return 0 ;
        }

        if (_pExePatchName != NULL)
        {
                if (FALSE == SetExePathName(_pExePatchName))
                {
                        return 0 ;
                }
        }

        GetStartupInfo(&m_StartupInfo) ;
        m_PEInformation.Analyse(m_pExePathName) ;
        BOOL bRet = CreateProcess(m_pExePathName, NULL, NULL, NULL, FALSE, 
                        DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS, NULL, NULL,
                        &m_StartupInfo, &m_pi) ;
        
        m_dwThreadID = m_pi.dwThreadId ;
        m_dwProcessID = m_pi.dwProcessId ;
        m_hProcess = m_pi.hProcess ;

        // 因为线程不一定是我们要的，所以先不用给先
        DWORD dwContinueStatus = DBG_CONTINUE; // exception continuation 
        
        for(;;) 
        { 
                
                // Wait for a debugging event to occur. The second parameter indicates 
                // that the function does not return until a debugging event occurs. 
                dwContinueStatus = DBG_CONTINUE ;
                
                WaitForDebugEvent(&m_DebugEvent, INFINITE); 
                
                // Process the debugging event code. 
                
                switch (m_DebugEvent.dwDebugEventCode) 
                { 
                        case EXCEPTION_DEBUG_EVENT: 
                        // Process the exception code. When handling 
                        // exceptions, remember to set the continuation 
                        // status parameter (dwContinueStatus). This value 
                        // is used by the ContinueDebugEvent function. 
                         {
                        
                                switch (m_DebugEvent.u.Exception.ExceptionRecord.ExceptionCode) 
                                { 
                                        case EXCEPTION_ACCESS_VIOLATION: 
                                        {
                                        // First chance: Pass this on to the system. 
                                        // Last chance: Display an appropriate error. 
                                                dwContinueStatus = ParseAccessVioltion() ;
                                        }

                                        break ;
                                
                                        case EXCEPTION_BREAKPOINT: 
                                        {
                                        // First chance: Display the current 
                                        // instruction and register values. 
                                                if (1 == isMyBreakPoint())
                                                {
                                                        dwContinueStatus = ParseBreakPoint() ;
                                                        // 处理用户的请求
                                                        if (FALSE == m_bIsRecord)
                                                        {
                                                                ParseRequest() ;
                                                        }
                                                }


                                        }
                                        break;
                                
                                        case EXCEPTION_DATATYPE_MISALIGNMENT: 
                                        {
                                        // First chance: Pass this on to the system. 
                                        // Last chance: Display an appropriate error. 
                                        }
                                        break;
                                
                                        case EXCEPTION_SINGLE_STEP: 
                                        {
                                        // First chance: Update the display of the 
                                        // current instruction and register values. 
                                        // 这里假设用户的代码中不会修改单步标志
                                                // 先看看是不是软断点需要恢复
                                                ParseSingleSetp() ;
                                        }
                                        break ;

                                
                                        case DBG_CONTROL_C: 
                                        {
                                        // First chance: Pass this on to the system. 
                                        // Last chance: Display an appropriate error. 
                        
                                        // Handle other exceptions. 
                                                //ExitProcess(0) ;
                                        }
                                        break;
                                } // end of DebugEv.u.Exception.ExceptionRecord.ExceptionCode
                                break ; 
                         } // end of EXCEPTION_DEBUG_EVENT
                        
                        case CREATE_THREAD_DEBUG_EVENT: 
                        {
                                // As needed, examine or change the thread's registers 
                                // with the GetThreadContext and SetThreadContext functions; 
                                // and suspend and resume thread execution with the 
                                // SuspendThread and ResumeThread functions. 
                                m_hThread = m_DebugEvent.u.CreateThread.hThread ;
#ifdef _MICROQ
                                MessageBox(NULL, TEXT("线程创建"), TEXT("Tips"), MB_OK) ;
#endif
                         }
                        break ;
                               
                                
                        case CREATE_PROCESS_DEBUG_EVENT: 
                        {
                                // As needed, examine or change the registers of the 
                                // process's initial thread with the GetThreadContext and 
                                // SetThreadContext functions; read from and write to the 
                                // process's virtual memory with the ReadProcessMemory and 
                                // WriteProcessMemory functions; and suspend and resume 
                                // thread execution with the SuspendThread and ResumeThread 
                                // functions. 
                                
                                // 取到程序的线程句柄等等
                                m_hThread = m_DebugEvent.u.CreateProcessInfo.hThread ;
                                m_hProcess = m_DebugEvent.u.CreateProcessInfo.hProcess ;
                                m_lpStartAddress = (DWORD)m_DebugEvent.u.CreateProcessInfo.lpStartAddress ;
                                m_lpBaseOfImage = m_DebugEvent.u.CreateProcessInfo.lpBaseOfImage ;
                                
                                // 在程序入口处下个断点
                                char ch ;
                                BOOL bRet ;
                                DWORD dwNumberOfBytesRead ;
                                // 因为要让程序停在入口，所以这里在入口下个CC断点，
                                // 先将原来的值保存
                                bRet = ReadProcessMemory(m_hProcess,(LPCVOID)m_lpStartAddress,
                                        (PVOID)&ch, sizeof(char), &dwNumberOfBytesRead) ;
                                if (FALSE == bRet)
                                {
                                        OutputDebugString("读目标进程出错了\r\n") ;
                                        break ;
                                }

                                // 写下CC 将断点信息放到临时断点结构体里
                                //m_BreakPointList.Insert(m_lpStartAddress, ch) ;

                                m_TmpBreakPoint.byOldValue = ch ;
                                m_TmpBreakPoint.nAddr = m_lpStartAddress ;
                                m_TmpBreakPoint.StdState() ;

                                bRet = WriteProcessMemory(m_hProcess,(LPVOID)m_lpStartAddress,
                                        (PVOID)&chSoftBreak,sizeof(char),&dwNumberOfBytesRead) ;
                                if (FALSE == bRet)
                                {
                                        OutputDebugString("写目标进程出错了\r\n") ;
                                        break ;
                                }

                                // 显示PE信息
                                m_PEInformation.DispPEInformation() ;
                                //m_MemList.RemoveAll() ;
                                // 枚举系统有效内存分页
                                //m_MemList.EnumDestProcessEffectivePage(m_hProcess) ;
                                // 输出被调试进程有效的内存页
                                //m_MemList.Display() ;
                                printf("\r\n") ;
                                m_DllExportFun.SetProcessValue(m_hProcess) ;
                        }
                        break ;
                                
                        case EXIT_THREAD_DEBUG_EVENT: 
                        {
                                // Display the thread's exit code. 
                                OutputDebugString("目标线程创建!\r\n") ;
                                //ExitProcess(0) ;
                        }
                        break ;
                                
                        case EXIT_PROCESS_DEBUG_EVENT: 
                        {
                        // Display the process's exit code. 
                                if (TRUE == m_bIsRecord)
                                {
                                        WriteRecordToFile() ;
                                }
                                OutputDebugString( "目标进程退出!\r\n") ;
                                //ExitProcess(0) ;
                        }
                        break;
                                
                        case LOAD_DLL_DEBUG_EVENT: 
                        {
                        // Read the debugging information included in the newly 
                        // loaded DLL. 
                        // 加载dll
                                if ((int)m_DebugEvent.u.LoadDll.lpBaseOfDll & 0xff0000)
                                {
                                        ParseLoadDllExportTable(m_DebugEvent) ;
                                }
                        }
                        break;
                                
                        case UNLOAD_DLL_DEBUG_EVENT: 
                        {
                        // Display a message that the DLL has been unloaded. 
                        }
                        break;
                                
                        case OUTPUT_DEBUG_STRING_EVENT: 
                        {
                        // Display the output debugging string. 
                        }
                        break;
                                
                } 
                
                // Resume executing the thread that reported the debugging event. 
                
                ContinueDebugEvent(m_DebugEvent.dwProcessId, 
                        m_DebugEvent.dwThreadId, dwContinueStatus); 
                
        } 


        return 0 ;                      
}

/*******************************************************************************
*
*  函 数 名 : GetCurrentEip
*  功能描述 : 取得当前被调试线程的EIP
*  参数列表 : IN dwThreadId:    线程id
*  说    明 : 
*  返回结果 : 成功返回eip值，失败返回0
*
*******************************************************************************/
int Debug::GetCurrentEip(IN DWORD dwThreadId)
{
        if (0 == dwThreadId)
        {
                return 0 ;
        }

        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        if(FALSE == GetCurrentThreadContext(&context))
        {
                return 0 ;
        }

        return context.Eip ;
}

/*******************************************************************************
*
*  函 数 名 : PrintContext
*  功能描述 : 输出寄存器环境
*  参数列表 : 
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
void Debug::PrintContext(void)
{
        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        if(FALSE == GetCurrentThreadContext(&context))
        {
                return ;
        }

        printf("EAX=%p  ",   context.Eax) ;
        printf("EBX=%p  ",   context.Ebx) ;
        printf("ECX=%p  ",   context.Ecx) ;
        printf("EDX=%p\r\n", context.Edx) ;

        printf("ESI=%p  ",  context.Esi) ;
        printf("EDI=%p  ",context.Edi) ;

        printf("ESP=%p  ",  context.Esp) ;
        printf("EBP=%p\r\n",  context.Ebp) ;


        printf("EIP=%p  ", context.Eip) ;
        printf("iopl=%2d  ", context.EFlags & 0x3000) ;

        printf("    %3s", (context.EFlags & 0x800)?"ov":"nv") ;
        printf("%3s", (context.EFlags & 0x400)?"dn":"up") ;
        printf("%3s", (context.EFlags & 0x200)?"ei":"di") ;
        printf("%3s", (context.EFlags & 0x80 )?"ng":"pl") ;
        printf("%3s", (context.EFlags & 0x40 )?"zr":"nz") ;
        printf("%3s", (context.EFlags & 0x10 )?"ac":"nc") ;
        printf("%3s", (context.EFlags & 0x4  )?"pe":"po") ;
        printf("%3s\r\n", (context.EFlags & 0x1  )?"cy":"nc") ;
        //cs=001b  ss=0023  ds=0023  es=0023  fs=003b  gs=0000
        printf("cs=%04X  ss=%04X  ds=%04X  es=%04X  fs=%04X  gs=%04X\r\n",
                context.SegCs, context.SegSs, context.SegDs, context.SegEs,
                context.SegFs, context.SegGs) ;
}


/*******************************************************************************
*
*  函 数 名 : PrintInstruction
*  功能描述 : 输出指令
*  参数列表 : IN Eip         ---    Eip值
              IN bIsContimue ---    是否接着上次的输出
              IN nItem       ---    预输出指令条数

*  说    明 : 
*  返回结果 : 成功返回指令，失败返回0
*
*******************************************************************************/
int Debug::PrintInstruction(IN int Eip, IN BOOL bIsContimue, IN int nItem)
{
        if (INVALID_HANDLE_VALUE == m_hProcess )
        {
                OutputDebugString("进程句柄无效\r\n") ;
                return 0;
        }

        DWORD dwTmpEip = 0 ;
        DWORD dwProtect = 0 ;
        dwTmpEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;

        if (0 != Eip)
        {
                if( FALSE == VirtualProtectEx(m_hProcess, (LPVOID)Eip, BUFFER_MAX, 
                                PAGE_EXECUTE_READWRITE, &dwProtect))
                {
                        return 0 ;
                }
        }
        else
        {
                if( FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwTmpEip, BUFFER_MAX, 
                                PAGE_EXECUTE_READWRITE, &dwProtect))
                {
                        return 0 ;
                }
        }

        static DWORD dwEip = 0 ;
        if (FALSE == bIsContimue)
        {
                dwEip = Eip ;
        }

        // 先恢复原来的指令
        RevertAllBreakPoint() ;

        char szBuf[BUFFER_MAX] = {0} ;
        DWORD  dwNumberOfBytesRead ;
        if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                                        BUFFER_MAX, &dwNumberOfBytesRead))
        {
                OutputDebugString("读目标进程失败\r\n") ;
                return 0;
        }
        
        t_disasm da = {0};
        int nIndex = 0 ;

        int nInstrLen = 0 ;

        for (int i = 0 ; i < nItem; ++i)
        {
                nInstrLen = Disasm(&szBuf[nIndex], 20, dwEip, &da,DISASM_CODE);
                printf("%p  %-16s  %s", da.ip, da.dump, da.result) ;
                char szDllName[MAXBYTE] = {0} ;
                char szFunName[MAXBYTE] = {0} ;
                if(m_DllExportFun.AddressToFunName(da, szDllName, szFunName))
                {
                        printf(" <%s::", szDllName) ;
                        printf("%s>",  szFunName) ;      
                }
                else if (1 == nItem)
                {
                        if (ParseFunNameOfRegisterValue(da, szDllName, szFunName))
                        {
                                printf(" <%s::", szDllName) ;
                                printf("%s>",  szFunName) ;   
                        }
                }
                printf("\r\n") ;
                dwEip += nInstrLen ;
                nIndex += nInstrLen ;
        }
        // 再重新将断点设回去
        SetAllBreakPoint() ;
        if (0 != Eip)
        {
                if (FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, 
                                dwProtect, &dwProtect))
                {
                        return 0 ;
                }
        }
        else
        {
                if (FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwTmpEip, BUFFER_MAX, 
                        dwProtect, &dwProtect))
                {
                        return 0 ;
                }
        }
        return nIndex ;
}

/*******************************************************************************
*
*  函 数 名 : GetInstructionLen
*  功能描述 : 取得指定线程下一条指令的长度
*  参数列表 : IN  dwThreadId:              线程id
              OUT pLen:                    指令长度
*  说    明 : 
*  返回结果 : 如果成功返回Eip，否则返回0
*
*******************************************************************************/
int Debug::GetInstructionLen(IN DWORD dwThreadId, OUT int *pLen)
{        
        if (NULL == m_hProcess || INVALID_HANDLE_VALUE == m_hProcess)
        {
                return 0 ;
        }

        *pLen = 0 ;
        char szBuf[BUFFER_MAX] = {0} ;
        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;
        if (FALSE == GetCurrentThreadContext(&context) )
        {
                return 0 ;
        }

        DWORD dwProtect = 0 ;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)context.Eip, BUFFER_MAX, 
                        PAGE_EXECUTE_READWRITE, &dwProtect))
        {
                OutputDebugString("修改内存属性出错!\r\n") ;
                return 0 ;
        }

        DWORD   dwNumberOfBytesRead ;
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)context.Eip, szBuf, 
                                                BUFFER_MAX, &dwNumberOfBytesRead))
        {
                CloseHandle(m_hThread) ;
                m_hThread = INVALID_HANDLE_VALUE ;
        }

        // 这里是调用OD的反汇编引擎
        t_disasm td ;
        *pLen = Disasm(szBuf, 20, 0, &td, DISASM_CODE) ;

        VirtualProtectEx(m_hProcess, (LPVOID)context.Eip, BUFFER_MAX, 
                                                dwProtect, &dwProtect) ;

        return context.Eip;
}

/*******************************************************************************
*
*  函 数 名 : ParseAccessVioltion
*  功能描述 : 处理访问异常
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
DWORD Debug::ParseAccessVioltion(void)
{
        DWORD nType = m_DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[0] ;
        LPVOID pAddr = (LPVOID)m_DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[1] ;

        // 加1和自己定义的 1是读，2是写
        ++nType ;
        int nPage = 0 ;


        // 取得内存页id
        if ( !m_MemList.HaveIn(pAddr, nPage))
        {
                return DBG_EXCEPTION_NOT_HANDLED ;
        }

        // 判断这个页有没有断点信息，没有的话返回
        aList *p = &m_PageMapBreak[nPage] ;
        if (NULL == p)
        {
                return DBG_EXCEPTION_NOT_HANDLED ;
        }
        // 如果没有断点信息
        int nBreakCount = p->GetSize() ;
        if (0 == nBreakCount)
        {
                return DBG_EXCEPTION_NOT_HANDLED ;
        }

        
        PMemBreakNode pBreakNode = NULL ;
        // 取得保存内存页属性的节点
        PMemNode pMemNode = m_MemList.FindById(nPage) ;
        m_nPageAddress = pAddr ;
        DWORD dwProtect = 0 ;
        if (NULL == pMemNode)
        {
                return 0 ;
        }

        if(FALSE == VirtualProtectEx(m_hProcess, pAddr, MEMPAGE_LEN, 
                                pMemNode->Protect, &dwProtect))
        {
                return 0 ;
        }
        m_dwProtect = dwProtect ;
        // 再判断一下有没有命中
        for (int i = 0; i < nBreakCount; ++i)
        {
                // 得到那个分页中第i个结点
                DistPosition pNode = m_PageMapBreak[nPage][i] ;
                // 通过pNode->data得到断点号,再通过FinByID得到断点信息的指针
                pBreakNode = m_MemBreak.FindById(pNode->data) ;
                if (HaveInBreakArea((int)pAddr, pBreakNode)
                        && pBreakNode->Protect == nType)
                {
                        // 如果是处于指令记录的话，就不用给用户输入请示
                        if (TRUE == m_bIsRecord)
                        {
                                InstructionRecord() ;
                                SetTFRegister() ;
                                return DBG_CONTINUE ;
                        }
                        else if (FALSE == m_bIsRecord)
                        {
                                PrintContext() ;
                                PrintInstruction(GetCurrentEip(m_DebugEvent.dwThreadId), FALSE, 1) ;
                                SetTFRegister() ;
                                printf("内存中断!\r\n") ;
                                ParseRequest() ;
                                return DBG_CONTINUE ;
                        }
                }
                
        }
        
        // 置单步
        SetTFRegister() ;

        return DBG_CONTINUE ;
}

/*******************************************************************************
*
*  函 数 名 : isMyBreakPoint
*  功能描述 : 是否是用户设的断点或者是系统第一次调用的软件中断
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果是自己能处理的断点，则返回1，否则返回0
*
*******************************************************************************/
int Debug::isMyBreakPoint(void)
{
        // 先判断是不是系统自动调的 BreakPoint
        if (m_nBreakCount < 1)
        {
                ++m_nBreakCount ;
                return 0 ;
        }

        if (m_nBreakCount == 1)
        {
                ++m_nBreakCount ;
        }
        
        
        char ch = 0 ;
        unsigned int nAddr = (int)m_DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress ;   
        DWORD dwNumberOfBytesWritten = 0 ;
        
        // 先在Temp断点中找
        if (m_TmpBreakPoint.bIsActive && m_TmpBreakPoint.nAddr == nAddr)
        {
                return 1 ;
        }
        
        // 在断点链表中找
        int nRet = m_BreakPointList.GetValue(nAddr, &ch) ;
        if (nRet != 0 )
        {
                return 1 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : ParseBreakPoint
*  功能描述 : 处理断点异常
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果能处理，返回DBG_CONTINUE，否则返回DBG_EXCEPTION_NOT_HANDLED
*
*******************************************************************************/
int Debug::ParseBreakPoint(void)
{
        // 先判断是不是系统自动调的 BreakPoint
        if (m_nBreakCount < 1)
        {
                ++m_nBreakCount ;
                return DBG_CONTINUE ;
        }

        char ch = 0 ;
        unsigned int nAddr = (int)m_DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress ;   
        DWORD dwNumberOfBytesWritten = 0 ;
        m_dwThreadID = m_DebugEvent.dwThreadId ;

        DWORD dwProtect = 0 ;
        if (NULL == VirtualProtectEx(m_hProcess, (LPVOID)nAddr, MEMPAGE_LEN, 
                                PAGE_EXECUTE_READWRITE, &dwProtect))
        {
                OutputDebugString("修改内存属性出错!\r\n") ;
                return 0 ;
        }

        // 先在Temp断点中找
        if (m_TmpBreakPoint.bIsActive && m_TmpBreakPoint.nAddr == nAddr)
        {
                ch = m_TmpBreakPoint.byOldValue ;
                if (WriteProcessMemory(m_hProcess, (LPVOID)nAddr, &ch, sizeof(char),
                                        &dwNumberOfBytesWritten))
                m_TmpBreakPoint.ClsState() ;
                
                CONTEXT context ;
                context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

                if (FALSE == GetCurrentThreadContext(&context))
                {
                        return DBG_EXCEPTION_NOT_HANDLED ;
                }

                context.Eip -= 1 ;

                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return DBG_EXCEPTION_NOT_HANDLED ;
                }

                if(FALSE == m_bIsRecord)
                {
                        PrintContext() ;
                        PrintInstruction(context.Eip, FALSE, 1) ;
                        printf("int3中断!\r\n") ;
                }
                else
                {
                        t_disasm da = {0};
                        char szBuf[BUFFER_MAX] = {0};
                        DWORD dwNoting = 0 ;
                        ReadProcessMemory(m_hProcess, (LPVOID)context.Eip,szBuf, BUFFER_MAX, &dwNoting);

                        int nLen = Disasm(szBuf, 20, context.Eip, &da, DISASM_CODE);
                        AberrTreeNode * p = m_InstructionRecord.Insert(context.Eip, 
                                                                        szBuf, nLen) ;
                        int nJmpAddr = 0 ;

                        nJmpAddr = m_DllExportFun.AddressToFunName(da, szBuf, szBuf) ;
                        if (0 != nJmpAddr)
                        {
                                p->jmpconst = (LPVOID)nJmpAddr ;
                        }
                        VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX, 
                                                dwProtect, &dwProtect) ;
                        InstructionRecord() ;
                        return DBG_CONTINUE ;
                }

                m_TmpBreakPoint.bIsActive = FALSE ;
                VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX, 
                                        dwProtect, &dwProtect);
                return DBG_CONTINUE ;
        }

        // 在断点链表中找
        if (0 != m_BreakPointList.GetValue(nAddr, &ch))
        {
                if (WriteProcessMemory(m_hProcess, (LPVOID)nAddr, &ch,
                                sizeof(char), &dwNumberOfBytesWritten))
                {
                        // eip减1
                        CONTEXT context ;
                        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

                        if (FALSE == GetCurrentThreadContext(&context))
                        {
                                return DBG_EXCEPTION_NOT_HANDLED ;
                        }

                        context.Eip -= 1 ;
                        
                        if (FALSE == SetCurrentThreadContext(&context))
                        {
                                return DBG_EXCEPTION_NOT_HANDLED ;
                        }


                        // 下次要恢复的软断点标志
                        m_uSoftBreakResume = nAddr ;
                        // 将这个断点设为失效
                        m_BreakPointList.Find(nAddr)->ClsState() ; 
                        VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX,
                                        dwProtect, &dwProtect) ;

                        if(FALSE == m_bIsRecord)
                        {
                                SetTFRegister() ;
                                PrintContext() ;
                                PrintInstruction(context.Eip, FALSE, 1) ;
                                printf("int3中断!\r\n") ;
                                SetTFRegister();
                        }
                        else
                        {
                                InstructionRecord() ;
                        }
                        return DBG_CONTINUE ;
                }


        }
        VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX,
                                        dwProtect, &dwProtect) ;
        if (TRUE == m_bIsRecord)
        {
                InstructionRecord() ;
        }
        return DBG_EXCEPTION_NOT_HANDLED ;
}


/*******************************************************************************
*
*  函 数 名 : ParseRequest
*  功能描述 : 处理用户输入的请求
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果能够处理的话返回1,不能处理返回0
*
*******************************************************************************/
int Debug::ParseRequest(void)
{
        BOOL  bFlag = TRUE ;
        BOOL  isUCommand = FALSE ;
        static FILE  *fp = NULL ;                      // 文件指针,处理脚本用
        static BOOL  bIsScript = FALSE ;               // 当前是否从脚本输入
        
        while (bFlag)
        {
                printf("-") ;
                char szBuffer[BUFFER_MAX] = {0} ;

                // 控制输入，从文件输入还是等待用户从键盘输入
                if (FALSE == bIsScript)
                {
                        // 防止溢出,安全的输入
                        SafeInput(szBuffer, BUFFER_MAX) ;
                }
                else
                {
                        if (feof(fp) || 0 == GetLine(fp, szBuffer, BUFFER_MAX))
                        {
                                bIsScript = FALSE ;
                                fclose(fp) ;
                                fp = NULL ;
                                SafeInput(szBuffer, BUFFER_MAX) ;
                        }
                        else
                        {
                                printf("%s\r\n", szBuffer) ;
                        }
                }

                // 去除空格
                SkipSpace(szBuffer, BUFFER_MAX) ;

                // 处理用户命令
                // 这里没有用消息表，我觉得这样处理会更加灵活
                // 但是函数会显得很肥胖
                switch(szBuffer[0])
                {
                case 'b':
                case 'B':
                        // 处理B命令的命令，比如bp, ba, bd
                        isUCommand = FALSE ;
                        ParseBCommand(szBuffer) ;
                        break ;
                case 'e':
                case 'E':
                        // 这个功能还没有做，修改内存的值
                        {
                                isUCommand = FALSE ;
                                LPVOID pAddr = 0 ;
                                int nParamCount = GetParamCount(szBuffer) ;
                                if (1 == nParamCount)
                                {
                                        pAddr = (LPVOID)GetCurrentEip(m_DebugEvent.dwThreadId) ;
                                        ParseEditMemory(pAddr) ;
                                }
                                if (2 == nParamCount)
                                {
                                        sscanf(szBuffer, "%s%x", stderr, &pAddr) ;
                                        ParseEditMemory(pAddr) ;
                                }
                        }
                        break ;

                case 't':
                case 'T':
                        // 单步步进
                        isUCommand = FALSE ;
                        m_nIsTCommand = TRUE ;
                        SetTFRegister() ;
                        bFlag = FALSE ;
                        break ;
                case 'p':
                case 'P':
                        // 单步步过
                        isUCommand = FALSE ;
                        SetSingleStep() ;
                        bFlag = FALSE ;
                        break ;
                case 'r':
                case 'R':
                        isUCommand = FALSE ;
                        
                        if (2 == GetParamCount(szBuffer))
                        {
                                EditRegisterValue(szBuffer) ;
                        }
                        PrintContext() ;
                        break ;
                case 'u':
                case 'U': 
                        RevertAllBreakPoint() ;
                        // 这里要处理一下u后面的参数
                        ParseUCommand(szBuffer, isUCommand) ;
                        if (FALSE == isUCommand)
                        {
                                isUCommand = TRUE ;     
                        }
                        SetAllBreakPoint() ;
                        break ;
                case '?':
                case 'h':
                case 'H':
                        PrintCommandHelp(0) ;
                        break ;
                        // 这里要处理一下g后面的参数
                case 'g':
                case 'G':
                        ParseGCommand(szBuffer) ;
                        bFlag = FALSE ;
                        break ;
                case 'l':
                case 'L':
                        isUCommand = FALSE ;
                        m_PEInformation.DispPEInformation() ;
                        break ;
                case 'V':
                case 'v':
                        DispCopyright() ;
                        break ;
                case 'q':
                case 'Q':
                        ExitProcess(0) ;
                        bFlag = FALSE ;
                        return 0 ;
                case 'd':
                case 'D':
                        // 显示内存数据
                        ParseDCommand(szBuffer) ;
                        break ;
                case 's':
                case 'S':
                        {
                             int nParamCount = GetParamCount(szBuffer) ;
                             if (3 != nParamCount && 4 != nParamCount)
                             {
                                     break ;
                             }
                             char szParam1[BUFFER_MAX] = {0} ;
                             char szParam2[BUFFER_MAX] = {0} ;
                             char szParam3[BUFFER_MAX] = {0} ;

                             // 将内存断点全部移除
                             //先还原内存属性
                             m_MemList.RevertMemoryProtectAll(m_hProcess) ;
                             //再清除内存断点的所有结点
                             m_MemBreak.RemoveAll() ;
                             // 再把邻接表的关系清空
                             m_PageMapBreak.Clear() ;
                             m_BreakMapPage.Clear() ;

                             // 清除所有的int3和硬件断点
                             RemoveAllBreakPoint() ;

                             if (0 != m_nPageAddress)
                             {
                                     m_nPageAddress = 0 ;
                             }


                             if (3 == nParamCount )
                             {
                                     sscanf(szBuffer, "%s%s%s", stderr, szParam1, szParam2) ;
                                     if (0 == isXNumString(szParam1)\
                                             || 0 == isXNumString(szParam2))
                                     {
                                             printf("Error!\r\n") ;
                                             break ;
                                     }
                                     int nBeginAddr ;
                                     int nEndAddr ;
                                     sscanf(szParam1, "%x", &nBeginAddr) ;
                                     sscanf(szParam2, "%x", &nEndAddr) ;
                                     if (0 == AppendMemoryBreak((LPVOID)nBeginAddr,
                                              nEndAddr - nBeginAddr, 1))
                                     {
                                             printf("参数出错了吧!\r\n") ;
                                             break ;
                                     }
                                     if (m_pRecordFileName)
                                     {
                                             delete [] m_pRecordFileName ;
                                             m_pRecordFileName = NULL ;
                                     }

                                     int nLen = strlen("InstructionRecord.txt") ;
                                     ++nLen ;
                                     m_pRecordFileName = new char[nLen] ;

                                     if (NULL == m_pRecordFileName)
                                     {
                                             printf("申请内存出错!\r\n") ;
                                             break ;
                                     }
                                     memset(m_pRecordFileName, 0, sizeof(char)*nLen) ;
                                     strcpy(m_pRecordFileName, "InstructionRecord.txt") ;
                                     m_bIsRecord = TRUE ;
                                     bFlag = FALSE ;
                             }
                             else if (4 == nParamCount)
                             {
                                     sscanf(szBuffer, "%s%s%s%s", stderr, szParam1,\
                                                szParam2, szParam3) ;
                                     if (0 == isXNumString(szParam1)\
                                             || 0 == isXNumString(szParam2))
                                     {
                                             printf("Error!\r\n") ;
                                             break ;
                                     }
                                     int nBeginAddr ;
                                     int nEndAddr ;
                                     sscanf(szParam1, "%x", &nBeginAddr) ;
                                     sscanf(szParam2, "%x", &nEndAddr) ;
                                     if (0 == AppendMemoryBreak((LPVOID)nBeginAddr,
                                             nEndAddr - nBeginAddr, 1))
                                     {
                                             printf("参数出错了吧!\r\n") ;
                                             break ;
                                     }
                                     if (m_pRecordFileName)
                                     {
                                             delete [] m_pRecordFileName ;
                                             m_pRecordFileName = NULL ;
                                     }

                                     int nLen = strlen(szParam3) ;
                                     ++nLen ;
                                     m_pRecordFileName = new char[nLen] ;
                                     
                                     if (NULL == m_pRecordFileName)
                                     {
                                             printf("申请内存出错!\r\n") ;
                                             break ;
                                     }
                                     
                                     memset(m_pRecordFileName, 0, sizeof(char)*nLen) ;
                                     strcpy(m_pRecordFileName, szParam3) ;
                                     m_bIsRecord = TRUE ;
                                     bFlag = FALSE ;
                             }
                        }
                        break ;
                case 'o':
                case 'O':
                        // 处理脚本
                        {
                                char szScript[BUFFER_MAX] = {0} ;
                                if(2 == GetParamCount(szBuffer))
                                {
                                        sscanf(szBuffer, "%s%s", stderr, szScript) ;
                                        fp = fopen(szScript, "r") ;
                                        if (NULL != fp)
                                        {
                                                bIsScript = TRUE ;
                                        }
                                }
                                
                        }
                        break ;
                case '.':
                        // 处理扩展命令
                        {
                                int nRet = ParseExternCommand(szBuffer) ;
                                if (nRet != 0)
                                {
                                        return nRet ;
                                }
                                else
                                {
                                        printf(" ^Error\r\n") ;
                                }
                                break ;
                        }
                default:
                        printf("^Error\r\n") ;
                }
        }
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetSingleStep
*  功能描述 : 设CC中断，置单步(单步步过)
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果成功返回1,不能处理返回0
*
*******************************************************************************/
int Debug::SetSingleStep(void)
{
        DWORD dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        if (0 == dwEip)
        {
                return 0 ;
        }

        DWORD dwProtect ;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX,
                                           PAGE_EXECUTE_READWRITE, &dwProtect))
        {
                return 0 ;
        }

        char szBuf[BUFFER_MAX] = {0} ;
        DWORD dwNumberOfBytes = 0 ;
        // 先去读指令
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                sizeof(char) * BUFFER_MAX, &dwNumberOfBytes) )
        {
                return 0;
        }

        // 这里判断是不是call,不是call的话直接设TF标志
        t_disasm da ;
        int nInstrLen = Disasm(&szBuf[0], 20, dwEip, &da,DISASM_CODE) ;
        // 如果等于call
        if (0x70 == da.cmdtype)
        {
                // 在call的下一条指令处下CC
                // 可以用G代替
                return ParseGoTo(dwEip + nInstrLen) ; 
        }
        else
        {
                m_nIsTCommand = TRUE ;
                SetTFRegister() ;
        }
        VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX,
                                           dwProtect, &dwProtect) ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetTFRegister
*  功能描述 : 设TF位，置单步(单步步进)
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果成功返回1,不能处理返回0
*
*******************************************************************************/
int Debug::SetTFRegister(void)
{
        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;
        if (FALSE ==  GetCurrentThreadContext(&context) )
        {
                return 0 ;
        }
        
        context.EFlags |= 0x100 ;
        
        if (FALSE == SetCurrentThreadContext(&context))
        {
                return 0 ;
        }
        
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : ParseLoadDllExportTable
*  功能描述 : 处理加载dll事件，分析dll的导出表，用来解析call后面的函数名
*  参数列表 : debugEvent        -- 调试事件
*  说    明 : 
*  返回结果 : 如果成功返回1,不能处理返回0
*
*******************************************************************************/
int Debug::ParseLoadDllExportTable(IN DEBUG_EVENT &debugEvent)
{
        // 分析dll的导出表
        return m_DllExportFun.InsertDll((HMODULE)(debugEvent.u.LoadDll.lpBaseOfDll)) ;
}

/*******************************************************************************
*
*  函 数 名 : DebugHelp
*  功能描述 : 输出帮助
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void Debug::DebugHelp(void)
{
        printf("支持Debug命令\r\n") ;
        return ;
}

/*******************************************************************************
*
*  函 数 名 : ParseUCommand
*  功能描述 : 处理U命令
*  参数列表 : pszCmd       --   命令字符串
*             bISContinue  --   是否在原来的基础上继续反汇编
*  说    明 : 
*  返回结果 : 成功返回1, 否则返回0
*
*******************************************************************************/
int Debug::ParseUCommand(IN char *pszCmd, IN BOOL bISContinue)
{
        DWORD dwEip = 0 ;
        if (FALSE == bISContinue)
        {
                dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        }

        int nStrLen = strlen(pszCmd) ;
        if (1 == nStrLen || nStrLen >= BUFFER_MAX)
        {
                PrintInstruction(dwEip,bISContinue, 8) ;
                return 1 ;
        }

        int nArgc = GetParamCount(pszCmd) ;

        // 如果只有二个参数
        // 这里为了方便以后扩展功能，同时做简单的安全检查
        if (2 == nArgc)
        {
                sscanf(pszCmd, "%s%x", stderr, &dwEip) ;

                /*
                // 这里判断地址是否合法
                if ((dwEip & 0x80000000) || (dwEip <= 0x4096))
                {
                        return 0 ;
                }
                */
                // 这里判断一下地址是否处于有效的内存分页当中
                int nPage = 0 ;
                MEMORY_BASIC_INFORMATION mbi = {0} ;
                if (0 == IsEffectiveAddress((LPVOID)dwEip, &mbi))
                {
                        printf("目标地址不存在!\r\n") ;
                        return 0 ;
                }
                PrintInstruction(dwEip,FALSE, 8) ;
        }
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : PrintCurrentRegister
*  功能描述 : 输出当前寄存器环境
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void Debug::PrintCurrentRegister()
{
        printf("\r\n") ;
        // Display the register values
        PrintContext() ;
        int Eip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        if (Eip != 0)
        {
                // Display the current instruction
                PrintInstruction(Eip, FALSE, 1) ;
        }
}

/*******************************************************************************
*
*  函 数 名 : ParseGoTo
*  功能描述 : 处理GoTo函数
*  参数列表 : nAddr             --              要跳去的地址
*  说    明 : 
*  返回结果 : 成功返回1,失败返回0
*
*******************************************************************************/
int Debug::ParseGoTo(IN unsigned int nAddr)
{
        if (NULL == m_hProcess || INVALID_HANDLE_VALUE == m_hProcess)
        {
                return 0 ;
        }

        DWORD dwProtect = 0 ;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX,
                                           PAGE_EXECUTE_READWRITE, &dwProtect) )
        {
                return 0 ;
        }

        DWORD dwNumberOfBytes ;
        char ch ;
        
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)nAddr, &ch, 
                sizeof(char), &dwNumberOfBytes) )
        {
                return 0;
        }
        
        if (FALSE == WriteProcessMemory(m_hProcess, (PVOID)nAddr, &chSoftBreak, 
                sizeof(char), &dwNumberOfBytes) )
        {
                return 0 ;
        }
        
        m_TmpBreakPoint.StdState() ;
        m_TmpBreakPoint.byOldValue = ch ;
        m_TmpBreakPoint.nAddr = nAddr ; 

        VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX,
                                           dwProtect, &dwProtect) ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : ParseGCommand
*  功能描述 : 处理G请求
*  参数列表 : pszBuffer         --       命令字符串
*  说    明 : 
*  返回结果 : 成功返回1,失败返回0
*
*******************************************************************************/
int Debug::ParseGCommand(IN char *pszCmd)
{
        if (NULL == pszCmd)
        {
                //DWORD dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
                //SetMemoryPageProtect((LPVOID)dwEip) ;
                return 0 ;
        }

        int nStrLen = strlen(pszCmd) ;
        if (1 >= nStrLen || BUFFER_MAX <= nStrLen)
        {
                return 0 ;
        }

        int nArgc = GetParamCount(pszCmd) ;

        if (2 == nArgc)
        {
                // 先取消所有的断点
                RevertAllBreakPoint() ;
                unsigned int nDestAddr = 0 ;
                sscanf(pszCmd, "%s%x", stderr, &nDestAddr) ;
                if ((nDestAddr & 0x80000000) || (nDestAddr < 0x8096))
                {
                        return 0 ;
                }
                ParseGoTo(nDestAddr) ;
                // 恢复所有的断点
                SetAllBreakPoint() ;
        }

        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : ParseBCommand
*  功能描述 : 处理B命令
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果能处理返回1，否则返回0
*
*******************************************************************************/
int Debug::ParseBCommand(IN char *pszCmd)
{
        if (NULL == pszCmd)
        {
                return 0 ;
        }

        switch(pszCmd[1])
        {
                // 清除所有的断点
        case 'c':
        case 'C':
                // 在清除所有的断点之前，要把所有的数据写回去
                RemoveAllBreakPoint() ;
                return 1 ;

                // 下int3断点
        case 'p':
        case 'P':
                if (2 == GetParamCount(pszCmd))
                {
                        unsigned int nAddr = 0 ;
                        sscanf(pszCmd, "%s%x", stderr, &nAddr) ;
                        if (1 == AddSoftBreakPoint(nAddr))
                        {
                                printf("添加断点成功\r\n") ;
                        }
                        else
                        {
                                printf("添加断点失败\r\n") ;
                        }
                }
                else
                {
                        printf("^Error\r\n") ;
                }
                return 1 ;
                // 显示所有的断点
        case 'l':
        case 'L':
                m_BreakPointList.Display() ;
                DispDrBreakPoint() ;
                m_MemBreak.Display() ;
                return 1 ;
                // 
        case 'a':
        case 'A':
                if (4 == GetParamCount(pszCmd) )
                {
                        unsigned int nAddr = 0;
                        int nLen = 0 ;
                        char szPurview[MAXBYTE] ;
                        int nPurview = 0 ;
                        // 这里要改，以后可能会存在overflow的问题
                        sscanf(pszCmd, "%s%x%x%s", stderr, &nAddr, &nLen, szPurview) ;
                        
                        // 判断地址是否有效或者断点地址存在
                        if(0 != BreakPointIsHave(nAddr))
                        {
                                printf("断点地址无效或者已经存在!\r\n") ;
                                break ;
                        }

                        switch(szPurview[0])
                        {
                        case 'E':
                        case 'e':
                                nPurview = 0 ;
                                nLen = 1 ;
                                break ;
                        case 'r':
                        case 'R':
                                nPurview = 3 ;
                                break ;
                        case 'w':
                        case 'W':
                                nPurview = 1 ;
                                break ;
                        }

                        int nPos = GetFreeDrRegister() ;
                        if (FALSE == SetDrBreakPoint(nPos, nAddr, nLen, nPurview) )
                        {
                                nPos = OnceDrBreakToSoftBreak() ;
                                if (0 != nPos)
                                {
                                        if (SetDrBreakPoint(nPos, nAddr, nLen, nPurview))
                                        {
                                                printf("添加硬件断点成功!\r\n") ;
                                        }
                                        else
                                        {
                                                printf("添加硬件断点失败!\r\n") ;
                                        }
                                }
                        }
                        else
                        {
                                printf("添加硬件断点成功!\r\n") ;
                        }
                }
                else
                {
                        printf("参数错误\r\n,带三个参数:地址 长度 权限\r\n") ;
                }
                
                return 1 ;
        case 'E':
        case 'e':
                if (2 == GetParamCount(pszCmd))
                {
                        unsigned int nAddr = 0 ;
                        sscanf(pszCmd, "%s%x", stderr, &nAddr) ;
                        if (1 == ActiveBreakPoint(nAddr))
                        {
                                printf("激活断点成功\r\n") ;
                        }
                        else
                        {
                                printf("激活断点失败\r\n") ;
                        }
                }
                else
                {
                        printf("^Error\r\n") ;
                }
                return 1 ;
        case 'd':
        case 'D':
                if (2 == GetParamCount(pszCmd))
                {
                        unsigned int nAddr = 0 ;
                        sscanf(pszCmd, "%s%x", stderr, &nAddr) ;
                        if (1 == GrayBreakPoint(nAddr, TRUE))
                        {
                                printf("断点失效成功\r\n") ;
                        }
                        else
                        {
                                printf("断点失效失败\r\n") ;
                        }
                }
                else
                {
                        printf("^Error\r\n") ;
                }
                return 1 ;
        case 'r':
        case 'R':
                if (2 == GetParamCount(pszCmd))
                {
                        unsigned int nAddr = 0 ;
                        sscanf(pszCmd, "%s%x", stderr, &nAddr) ;
                        
                        
                        if (1 == GrayBreakPoint(nAddr, TRUE))
                        {
                                // 看下断点是不是在链表中
                                if (m_BreakPointList.Find(nAddr))
                                {
                                        if (m_BreakPointList.Delete(nAddr))
                                        {
                                                printf("移除断点成功\r\n") ;
                                        }
                                        else
                                        {
                                                printf("断点已失效，但无法删除\r\n") ;
                                        }

                                }
                                else
                                {
                                      int nBreakId = GetDrNumberOfAddress((LPVOID)nAddr) ;
                                      if (RemoveDrRegister(nBreakId))
                                      {
                                              printf("移除硬件断点成功\r\n") ;
                                      }
                                      else
                                      {
                                              printf("移除硬件断点失败\r\n") ;
                                      }

                                }
                        }
                        else
                        {
                                printf("移除断点失效\r\n") ;
                        }
                }
                else
                {
                        printf("^Error\r\n") ;
                }
                return 1 ;
        case 'm':
        case 'M':
                if (4 == GetParamCount(pszCmd))
                {
                       LPVOID nAddr = 0 ;
                       SIZE_T nLen = 0 ;
                       char szPurview[16] = {0} ;
                       int nPurview = 0;
                       sscanf(pszCmd, "%s%x%x%s", stderr, &nAddr, &nLen, szPurview) ;
                       SkipSpace(szPurview, 16) ;
                       if (strlen(szPurview) > 15)
                       {
                               printf("你想干啥?\r\n") ;
                               break ;
                       }
                       switch(szPurview[0])
                       {
                       case 'r':
                       case 'R':
                               nPurview = 1 ;
                               break ;
                       case 'w':
                       case 'W':
                               nPurview = 2 ;
                               break ;
                       }

                       if (0 == nPurview)
                       {
                               printf("您设的属性不对吧! 属性为:Write, Read\r\n") ;
                               break ;
                       }

                       if(AppendMemoryBreak(nAddr, nLen, nPurview))
                       {
                               printf("添加内存断点成功!\r\n") ;

                               //DisplayAdjacencyInformation() ;
                       }
                       else
                       {
                               printf("添加内存断点失效，请检查失效原因!\r\n") ;
                       }
                }
                else
                {
                        printf("^Error\r\n") ;
                }

                return 1 ;

        case 'y':
        case 'Y':
                if (2 == GetParamCount(pszCmd))
                {
                        LPVOID nAddr = 0 ;
                        sscanf(pszCmd, "%s%x", stderr, &nAddr) ;
                        if(RemoveMemoryBreak(nAddr))
                        {
                                printf("移除内存断点成功!\r\n") ;
                        }
                        else
                        {
                                printf("移除内存断点失败!\r\n") ;
                        }
                }
                else
                {
                        printf("^Error\r\n") ;
                }
                
                return 1 ;
        case 'u':
        case 'U':
                if (2 == GetParamCount(pszCmd))
                {
                        LPVOID pAddr = NULL ;
                        sscanf(pszCmd, "%s%x", stderr, &pAddr) ;
                        int nId = GetDrNumberOfAddress(pAddr) ;
                        if(0 == nId)
                        {
                                printf("找不到此硬件断点!\r\n") ;
                        }
                        else
                        {
                                if (0 != RemoveDrRegister(nId))
                                {
                                        printf("删除硬件断点成功!\r\n") ;
                                }
                        }
                }
                return 1 ;

        default:
                printf("^Error\r\n") ;
                return 0 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : DispCopyright
*  功能描述 : 输出版权信息
*  参数列表 : 
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int Debug::DispCopyright(void)
{
        printf("\r\n") ;
        printf("                              Tiny Debugger\r\n") ;
        printf("                                            V0.1 版\r\n") ;
        printf("               版权所有 2010-2010 The Tiny Debugger Project\r\n") ;
        printf("                               作 者: 武汉科锐五班学员 黄奇\r\n\r\n") ;
        printf("            如对本程序有意见或者建议，可以联系作者: evilknight.1@163.com\r\n") ;
        printf("\r\n") ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetAllBreakPoint
*  功能描述 : 设置所有的断点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回设置断点的个数
*
*******************************************************************************/
int Debug::SetAllBreakPoint(void)
{
        
        int nCount = m_BreakPointList.GetNodeCount() ;
        Node *p = m_BreakPointList.GetListHead() ;
        int i = 0 ;

        DWORD dwNumberOfBytesWritten = 0;
        DWORD dwProtect = 0 ;
        for (; NULL != p && i < nCount; p = p->pNext, ++i)
        {
                if (TRUE == p->bIsActive)
                {
                        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, 
                                                PAGE_EXECUTE_READWRITE,&dwProtect) )
                        {
                                OutputDebugString("SetAllBreakPoint VirtualProtectEx出错!\r\n") ;
                                return 0 ;

                        }
                        WriteProcessMemory(m_hProcess, (PVOID)p->nAddr,  &chSoftBreak, 
                                                sizeof(char), &dwNumberOfBytesWritten) ;
                        VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, dwProtect,
                                        &dwProtect) ;
                }
        }
        return i ;
}

/*******************************************************************************
*
*  函 数 名 : RevertAllBreakPoint
*  功能描述 : 取消所有的断点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回设置断点的个数
*
*******************************************************************************/
int Debug::RevertAllBreakPoint(void)
{
        int nCount = m_BreakPointList.GetNodeCount() ;
        Node *p = m_BreakPointList.GetListHead() ;
        int i = 0 ;
        
        DWORD dwProtect = 0 ;
        DWORD dwNumberOfBytesWritten = 0;
        char byOldValue = 0 ;
        for (; NULL != p && i < nCount; p = p->pNext, ++i)
        {
                if(1 == p->bIsActive)
                {
                        byOldValue = p->byOldValue ;
                        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, 
                                PAGE_EXECUTE_READWRITE,&dwProtect) )
                        {
                                OutputDebugString("RevertAllBreakPoint VirtualProtectEx出错!\r\n") ;
                                return 0 ;
                        
                        }
                        WriteProcessMemory(m_hProcess, (PVOID)p->nAddr,  &byOldValue, 
                                                sizeof(char), &dwNumberOfBytesWritten) ;
                        VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, dwProtect,
                                                &dwProtect) ;
                }
        }

        if (m_TmpBreakPoint.bIsActive)
        {
                byOldValue = m_TmpBreakPoint.byOldValue ;
                if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)m_TmpBreakPoint.nAddr, 
                        BUFFER_MAX,  PAGE_EXECUTE_READWRITE,&dwProtect) )
                {
                        OutputDebugString("RevertAllBreakPoint VirtualProtectEx出错!\r\n") ;
                        return 0 ;
                        
                }
                WriteProcessMemory(m_hProcess, (PVOID)m_TmpBreakPoint.nAddr,  
                        &byOldValue,  sizeof(char), &dwNumberOfBytesWritten) ;
                VirtualProtectEx(m_hProcess, (LPVOID)m_TmpBreakPoint.nAddr, 
                                                BUFFER_MAX, dwProtect,
                                                        &dwProtect) ;
        }
        return i ;
}

/*******************************************************************************
*
*  函 数 名 : AddSoftBreakPoint
*  功能描述 : 增加一个软件断点
*  参数列表 : nAddr     --  断点的地址
*  说    明 : 
*  返回结果 : 成功返回1,失败返回0
*
*******************************************************************************/
int Debug::AddSoftBreakPoint(IN unsigned int nAddr)
{
        // 这里判断一下地址是否处于有效的内存分页当中

        if (0 != BreakPointIsHave((unsigned int)nAddr))
        {
                printf("断点地址无效或者断点地址已经存在!\r\n") ;
                return 0 ;
        }

        char ch = 0 ;
        DWORD dwNumberOfBytesWritten = 0 ;

        // 先判断有没有空的dr寄存器可以使用
        // 可以的话直接用，提高效率
        // m_bIsTmpDrBreak
        int nDrNum = GetFreeDrRegister() ;
        if (0 != nDrNum)
        {
                if(0 == SetDrBreakPoint(nDrNum, nAddr, 1, 0) )
                {
                        OutputDebugString("添加断点出错!\r\n") ;
                        return 0 ;
                }
                m_nIsTmpDrBreak |= (1 << (nDrNum-1)) ;
                return 1 ;
        }
        
        DWORD dwProtect = 0;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX, 
                PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("AddSoftBreakPoint VirtualProtectEx出错!\r\n") ;
                return 0 ;
                
        }

        // 如果读失败了的话，返回0,因为按理说，调试器是可以访问被调试进程
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)nAddr, &ch, sizeof(char), 
                &dwNumberOfBytesWritten))
        {
                return 0 ;
        }

        if (FALSE == WriteProcessMemory(m_hProcess, (PVOID)nAddr, &chSoftBreak, sizeof(char), 
                                &dwNumberOfBytesWritten))
        {
                return 0 ;
        }

        VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX, 
                                dwProtect,&dwProtect) ;
        m_BreakPointList.Insert(nAddr, ch) ;

        return 1 ;        
}

/*******************************************************************************
*
*  函 数 名 : ActiveBreakPoint
*  功能描述 : 激活一个断点
*  参数列表 : nAddr     --  断点的地址
*  说    明 : 
*  返回结果 : 成功返回1,失败返回0
*
*******************************************************************************/
int Debug::ActiveBreakPoint(IN unsigned int nAddr)
{
        // 这里判断一下地址是否处于有效的内存分页当中
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("地址不存在!\r\n") ;
                return 0 ;
        }

        if (NULL == m_hProcess)
        {
                return 0 ;
        }

        Node * p = NULL ;
        p = m_BreakPointList.Find(nAddr) ;
        if (NULL != p)
        {
                DWORD dwNumberOfBytesWritten = 0 ;
                if (FALSE == WriteProcessMemory(m_hProcess, (PVOID)p->nAddr, &chSoftBreak, sizeof(char),
                        &dwNumberOfBytesWritten))
                {
                        return 0 ;
                }
                p->StdState() ;
                return 1 ;
        }

        // 这里可以先比较一下有没有寄存器给使用先
        if (0 == (m_UseDrRegister & 0xf))
        {
                return 0 ;
        }
        
        // 再判断是不是在DRx断点中
        if (0 == m_DebugEvent.dwThreadId)
        {
                return 0 ;
        }
        
        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;
        
        if (FALSE == GetCurrentThreadContext(&context) )
        {
                return 0 ;
        }
        
        // 先判断DR0的地址是不是相同
        if (nAddr == context.Dr0)
        {
                // 如果相同的话，置一下L0位
                context.Dr7 |= 0x1 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        if (nAddr == context.Dr1)
        {
                // 如果相同的话，置一下L1位
                context.Dr7 |= 0x4 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        
        if (nAddr == context.Dr2)
        {
                // 如果相同的话，置一下L1位
                context.Dr7 |= 0x10 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        
        if (nAddr == context.Dr3)
        {
                // 如果相同的话，置一下L1位
                context.Dr7 |= 0x40 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : GrayBreakPoint
*  功能描述 : 使一个断点失效
*  参数列表 : nAddr     --    断点的地址
*             isTmp     --    一次失效还是永久失效  
*                             如果值为FALSE的话一次失效，值为TRUE的话为永久失效
*                             只是硬件断点受影响
*  说    明 : 
*  返回结果 : 成功返回1,失败返回0
*
*******************************************************************************/
int Debug::GrayBreakPoint(IN unsigned int nAddr, IN BOOL isTmp)
{
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("地址不存在!\r\n") ;
                return 0 ;
        }

        if (NULL == m_hProcess)
        {
                return 0 ;
        }

        Node * p = NULL ;
        p = m_BreakPointList.Find(nAddr) ;
        DWORD dwProtect = 0 ;
        // 如果在链表中
        if (NULL != p)
        {
                DWORD dwNumberOfBytesWritten = 0 ;
                char ch = p->byOldValue ;
                if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, 
                                PAGE_EXECUTE_READWRITE,&dwProtect) )
                {
                        OutputDebugString("GrayBreakPoint VirtualProtectEx出错!\r\n") ;
                        return 0 ;
                        
                }
                if (FALSE == WriteProcessMemory(m_hProcess, (PVOID)p->nAddr, &ch, sizeof(char),
                        &dwNumberOfBytesWritten))
                {
                        VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, 
                                        dwProtect,&dwProtect) ;
                        return 0 ;
                }
                VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, 
                                        dwProtect,&dwProtect) ;
                p->ClsState() ;
                return 1 ;
        }
        // 这里可以先比较一下有没有寄存器给使用先
        if (0 == (m_UseDrRegister & 0xf))
        {
                return 0 ;
        }

        // 再判断是不是在DRx断点中
        if (0 == m_DebugEvent.dwThreadId)
        {
                return 0 ;
        }

        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        if (FALSE == GetCurrentThreadContext(&context) )
        {
                return 0 ;
        }

        DR7 dr7 ;
        dr7.dwDr7 = context.Dr7 ;
        // 先判断DR0的地址是不是相同
        if (nAddr == context.Dr0)
        {
                // 如果相同的话，清一下L0位
                dr7.DRFlag.L0 = 0 ;
                context.Dr7 = dr7.dwDr7 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }

                if (TRUE == isTmp && 1 == m_uDRBreakResume)
                {
                        m_uDRBreakResume = 0 ;
                }
                return 1 ;
        }
        if (nAddr == context.Dr1)
        {
                // 如果相同的话，清一下L1位
                dr7.DRFlag.L1 = 0 ;
                context.Dr7 = dr7.dwDr7 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }

                if (TRUE == isTmp && 2 == m_uDRBreakResume)
                {
                        m_uDRBreakResume = 0 ;
                }
                return 1 ;
        }

        if (nAddr == context.Dr2)
        {
                // 如果相同的话，清一下L2位
                dr7.DRFlag.L2 = 0 ;
                context.Dr7 = dr7.dwDr7 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }

                if (TRUE == isTmp && 3 == m_uDRBreakResume)
                {
                        m_uDRBreakResume = 0 ;
                }
                return 1 ;
        }

        if (nAddr == context.Dr3)
        {
                // 如果相同的话，清一下L3位
                dr7.DRFlag.L3 = 0 ;
                context.Dr7 = dr7.dwDr7 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }

                if (TRUE == isTmp && 4 == m_uDRBreakResume)
                {
                        m_uDRBreakResume = 0 ;
                }
                return 1 ;
        }

        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : GetFreeDrRegister
*  功能描述 : 取得空闲的DR寄存器
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 成功返回寄存器序号(从1开始),失败返回0
*
*******************************************************************************/
int Debug::GetFreeDrRegister(void)
{
        // 判断有没有空的寄存器了
        if (0xf == (m_UseDrRegister & 0xf))
        {
                return 0 ;
        }

        //有的话，再一个一个的判断，看下哪个没有使用
        if (0 == (m_UseDrRegister & 0x1))
        {
                return 1 ;
        }

        if (0 == (m_UseDrRegister & 0x2))
        {
                return 2 ;
        }
        if (0 == (m_UseDrRegister & 0x4))
        {
                return 3 ;
        }
        if (0 == (m_UseDrRegister & 0x8))
        {
                return 4 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : SetFreeDrRegister
*  功能描述 : 将DRx寄存器设为空闲
*  参数列表 : nNum      --      DR寄存器序号
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::SetFreeDrRegister(IN int nDrID)
{
        if (nDrID < 1 || nDrID > 4)
        {
                OutputDebugString("SetFreeDrRegister 参数出错!\r\n") ;
                return 0 ;
        }
        m_UseDrRegister &= ~(1 << (nDrID-1)) ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetDrBreakPoint
*  功能描述 : 设置DRx断点
*  参数列表 : nNum      --      DR寄存器序号
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::SetDrBreakPoint(IN int nDrID, IN unsigned int nAddr, IN int nLen, IN int nPurview)
{
        if (nDrID < 1 || nDrID > 4)
        {
                return 0 ;
        }

        // 这里判断一下地址是否处于有效的内存分页当中
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("地址不存在!\r\n") ;
                return 0 ;
        }

        if (1 != nLen && 2 != nLen && 4 != nLen)
        {
                OutputDebugString("SetDrBreakPoint 长度不对") ;
                return 0 ;
        }

        if ((0 != nPurview) && (1 != nPurview) && (3 != nPurview))
        {
                OutputDebugString("SetDrBreakPoint 权限不对") ;
                return 0 ;
        }
   
        if (NULL == m_DebugEvent.dwThreadId)
        {
                return 0 ;
        }

        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        if (FALSE == GetCurrentThreadContext(&context))
        {
                return 0 ;
        }

        DR7 dr7 ;
        dr7.dwDr7 = context.Dr7 ;

        // 将断点地址放进相应的dr寄存器里
        switch(nDrID)
        {
        case 1:
                context.Dr0 = nAddr ;
                dr7.DRFlag.L0 = 1 ;
                dr7.DRFlag.len0 = nLen - 1 ;
                dr7.DRFlag.rw0 = nPurview ;
                break ;
        case 2:
                context.Dr1 = nAddr ;
                dr7.DRFlag.L1 = 1 ;
                dr7.DRFlag.len1 = nLen - 1 ;
                dr7.DRFlag.rw1 = nPurview ;
                break ;
        case 3:
                context.Dr2 = nAddr ;
                dr7.DRFlag.L2 = 1 ;
                dr7.DRFlag.len2 = nLen - 1 ;
                dr7.DRFlag.rw2 = nPurview ;
                break ;
        case 4:
                context.Dr3 = nAddr ;
                dr7.DRFlag.L3 = 1 ;
                dr7.DRFlag.len3 = nLen - 1 ;
                dr7.DRFlag.rw3 = nPurview ;
                break ;
        default:
                return 0 ;
        }

        context.Dr7 = dr7.dwDr7 ;

        
        // 将寄存器设为使用
        m_UseDrRegister |= (1 << (nDrID - 1)) ;
        if (FALSE == SetCurrentThreadContext(&context))
        {
                OutputDebugString("SetThreadContext failed!\r\n") ;
                return 0 ;
        }

        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : DispDrBreakPoint
*  功能描述 : 显示硬件断点信息
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void Debug::DispDrBreakPoint(void)
{
        // 先判断有没有dr寄存器给使用
        if (0 == (m_UseDrRegister & 0xf))
        {
                printf("无硬件断点信息\r\n") ;
                return ;
        }
        else
        {
                printf("硬件断点信息\r\n") ;
        }


        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;
        if (FALSE == GetCurrentThreadContext(&context) )
        {
                return ;
        }
        DR7 dr7 ;
        dr7.dwDr7 = context.Dr7 ;
        printf("Drx BreakPoint:\r\n") ;

        //如果有的话，再依次判断dr0-4
        // 先判断dr0有没有给使用
        if (m_UseDrRegister & 0x1)
        {
                printf("DR0: %p\tLen: %d\t", context.Dr0, dr7.DRFlag.len0 + 1) ;
                switch(dr7.DRFlag.rw0)
                {
                case 0:
                        printf("Purview: %12s ", "Execute") ;
                        break ;
                case 1:
                        printf("Purview: %12s ", "Write") ;
                        break ;
                case 2:
                        printf("Purview: %12s ", "I/O") ;
                        break ;
                case 3:
                        printf("Purview: %12s ", "ReadOrWrite") ;
                        break ;
                }
                printf(" State:%s\r\n",dr7.DRFlag.L0?"生效":"失效") ;
        }

        if (m_UseDrRegister & 0x2)
        {
               printf("DR1: %p\tLen: %d\t", context.Dr1, dr7.DRFlag.len1 + 1) ;
                switch(dr7.DRFlag.rw1)
                {
                case 0:
                        printf("Purview: %12s ", "Execute") ;
                        break ;
                case 1:
                        printf("Purview: %12s ", "Write") ;
                        break ;
                case 2:
                        printf("Purview: %12s ", "I/O") ;
                        break ;
                case 3:
                        printf("Purview: %12s ", "ReadOrWrite") ;
                        break ;
                }
                printf(" State:%s\r\n",dr7.DRFlag.L1?"生效":"失效") ;
        }

        if (m_UseDrRegister & 0x4)
        {
               printf("DR2: %p\tLen: %d\t", context.Dr2, dr7.DRFlag.len2 + 1) ;
                switch(dr7.DRFlag.rw2)
                {
                case 0:
                        printf("Purview: %12s ", "Execute") ;
                        break ;
                case 1:
                        printf("Purview: %12s ", "Write") ;
                        break ;
                case 2:
                        printf("Purview: %12s ", "I/O") ;
                        break ;
                case 3:
                        printf("Purview: %12s ", "ReadOrWrite") ;
                        break ;
                }
                printf(" State:%s\r\n",dr7.DRFlag.L2?"生效":"失效") ;
        }

        if (m_UseDrRegister & 0x8)
        {
               printf("DR3: %p\tLen: %d\t", context.Dr3, dr7.DRFlag.len3 + 1) ;
                switch(dr7.DRFlag.rw3)
                {
                case 0:
                        printf("Purview: %12s ", "Execute") ;
                        break ;
                case 1:
                        printf("Purview: %12s ", "Write") ;
                        break ;
                case 2:
                        printf("Purview: %12s ", "I/O") ;
                        break ;
                case 3:
                        printf("Purview: %12s ", "ReadOrWrite") ;
                        break ;
                }
                printf(" State:%s\r\n",dr7.DRFlag.L3?"生效":"失效") ;
        }
        return  ;
}

/*******************************************************************************
*
*  函 数 名 : GetCurrentThreadContext
*  功能描述 : 取得当前线程上下文
*  参数列表 : pContext    --  指向CONTEXT结构
*  说    明 : 
*  返回结果 : 如果返回FALSE的话出错,成功返回TRUE
*
*******************************************************************************/
BOOL Debug::GetCurrentThreadContext(OUT CONTEXT *pContext)
{
        if (NULL == pContext)
        {
                OutputDebugString("Context数组指针为空!\r\n") ;
                return FALSE ; 
        }

        if (0 == m_DebugEvent.dwThreadId)
        {
                OutputDebugString("线程id无效\r\n") ;
                return FALSE ;
        }

        m_hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_DebugEvent.dwThreadId) ;

        if ( NULL == m_hThread)
        {
                return FALSE ;
        }

        if (FALSE == GetThreadContext(m_hThread, pContext))
        {
                CloseHandle(m_hThread) ;
                m_hThread = NULL ;
                return FALSE ;
        }

        CloseHandle(m_hThread) ;
        m_hThread = NULL ;
        return TRUE ;
}

/*******************************************************************************
*
*  函 数 名 : SetCurrentThreadContext
*  功能描述 : 设置当前线程上下文
*  参数列表 : pContext    --  指向CONTEXT结构
*  说    明 : 
*  返回结果 : 如果返回FALSE的话出错,成功返回TRUE
*
*******************************************************************************/
BOOL Debug::SetCurrentThreadContext(IN CONTEXT *pContext)
{
        if (NULL == pContext)
        {
                return FALSE ; 
        }
        
        if (0 == m_DebugEvent.dwThreadId)
        {
                return FALSE ;
        }
        
        m_hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_DebugEvent.dwThreadId) ;
        
        if ( NULL == m_hThread)
        {
                return FALSE ;
        }
        
        if (FALSE == SetThreadContext(m_hThread, pContext))
        {
                CloseHandle(m_hThread) ;
                m_hThread = NULL ;
                return FALSE ;
        }
        
        CloseHandle(m_hThread) ;
        m_hThread = NULL ;
        return TRUE ;
}

/*******************************************************************************
*
*  函 数 名 : ParseSingleSetp
*  功能描述 : 处理单步
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果可以处理返回DBG_CONTINUE、不能处理返回DBG_EXCEPTION_NOT_HANDLED
*
*******************************************************************************/
int Debug::ParseSingleSetp(void)
{
        BOOL bIsParse = FALSE ; // 是否可以处理
        BOOL bIsDrBreak = FALSE ;

        CONTEXT context ;
        unsigned int nAddr = 0 ;
        DWORD dwProtect = 0 ;

        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        GetCurrentThreadContext(&context) ;

        // 判断一下是不是软断点需要恢复
        if (0 != m_uSoftBreakResume)
        {
                ActiveBreakPoint(m_uSoftBreakResume) ;
                m_uSoftBreakResume = 0 ;
                bIsParse = TRUE ;
        }

        // 判断一下是不是硬件中断需要恢复
        if (0 != m_uDRBreakResume)
        {

                if (m_uDRBreakResume >= 1 && m_uDRBreakResume <= 3)
                {
                        switch(m_uDRBreakResume)
                        {
                        case 1:
                                nAddr = context.Dr0 ;
                                break ;
                        case 2:
                                nAddr = context.Dr1 ;
                                break ;
                        case 3:
                                nAddr = context.Dr2 ;
                                break ;
                        case 4:
                                nAddr = context.Dr3 ;
                                break ;
                        }
                        m_uDRBreakResume = 0 ;
                        ActiveBreakPoint(nAddr) ;
                        bIsParse = TRUE ;
                }
        }

        // 再判断一下是不是因为硬件断点引起的异常
        if (0 != (context.Dr6 & 0xf))
        {
                int nPos = context.Dr6 &0xf ;
                
                switch(nPos)
                {
                case 0x1:
                        nAddr = context.Dr0 ;
                        m_uDRBreakResume = 1 ;
                        SetTFRegister() ;
                        break ;
                case 0x2:
                        nAddr = context.Dr1 ;
                        m_uDRBreakResume = 2 ;
                        SetTFRegister() ;
                        break ;
                case 0x4:
                        nAddr = context.Dr2 ;
                        m_uDRBreakResume = 3 ;
                        SetTFRegister() ;
                        break ;
                case 0x8:
                        nAddr = context.Dr3 ;
                        m_uDRBreakResume = 4 ;
                        SetTFRegister() ;
                        break ;
                }
                bIsDrBreak = TRUE ;
                GrayBreakPoint(nAddr, FALSE) ;
                


                // 输出调戏信息以及处理用户请求
                if(m_uDRBreakResume)
                {
                        PrintContext() ;
                        PrintInstruction(context.Eip, FALSE, 1) ;
                        printf("硬件中断!\r\n") ;
                        ParseRequest() ;
                }
                /*
                else
                {
                        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)context.Eip, BUFFER_MAX, 
                                                PAGE_EXECUTE_READWRITE,&dwProtect) )
                        {
                                OutputDebugString("ParseSingleSetp VirtualProtectEx出错!\r\n") ;
                                return 0 ;
                                
                        }
                        
                        t_disasm da = {0};
                        char szBuf[BUFFER_MAX] = {0};
                        DWORD dwNoting = 0 ;
                        ReadProcessMemory(m_hProcess, (LPVOID)context.Eip,szBuf, BUFFER_MAX, &dwNoting);
                        
                        int nLen = Disasm(szBuf, 20, context.Eip, &da, DISASM_CODE);
                        SkipSpace(da.dump, DUMPSIZE) ;
                        AberrTreeNode * p = m_InstructionRecord.Insert(context.Eip, 
                                                                        szBuf,
                                                                        nLen) ;
                        int nJmpAddr = 0 ;
                        
                        nJmpAddr = m_DllExportFun.AddressToFunName(da, szBuf, szBuf) ;
                        if (0 != nJmpAddr)
                        {
                                p->jmpconst = (LPVOID)nJmpAddr ;
                        }
                        InstructionRecord() ;
                        VirtualProtectEx(m_hProcess, (LPVOID)context.Eip, BUFFER_MAX,
                                                dwProtect, &dwProtect) ;
                }
                */
                
                bIsParse = TRUE ;
        }

        // 判断一下是不是内存属性要恢复
        if (m_nPageAddress != 0)
        {
                VirtualProtectEx(m_hProcess, m_nPageAddress, MEMPAGE_LEN, m_dwProtect, &dwProtect) ;
                m_dwProtect = 0 ;
                m_nPageAddress = 0 ;
                bIsParse = TRUE ;

                if (m_bIsRecord)
                {
                        return DBG_CONTINUE ;
                }

        }

        if (TRUE == m_nIsTCommand && FALSE == bIsDrBreak)
        {
                m_nIsTCommand = FALSE ;
                PrintContext() ;
                PrintInstruction(GetCurrentEip(m_DebugEvent.dwThreadId),FALSE,1) ;
                printf("单步中断!\r\n") ;
                ParseRequest() ;
                bIsParse = TRUE ;
        }
        if (TRUE == bIsParse && FALSE == m_bIsRecord)
        {
                return DBG_CONTINUE ;
        }
       
        return DBG_EXCEPTION_NOT_HANDLED ;
}

/*******************************************************************************
*
*  函 数 名 : AppendMemoryBreak
*  功能描述 : 添加内存断点
*  参数列表 : nAddr     ---   内存断点地址
              nLen      ---   长度
              dwPurview ---   在执行什么操作时断下(1:读 2:写) 
*  说    明 : 
*  返回结果 : 如果可以处理返回DBG_CONTINUE、不能处理返回DBG_EXCEPTION_NOT_HANDLED
*
*******************************************************************************/
int Debug::AppendMemoryBreak(IN LPVOID nAddr, IN SIZE_T nLen, IN DWORD dwPurview)
{
        if (NULL == m_hProcess)
        {
                return 0 ;
        }
        // 检查参数
        if (NULL == nAddr)
        {
                return 0 ;
        }

        // 后来扩充的代码，可能有部分重复
        if (0 != BreakPointIsHave((unsigned int)nAddr))
        {
                printf("断点地址无效或者断点地址已经存在!\r\n") ;
                return 0 ;
        }
        /*
        #define PAGE_NOACCESS          0x01     
        #define PAGE_READONLY          0x02     
        #define PAGE_READWRITE         0x04 
        */
        if (dwPurview < 1 || dwPurview > 2)
        {
                return 0 ;
        }

        // 检查nAddr是不是处于有效分页内
        BOOL bIsEffective = FALSE ;
        int nPageID = 0 ;

        MEMORY_BASIC_INFORMATION mbi = {0} ;

        if (0 == IsEffectiveAddress(nAddr, &mbi))
        {
                printf("内存地址无效!\r\n") ;
                return 0 ;
        }

        nPageID = InsertEffectiveMemoryPage(nAddr, &mbi) ;

        if (0 == nPageID)
        {
                OutputDebugString("找不到那个内存分页!\r\n") ;
                return 0 ;
        }
      
        PMemNode p = m_MemList.Find(&mbi) ;
                
        int nBreakID = 0 ;
        
        // 判断这个断点是否已经存在
        if (m_MemBreak.HaveIn(nAddr, nBreakID))
        {
                OutputDebugString("这个内存断点已经存在\r\n") ;
                printf("这个内存断点地址已经存在了\r\n") ;
                return 0 ;
        }

        // 判断这个内存分页是否具有这个属性
        if (! IsHavememoryProtect(p->Protect, dwPurview))
        {
#ifdef _DEBUG
                printf("这个内存页没有这个属性") ;
#endif
                return 0 ;
        }
        
        // 如果所处内存页有效的话，将断点信息加入到内存断点表里
        nBreakID = m_MemBreak.Insert(nAddr, nLen, dwPurview) ;
        if (0 == nBreakID)
        {
                return 0 ;
        }


        // 这里处理内存对应断点、断点对应内存的关系
        DWORD nEndAddress = (DWORD)nAddr ;
        //MEMORY_BASIC_INFORMATION mbi = {0} ;
        BOOL isContinue = TRUE ;
        do 
        {
                m_BreakMapPage[nBreakID].Push(nPageID, NULL) ;
                m_PageMapBreak[nPageID].Push(nBreakID, NULL) ;
                DWORD dwNothing = 0 ;
                isContinue = FALSE ;
                unsigned int newProtect = RemoveMemoryProtect(p->newProtect, dwPurview) ;
                
                if (p->newProtect != newProtect)
                {
                        if( FALSE == VirtualProtectEx(m_hProcess, (LPVOID)nEndAddress, 
                                       nLen, newProtect, &dwNothing))
                        {
                                GetLastError() ;

                                m_MemBreak.Remove(nAddr) ;
                                printf("修改内存分页属性失败!\r\n") ;
                                return 0 ;
                        }
                }
                p->newProtect = newProtect ;
                
                nEndAddress = (DWORD)p->BaseAddress + p->RegionSize ;
                // 判断有没有跨页
                // 如果下一个内存页已经不存在的话，结束，同时修改一下长度
                if (((DWORD)nAddr + nLen) > nEndAddress 
                        && m_MemList.HaveIn((LPVOID)(nEndAddress), nPageID)
                        && IsHavememoryProtect(p->newProtect, dwPurview))
                {
                        nPageID = InsertEffectiveMemoryPage((LPVOID)(nEndAddress + 1), &mbi) ;
                        
                        if (0 == nPageID)
                        {
                                OutputDebugString("找不到那个内存分页!\r\n") ;
                                isContinue = FALSE ;
                        }
                        else
                        {
                                p = m_MemList.Find(&mbi) ;
                                isContinue = TRUE ;
                        }
                }
        } while ( isContinue );

        if (nLen > (nEndAddress - (DWORD)nAddr))
        {
                nLen = nEndAddress - (DWORD)nAddr ;
        }
        
        // 修改内存断点的长度
        PMemBreakNode pBreak = m_MemBreak.FindById(nBreakID) ;
        if (NULL != pBreak)
        {
                pBreak->BreakLen = nLen;
        }

        return 1 ;
}


/*******************************************************************************
*
*  函 数 名 : RemoveMemoryProtect
*  功能描述 : 移除内存属性
*  参数列表 : nProtect    --  内存原属性
              Attrib      --  要移除的属性
              1: 读属性
              2: 写属性
              4: 执行属性
*  说    明 : 这个函数还不完整
*  返回结果 : 返回新的属性,如果返回0的话说明出错
*
*******************************************************************************/
int Debug::RemoveMemoryProtect(IN int nProtect, IN int Attrib)
{
        if (0 == nProtect)
        {
                return 0 ;
        }

        if (0 == Attrib)
        {
                return 0 ;
        }

        BOOL bIsRead = FALSE ;
        BOOL bIsWrite = FALSE ;
        BOOL bIsExecte = FALSE ;
        BOOL bIsGuard = FALSE ;
        BOOL bIsCopy = FALSE ;

        // 设置标志位
        if( nProtect & 0x02)
        {
                bIsRead = TRUE ;
        }
                
        if( nProtect & 0x04)
        {
                bIsRead = TRUE ;
                bIsWrite = TRUE ;
        }

        if( nProtect & 0x08)
        {
                bIsWrite = TRUE ;
                bIsCopy = TRUE ;
                bIsRead = TRUE ;
        }

        if( nProtect & 0x10)
        {
                bIsExecte = TRUE ;
        }

        if( nProtect & 0x20)
        {
                bIsExecte = TRUE ;
                bIsRead = TRUE ;
        }
        if( nProtect & 0x40)
        {
                bIsRead = TRUE ;
                bIsWrite = TRUE ;
                bIsExecte = TRUE ;
        }
        if( nProtect & 0x80)
        {
                bIsExecte = TRUE ;
                bIsRead = TRUE ;
                bIsCopy = TRUE ;
        }
        if( nProtect & 0x100)
        {
                bIsGuard = TRUE ;
        }
        /* 除除属性
        0x1: 读属性
        0x2: 写属性
        0x4: 执行属性
        */
        if (Attrib & 0x1)
        {
                bIsRead = FALSE ;
                bIsExecte = FALSE ;
        }

        if (Attrib & 0x2)
        {
                bIsWrite = FALSE ;
        }

        if (Attrib & 0x4)
        {
                bIsExecte = FALSE ;
        }

        // 去除属性
        /*
        if( nProtect & 0x02)
        {
                bIsRead = FALSE ;
        }
        
        if( nProtect & 0x04)
        {
                bIsRead = FALSE ;
                bIsWrite = FALSE ;
        }
        
        if( nProtect & 0x08)
        {
                bIsWrite = FALSE ;
                bIsCopy = FALSE ;
        }
        
        if( nProtect & 0x10)
        {
                bIsExecte = FALSE ;
        }
        
        if( nProtect & 0x20)
        {
                bIsExecte = FALSE ;
                bIsRead = FALSE ;
        }
        if( nProtect & 0x40)
        {
                bIsRead = FALSE ;
                bIsWrite = FALSE ;
                bIsExecte = FALSE ;
        }
        if( nProtect & 0x80)
        {
                bIsExecte = FALSE ;
                bIsRead = FALSE ;
                bIsCopy = FALSE ;
        }
        if( nProtect & 0x100)
        {
                bIsGuard = FALSE ;
        }
        */
        int newProtect = 1 ;

        // 将新的结果返回
        // 如果具有执行属性和拷备属性的话
        if (bIsExecte && bIsCopy && bIsWrite && !bIsRead)
        {
                newProtect = PAGE_EXECUTE_WRITECOPY ;
        }

        // 如果具有读属性
        else if (!bIsExecte && !bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_READONLY ;
        }

        // 如果具有读写属性
        else if (!bIsExecte && !bIsCopy && bIsWrite && bIsRead)
        {
                newProtect = PAGE_READWRITE ;
        }

        // 如果具有写和拷备
        else if (!bIsExecte && bIsCopy && bIsWrite && bIsRead)
        {
                newProtect = PAGE_WRITECOPY ;
        }

        // 如果具有执行属性和拷备属性的话
        else if (bIsExecte && !bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE_READ ;
        }

        // 如果具有执行属性和拷备属性的话
        else if (bIsExecte && !bIsCopy && bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE_READWRITE ;
        }

        // 如果具有执行属性和拷备属性的话
        else if (bIsExecte && bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE_WRITECOPY ;
        }

        // 如果具有执行属性和拷备属性的话
        else if (bIsExecte && !bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE ;
        }

        // 如果只有执行属性的话
        else if (bIsExecte && !bIsCopy && !bIsWrite && !bIsRead)
        {
                newProtect = PAGE_EXECUTE ;
        }

        else if (bIsRead && bIsCopy)
        {
                newProtect = PAGE_READONLY ;
        }

        if (bIsGuard)
        {
                newProtect |= 0x100 ;
        }

        return newProtect ;
}

/*******************************************************************************
*
*  函 数 名 : IsHavememoryProtect
*  功能描述 : 判断内存属性中是否有指定的属性
*  参数列表 : nProtect    --  内存原属性
              Attrib      --  要检查的属性
              1: 读属性
              2: 写属性
              4: 执行属性
*  说    明 : 这个函数还不完整
*  返回结果 : 如果成功返回1,否则返回0
*
*******************************************************************************/
int Debug::IsHavememoryProtect(IN int nProtect, IN int Attrib)
{
        if (0 == nProtect)
        {
                return 0 ;
        }

        BOOL bIsRead = FALSE ;
        BOOL bIsWrite = FALSE ;
        BOOL bIsExecte = FALSE ;
        BOOL bIsGuard = FALSE ;
        BOOL bIsCopy = FALSE ;

        // 设置标志位
        if( nProtect & 0x02)
        {
                bIsRead = TRUE ;
        }
                
        if( nProtect & 0x04)
        {
                bIsRead = TRUE ;
                bIsWrite = TRUE ;
        }

        if( nProtect & 0x08)
        {
                bIsWrite = TRUE ;
                bIsCopy = TRUE ;
                bIsRead = TRUE ;
        }

        if( nProtect & 0x10)
        {
                bIsExecte = TRUE ;
        }

        if( nProtect & 0x20)
        {
                bIsExecte = TRUE ;
                bIsRead = TRUE ;
        }
        if( nProtect & 0x40)
        {
                bIsRead = TRUE ;
                bIsWrite = TRUE ;
                bIsExecte = TRUE ;
        }
        if( nProtect & 0x80)
        {
                bIsExecte = TRUE ;
                bIsRead = TRUE ;
                bIsCopy = TRUE ;
        }
        if( nProtect & 0x100)
        {
                bIsGuard = TRUE ;
        }
        /* 除除属性
        0x1: 读属性
        0x2: 写属性
        0x4: 执行属性
        */
        if (Attrib & 0x1)
        {
                if(FALSE == bIsRead)
                {
                        return 0 ;
                }
        }

        if (Attrib & 0x2)
        {
                if(FALSE == bIsWrite)
                {
                        return 0 ;
                }
        }

        if (Attrib & 0x4)
        {
                if(FALSE == bIsExecte)
                {
                        return 0 ;
                }
        }

        return 1 ;
}


/*******************************************************************************
*
*  函 数 名 : RemoveMemoryBreak
*  功能描述 : 移除内存断点
*  参数列表 : pAddr  --  断点地址
*  说    明 : 
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::RemoveMemoryBreak(IN LPVOID pAddr)
{
        if (NULL == pAddr)
        {
                OutputDebugString("目标地址为空!\r\n") ;
                return  0 ;
        }

        int nBreakID = 0 ;
        DWORD dwProtect = 0 ;
        if( 0 == m_MemBreak.HaveIn(pAddr, nBreakID))
        {
                OutputDebugString("找不到目标断点信息!\r\n") ;
                return 0 ;
        }
        // 取得内存页ID
        int nPageID = 0 ;
        if (0 == m_MemList.HaveIn(pAddr, nPageID))
        {
                return 0 ;
        }

        int nPageCount = m_BreakMapPage[nBreakID].GetSize() ;

        for (int i = 0; i < nPageCount; ++i)
        {
                // 将当前断点从那个内存页中移除
                m_PageMapBreak[ m_BreakMapPage[nBreakID][i]->data ].Delete(nBreakID) ;
                // 再重新计算内存页的属性
                RepairMemoryProtect(m_BreakMapPage[nBreakID][i]->data) ;
                // 重新设置内存页的属性
                PMemNode p = m_MemList.FindById(m_BreakMapPage[nBreakID][i]->data) ;
                if (NULL != p)
                {
                        // 原来是,但是引发移除断点后不能正确设置属性
                        // VirtualProtectEx(m_hProcess, p->BaseAddress, BUFFER_MAX, p->newProtect, &dwProtect) ;
                        VirtualProtectEx(m_hProcess, p->BaseAddress, BUFFER_MAX, p->newProtect, &dwProtect) ;

                        // 判断这个地址有没有断点了，如果没有了，下次不用再恢复内存属性了
                        if (m_nPageAddress)
                        {
                                int nTmpPageID = 0 ;
                                m_MemList.HaveIn(m_nPageAddress, nTmpPageID) ;
                                if (m_PageMapBreak[nTmpPageID].GetSize() == 0)
                                {
                                        m_nPageAddress = 0 ;
                                }
                        }
                        
                }
        }
        m_BreakMapPage[nBreakID].Clear() ;
        m_MemBreak.Remove(pAddr) ;
        
        //DisplayAdjacencyInformation() ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : RepairMemoryProtect
*  功能描述 : 还原内存断点所在内存页属性
*  参数列表 : nPageID  --  内存分页ID
*  说    明 : 
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::RepairMemoryProtect(IN int nPageID)
{
        if (0 == nPageID)
        {
                OutputDebugString("内存页号不能为0\r\n") ; 
                return 0 ;
        }

        PMemNode p = m_MemList.FindById(nPageID) ;
        if (NULL == p)
        {
                OutputDebugString("找不到此页号的有效内存号!\r\n") ;
                return 0 ;
        }

        int nBreakCount = m_PageMapBreak[nPageID].GetSize() ;
        int nProtect = p->Protect ;
        PMemBreakNode pBreakNode = NULL ;
        for (int i = 0; i < nBreakCount; ++i)
        {
                pBreakNode = m_MemBreak.FindById(m_PageMapBreak[nPageID][i]->data) ;
                if (NULL != pBreakNode)
                {
                        nProtect = RemoveMemoryProtect(nProtect, pBreakNode->Protect) ;
                }
        }
        p->newProtect = nProtect ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : HaveInBreakArea
*  功能描述 : 判断地址是否在断点地址之内
*  参数列表 : nAddrm     --  断点地址
              pBreakNode --  断点信息结构指针
*  说    明 : 
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::HaveInBreakArea(IN int nAddrm, IN PMemBreakNode pBreakNode)
{
        if (NULL == pBreakNode)
        {
                OutputDebugString("指针为空!\r\n") ;
                return 0 ;
        }
        int nBegin = (int)pBreakNode->lpBreakBase ;
        int nEnd = nBegin + pBreakNode->BreakLen ;

        if (nAddrm >= nBegin && nAddrm < nEnd)
        {
                return 1 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : DisplayAdjacencyInformation
*  功能描述 : 输出断点与内存页相互的关系
*  参数列表 : 
*  说    明 : 调试用的
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::DisplayAdjacencyInformation(void)
{
        int i = 0 ;
        int nCount = m_PageMapBreak.GetCapacity() ;
        printf("输出内存对应断点信息:\r\n") ;

        for (i = 0; i < nCount; ++i)
        {
                printf("%2d:", i) ;
                for(int j = 0; j < m_PageMapBreak[i].GetSize() ; ++j)
                {
                        printf("%3d", m_PageMapBreak[i][j]->data) ;
                }
                printf("\r\n");
        }

        printf("输出断点对应内存信息:\r\n") ;
        nCount = m_BreakMapPage.GetCapacity() ;

        for (i = 0; i < nCount; ++i)
        {
                printf("%2d:", i) ;
                for(int j = 0; j < m_BreakMapPage[i].GetSize() ; ++j)
                {
                        printf("%3d", m_BreakMapPage[i][j]->data) ;
                }
                printf("\r\n");
        }

        printf("输出断点信息!\r\n") ;
        m_MemBreak.Display() ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : GetDrNumberOfAddress
*  功能描述 : 通过断点地址得到使用DR寄存器的序号
*  参数列表 : IN pAddr
*  说    明 : 
*  返回结果 : 如果成功返回序号(从1开始), 失败返回0
*
*******************************************************************************/
int Debug::GetDrNumberOfAddress(IN LPVOID pAddr)
{
        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        if (0 == GetCurrentThreadContext(&context))
        {
                return 0 ;
        }
        
        if (pAddr == (LPVOID)context.Dr0)
        {
                return 1 ;
        }

        if (pAddr == (LPVOID)context.Dr1)
        {
                return 2 ;
        }

        if (pAddr == (LPVOID)context.Dr2)
        {
                return 3 ;
        }

        if (pAddr == (LPVOID)context.Dr3)
        {
                return 4 ;
        }

        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : RemoveDrRegister
*  功能描述 : 移除硬件断点
*  参数列表 : IN nDrID:         Dr寄存器序号
*  说    明 : 
*  返回结果 : 如果成功返回序号(从1开始), 失败返回0
*
*******************************************************************************/
int Debug::RemoveDrRegister(IN int nDrID)
{
        if (nDrID < 1 || nDrID > 4)
        {
                OutputDebugString("Dr寄存器的序号不对!\r\n") ;
                return 0 ;
        }
        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;
        DR7 dr7 ;

        if (FALSE == GetCurrentThreadContext(&context))
        {
                return 0 ;
        }

        dr7.dwDr7 = context.Dr7 ;

        // 无聊把drx也全清了吧
        switch(nDrID)
        {
        case 1:
                context.Dr0 = 0 ;
                dr7.DRFlag.L0 = 0 ;
                dr7.DRFlag.len0 = 0 ;
                dr7.DRFlag.rw0 = 0 ;
                break ;
        case 2:
                context.Dr1 = 0 ;
                dr7.DRFlag.L1 = 0 ;
                dr7.DRFlag.len1 = 0 ;
                dr7.DRFlag.rw1 = 0 ;
                break ;
        case 3:
                context.Dr2 = 0 ;
                dr7.DRFlag.L2 = 0 ;
                dr7.DRFlag.len2 = 0 ;
                dr7.DRFlag.rw2 = 0 ;
                break ;
        case 4:
                context.Dr3 = 0 ;
                dr7.DRFlag.L3 = 0 ;
                dr7.DRFlag.len3 = 0 ;
                dr7.DRFlag.rw3 = 0 ;
                break ;
        }
        
        // 将寄存器设为未使用
        m_UseDrRegister &= ~(1 << (nDrID - 1)) ;

        if (FALSE == SetCurrentThreadContext(&context))
        {
                OutputDebugString("SetThreadContext failed!\r\n") ;
                return 0 ;
        }
        
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : ParseDCommand
*  功能描述 : 处理D命令
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::ParseDCommand(IN char *pszCmd)
{
        if(NULL == pszCmd)
        {
                OutputDebugString("参数指针为空!\r\n") ;
                return 0 ;
        }

        int nParamCount = GetParamCount(pszCmd) ;
        LPVOID dwEip = 0 ;
        int nLen = 0x80 ;

        if (1 == nParamCount)
        {
                dwEip = (LPVOID)GetCurrentEip(m_DebugEvent.dwThreadId) ;
        }

        else if (2 == nParamCount)
        {
                sscanf(pszCmd, "%s%x", stderr, &dwEip) ;
        }

        else if (3 == nParamCount)
        {
                sscanf(pszCmd, "%s%x%x", stderr, &dwEip, &nLen) ;
        }

        DisplayDestProcessMemory(dwEip, nLen) ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : DisplayDestProcessMemory
*  功能描述 : 显示调试进程目标内存数据
*  参数列表 : IN pAddr:         目标内存地址
              IN nLen:          读取长度
*  说    明 : 
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::DisplayDestProcessMemory(IN LPVOID pAddr, IN int nLen)
{
        int nPageID = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;

        // 判断地址是否存在
        if (0 == IsEffectiveAddress(pAddr, &mbi))
        {
                OutputDebugString("内存地址无效!\r\n") ;
                return 0 ;
        }

        char *pBuf = new char[nLen+sizeof(char)] ;

        if (NULL == pBuf)
        {
                OutputDebugString("申请内存失效!\r\n") ;
                return 0 ;
        }

        if (nLen <= 0)
        {
                OutputDebugString("长度出错!\r\n") ;
                return 0 ;
        }

        if (nLen > (int)pAddr)
        {
                nLen -= (int)pAddr ;
                ++nLen ;
        }

        DWORD dwProtect = 0 ;

        // 防止下了内存断点，目标内存页没有读的属性，先将读的属性加上去
        if(FALSE == VirtualProtectEx(m_hProcess, pAddr, BUFFER_MAX, 
                                        PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("DisplayDestProcessMemory VirtualProtectEx出错!\r\n") ;
                return 0 ;
                
        }

        DWORD dwNothing = 0 ;

        if(FALSE == ReadProcessMemory(m_hProcess, pAddr, pBuf, 
                sizeof(char)*nLen,&dwNothing))
        {
                OutputDebugString("读目标进程出错!\r\n") ;
                return 0 ;
        }
        // 将属性还原
        VirtualProtectEx(m_hProcess, pAddr, BUFFER_MAX, 
                                dwProtect,&dwProtect) ;

        /*
        // 打印出数据，但是觉得算法不太好
        int i = 0 ;
        for (; i < nLen; ++i)
        {
                if (0 == (i & 0xf))
                {
                        printf("%p  ", pAddr) ;
                        pAddr = (LPVOID)((DWORD)pAddr + 0x10) ;
                }

                printf("%02x", pBuf[i]) ;
                if (0 == ((i+1) % 8) && 0 != (i+1)%0x10)
                {
                        printf("-") ;
                }
                else
                {
                        printf(" ") ;
                }

                if (i >= 0xf && ((i+1) % 0x10 == 0))
                {
                        int j = i - 0xf ;
                        printf("   ") ;
                        for (; j <= i; ++j)
                        {
                                char ch = pBuf[j] ;
                                printf("%c", (0 == ch || 0xff == ch)?'.':ch) ;
                        }
                        printf("\r\n") ;
                }
        }
        i = nLen % 0x10 ;

        if (i < 0x10)
        {
                printf("\r\n") ;
                int j = 0 ;
                for (; j <= (0x10 - i); ++j)
                {
                        printf("   ") ;
                }
                for (j = nLen - i; j < nLen; ++j)
                {
                        char ch = pBuf[j] ;
                        printf("%c", (0 == ch || 0xff == ch)?'.':ch) ;
                }
                printf("\r\n") ;
        }
        */

        int nCount = 0;
        for(int i = nCount; i < nLen;)
        {
                // 输出前面的地址
                if (0 == (i & 0xf))
                {
                        printf("%p  ", pAddr) ;
                        pAddr = (LPVOID)((DWORD)pAddr + 0x10) ;
                }

                // 输出数据16进制
                int nIndex(0) ;
                for (; nIndex < 0x10 && i < nLen; ++i, ++nIndex)
                {
                        if (0 == (i%8) && 0 != (i &0xf))
                        {
                                printf("- ") ;
                        }
                        printf("%02X ", pBuf[i]) ;
                }

                if (nIndex <= 8)
                {
                        printf("  ") ;
                }

                // 如果不足0x10个的话，补足
                for(int k = 0x10 - nIndex; k >= 0; --k)
                {
                        printf("   ") ;
                }

                for(; nCount < i; ++nCount)
                {
                        char ch = pBuf[nCount] ;
                        // isgraph 是否是可显示字符
                        printf("%c", (isgraph(ch))?ch:'.') ;
                }
                printf("\r\n") ;
        }

        if (NULL != pBuf)
        {
                delete [] pBuf ;
                pBuf = NULL ;
        }

        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : ParseFunNameOfRegisterValue
*  功能描述 : 判断寄存器的值是否是api地址,同时取得dll名和函数名
*  参数列表 : IN  da        --      t_diasm结构体
              OUT pszDllName--      用来存储dll名字的指针
              OUT pszFunName--      用来存储函数名字的指针
*  说    明 : 
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::ParseFunNameOfRegisterValue(IN t_disasm &da, OUT char *pszDllName, OUT char *pszFunName)
{
        if (2 != GetParamCount(da.result))
        {
                return 0 ;
        }

        if (0x70 != da.cmdtype)
        {
                return 0 ;
        }

        char szBuf[MAXBYTE] = {0} ;
        BOOL bIsEffective = FALSE ;
        sscanf(da.result, "%s%s", stderr, szBuf) ;

        CONTEXT context;
        context.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_FULL ;
        char *pDllName = NULL ;
        char *pFunName = NULL ;
        int nJmpAddress = NULL ;

        if (FALSE == GetCurrentThreadContext(&context))
        {
                return 0 ;
        }

        if (0 == stricmp(szBuf, "eax"))
        {
                nJmpAddress = m_DllExportFun.GetFunName(context.Eax, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
  
        }

        else if (0 == stricmp(szBuf, "ebx"))
        {
                nJmpAddress = m_DllExportFun.GetFunName(context.Ebx, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
        }

        else if (0 == stricmp(szBuf, "ecx"))
        {
                nJmpAddress = m_DllExportFun.GetFunName(context.Ecx, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
        }

        else if (0 == stricmp(szBuf, "edx"))
        {
                nJmpAddress = m_DllExportFun.GetFunName(context.Edx, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
        }

        else if (0 == stricmp(szBuf, "esi"))
        {
                nJmpAddress = m_DllExportFun.GetFunName(context.Esi, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
        }

        else if (0 == stricmp(szBuf, "edi"))
        {

                nJmpAddress = m_DllExportFun.GetFunName(context.Edi, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
        }

        else if (0 == stricmp(szBuf, "ebp"))
        {

                nJmpAddress = m_DllExportFun.GetFunName(context.Ebp, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
        }

        else if (0 == stricmp(szBuf, "esp"))
        {
                nJmpAddress = m_DllExportFun.GetFunName(context.Esp, pDllName, pFunName) ;
                if (0 == nJmpAddress)
                {
                        return 0 ;
                }
                bIsEffective = TRUE ;
        }

        if (TRUE == bIsEffective)
        {
                memcpy(pszDllName, pDllName, sizeof(char)*MAXBYTE);

                DWORD dwNothing = 0 ;
                if (FALSE == ReadProcessMemory(m_hProcess, pFunName, pszFunName, 
                                sizeof(char)*MAXBYTE, &dwNothing))
                {
                        return 0 ;
                }
                return nJmpAddress ;
        }

        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : PrintCommandHelp
*  功能描述 : 输出帮助信息
*  参数列表 : IN ch        ---            提示输出类别的帮助
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void Debug::PrintCommandHelp(IN char ch)
{
        if ('b' == ch || 0 == ch)
        {
                printf("ba (添加一个硬件断点)            断点地址 长度 权限\r\n") ;
                printf("bp (int3断点)                    断点地址\r\n") ;
                printf("bl (显示所有断点信息)\r\n") ;
                printf("bc (清除所有int3和硬件断点信息)\r\n") ;
                printf("be (激活一个断点)                断点地址\r\n") ;
                printf("bd (禁用一个断点)                断点地址\r\n") ;
                printf("br (移除一个int3断点或硬件断点)  断点地址\r\n") ;
                printf("bm (添加一个内存断点)            断点地址 长度 权限\r\n") ;
                printf("by (移除一个内存断点)            断点地址\r\n") ;
        }

        if (0 == ch)
        {
                printf("t                                单步进入\r\n") ;
                printf("p                                单步步过\r\n") ;
                printf("r                                查看修改寄存器\r\n") ;
                printf("u [目标地址]                     反汇编\r\n") ;
                printf("? 或  h                          查看帮助\r\n") ;
                printf("g [目标地址]                     执行到目标地址处\r\n") ;
                printf("\t如果后面指定地址，中间的断点将全部失效\r\n") ;
                printf("l                                显示PE信息\r\n") ;
                printf("d [目标起始地址] [目标终址地址]/[长度] 查看内存\r\n") ;
                printf("e [目标起始地址]                 修改内存\r\n") ;
                printf("q                                退出\r\n") ;
                printf("s 记录范围起始地址 记录范围终止地址 [保存文件名]\r\n") ;
                printf("o [脚本路径名]                   运行脚本\r\n") ;

                printf("扩展命令:\r\n") ;
                printf(".kill                            结束被调试进程\r\n") ;
                printf(".restart                         重新加载被调试进程(测试)\r\n") ;
                printf(".show                            显示已加载模块\r\n") ;
        }
        printf("\r\n") ;
        return ;
}

/*******************************************************************************
*
*  函 数 名 : ParseEditMemory
*  功能描述 : 处理编辑内存(相当于debug的e命令)
*  参数列表 : IN pAddr        ---           修改目标内存地址
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
int Debug::ParseEditMemory(LPVOID pAddr)
{
        int     nPageID = 0 ;
        if (0 == m_MemList.HaveIn(pAddr, nPageID))
        {
                return 0 ;
        }

        DWORD dwProtect = 0 ;
        if(FALSE == VirtualProtectEx(m_hProcess, pAddr, BUFFER_MAX, 
                                PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("ParseEditMemory VirtualProtectEx出错!\r\n") ;
                return 0 ;
                
        }

        int     i;
        char    szDestBuf[MAXBYTE] = {0} ;
        char    ch = 0 ;
        char    szBuf[4] = {0} ;
        char    szOldBuf[MAXBYTE] = {0} ;
        BOOL    bIsContinue = TRUE ;
        DWORD   dwNothing = 0 ;

        printf("%p ", pAddr) ;
        printf(" %02x:", szOldBuf[0]) ;

        for (i = 0; bIsContinue && (2 == SafeInput(szBuf, 3)); ++i)
        {
                printf("%p ", pAddr) ;
                // 判断是不是超过最大长度了，是的话先写进目标进程
                if (i == (MAXBYTE - 1))
                {
                        if(FALSE == WriteProcessMemory(m_hProcess, pAddr,
                                szBuf, sizeof(char)*MAXBYTE, &dwNothing))
                        {
                                return 0 ;
                        }
                        i = 0 ;
                        memset(szDestBuf, 0, sizeof(char) * MAXBYTE) ;
                        pAddr = (LPVOID)((DWORD)pAddr + MAXBYTE) ;
                }

                // 判断是不是16进制的数
                if (isxdigit(szBuf[0]) && isxdigit(szBuf[1]))
                {
                        // 是的话放进szDestBuf中去
                        int nDestValue = 0 ;
                        sscanf(szBuf, "%x", &nDestValue);
                        ch = nDestValue ;
                        szDestBuf[i] = ch ;
                }
                else
                {
                        bIsContinue = FALSE ;
                }
                printf(" %02x:", szOldBuf[i+1]) ;
                if ((i + 1)%0x10 == 0)
                {
                        pAddr = (LPVOID)((DWORD)pAddr + 0x10) ;
                        printf("%p ", pAddr) ;
                }
        }

        VirtualProtectEx(m_hProcess, pAddr, BUFFER_MAX, dwProtect,&dwProtect) ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : OnceDrBreakToSoftBreak
*  功能描述 : 将用硬件断点加速软件断点的信息转成软件断点存储
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果有用硬件断点实现软中断的话，返回可用的Dr编号，否则返回0
*
*******************************************************************************/
int Debug::OnceDrBreakToSoftBreak(void)
{
        // 如果没有的话，返回0
        if(0 == 0xf & m_nIsTmpDrBreak)
        {
                return 0 ;
        }

        CONTEXT context ;
        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        if (FALSE == GetCurrentThreadContext(&context))
        {
                return 0 ;
        }

        if (0x1 & m_nIsTmpDrBreak)
        {
                unsigned int nAddr = context.Dr0 ;
                context.Dr0 = 0 ;
                SetCurrentThreadContext(&context) ;
                if (0 == AddSoftBreakPoint(nAddr))
                {
                        return 0 ;
                }
                RemoveDrRegister(1) ;
                m_nIsTmpDrBreak &= ~1 ;
                return 1 ;
        }

        if (0x2 & m_nIsTmpDrBreak)
        {
                unsigned int nAddr = context.Dr1 ;
                context.Dr1 = 0 ;
                SetCurrentThreadContext(&context) ;
                if (0 == AddSoftBreakPoint(nAddr))
                {
                        return 0 ;
                }
                RemoveDrRegister(2) ;
                m_nIsTmpDrBreak &= ~2 ;
                return 2 ;
        }

        if (0x4 & m_nIsTmpDrBreak)
        {
                unsigned int nAddr = context.Dr2 ;
                context.Dr2 = 0 ;
                SetCurrentThreadContext(&context) ;
                if (0 == AddSoftBreakPoint(nAddr))
                {
                        return 0 ;
                }
                RemoveDrRegister(3) ;
                m_nIsTmpDrBreak &= ~4 ;
                return 3 ;
        }

        if (0x8 & m_nIsTmpDrBreak)
        {
                unsigned int nAddr = context.Dr3 ;
                context.Dr3 = 0 ;
                SetCurrentThreadContext(&context) ;
                if (0 == AddSoftBreakPoint(nAddr))
                {
                        return 0 ;
                }
                RemoveDrRegister(4) ;
                m_nIsTmpDrBreak &= ~8 ;
                return 4 ;
        }

        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : EditRegisterValue
*  功能描述 : 编辑寄存器的值
*  参数列表 : IN pszCmd:        命令字符串
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int Debug::EditRegisterValue(IN char *pszCmd)
{
        if (NULL == pszCmd)
        {
                return 0 ;
        }
        
        char szRegister[BUFFER_MAX] = {0} ;
        unsigned int nRegisterValue = 0 ;
        sscanf(pszCmd, "%s%s", stderr, szRegister) ;

        CONTEXT context;
        context.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_FULL ;
        
        if (FALSE == GetCurrentThreadContext(&context))
        {
                return 0 ;
        }

        if (0 == stricmp(szRegister, "eax"))
        {
                printf("EAX: %p\r\nEAX: ", context.Eax) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Eax = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "ax"))
        {
                printf("AX: %p\r\nAX: ", 0xffff & context.Eax) ;

                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;

                context.Eax  = (context.Eax >> 16 << 16) ;
                context.Eax |= (nRegisterValue & 0xffff) ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "ebx"))
        {
                printf("EBX: %p\r\nEBX: ", context.Ebx) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Ebx = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "bx"))
        {
                printf("BX: %p\r\nBX: ", 0xffff & context.Ebx) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Ebx  = (context.Ebx >> 16 << 16) ;
                context.Ebx |= (nRegisterValue & 0xffff) ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "ecx"))
        {
                printf("ECX: %p\r\nECX: ", context.Ecx) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Ecx = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "cx"))
        {
                printf("CX: %p\r\nCX: ", 0xffff & context.Ecx) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Ecx  = (context.Ecx >> 16 << 16) ;
                context.Ecx |= (nRegisterValue & 0xffff) ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "edx"))
        {
                printf("EDX: %p\r\nEDX: ", context.Edx) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Edx = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "dx"))
        {
                printf("DX: %p\r\nDX: ", 0xffff & context.Edx) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Edx  = (context.Edx >> 16 << 16) ;
                context.Edx |= (nRegisterValue & 0xffff) ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "esi"))
        {
                printf("ESI: %p\r\nESI: ", context.Esi) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Esi = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "edi"))
        {
                printf("EDI: %p\r\nEDI: ", context.Edi) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Edi = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "eip"))
        {
                printf("EIP: %p\r\nEIP: ", context.Eip) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Eip = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "esp"))
        {
                printf("ESP: %p\r\nESP: ", context.Esp) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Esp = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "ebp"))
        {
                printf("EBP: %p\r\nEBP: ", context.Ebp) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.Ebp = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "cs"))
        {
                printf("CS: %p\r\nCS: ", context.SegCs) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.SegCs = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "ss"))
        {
                printf("SS: %p\r\nSS: ", context.SegSs) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.SegSs = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "ds"))
        {
                printf("DS: %p\r\nDS: ", context.SegDs) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.SegDs = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "es"))
        {
                printf("ES: %p\r\nES: ", context.SegEs) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.SegEs = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "fs"))
        {
                printf("FS: %p\r\nFS: ", context.SegFs) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.SegFs = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        else if (0 == stricmp(szRegister, "gs"))
        {
                printf("GS: %p\r\nGS: ", context.SegGs) ;
                if (0 == SafeHexInput(szRegister, BUFFER_MAX))
                {
                        return 0 ;
                }
                sscanf(szRegister, "%x", &nRegisterValue) ;
                context.SegGs = nRegisterValue ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  这个函数没有使用，原来判断call jmp时的函数
*
*  函 数 名 : ParseSCommand
*  功能描述 : 处理动态跟踪的功能
*  参数列表 : IN pszCmd:        命令字符串
*  说    明 : 对于参数的检查不严格  
*  返回结果 : 
*
*******************************************************************************/
int Debug::ParseSCommand(IN char *pszCmd)
{
        if (NULL == pszCmd)
        {
                return 0 ;
        }

        int nParamCount = GetParamCount(pszCmd) ;
        char szBuf[BUFFER_MAX] = {0} ;

        DWORD dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        DWORD dwNothing = 0 ;
        
        if (NULL == strstr(pszCmd, "NoIntoAPI"))
        {
                m_bIsIntoAPI = FALSE ;
        }
        else
        {
                m_bIsIntoAPI = TRUE ;
        }

        if (1 == nParamCount)
        {
                if (NULL != m_pRecordFileName)
                {
                        delete [] m_pRecordFileName ;
                        m_pRecordFileName = NULL ;
                }
                
                m_pRecordFileName = new char[24] ;
                if (NULL == m_pRecordFileName)
                {
                        OutputDebugString("申请内存出错了!\r\n") ;
                        return 0 ;
                }
                strcpy(m_pRecordFileName, "InstructionRecord.txt") ;
                
                InstructionRecord() ;
                return 1 ;
        }

        if (2 == nParamCount)
        {
                sscanf(pszCmd, "%s%s", stderr, szBuf) ;
                int nStrLen = strlen(szBuf) ;
                if (nStrLen >= BUFFER_MAX)
                {
                        OutputDebugString("Overflow!\r\n") ;
                        return 0 ;
                }
                if (strchr(szBuf, '.'))
                {
                        int nLen = strlen(szBuf) ;
                        ++nLen ;
                        m_pRecordFileName = new char[nLen] ;
                        
                        if (NULL == m_pRecordFileName)
                        {
                                OutputDebugString("申请内存出错了!\r\n") ;
                                return 0 ;
                        }
                        memset(m_pRecordFileName, 0, sizeof(char) * nLen) ;

                        strcpy(m_pRecordFileName, szBuf) ;
                        InstructionRecord() ;
                        return 1 ;
                }
        }

        if (3 == nParamCount)
        {
                char szFileName[BUFFER_MAX] = {0} ;
                sscanf(pszCmd, "%s%s%s", stderr, szBuf, szFileName) ;
                int nStrLen = strlen(szBuf) ;

                if (nStrLen >= BUFFER_MAX)
                {
                        OutputDebugString("Overflow!\r\n") ;
                        return 0 ;
                }

                int nFileNameLen = strlen(szFileName) ;
                if (nFileNameLen > BUFFER_MAX)
                {
                        OutputDebugString("FileName Overflow!\r\n") ;
                        return 0 ;
                }
                
                // 这里参数检查，看哪个是存放文件名的
                if (stricmp(szBuf,"NoIntoAPI") || stricmp(szBuf, "IntoAPI"))
                {
                        nFileNameLen = strlen(szFileName) ;
                        if (NULL != m_pRecordFileName)
                        {
                                delete [] m_pRecordFileName ;
                                m_pRecordFileName = NULL ;
                        }
                        ++nFileNameLen ;
                        m_pRecordFileName = new char[nFileNameLen] ;

                        if (NULL == nFileNameLen)
                        {
                                OutputDebugString("申请内存出错了!\r\n") ;
                                return 0 ;
                        }

                        memset(m_pRecordFileName, 0, nFileNameLen) ;
                        strncpy(m_pRecordFileName, szFileName, nFileNameLen - 1) ;

                        InstructionRecord() ;
                        return 1 ;
                }
                else
                {
                        nFileNameLen = strlen(szBuf) ;
                        if (NULL != m_pRecordFileName)
                        {
                                delete [] m_pRecordFileName ;
                                m_pRecordFileName = NULL ;
                        }
                        ++nFileNameLen ;
                        m_pRecordFileName = new char[nFileNameLen] ;
                        
                        if (NULL == nFileNameLen)
                        {
                                OutputDebugString("申请内存出错了!\r\n") ;
                                return 0 ;
                        }
                        
                        memset(m_pRecordFileName, 0, nFileNameLen) ;
                        strncpy(m_pRecordFileName, szBuf, nFileNameLen - 1) ;
                        InstructionRecord() ;
                        return 1 ;         
                }
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : InstructionRecord
*  功能描述 : 指令记录
*  参数列表 : 无
*  说    明 : 处理指令记录功能
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int Debug::InstructionRecord(void)
{
        DWORD dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        DWORD dwImageBase = m_PEInformation.GetImageBase() ;
        
        
        // 将内存页的属性加强
        DWORD dwProtect = 0 ;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, 
                PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("InstructionRecord VirtualProtectEx出错!\r\n") ;
                return 0 ;
                
        }

        char szBuf[BUFFER_MAX] = {0} ;
        DWORD dwNumberOfBytes  =  0  ;
        
        // 先去读指令
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                sizeof(char) * BUFFER_MAX, &dwNumberOfBytes) )
        {
                OutputDebugString("读目标进程内存出错!\r\n") ;
                return 0;
        }
        
        // 这里判断是不是call,不是call的话直接设TF标志
        // 否则置单步
        t_disasm da ;
        int nInstrLen = Disasm(&szBuf[0], 20, dwEip, &da,DISASM_CODE) ;
        
        char szDllName[MAXBYTE] = {0} ;
        char szFunName[MAXBYTE] = {0} ;

        // 如果等于call
        unsigned int nJmp = m_DllExportFun.AddressToFunName(da, szDllName, szFunName) ;
        
        if (0x70 == da.cmdtype && 0 == nJmp)
        {
                nJmp = ParseFunNameOfRegisterValue(da, szDllName, szFunName) ;
        }
        
        VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, dwProtect, &dwProtect) ;
        
        // 先判断树中这个结点是否存在
        AberrTreeNode * p = m_InstructionRecord.FindEx(dwEip, szBuf, nInstrLen) ;
        if (NULL == p)
        {
                // 将记录加到树中，并且打印出来
                p = m_InstructionRecord.Insert(dwEip, szBuf, nInstrLen) ;
                if (NULL != p)
                {
                        p->jmpconst = (PVOID)nJmp ;
                }
                else
                {
                        printf("指令加入失败!\r\n") ;
                }
        }
        else
        {
                ++p->nCount ;
        }

        // 这里可以根据条件输出
        printf("%p: %s", dwEip, da.result) ;
        if (nJmp)
        {
                printf(" <%s::%s>", szDllName, szFunName) ;
        }
        printf("\r\n") ;
        return 1 ;
        /*
        // 原来从第一条一直记录下去，如果call api的话就跳过
        DWORD dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        DWORD dwImageBase = m_PEInformation.GetImageBase() ;

        // 因为有可能出错，导致进入到api里面，如果当前eip不在用户空间内，跳过之
        if (dwImageBase != (dwEip & dwImageBase) && FALSE == m_bIsIntoAPI)
        {
                SetTFRegister() ;
                return 1 ;
        }

        if (0 == dwEip)
        {
                return 0 ;
        }

        // 将内存页的属性加强
        DWORD dwProtect = 0 ;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, 
                PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("InstructionRecord VirtualProtectEx出错!\r\n") ;
                return 0 ;
                
        }

        char szBuf[BUFFER_MAX] = {0} ;
        DWORD dwNumberOfBytes = 0 ;

        // 先去读指令
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                sizeof(char) * BUFFER_MAX, &dwNumberOfBytes) )
        {
                return 0;
        }

        // 这里判断是不是call,不是call的话直接设TF标志
        t_disasm da ;
        int nInstrLen = Disasm(&szBuf[0], 20, dwEip, &da,DISASM_CODE) ;
        // 如果等于call
        char szDllName[MAXBYTE] = {0} ;
        char szFunName[MAXBYTE] = {0} ;
        unsigned int nJmp = m_DllExportFun.AddressToFunName(da, szDllName, szFunName) ;

        if (0x70 == da.cmdtype && 0 == nJmp)
        {
                nJmp = ParseFunNameOfRegisterValue(da, szDllName, szFunName) ;
        }
        
        if (nJmp)
        {
                // 如果call API的话，在下一条指令处下CC
                // 可以用G代替
                ParseGoTo(dwEip + nInstrLen) ; 
        }
        else
        {
                //m_nIsTCommand = TRUE ;
                SetTFRegister() ;
        }


        VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, dwProtect, &dwProtect) ;

        // 将记录加到树中，并且打印出来
        AberrTreeNode* p = NULL ;
        p = m_InstructionRecord.Insert(dwEip) ;
        if (NULL == p)
        {
                return 0 ;
        }
        p->SetDump(szBuf, nInstrLen) ;
        p->jmpconst = (PVOID)nJmp ;
        printf("%p: %s", dwEip, da.result) ;
        if (nJmp)
        {
                printf(" <%s::%s>", szDllName, szFunName) ;
        }
        printf("\r\n") ;

        return 1 ;
        */
}

/*******************************************************************************
*
*  函 数 名 : WriteRecordToFile
*  功能描述 : 写入文件
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int Debug::WriteRecordToFile(void)
{
        if (NULL == m_pRecordFileName)
        {
                return 0 ;
        }

        HANDLE hFile = CreateFile(m_pRecordFileName, 
                                  GENERIC_WRITE, 
                                  FILE_SHARE_READ,
                                  NULL, 
                                  CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL) ;

        if (INVALID_HANDLE_VALUE == hFile)
        {
                OutputDebugString("打开文件出错了!\r\n") ;
                return 0 ;
        }

        AberrTreeNode *p = m_InstructionRecord.GetListHead() ;

        t_disasm da ;
        char szFunName[BUFFER_MAX] = {0} ;
        char *pszDllName = 0 ;
        char *pszFunName = 0 ;
        DWORD dwNumberOfBytesWritten = 0 ;
        char ch = '\n' ;
        char cSpace = ' ';
        char szDestBuf[MAXBYTE] ;
        char szBuf[MAXBYTE] ;

        for (; NULL != p ; p = p->pNext)
        {
                memset(szDestBuf, 0, sizeof(char) * MAXBYTE) ;
                memset(szBuf, 0, sizeof(char) * MAXBYTE) ;
                int nLen = 0 ;
                Disasm(p->szDump, 20, p->nAddress, &da,DISASM_CODE) ;
                
                // 指令跑的次数
                sprintf(szDestBuf, "%4d ", p->nCount) ;

                // 指令地址
                sprintf(szBuf, "%p ", p->nAddress) ;
                strcat(szDestBuf, szBuf) ;

                // 反汇编结果
                sprintf(szBuf, "%s ", da.result) ;
                strcat(szDestBuf, szBuf) ;  
                
                // 判断是不是调用api,是的话再将dll名和函数名一起输出
                if (m_DllExportFun.GetFunName((unsigned int)p->jmpconst, pszDllName, pszFunName))
                {
                        if(FALSE == ReadProcessMemory(m_hProcess, pszFunName, 
                                                        szFunName, BUFFER_MAX,
                                                        &dwNumberOfBytesWritten))
                        {
                                CloseHandle(hFile) ;
                                hFile = NULL ;
                        }

                        sprintf(szBuf, "<%s::", pszDllName) ;
                        strcat(szDestBuf, szBuf) ;
                        sprintf(szBuf, "%s>", szFunName) ;
                        strcat(szDestBuf, szBuf) ;
                }

                strcat(szDestBuf, "\r\n") ;

                WriteFile(hFile, szDestBuf, sizeof(char) * strlen(szDestBuf), 
                                                &dwNumberOfBytesWritten, NULL) ;
        }
        
        CloseHandle(hFile) ;
        hFile = NULL ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : ParseExternCommand
*  功能描述 : 处理扩展命令
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int Debug::ParseExternCommand(char *szBuffer)
{
        if (NULL == szBuffer)
        {
                return 0 ;
        }

        for (int i = 0; i < sizeof(c_ExternCommand)/sizeof(c_ExternCommand[0]); ++i)
        {
                if (0 == stricmp(c_ExternCommand[i].pCommand, szBuffer))
                {
                        if (NULL != c_ExternCommand[i].pFun)
                        {
                                return (this->*c_ExternCommand[i].pFun)() ;
                        }
                }
        }
        return 0 ;
}


/*******************************************************************************
*
*  函 数 名 : Restart
*  功能描述 : 重新启动被调试程序，不过有bug
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int Debug::Restart()
{
        if (0 == m_MemList.RevertMemoryProtectAll(m_hProcess))
        {
                return 0 ;
        }
        m_MemBreak.RemoveAll() ;
        m_BreakPointList.Clean() ;
        m_BreakMapPage.Clear() ;
        m_PageMapBreak.Clear() ;
        // 再清下硬件断点
        if (m_UseDrRegister & 0xf != 0)
        {
                if (m_UseDrRegister & 1)
                {
                        RemoveDrRegister(1) ;
                }
                if (m_UseDrRegister & 2)
                {
                        RemoveDrRegister(2) ;
                }
                if (m_UseDrRegister & 4)
                {
                        RemoveDrRegister(3) ;
                }
                if (m_UseDrRegister & 8)
                {
                        RemoveDrRegister(4) ;
                }
         }
        
        if (0 == Kill())
        {
                printf(".Restart Filed!\r\n") ;
                return 0 ;
        }

        GetStartupInfo(&m_StartupInfo) ;
        
        BOOL bRet = CreateProcess(m_pExePathName, NULL, NULL, NULL, FALSE, 
                DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS, NULL, 
                NULL, &m_StartupInfo, &m_pi) ;
        if (FALSE == bRet)
        {
                return 0 ;
        }

        DWORD dwContinueStatus = DBG_CONTINUE ;
        ContinueDebugEvent(m_DebugEvent.dwProcessId, 
                                   m_DebugEvent.dwThreadId, dwContinueStatus); 

        return bRet ;
}

/*******************************************************************************
*
*  函 数 名 : Kill
*  功能描述 : 杀死被调试程序
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int Debug::Kill()
{
        if (NULL == m_pExePathName)
        {
                return 0 ;
        }
        
        if (NULL == m_hProcess)
        {
                return 0 ;
        }

        if (0 == m_dwProcessID)
        {
                return 0 ;
        }

        if (NULL == m_hProcess)
        {
                return 0 ;
        }
    
        if (FALSE == TerminateProcess(m_hProcess, 0))
        {
                return 0 ;
        }

        DWORD dwContinueStatus = DBG_CONTINUE ;
        ContinueDebugEvent(m_DebugEvent.dwProcessId, 
                                   m_DebugEvent.dwThreadId, dwContinueStatus) ;
        ParseGoTo(0) ;

        // FatalExit(1) ;


        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : DisplayLoadDllName
*  功能描述 : 显示当前被调试进程已经加载的Dll名称
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int Debug::DisplayLoadDllName()
{
        m_DllExportFun.DisplayLoadDllName() ;
        ParseRequest() ;
        return 1 ;
}

int Debug::ReDebug()
{
        __asm
        {
                mov eax, fs:[0x18]
                mov eax, [eax + 0x30]
                mov byte ptr [eax+2], 0
        }
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : IsEffectiveAddress
*  功能描述 : 判断是否是有效地址
*  参数列表 : IN lpAddr    --      内存地址
              IN pMbi      --      用来存储内存分页信息
*  说    明 : 
*  返回结果 : 如果地址处于有效内存分页的话，返回1,否则返回0
*
*******************************************************************************/
int Debug::IsEffectiveAddress(IN LPVOID lpAddr, IN PMEMORY_BASIC_INFORMATION pMbi)
{
        if (NULL == pMbi)
        {
                return 0 ;
        }
        if (sizeof(MEMORY_BASIC_INFORMATION) 
                != VirtualQueryEx(m_hProcess, lpAddr, pMbi, 
                                sizeof(MEMORY_BASIC_INFORMATION)))
        {
                return 0 ;
        }
        
        if (MEM_COMMIT == pMbi->State)
        {
                return 1 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : InsertEffectiveMemoryPage
*  功能描述 : 插入一个有效的内存分页
*  参数列表 : IN lpAddr    --      内存地址
              IN pMbi      --      用来存储内存分页信息
*  说    明 : 
*  返回结果 : 如果插入成功，返回有效的内存页ID,否则返回0
*
*******************************************************************************/
int Debug::InsertEffectiveMemoryPage(IN LPVOID lpAddr, IN PMEMORY_BASIC_INFORMATION pMbi)
{
        int nPageID = 0 ;
        if (NULL == pMbi)
        {
                return 0 ;
        }
        if (0 == IsEffectiveAddress(lpAddr, pMbi))
        {
                OutputDebugString("地址不存在!\r\n") ;
                return 0 ;
        }
        
        PMemNode p = NULL ;
        
        // 先判断这个分页是否存在
        if( 0 == m_MemList.IsHaving(pMbi))
        {
                nPageID = m_MemList.Insert(pMbi) ;
                p = m_MemList.Find(pMbi) ;
                if (NULL == p)
                {
                        OutputDebugString("找不到那个内页属性页!\r\n") ;
                        return 0 ;
                }
        }
        else
        {
                p = m_MemList.Find(pMbi) ;
                if (NULL == p)
                {
                        OutputDebugString("找不到那个内页属性页!\r\n") ;
                        return 0 ;
                }
                nPageID = p->nID ;
        }
        return nPageID ;
}

/*******************************************************************************
*
*  函 数 名 : RemoveAllBreakPoint
*  功能描述 : 移除所有的int3断点和硬件断点
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 返回1
*
*******************************************************************************/
int Debug::RemoveAllBreakPoint()
{
        RevertAllBreakPoint() ;
        m_BreakPointList.Clean() ;
        // 再清下硬件断点
        if (m_UseDrRegister & 0xf != 0)
        {
                if (m_UseDrRegister & 1)
                {
                        RemoveDrRegister(1) ;
                }
                if (m_UseDrRegister & 2)
                {
                        RemoveDrRegister(2) ;
                }
                if (m_UseDrRegister & 4)
                {
                        RemoveDrRegister(3) ;
                }
                if (m_UseDrRegister & 8)
                {
                        RemoveDrRegister(4) ;
                }
        }
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : BreakPointIsHave
*  功能描述 : 断点地址是否存在
*  参数列表 : IN nAddr:  地址值
*  说    明 : 
*  返回结果 : 如果地址无效，返回-1,如果不存在返回0,如果存在返回1
*
*******************************************************************************/
int Debug::BreakPointIsHave(IN unsigned int nAddr)
{
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("地址不存在!\r\n") ;
                return -1 ;
        }
        
        if (NULL == m_hProcess)
        {
                return -1 ;
        }
        
        // 这里判断这个断点是否已经存在了
        if (NULL != m_BreakPointList.Find(nAddr))
        {
                OutputDebugString("断点已经存在了!\r\n") ;
                return 1 ;
        }
        
        // 再判断是否存在硬件断点中
        if (0 != GetDrNumberOfAddress((LPVOID)nAddr))
        {
                OutputDebugString("断点已经存在硬件断点中了!\r\n") ;
                return 1 ;
        }
        
        // 再判断在不在内存断点中
        if (0 != m_MemBreak.HaveIn((LPVOID)nAddr, nPage))
        {
                OutputDebugString("断点已经存在内存断点中了!\r\n") ;
                return 1 ;
        }
        return 0 ;
}
