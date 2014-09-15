/*******************************************************************************
*
*  Copyright (c) all 2010 ���� All rights reserved
*  FileName : debug.cpp
*  D a t e  : 2010.3.4
*  ��   ��  : ���Թ���ʵ��Դ�ļ�
*  ˵   ��  :
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

// ��չ����
Command Debug::c_ExternCommand[] = {".restart", Restart,
                                    ".show",    DisplayLoadDllName,
                                    ".kill",    Kill} ;

/*******************************************************************************
*
*  �� �� �� : SetExePathName
*  �������� : ���ñ����Կ�ִ�г��������
*  �����б� : IN _pExePatchName    --  �����Գ��������
*  ˵    �� : 
*  ���ؽ�� : �������FALSE�Ļ�����,�ɹ�����TRUE
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
                printf("��磬������?\r\n") ;
                return FALSE ;
        }

        if (nLen < 5)
        {
                printf("�ļ���̫���˰�!\r\n") ;
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
*  �� �� �� : StartDebug
*  �������� : �������Թ�����ں���
*  �����б� : IN _pExePatchName    --  �����Գ��������
*  ˵    �� : 
*  ���ؽ�� : �������0�Ļ����ļ�������,�������̳����򷵻�-1��
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

        // ��Ϊ�̲߳�һ��������Ҫ�ģ������Ȳ��ø���
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
                                                        // �����û�������
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
                                        // ��������û��Ĵ����в����޸ĵ�����־
                                                // �ȿ����ǲ�����ϵ���Ҫ�ָ�
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
                                MessageBox(NULL, TEXT("�̴߳���"), TEXT("Tips"), MB_OK) ;
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
                                
                                // ȡ��������߳̾���ȵ�
                                m_hThread = m_DebugEvent.u.CreateProcessInfo.hThread ;
                                m_hProcess = m_DebugEvent.u.CreateProcessInfo.hProcess ;
                                m_lpStartAddress = (DWORD)m_DebugEvent.u.CreateProcessInfo.lpStartAddress ;
                                m_lpBaseOfImage = m_DebugEvent.u.CreateProcessInfo.lpBaseOfImage ;
                                
                                // �ڳ�����ڴ��¸��ϵ�
                                char ch ;
                                BOOL bRet ;
                                DWORD dwNumberOfBytesRead ;
                                // ��ΪҪ�ó���ͣ����ڣ���������������¸�CC�ϵ㣬
                                // �Ƚ�ԭ����ֵ����
                                bRet = ReadProcessMemory(m_hProcess,(LPCVOID)m_lpStartAddress,
                                        (PVOID)&ch, sizeof(char), &dwNumberOfBytesRead) ;
                                if (FALSE == bRet)
                                {
                                        OutputDebugString("��Ŀ����̳�����\r\n") ;
                                        break ;
                                }

                                // д��CC ���ϵ���Ϣ�ŵ���ʱ�ϵ�ṹ����
                                //m_BreakPointList.Insert(m_lpStartAddress, ch) ;

                                m_TmpBreakPoint.byOldValue = ch ;
                                m_TmpBreakPoint.nAddr = m_lpStartAddress ;
                                m_TmpBreakPoint.StdState() ;

                                bRet = WriteProcessMemory(m_hProcess,(LPVOID)m_lpStartAddress,
                                        (PVOID)&chSoftBreak,sizeof(char),&dwNumberOfBytesRead) ;
                                if (FALSE == bRet)
                                {
                                        OutputDebugString("дĿ����̳�����\r\n") ;
                                        break ;
                                }

                                // ��ʾPE��Ϣ
                                m_PEInformation.DispPEInformation() ;
                                //m_MemList.RemoveAll() ;
                                // ö��ϵͳ��Ч�ڴ��ҳ
                                //m_MemList.EnumDestProcessEffectivePage(m_hProcess) ;
                                // ��������Խ�����Ч���ڴ�ҳ
                                //m_MemList.Display() ;
                                printf("\r\n") ;
                                m_DllExportFun.SetProcessValue(m_hProcess) ;
                        }
                        break ;
                                
                        case EXIT_THREAD_DEBUG_EVENT: 
                        {
                                // Display the thread's exit code. 
                                OutputDebugString("Ŀ���̴߳���!\r\n") ;
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
                                OutputDebugString( "Ŀ������˳�!\r\n") ;
                                //ExitProcess(0) ;
                        }
                        break;
                                
                        case LOAD_DLL_DEBUG_EVENT: 
                        {
                        // Read the debugging information included in the newly 
                        // loaded DLL. 
                        // ����dll
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
*  �� �� �� : GetCurrentEip
*  �������� : ȡ�õ�ǰ�������̵߳�EIP
*  �����б� : IN dwThreadId:    �߳�id
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����eipֵ��ʧ�ܷ���0
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
*  �� �� �� : PrintContext
*  �������� : ����Ĵ�������
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : 
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
*  �� �� �� : PrintInstruction
*  �������� : ���ָ��
*  �����б� : IN Eip         ---    Eipֵ
              IN bIsContimue ---    �Ƿ�����ϴε����
              IN nItem       ---    Ԥ���ָ������

*  ˵    �� : 
*  ���ؽ�� : �ɹ�����ָ�ʧ�ܷ���0
*
*******************************************************************************/
int Debug::PrintInstruction(IN int Eip, IN BOOL bIsContimue, IN int nItem)
{
        if (INVALID_HANDLE_VALUE == m_hProcess )
        {
                OutputDebugString("���̾����Ч\r\n") ;
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

        // �Ȼָ�ԭ����ָ��
        RevertAllBreakPoint() ;

        char szBuf[BUFFER_MAX] = {0} ;
        DWORD  dwNumberOfBytesRead ;
        if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                                        BUFFER_MAX, &dwNumberOfBytesRead))
        {
                OutputDebugString("��Ŀ�����ʧ��\r\n") ;
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
        // �����½��ϵ����ȥ
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
*  �� �� �� : GetInstructionLen
*  �������� : ȡ��ָ���߳���һ��ָ��ĳ���
*  �����б� : IN  dwThreadId:              �߳�id
              OUT pLen:                    ָ���
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����Eip�����򷵻�0
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
                OutputDebugString("�޸��ڴ����Գ���!\r\n") ;
                return 0 ;
        }

        DWORD   dwNumberOfBytesRead ;
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)context.Eip, szBuf, 
                                                BUFFER_MAX, &dwNumberOfBytesRead))
        {
                CloseHandle(m_hThread) ;
                m_hThread = INVALID_HANDLE_VALUE ;
        }

        // �����ǵ���OD�ķ��������
        t_disasm td ;
        *pLen = Disasm(szBuf, 20, 0, &td, DISASM_CODE) ;

        VirtualProtectEx(m_hProcess, (LPVOID)context.Eip, BUFFER_MAX, 
                                                dwProtect, &dwProtect) ;

        return context.Eip;
}

/*******************************************************************************
*
*  �� �� �� : ParseAccessVioltion
*  �������� : ��������쳣
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : 
*
*******************************************************************************/
DWORD Debug::ParseAccessVioltion(void)
{
        DWORD nType = m_DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[0] ;
        LPVOID pAddr = (LPVOID)m_DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[1] ;

        // ��1���Լ������ 1�Ƕ���2��д
        ++nType ;
        int nPage = 0 ;


        // ȡ���ڴ�ҳid
        if ( !m_MemList.HaveIn(pAddr, nPage))
        {
                return DBG_EXCEPTION_NOT_HANDLED ;
        }

        // �ж����ҳ��û�жϵ���Ϣ��û�еĻ�����
        aList *p = &m_PageMapBreak[nPage] ;
        if (NULL == p)
        {
                return DBG_EXCEPTION_NOT_HANDLED ;
        }
        // ���û�жϵ���Ϣ
        int nBreakCount = p->GetSize() ;
        if (0 == nBreakCount)
        {
                return DBG_EXCEPTION_NOT_HANDLED ;
        }

        
        PMemBreakNode pBreakNode = NULL ;
        // ȡ�ñ����ڴ�ҳ���ԵĽڵ�
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
        // ���ж�һ����û������
        for (int i = 0; i < nBreakCount; ++i)
        {
                // �õ��Ǹ���ҳ�е�i�����
                DistPosition pNode = m_PageMapBreak[nPage][i] ;
                // ͨ��pNode->data�õ��ϵ��,��ͨ��FinByID�õ��ϵ���Ϣ��ָ��
                pBreakNode = m_MemBreak.FindById(pNode->data) ;
                if (HaveInBreakArea((int)pAddr, pBreakNode)
                        && pBreakNode->Protect == nType)
                {
                        // ����Ǵ���ָ���¼�Ļ����Ͳ��ø��û�������ʾ
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
                                printf("�ڴ��ж�!\r\n") ;
                                ParseRequest() ;
                                return DBG_CONTINUE ;
                        }
                }
                
        }
        
        // �õ���
        SetTFRegister() ;

        return DBG_CONTINUE ;
}

/*******************************************************************************
*
*  �� �� �� : isMyBreakPoint
*  �������� : �Ƿ����û���Ķϵ������ϵͳ��һ�ε��õ�����ж�
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ������Լ��ܴ���Ķϵ㣬�򷵻�1�����򷵻�0
*
*******************************************************************************/
int Debug::isMyBreakPoint(void)
{
        // ���ж��ǲ���ϵͳ�Զ����� BreakPoint
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
        
        // ����Temp�ϵ�����
        if (m_TmpBreakPoint.bIsActive && m_TmpBreakPoint.nAddr == nAddr)
        {
                return 1 ;
        }
        
        // �ڶϵ���������
        int nRet = m_BreakPointList.GetValue(nAddr, &ch) ;
        if (nRet != 0 )
        {
                return 1 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : ParseBreakPoint
*  �������� : ����ϵ��쳣
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ����ܴ�������DBG_CONTINUE�����򷵻�DBG_EXCEPTION_NOT_HANDLED
*
*******************************************************************************/
int Debug::ParseBreakPoint(void)
{
        // ���ж��ǲ���ϵͳ�Զ����� BreakPoint
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
                OutputDebugString("�޸��ڴ����Գ���!\r\n") ;
                return 0 ;
        }

        // ����Temp�ϵ�����
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
                        printf("int3�ж�!\r\n") ;
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

        // �ڶϵ���������
        if (0 != m_BreakPointList.GetValue(nAddr, &ch))
        {
                if (WriteProcessMemory(m_hProcess, (LPVOID)nAddr, &ch,
                                sizeof(char), &dwNumberOfBytesWritten))
                {
                        // eip��1
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


                        // �´�Ҫ�ָ�����ϵ��־
                        m_uSoftBreakResume = nAddr ;
                        // ������ϵ���ΪʧЧ
                        m_BreakPointList.Find(nAddr)->ClsState() ; 
                        VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX,
                                        dwProtect, &dwProtect) ;

                        if(FALSE == m_bIsRecord)
                        {
                                SetTFRegister() ;
                                PrintContext() ;
                                PrintInstruction(context.Eip, FALSE, 1) ;
                                printf("int3�ж�!\r\n") ;
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
*  �� �� �� : ParseRequest
*  �������� : �����û����������
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ����ܹ�����Ļ�����1,���ܴ�����0
*
*******************************************************************************/
int Debug::ParseRequest(void)
{
        BOOL  bFlag = TRUE ;
        BOOL  isUCommand = FALSE ;
        static FILE  *fp = NULL ;                      // �ļ�ָ��,����ű���
        static BOOL  bIsScript = FALSE ;               // ��ǰ�Ƿ�ӽű�����
        
        while (bFlag)
        {
                printf("-") ;
                char szBuffer[BUFFER_MAX] = {0} ;

                // �������룬���ļ����뻹�ǵȴ��û��Ӽ�������
                if (FALSE == bIsScript)
                {
                        // ��ֹ���,��ȫ������
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

                // ȥ���ո�
                SkipSpace(szBuffer, BUFFER_MAX) ;

                // �����û�����
                // ����û������Ϣ���Ҿ������������������
                // ���Ǻ������Եúܷ���
                switch(szBuffer[0])
                {
                case 'b':
                case 'B':
                        // ����B������������bp, ba, bd
                        isUCommand = FALSE ;
                        ParseBCommand(szBuffer) ;
                        break ;
                case 'e':
                case 'E':
                        // ������ܻ�û�������޸��ڴ��ֵ
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
                        // ��������
                        isUCommand = FALSE ;
                        m_nIsTCommand = TRUE ;
                        SetTFRegister() ;
                        bFlag = FALSE ;
                        break ;
                case 'p':
                case 'P':
                        // ��������
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
                        // ����Ҫ����һ��u����Ĳ���
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
                        // ����Ҫ����һ��g����Ĳ���
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
                        // ��ʾ�ڴ�����
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

                             // ���ڴ�ϵ�ȫ���Ƴ�
                             //�Ȼ�ԭ�ڴ�����
                             m_MemList.RevertMemoryProtectAll(m_hProcess) ;
                             //������ڴ�ϵ�����н��
                             m_MemBreak.RemoveAll() ;
                             // �ٰ��ڽӱ�Ĺ�ϵ���
                             m_PageMapBreak.Clear() ;
                             m_BreakMapPage.Clear() ;

                             // ������е�int3��Ӳ���ϵ�
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
                                             printf("���������˰�!\r\n") ;
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
                                             printf("�����ڴ����!\r\n") ;
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
                                             printf("���������˰�!\r\n") ;
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
                                             printf("�����ڴ����!\r\n") ;
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
                        // ����ű�
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
                        // ������չ����
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
*  �� �� �� : SetSingleStep
*  �������� : ��CC�жϣ��õ���(��������)
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1,���ܴ�����0
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
        // ��ȥ��ָ��
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                sizeof(char) * BUFFER_MAX, &dwNumberOfBytes) )
        {
                return 0;
        }

        // �����ж��ǲ���call,����call�Ļ�ֱ����TF��־
        t_disasm da ;
        int nInstrLen = Disasm(&szBuf[0], 20, dwEip, &da,DISASM_CODE) ;
        // �������call
        if (0x70 == da.cmdtype)
        {
                // ��call����һ��ָ���CC
                // ������G����
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
*  �� �� �� : SetTFRegister
*  �������� : ��TFλ���õ���(��������)
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1,���ܴ�����0
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
*  �� �� �� : ParseLoadDllExportTable
*  �������� : �������dll�¼�������dll�ĵ�������������call����ĺ�����
*  �����б� : debugEvent        -- �����¼�
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1,���ܴ�����0
*
*******************************************************************************/
int Debug::ParseLoadDllExportTable(IN DEBUG_EVENT &debugEvent)
{
        // ����dll�ĵ�����
        return m_DllExportFun.InsertDll((HMODULE)(debugEvent.u.LoadDll.lpBaseOfDll)) ;
}

/*******************************************************************************
*
*  �� �� �� : DebugHelp
*  �������� : �������
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void Debug::DebugHelp(void)
{
        printf("֧��Debug����\r\n") ;
        return ;
}

/*******************************************************************************
*
*  �� �� �� : ParseUCommand
*  �������� : ����U����
*  �����б� : pszCmd       --   �����ַ���
*             bISContinue  --   �Ƿ���ԭ���Ļ����ϼ��������
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ���򷵻�0
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

        // ���ֻ�ж�������
        // ����Ϊ�˷����Ժ���չ���ܣ�ͬʱ���򵥵İ�ȫ���
        if (2 == nArgc)
        {
                sscanf(pszCmd, "%s%x", stderr, &dwEip) ;

                /*
                // �����жϵ�ַ�Ƿ�Ϸ�
                if ((dwEip & 0x80000000) || (dwEip <= 0x4096))
                {
                        return 0 ;
                }
                */
                // �����ж�һ�µ�ַ�Ƿ�����Ч���ڴ��ҳ����
                int nPage = 0 ;
                MEMORY_BASIC_INFORMATION mbi = {0} ;
                if (0 == IsEffectiveAddress((LPVOID)dwEip, &mbi))
                {
                        printf("Ŀ���ַ������!\r\n") ;
                        return 0 ;
                }
                PrintInstruction(dwEip,FALSE, 8) ;
        }
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : PrintCurrentRegister
*  �������� : �����ǰ�Ĵ�������
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ��
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
*  �� �� �� : ParseGoTo
*  �������� : ����GoTo����
*  �����б� : nAddr             --              Ҫ��ȥ�ĵ�ַ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1,ʧ�ܷ���0
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
*  �� �� �� : ParseGCommand
*  �������� : ����G����
*  �����б� : pszBuffer         --       �����ַ���
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1,ʧ�ܷ���0
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
                // ��ȡ�����еĶϵ�
                RevertAllBreakPoint() ;
                unsigned int nDestAddr = 0 ;
                sscanf(pszCmd, "%s%x", stderr, &nDestAddr) ;
                if ((nDestAddr & 0x80000000) || (nDestAddr < 0x8096))
                {
                        return 0 ;
                }
                ParseGoTo(nDestAddr) ;
                // �ָ����еĶϵ�
                SetAllBreakPoint() ;
        }

        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : ParseBCommand
*  �������� : ����B����
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ܴ�����1�����򷵻�0
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
                // ������еĶϵ�
        case 'c':
        case 'C':
                // ��������еĶϵ�֮ǰ��Ҫ�����е�����д��ȥ
                RemoveAllBreakPoint() ;
                return 1 ;

                // ��int3�ϵ�
        case 'p':
        case 'P':
                if (2 == GetParamCount(pszCmd))
                {
                        unsigned int nAddr = 0 ;
                        sscanf(pszCmd, "%s%x", stderr, &nAddr) ;
                        if (1 == AddSoftBreakPoint(nAddr))
                        {
                                printf("��Ӷϵ�ɹ�\r\n") ;
                        }
                        else
                        {
                                printf("��Ӷϵ�ʧ��\r\n") ;
                        }
                }
                else
                {
                        printf("^Error\r\n") ;
                }
                return 1 ;
                // ��ʾ���еĶϵ�
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
                        // ����Ҫ�ģ��Ժ���ܻ����overflow������
                        sscanf(pszCmd, "%s%x%x%s", stderr, &nAddr, &nLen, szPurview) ;
                        
                        // �жϵ�ַ�Ƿ���Ч���߶ϵ��ַ����
                        if(0 != BreakPointIsHave(nAddr))
                        {
                                printf("�ϵ��ַ��Ч�����Ѿ�����!\r\n") ;
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
                                                printf("���Ӳ���ϵ�ɹ�!\r\n") ;
                                        }
                                        else
                                        {
                                                printf("���Ӳ���ϵ�ʧ��!\r\n") ;
                                        }
                                }
                        }
                        else
                        {
                                printf("���Ӳ���ϵ�ɹ�!\r\n") ;
                        }
                }
                else
                {
                        printf("��������\r\n,����������:��ַ ���� Ȩ��\r\n") ;
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
                                printf("����ϵ�ɹ�\r\n") ;
                        }
                        else
                        {
                                printf("����ϵ�ʧ��\r\n") ;
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
                                printf("�ϵ�ʧЧ�ɹ�\r\n") ;
                        }
                        else
                        {
                                printf("�ϵ�ʧЧʧ��\r\n") ;
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
                                // ���¶ϵ��ǲ�����������
                                if (m_BreakPointList.Find(nAddr))
                                {
                                        if (m_BreakPointList.Delete(nAddr))
                                        {
                                                printf("�Ƴ��ϵ�ɹ�\r\n") ;
                                        }
                                        else
                                        {
                                                printf("�ϵ���ʧЧ�����޷�ɾ��\r\n") ;
                                        }

                                }
                                else
                                {
                                      int nBreakId = GetDrNumberOfAddress((LPVOID)nAddr) ;
                                      if (RemoveDrRegister(nBreakId))
                                      {
                                              printf("�Ƴ�Ӳ���ϵ�ɹ�\r\n") ;
                                      }
                                      else
                                      {
                                              printf("�Ƴ�Ӳ���ϵ�ʧ��\r\n") ;
                                      }

                                }
                        }
                        else
                        {
                                printf("�Ƴ��ϵ�ʧЧ\r\n") ;
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
                               printf("�����ɶ?\r\n") ;
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
                               printf("��������Բ��԰�! ����Ϊ:Write, Read\r\n") ;
                               break ;
                       }

                       if(AppendMemoryBreak(nAddr, nLen, nPurview))
                       {
                               printf("����ڴ�ϵ�ɹ�!\r\n") ;

                               //DisplayAdjacencyInformation() ;
                       }
                       else
                       {
                               printf("����ڴ�ϵ�ʧЧ������ʧЧԭ��!\r\n") ;
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
                                printf("�Ƴ��ڴ�ϵ�ɹ�!\r\n") ;
                        }
                        else
                        {
                                printf("�Ƴ��ڴ�ϵ�ʧ��!\r\n") ;
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
                                printf("�Ҳ�����Ӳ���ϵ�!\r\n") ;
                        }
                        else
                        {
                                if (0 != RemoveDrRegister(nId))
                                {
                                        printf("ɾ��Ӳ���ϵ�ɹ�!\r\n") ;
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
*  �� �� �� : DispCopyright
*  �������� : �����Ȩ��Ϣ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : 
*
*******************************************************************************/
int Debug::DispCopyright(void)
{
        printf("\r\n") ;
        printf("                              Tiny Debugger\r\n") ;
        printf("                                            V0.1 ��\r\n") ;
        printf("               ��Ȩ���� 2010-2010 The Tiny Debugger Project\r\n") ;
        printf("                               �� ��: �人�������ѧԱ ����\r\n\r\n") ;
        printf("            ��Ա�������������߽��飬������ϵ����: evilknight.1@163.com\r\n") ;
        printf("\r\n") ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : SetAllBreakPoint
*  �������� : �������еĶϵ�
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : �������öϵ�ĸ���
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
                                OutputDebugString("SetAllBreakPoint VirtualProtectEx����!\r\n") ;
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
*  �� �� �� : RevertAllBreakPoint
*  �������� : ȡ�����еĶϵ�
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : �������öϵ�ĸ���
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
                                OutputDebugString("RevertAllBreakPoint VirtualProtectEx����!\r\n") ;
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
                        OutputDebugString("RevertAllBreakPoint VirtualProtectEx����!\r\n") ;
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
*  �� �� �� : AddSoftBreakPoint
*  �������� : ����һ������ϵ�
*  �����б� : nAddr     --  �ϵ�ĵ�ַ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1,ʧ�ܷ���0
*
*******************************************************************************/
int Debug::AddSoftBreakPoint(IN unsigned int nAddr)
{
        // �����ж�һ�µ�ַ�Ƿ�����Ч���ڴ��ҳ����

        if (0 != BreakPointIsHave((unsigned int)nAddr))
        {
                printf("�ϵ��ַ��Ч���߶ϵ��ַ�Ѿ�����!\r\n") ;
                return 0 ;
        }

        char ch = 0 ;
        DWORD dwNumberOfBytesWritten = 0 ;

        // ���ж���û�пյ�dr�Ĵ�������ʹ��
        // ���ԵĻ�ֱ���ã����Ч��
        // m_bIsTmpDrBreak
        int nDrNum = GetFreeDrRegister() ;
        if (0 != nDrNum)
        {
                if(0 == SetDrBreakPoint(nDrNum, nAddr, 1, 0) )
                {
                        OutputDebugString("��Ӷϵ����!\r\n") ;
                        return 0 ;
                }
                m_nIsTmpDrBreak |= (1 << (nDrNum-1)) ;
                return 1 ;
        }
        
        DWORD dwProtect = 0;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)nAddr, BUFFER_MAX, 
                PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("AddSoftBreakPoint VirtualProtectEx����!\r\n") ;
                return 0 ;
                
        }

        // �����ʧ���˵Ļ�������0,��Ϊ����˵���������ǿ��Է��ʱ����Խ���
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
*  �� �� �� : ActiveBreakPoint
*  �������� : ����һ���ϵ�
*  �����б� : nAddr     --  �ϵ�ĵ�ַ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1,ʧ�ܷ���0
*
*******************************************************************************/
int Debug::ActiveBreakPoint(IN unsigned int nAddr)
{
        // �����ж�һ�µ�ַ�Ƿ�����Ч���ڴ��ҳ����
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("��ַ������!\r\n") ;
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

        // ��������ȱȽ�һ����û�мĴ�����ʹ����
        if (0 == (m_UseDrRegister & 0xf))
        {
                return 0 ;
        }
        
        // ���ж��ǲ�����DRx�ϵ���
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
        
        // ���ж�DR0�ĵ�ַ�ǲ�����ͬ
        if (nAddr == context.Dr0)
        {
                // �����ͬ�Ļ�����һ��L0λ
                context.Dr7 |= 0x1 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        if (nAddr == context.Dr1)
        {
                // �����ͬ�Ļ�����һ��L1λ
                context.Dr7 |= 0x4 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        
        if (nAddr == context.Dr2)
        {
                // �����ͬ�Ļ�����һ��L1λ
                context.Dr7 |= 0x10 ;
                if (FALSE == SetCurrentThreadContext(&context))
                {
                        return 0 ;
                }
                return 1 ;
        }
        
        if (nAddr == context.Dr3)
        {
                // �����ͬ�Ļ�����һ��L1λ
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
*  �� �� �� : GrayBreakPoint
*  �������� : ʹһ���ϵ�ʧЧ
*  �����б� : nAddr     --    �ϵ�ĵ�ַ
*             isTmp     --    һ��ʧЧ��������ʧЧ  
*                             ���ֵΪFALSE�Ļ�һ��ʧЧ��ֵΪTRUE�Ļ�Ϊ����ʧЧ
*                             ֻ��Ӳ���ϵ���Ӱ��
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1,ʧ�ܷ���0
*
*******************************************************************************/
int Debug::GrayBreakPoint(IN unsigned int nAddr, IN BOOL isTmp)
{
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("��ַ������!\r\n") ;
                return 0 ;
        }

        if (NULL == m_hProcess)
        {
                return 0 ;
        }

        Node * p = NULL ;
        p = m_BreakPointList.Find(nAddr) ;
        DWORD dwProtect = 0 ;
        // �����������
        if (NULL != p)
        {
                DWORD dwNumberOfBytesWritten = 0 ;
                char ch = p->byOldValue ;
                if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)p->nAddr, BUFFER_MAX, 
                                PAGE_EXECUTE_READWRITE,&dwProtect) )
                {
                        OutputDebugString("GrayBreakPoint VirtualProtectEx����!\r\n") ;
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
        // ��������ȱȽ�һ����û�мĴ�����ʹ����
        if (0 == (m_UseDrRegister & 0xf))
        {
                return 0 ;
        }

        // ���ж��ǲ�����DRx�ϵ���
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
        // ���ж�DR0�ĵ�ַ�ǲ�����ͬ
        if (nAddr == context.Dr0)
        {
                // �����ͬ�Ļ�����һ��L0λ
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
                // �����ͬ�Ļ�����һ��L1λ
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
                // �����ͬ�Ļ�����һ��L2λ
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
                // �����ͬ�Ļ�����һ��L3λ
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
*  �� �� �� : GetFreeDrRegister
*  �������� : ȡ�ÿ��е�DR�Ĵ���
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : �ɹ����ؼĴ������(��1��ʼ),ʧ�ܷ���0
*
*******************************************************************************/
int Debug::GetFreeDrRegister(void)
{
        // �ж���û�пյļĴ�����
        if (0xf == (m_UseDrRegister & 0xf))
        {
                return 0 ;
        }

        //�еĻ�����һ��һ�����жϣ������ĸ�û��ʹ��
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
*  �� �� �� : SetFreeDrRegister
*  �������� : ��DRx�Ĵ�����Ϊ����
*  �����б� : nNum      --      DR�Ĵ������
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::SetFreeDrRegister(IN int nDrID)
{
        if (nDrID < 1 || nDrID > 4)
        {
                OutputDebugString("SetFreeDrRegister ��������!\r\n") ;
                return 0 ;
        }
        m_UseDrRegister &= ~(1 << (nDrID-1)) ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : SetDrBreakPoint
*  �������� : ����DRx�ϵ�
*  �����б� : nNum      --      DR�Ĵ������
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::SetDrBreakPoint(IN int nDrID, IN unsigned int nAddr, IN int nLen, IN int nPurview)
{
        if (nDrID < 1 || nDrID > 4)
        {
                return 0 ;
        }

        // �����ж�һ�µ�ַ�Ƿ�����Ч���ڴ��ҳ����
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("��ַ������!\r\n") ;
                return 0 ;
        }

        if (1 != nLen && 2 != nLen && 4 != nLen)
        {
                OutputDebugString("SetDrBreakPoint ���Ȳ���") ;
                return 0 ;
        }

        if ((0 != nPurview) && (1 != nPurview) && (3 != nPurview))
        {
                OutputDebugString("SetDrBreakPoint Ȩ�޲���") ;
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

        // ���ϵ��ַ�Ž���Ӧ��dr�Ĵ�����
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

        
        // ���Ĵ�����Ϊʹ��
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
*  �� �� �� : DispDrBreakPoint
*  �������� : ��ʾӲ���ϵ���Ϣ
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void Debug::DispDrBreakPoint(void)
{
        // ���ж���û��dr�Ĵ�����ʹ��
        if (0 == (m_UseDrRegister & 0xf))
        {
                printf("��Ӳ���ϵ���Ϣ\r\n") ;
                return ;
        }
        else
        {
                printf("Ӳ���ϵ���Ϣ\r\n") ;
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

        //����еĻ����������ж�dr0-4
        // ���ж�dr0��û�и�ʹ��
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
                printf(" State:%s\r\n",dr7.DRFlag.L0?"��Ч":"ʧЧ") ;
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
                printf(" State:%s\r\n",dr7.DRFlag.L1?"��Ч":"ʧЧ") ;
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
                printf(" State:%s\r\n",dr7.DRFlag.L2?"��Ч":"ʧЧ") ;
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
                printf(" State:%s\r\n",dr7.DRFlag.L3?"��Ч":"ʧЧ") ;
        }
        return  ;
}

/*******************************************************************************
*
*  �� �� �� : GetCurrentThreadContext
*  �������� : ȡ�õ�ǰ�߳�������
*  �����б� : pContext    --  ָ��CONTEXT�ṹ
*  ˵    �� : 
*  ���ؽ�� : �������FALSE�Ļ�����,�ɹ�����TRUE
*
*******************************************************************************/
BOOL Debug::GetCurrentThreadContext(OUT CONTEXT *pContext)
{
        if (NULL == pContext)
        {
                OutputDebugString("Context����ָ��Ϊ��!\r\n") ;
                return FALSE ; 
        }

        if (0 == m_DebugEvent.dwThreadId)
        {
                OutputDebugString("�߳�id��Ч\r\n") ;
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
*  �� �� �� : SetCurrentThreadContext
*  �������� : ���õ�ǰ�߳�������
*  �����б� : pContext    --  ָ��CONTEXT�ṹ
*  ˵    �� : 
*  ���ؽ�� : �������FALSE�Ļ�����,�ɹ�����TRUE
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
*  �� �� �� : ParseSingleSetp
*  �������� : ������
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ������Դ�����DBG_CONTINUE�����ܴ�����DBG_EXCEPTION_NOT_HANDLED
*
*******************************************************************************/
int Debug::ParseSingleSetp(void)
{
        BOOL bIsParse = FALSE ; // �Ƿ���Դ���
        BOOL bIsDrBreak = FALSE ;

        CONTEXT context ;
        unsigned int nAddr = 0 ;
        DWORD dwProtect = 0 ;

        context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS ;

        GetCurrentThreadContext(&context) ;

        // �ж�һ���ǲ�����ϵ���Ҫ�ָ�
        if (0 != m_uSoftBreakResume)
        {
                ActiveBreakPoint(m_uSoftBreakResume) ;
                m_uSoftBreakResume = 0 ;
                bIsParse = TRUE ;
        }

        // �ж�һ���ǲ���Ӳ���ж���Ҫ�ָ�
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

        // ���ж�һ���ǲ�����ΪӲ���ϵ�������쳣
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
                


                // �����Ϸ��Ϣ�Լ������û�����
                if(m_uDRBreakResume)
                {
                        PrintContext() ;
                        PrintInstruction(context.Eip, FALSE, 1) ;
                        printf("Ӳ���ж�!\r\n") ;
                        ParseRequest() ;
                }
                /*
                else
                {
                        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)context.Eip, BUFFER_MAX, 
                                                PAGE_EXECUTE_READWRITE,&dwProtect) )
                        {
                                OutputDebugString("ParseSingleSetp VirtualProtectEx����!\r\n") ;
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

        // �ж�һ���ǲ����ڴ�����Ҫ�ָ�
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
                printf("�����ж�!\r\n") ;
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
*  �� �� �� : AppendMemoryBreak
*  �������� : ����ڴ�ϵ�
*  �����б� : nAddr     ---   �ڴ�ϵ��ַ
              nLen      ---   ����
              dwPurview ---   ��ִ��ʲô����ʱ����(1:�� 2:д) 
*  ˵    �� : 
*  ���ؽ�� : ������Դ�����DBG_CONTINUE�����ܴ�����DBG_EXCEPTION_NOT_HANDLED
*
*******************************************************************************/
int Debug::AppendMemoryBreak(IN LPVOID nAddr, IN SIZE_T nLen, IN DWORD dwPurview)
{
        if (NULL == m_hProcess)
        {
                return 0 ;
        }
        // ������
        if (NULL == nAddr)
        {
                return 0 ;
        }

        // ��������Ĵ��룬�����в����ظ�
        if (0 != BreakPointIsHave((unsigned int)nAddr))
        {
                printf("�ϵ��ַ��Ч���߶ϵ��ַ�Ѿ�����!\r\n") ;
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

        // ���nAddr�ǲ��Ǵ�����Ч��ҳ��
        BOOL bIsEffective = FALSE ;
        int nPageID = 0 ;

        MEMORY_BASIC_INFORMATION mbi = {0} ;

        if (0 == IsEffectiveAddress(nAddr, &mbi))
        {
                printf("�ڴ��ַ��Ч!\r\n") ;
                return 0 ;
        }

        nPageID = InsertEffectiveMemoryPage(nAddr, &mbi) ;

        if (0 == nPageID)
        {
                OutputDebugString("�Ҳ����Ǹ��ڴ��ҳ!\r\n") ;
                return 0 ;
        }
      
        PMemNode p = m_MemList.Find(&mbi) ;
                
        int nBreakID = 0 ;
        
        // �ж�����ϵ��Ƿ��Ѿ�����
        if (m_MemBreak.HaveIn(nAddr, nBreakID))
        {
                OutputDebugString("����ڴ�ϵ��Ѿ�����\r\n") ;
                printf("����ڴ�ϵ��ַ�Ѿ�������\r\n") ;
                return 0 ;
        }

        // �ж�����ڴ��ҳ�Ƿ�����������
        if (! IsHavememoryProtect(p->Protect, dwPurview))
        {
#ifdef _DEBUG
                printf("����ڴ�ҳû���������") ;
#endif
                return 0 ;
        }
        
        // ��������ڴ�ҳ��Ч�Ļ������ϵ���Ϣ���뵽�ڴ�ϵ����
        nBreakID = m_MemBreak.Insert(nAddr, nLen, dwPurview) ;
        if (0 == nBreakID)
        {
                return 0 ;
        }


        // ���ﴦ���ڴ��Ӧ�ϵ㡢�ϵ��Ӧ�ڴ�Ĺ�ϵ
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
                                printf("�޸��ڴ��ҳ����ʧ��!\r\n") ;
                                return 0 ;
                        }
                }
                p->newProtect = newProtect ;
                
                nEndAddress = (DWORD)p->BaseAddress + p->RegionSize ;
                // �ж���û�п�ҳ
                // �����һ���ڴ�ҳ�Ѿ������ڵĻ���������ͬʱ�޸�һ�³���
                if (((DWORD)nAddr + nLen) > nEndAddress 
                        && m_MemList.HaveIn((LPVOID)(nEndAddress), nPageID)
                        && IsHavememoryProtect(p->newProtect, dwPurview))
                {
                        nPageID = InsertEffectiveMemoryPage((LPVOID)(nEndAddress + 1), &mbi) ;
                        
                        if (0 == nPageID)
                        {
                                OutputDebugString("�Ҳ����Ǹ��ڴ��ҳ!\r\n") ;
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
        
        // �޸��ڴ�ϵ�ĳ���
        PMemBreakNode pBreak = m_MemBreak.FindById(nBreakID) ;
        if (NULL != pBreak)
        {
                pBreak->BreakLen = nLen;
        }

        return 1 ;
}


/*******************************************************************************
*
*  �� �� �� : RemoveMemoryProtect
*  �������� : �Ƴ��ڴ�����
*  �����б� : nProtect    --  �ڴ�ԭ����
              Attrib      --  Ҫ�Ƴ�������
              1: ������
              2: д����
              4: ִ������
*  ˵    �� : ���������������
*  ���ؽ�� : �����µ�����,�������0�Ļ�˵������
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

        // ���ñ�־λ
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
        /* ��������
        0x1: ������
        0x2: д����
        0x4: ִ������
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

        // ȥ������
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

        // ���µĽ������
        // �������ִ�����ԺͿ������ԵĻ�
        if (bIsExecte && bIsCopy && bIsWrite && !bIsRead)
        {
                newProtect = PAGE_EXECUTE_WRITECOPY ;
        }

        // ������ж�����
        else if (!bIsExecte && !bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_READONLY ;
        }

        // ������ж�д����
        else if (!bIsExecte && !bIsCopy && bIsWrite && bIsRead)
        {
                newProtect = PAGE_READWRITE ;
        }

        // �������д�Ϳ���
        else if (!bIsExecte && bIsCopy && bIsWrite && bIsRead)
        {
                newProtect = PAGE_WRITECOPY ;
        }

        // �������ִ�����ԺͿ������ԵĻ�
        else if (bIsExecte && !bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE_READ ;
        }

        // �������ִ�����ԺͿ������ԵĻ�
        else if (bIsExecte && !bIsCopy && bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE_READWRITE ;
        }

        // �������ִ�����ԺͿ������ԵĻ�
        else if (bIsExecte && bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE_WRITECOPY ;
        }

        // �������ִ�����ԺͿ������ԵĻ�
        else if (bIsExecte && !bIsCopy && !bIsWrite && bIsRead)
        {
                newProtect = PAGE_EXECUTE ;
        }

        // ���ֻ��ִ�����ԵĻ�
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
*  �� �� �� : IsHavememoryProtect
*  �������� : �ж��ڴ��������Ƿ���ָ��������
*  �����б� : nProtect    --  �ڴ�ԭ����
              Attrib      --  Ҫ��������
              1: ������
              2: д����
              4: ִ������
*  ˵    �� : ���������������
*  ���ؽ�� : ����ɹ�����1,���򷵻�0
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

        // ���ñ�־λ
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
        /* ��������
        0x1: ������
        0x2: д����
        0x4: ִ������
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
*  �� �� �� : RemoveMemoryBreak
*  �������� : �Ƴ��ڴ�ϵ�
*  �����б� : pAddr  --  �ϵ��ַ
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::RemoveMemoryBreak(IN LPVOID pAddr)
{
        if (NULL == pAddr)
        {
                OutputDebugString("Ŀ���ַΪ��!\r\n") ;
                return  0 ;
        }

        int nBreakID = 0 ;
        DWORD dwProtect = 0 ;
        if( 0 == m_MemBreak.HaveIn(pAddr, nBreakID))
        {
                OutputDebugString("�Ҳ���Ŀ��ϵ���Ϣ!\r\n") ;
                return 0 ;
        }
        // ȡ���ڴ�ҳID
        int nPageID = 0 ;
        if (0 == m_MemList.HaveIn(pAddr, nPageID))
        {
                return 0 ;
        }

        int nPageCount = m_BreakMapPage[nBreakID].GetSize() ;

        for (int i = 0; i < nPageCount; ++i)
        {
                // ����ǰ�ϵ���Ǹ��ڴ�ҳ���Ƴ�
                m_PageMapBreak[ m_BreakMapPage[nBreakID][i]->data ].Delete(nBreakID) ;
                // �����¼����ڴ�ҳ������
                RepairMemoryProtect(m_BreakMapPage[nBreakID][i]->data) ;
                // ���������ڴ�ҳ������
                PMemNode p = m_MemList.FindById(m_BreakMapPage[nBreakID][i]->data) ;
                if (NULL != p)
                {
                        // ԭ����,���������Ƴ��ϵ������ȷ��������
                        // VirtualProtectEx(m_hProcess, p->BaseAddress, BUFFER_MAX, p->newProtect, &dwProtect) ;
                        VirtualProtectEx(m_hProcess, p->BaseAddress, BUFFER_MAX, p->newProtect, &dwProtect) ;

                        // �ж������ַ��û�жϵ��ˣ����û���ˣ��´β����ٻָ��ڴ�������
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
*  �� �� �� : RepairMemoryProtect
*  �������� : ��ԭ�ڴ�ϵ������ڴ�ҳ����
*  �����б� : nPageID  --  �ڴ��ҳID
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::RepairMemoryProtect(IN int nPageID)
{
        if (0 == nPageID)
        {
                OutputDebugString("�ڴ�ҳ�Ų���Ϊ0\r\n") ; 
                return 0 ;
        }

        PMemNode p = m_MemList.FindById(nPageID) ;
        if (NULL == p)
        {
                OutputDebugString("�Ҳ�����ҳ�ŵ���Ч�ڴ��!\r\n") ;
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
*  �� �� �� : HaveInBreakArea
*  �������� : �жϵ�ַ�Ƿ��ڶϵ��ַ֮��
*  �����б� : nAddrm     --  �ϵ��ַ
              pBreakNode --  �ϵ���Ϣ�ṹָ��
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::HaveInBreakArea(IN int nAddrm, IN PMemBreakNode pBreakNode)
{
        if (NULL == pBreakNode)
        {
                OutputDebugString("ָ��Ϊ��!\r\n") ;
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
*  �� �� �� : DisplayAdjacencyInformation
*  �������� : ����ϵ����ڴ�ҳ�໥�Ĺ�ϵ
*  �����б� : 
*  ˵    �� : �����õ�
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::DisplayAdjacencyInformation(void)
{
        int i = 0 ;
        int nCount = m_PageMapBreak.GetCapacity() ;
        printf("����ڴ��Ӧ�ϵ���Ϣ:\r\n") ;

        for (i = 0; i < nCount; ++i)
        {
                printf("%2d:", i) ;
                for(int j = 0; j < m_PageMapBreak[i].GetSize() ; ++j)
                {
                        printf("%3d", m_PageMapBreak[i][j]->data) ;
                }
                printf("\r\n");
        }

        printf("����ϵ��Ӧ�ڴ���Ϣ:\r\n") ;
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

        printf("����ϵ���Ϣ!\r\n") ;
        m_MemBreak.Display() ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : GetDrNumberOfAddress
*  �������� : ͨ���ϵ��ַ�õ�ʹ��DR�Ĵ��������
*  �����б� : IN pAddr
*  ˵    �� : 
*  ���ؽ�� : ����ɹ��������(��1��ʼ), ʧ�ܷ���0
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
*  �� �� �� : RemoveDrRegister
*  �������� : �Ƴ�Ӳ���ϵ�
*  �����б� : IN nDrID:         Dr�Ĵ������
*  ˵    �� : 
*  ���ؽ�� : ����ɹ��������(��1��ʼ), ʧ�ܷ���0
*
*******************************************************************************/
int Debug::RemoveDrRegister(IN int nDrID)
{
        if (nDrID < 1 || nDrID > 4)
        {
                OutputDebugString("Dr�Ĵ�������Ų���!\r\n") ;
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

        // ���İ�drxҲȫ���˰�
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
        
        // ���Ĵ�����Ϊδʹ��
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
*  �� �� �� : ParseDCommand
*  �������� : ����D����
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::ParseDCommand(IN char *pszCmd)
{
        if(NULL == pszCmd)
        {
                OutputDebugString("����ָ��Ϊ��!\r\n") ;
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
*  �� �� �� : DisplayDestProcessMemory
*  �������� : ��ʾ���Խ���Ŀ���ڴ�����
*  �����б� : IN pAddr:         Ŀ���ڴ��ַ
              IN nLen:          ��ȡ����
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::DisplayDestProcessMemory(IN LPVOID pAddr, IN int nLen)
{
        int nPageID = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;

        // �жϵ�ַ�Ƿ����
        if (0 == IsEffectiveAddress(pAddr, &mbi))
        {
                OutputDebugString("�ڴ��ַ��Ч!\r\n") ;
                return 0 ;
        }

        char *pBuf = new char[nLen+sizeof(char)] ;

        if (NULL == pBuf)
        {
                OutputDebugString("�����ڴ�ʧЧ!\r\n") ;
                return 0 ;
        }

        if (nLen <= 0)
        {
                OutputDebugString("���ȳ���!\r\n") ;
                return 0 ;
        }

        if (nLen > (int)pAddr)
        {
                nLen -= (int)pAddr ;
                ++nLen ;
        }

        DWORD dwProtect = 0 ;

        // ��ֹ�����ڴ�ϵ㣬Ŀ���ڴ�ҳû�ж������ԣ��Ƚ��������Լ���ȥ
        if(FALSE == VirtualProtectEx(m_hProcess, pAddr, BUFFER_MAX, 
                                        PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("DisplayDestProcessMemory VirtualProtectEx����!\r\n") ;
                return 0 ;
                
        }

        DWORD dwNothing = 0 ;

        if(FALSE == ReadProcessMemory(m_hProcess, pAddr, pBuf, 
                sizeof(char)*nLen,&dwNothing))
        {
                OutputDebugString("��Ŀ����̳���!\r\n") ;
                return 0 ;
        }
        // �����Ի�ԭ
        VirtualProtectEx(m_hProcess, pAddr, BUFFER_MAX, 
                                dwProtect,&dwProtect) ;

        /*
        // ��ӡ�����ݣ����Ǿ����㷨��̫��
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
                // ���ǰ��ĵ�ַ
                if (0 == (i & 0xf))
                {
                        printf("%p  ", pAddr) ;
                        pAddr = (LPVOID)((DWORD)pAddr + 0x10) ;
                }

                // �������16����
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

                // �������0x10���Ļ�������
                for(int k = 0x10 - nIndex; k >= 0; --k)
                {
                        printf("   ") ;
                }

                for(; nCount < i; ++nCount)
                {
                        char ch = pBuf[nCount] ;
                        // isgraph �Ƿ��ǿ���ʾ�ַ�
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
*  �� �� �� : ParseFunNameOfRegisterValue
*  �������� : �жϼĴ�����ֵ�Ƿ���api��ַ,ͬʱȡ��dll���ͺ�����
*  �����б� : IN  da        --      t_diasm�ṹ��
              OUT pszDllName--      �����洢dll���ֵ�ָ��
              OUT pszFunName--      �����洢�������ֵ�ָ��
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
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
*  �� �� �� : PrintCommandHelp
*  �������� : ���������Ϣ
*  �����б� : IN ch        ---            ��ʾ������İ���
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void Debug::PrintCommandHelp(IN char ch)
{
        if ('b' == ch || 0 == ch)
        {
                printf("ba (���һ��Ӳ���ϵ�)            �ϵ��ַ ���� Ȩ��\r\n") ;
                printf("bp (int3�ϵ�)                    �ϵ��ַ\r\n") ;
                printf("bl (��ʾ���жϵ���Ϣ)\r\n") ;
                printf("bc (�������int3��Ӳ���ϵ���Ϣ)\r\n") ;
                printf("be (����һ���ϵ�)                �ϵ��ַ\r\n") ;
                printf("bd (����һ���ϵ�)                �ϵ��ַ\r\n") ;
                printf("br (�Ƴ�һ��int3�ϵ��Ӳ���ϵ�)  �ϵ��ַ\r\n") ;
                printf("bm (���һ���ڴ�ϵ�)            �ϵ��ַ ���� Ȩ��\r\n") ;
                printf("by (�Ƴ�һ���ڴ�ϵ�)            �ϵ��ַ\r\n") ;
        }

        if (0 == ch)
        {
                printf("t                                ��������\r\n") ;
                printf("p                                ��������\r\n") ;
                printf("r                                �鿴�޸ļĴ���\r\n") ;
                printf("u [Ŀ���ַ]                     �����\r\n") ;
                printf("? ��  h                          �鿴����\r\n") ;
                printf("g [Ŀ���ַ]                     ִ�е�Ŀ���ַ��\r\n") ;
                printf("\t�������ָ����ַ���м�Ķϵ㽫ȫ��ʧЧ\r\n") ;
                printf("l                                ��ʾPE��Ϣ\r\n") ;
                printf("d [Ŀ����ʼ��ַ] [Ŀ����ַ��ַ]/[����] �鿴�ڴ�\r\n") ;
                printf("e [Ŀ����ʼ��ַ]                 �޸��ڴ�\r\n") ;
                printf("q                                �˳�\r\n") ;
                printf("s ��¼��Χ��ʼ��ַ ��¼��Χ��ֹ��ַ [�����ļ���]\r\n") ;
                printf("o [�ű�·����]                   ���нű�\r\n") ;

                printf("��չ����:\r\n") ;
                printf(".kill                            ���������Խ���\r\n") ;
                printf(".restart                         ���¼��ر����Խ���(����)\r\n") ;
                printf(".show                            ��ʾ�Ѽ���ģ��\r\n") ;
        }
        printf("\r\n") ;
        return ;
}

/*******************************************************************************
*
*  �� �� �� : ParseEditMemory
*  �������� : ����༭�ڴ�(�൱��debug��e����)
*  �����б� : IN pAddr        ---           �޸�Ŀ���ڴ��ַ
*  ˵    �� : 
*  ���ؽ�� : ��
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
                OutputDebugString("ParseEditMemory VirtualProtectEx����!\r\n") ;
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
                // �ж��ǲ��ǳ�����󳤶��ˣ��ǵĻ���д��Ŀ�����
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

                // �ж��ǲ���16���Ƶ���
                if (isxdigit(szBuf[0]) && isxdigit(szBuf[1]))
                {
                        // �ǵĻ��Ž�szDestBuf��ȥ
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
*  �� �� �� : OnceDrBreakToSoftBreak
*  �������� : ����Ӳ���ϵ��������ϵ����Ϣת������ϵ�洢
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : �������Ӳ���ϵ�ʵ�����жϵĻ������ؿ��õ�Dr��ţ����򷵻�0
*
*******************************************************************************/
int Debug::OnceDrBreakToSoftBreak(void)
{
        // ���û�еĻ�������0
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
*  �� �� �� : EditRegisterValue
*  �������� : �༭�Ĵ�����ֵ
*  �����б� : IN pszCmd:        �����ַ���
*  ˵    �� : 
*  ���ؽ�� : 
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
*  �������û��ʹ�ã�ԭ���ж�call jmpʱ�ĺ���
*
*  �� �� �� : ParseSCommand
*  �������� : ����̬���ٵĹ���
*  �����б� : IN pszCmd:        �����ַ���
*  ˵    �� : ���ڲ����ļ�鲻�ϸ�  
*  ���ؽ�� : 
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
                        OutputDebugString("�����ڴ������!\r\n") ;
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
                                OutputDebugString("�����ڴ������!\r\n") ;
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
                
                // ���������飬���ĸ��Ǵ���ļ�����
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
                                OutputDebugString("�����ڴ������!\r\n") ;
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
                                OutputDebugString("�����ڴ������!\r\n") ;
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
*  �� �� �� : InstructionRecord
*  �������� : ָ���¼
*  �����б� : ��
*  ˵    �� : ����ָ���¼����
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int Debug::InstructionRecord(void)
{
        DWORD dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        DWORD dwImageBase = m_PEInformation.GetImageBase() ;
        
        
        // ���ڴ�ҳ�����Լ�ǿ
        DWORD dwProtect = 0 ;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, 
                PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("InstructionRecord VirtualProtectEx����!\r\n") ;
                return 0 ;
                
        }

        char szBuf[BUFFER_MAX] = {0} ;
        DWORD dwNumberOfBytes  =  0  ;
        
        // ��ȥ��ָ��
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                sizeof(char) * BUFFER_MAX, &dwNumberOfBytes) )
        {
                OutputDebugString("��Ŀ������ڴ����!\r\n") ;
                return 0;
        }
        
        // �����ж��ǲ���call,����call�Ļ�ֱ����TF��־
        // �����õ���
        t_disasm da ;
        int nInstrLen = Disasm(&szBuf[0], 20, dwEip, &da,DISASM_CODE) ;
        
        char szDllName[MAXBYTE] = {0} ;
        char szFunName[MAXBYTE] = {0} ;

        // �������call
        unsigned int nJmp = m_DllExportFun.AddressToFunName(da, szDllName, szFunName) ;
        
        if (0x70 == da.cmdtype && 0 == nJmp)
        {
                nJmp = ParseFunNameOfRegisterValue(da, szDllName, szFunName) ;
        }
        
        VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, dwProtect, &dwProtect) ;
        
        // ���ж������������Ƿ����
        AberrTreeNode * p = m_InstructionRecord.FindEx(dwEip, szBuf, nInstrLen) ;
        if (NULL == p)
        {
                // ����¼�ӵ����У����Ҵ�ӡ����
                p = m_InstructionRecord.Insert(dwEip, szBuf, nInstrLen) ;
                if (NULL != p)
                {
                        p->jmpconst = (PVOID)nJmp ;
                }
                else
                {
                        printf("ָ�����ʧ��!\r\n") ;
                }
        }
        else
        {
                ++p->nCount ;
        }

        // ������Ը����������
        printf("%p: %s", dwEip, da.result) ;
        if (nJmp)
        {
                printf(" <%s::%s>", szDllName, szFunName) ;
        }
        printf("\r\n") ;
        return 1 ;
        /*
        // ԭ���ӵ�һ��һֱ��¼��ȥ�����call api�Ļ�������
        DWORD dwEip = GetCurrentEip(m_DebugEvent.dwThreadId) ;
        DWORD dwImageBase = m_PEInformation.GetImageBase() ;

        // ��Ϊ�п��ܳ������½��뵽api���棬�����ǰeip�����û��ռ��ڣ�����֮
        if (dwImageBase != (dwEip & dwImageBase) && FALSE == m_bIsIntoAPI)
        {
                SetTFRegister() ;
                return 1 ;
        }

        if (0 == dwEip)
        {
                return 0 ;
        }

        // ���ڴ�ҳ�����Լ�ǿ
        DWORD dwProtect = 0 ;
        if(FALSE == VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, 
                PAGE_EXECUTE_READWRITE,&dwProtect) )
        {
                OutputDebugString("InstructionRecord VirtualProtectEx����!\r\n") ;
                return 0 ;
                
        }

        char szBuf[BUFFER_MAX] = {0} ;
        DWORD dwNumberOfBytes = 0 ;

        // ��ȥ��ָ��
        if (FALSE == ReadProcessMemory(m_hProcess, (PVOID)dwEip, szBuf, 
                sizeof(char) * BUFFER_MAX, &dwNumberOfBytes) )
        {
                return 0;
        }

        // �����ж��ǲ���call,����call�Ļ�ֱ����TF��־
        t_disasm da ;
        int nInstrLen = Disasm(&szBuf[0], 20, dwEip, &da,DISASM_CODE) ;
        // �������call
        char szDllName[MAXBYTE] = {0} ;
        char szFunName[MAXBYTE] = {0} ;
        unsigned int nJmp = m_DllExportFun.AddressToFunName(da, szDllName, szFunName) ;

        if (0x70 == da.cmdtype && 0 == nJmp)
        {
                nJmp = ParseFunNameOfRegisterValue(da, szDllName, szFunName) ;
        }
        
        if (nJmp)
        {
                // ���call API�Ļ�������һ��ָ���CC
                // ������G����
                ParseGoTo(dwEip + nInstrLen) ; 
        }
        else
        {
                //m_nIsTCommand = TRUE ;
                SetTFRegister() ;
        }


        VirtualProtectEx(m_hProcess, (LPVOID)dwEip, BUFFER_MAX, dwProtect, &dwProtect) ;

        // ����¼�ӵ����У����Ҵ�ӡ����
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
*  �� �� �� : WriteRecordToFile
*  �������� : д���ļ�
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : 
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
                OutputDebugString("���ļ�������!\r\n") ;
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
                
                // ָ���ܵĴ���
                sprintf(szDestBuf, "%4d ", p->nCount) ;

                // ָ���ַ
                sprintf(szBuf, "%p ", p->nAddress) ;
                strcat(szDestBuf, szBuf) ;

                // �������
                sprintf(szBuf, "%s ", da.result) ;
                strcat(szDestBuf, szBuf) ;  
                
                // �ж��ǲ��ǵ���api,�ǵĻ��ٽ�dll���ͺ�����һ�����
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
*  �� �� �� : ParseExternCommand
*  �������� : ������չ����
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : 
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
*  �� �� �� : Restart
*  �������� : �������������Գ��򣬲�����bug
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : 
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
        // ������Ӳ���ϵ�
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
*  �� �� �� : Kill
*  �������� : ɱ�������Գ���
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : 
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
*  �� �� �� : DisplayLoadDllName
*  �������� : ��ʾ��ǰ�����Խ����Ѿ����ص�Dll����
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : 
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
*  �� �� �� : IsEffectiveAddress
*  �������� : �ж��Ƿ�����Ч��ַ
*  �����б� : IN lpAddr    --      �ڴ��ַ
              IN pMbi      --      �����洢�ڴ��ҳ��Ϣ
*  ˵    �� : 
*  ���ؽ�� : �����ַ������Ч�ڴ��ҳ�Ļ�������1,���򷵻�0
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
*  �� �� �� : InsertEffectiveMemoryPage
*  �������� : ����һ����Ч���ڴ��ҳ
*  �����б� : IN lpAddr    --      �ڴ��ַ
              IN pMbi      --      �����洢�ڴ��ҳ��Ϣ
*  ˵    �� : 
*  ���ؽ�� : �������ɹ���������Ч���ڴ�ҳID,���򷵻�0
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
                OutputDebugString("��ַ������!\r\n") ;
                return 0 ;
        }
        
        PMemNode p = NULL ;
        
        // ���ж������ҳ�Ƿ����
        if( 0 == m_MemList.IsHaving(pMbi))
        {
                nPageID = m_MemList.Insert(pMbi) ;
                p = m_MemList.Find(pMbi) ;
                if (NULL == p)
                {
                        OutputDebugString("�Ҳ����Ǹ���ҳ����ҳ!\r\n") ;
                        return 0 ;
                }
        }
        else
        {
                p = m_MemList.Find(pMbi) ;
                if (NULL == p)
                {
                        OutputDebugString("�Ҳ����Ǹ���ҳ����ҳ!\r\n") ;
                        return 0 ;
                }
                nPageID = p->nID ;
        }
        return nPageID ;
}

/*******************************************************************************
*
*  �� �� �� : RemoveAllBreakPoint
*  �������� : �Ƴ����е�int3�ϵ��Ӳ���ϵ�
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ����1
*
*******************************************************************************/
int Debug::RemoveAllBreakPoint()
{
        RevertAllBreakPoint() ;
        m_BreakPointList.Clean() ;
        // ������Ӳ���ϵ�
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
*  �� �� �� : BreakPointIsHave
*  �������� : �ϵ��ַ�Ƿ����
*  �����б� : IN nAddr:  ��ֵַ
*  ˵    �� : 
*  ���ؽ�� : �����ַ��Ч������-1,��������ڷ���0,������ڷ���1
*
*******************************************************************************/
int Debug::BreakPointIsHave(IN unsigned int nAddr)
{
        int nPage = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;
        if (0 == IsEffectiveAddress((LPVOID)nAddr, &mbi))
        {
                OutputDebugString("��ַ������!\r\n") ;
                return -1 ;
        }
        
        if (NULL == m_hProcess)
        {
                return -1 ;
        }
        
        // �����ж�����ϵ��Ƿ��Ѿ�������
        if (NULL != m_BreakPointList.Find(nAddr))
        {
                OutputDebugString("�ϵ��Ѿ�������!\r\n") ;
                return 1 ;
        }
        
        // ���ж��Ƿ����Ӳ���ϵ���
        if (0 != GetDrNumberOfAddress((LPVOID)nAddr))
        {
                OutputDebugString("�ϵ��Ѿ�����Ӳ���ϵ�����!\r\n") ;
                return 1 ;
        }
        
        // ���ж��ڲ����ڴ�ϵ���
        if (0 != m_MemBreak.HaveIn((LPVOID)nAddr, nPage))
        {
                OutputDebugString("�ϵ��Ѿ������ڴ�ϵ�����!\r\n") ;
                return 1 ;
        }
        return 0 ;
}
