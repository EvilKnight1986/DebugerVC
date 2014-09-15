// DllExportFun.cpp: implementation of the DllExportFun class.
//
//////////////////////////////////////////////////////////////////////

#include "DllExportFun.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DllExportFun::DllExportFun():m_nDllCount(0),m_hProcess(NULL)
{

}

DllExportFun::~DllExportFun()
{

}

/*******************************************************************************
*
*  函 数 名 : InsertDll
*  功能描述 : 分析dll的导出表
*  参数列表 : 
*  说    明 : 如果已经存在，重新设置dll状态
*  返回结果 : 如果成功返回1,不能处理返回0
*
*******************************************************************************/
int DllExportFun::InsertDll(HMODULE  hModule)
{
        if (NULL == hModule || INVALID_HANDLE_VALUE == hModule)
        {
                return 0 ;
        }

        if (NULL == m_hProcess || INVALID_HANDLE_VALUE == m_hProcess)
        {
                return 0 ;
        }

        DWORD dwDllBase = (unsigned long)hModule ;
        DWORD dwNumberOfByte = 0;
        /* 原来的代码
        IMAGE_DOS_HEADER *pdosHeader = (IMAGE_DOS_HEADER*)dwDllBase;
        */
        IMAGE_DOS_HEADER DosHeader = {0} ;
        if(FALSE == ReadProcessMemory(m_hProcess, (LPVOID)dwDllBase, &DosHeader,
                                sizeof(IMAGE_DOS_HEADER), &dwNumberOfByte))
        {
                OutputDebugString("读DOS头出错了!\r\n") ;
                return 0 ;
        }

        // NT头的位置
        DWORD dwNTHeaderOffset = dwDllBase + DosHeader.e_lfanew ;

        /*原来的代码
        IMAGE_NT_HEADERS *pNtHeaders = (IMAGE_NT_HEADERS*)(dwDllBase + pdosHeader->e_lfanew);
        */

        IMAGE_NT_HEADERS NtHeaders = {0} ;

        if (FALSE == ReadProcessMemory( m_hProcess,
                                        (LPVOID)dwNTHeaderOffset,
                                        &NtHeaders,
                                        sizeof(IMAGE_NT_HEADERS),
                                        &dwNumberOfByte) )
        {
                OutputDebugString("读NT头出错了!\r\n") ;
                return 0 ;
        }

        // 这里要判断一下有没有导出表
        // 导出表是在第一项
        if (NtHeaders.OptionalHeader.NumberOfRvaAndSizes < 1)
        {
                OutputDebugString("没有导出表!\r\n") ;
                return 0 ;
        }

        DWORD ExportDirectoryOffset = (DWORD)(NtHeaders.OptionalHeader.DataDirectory[0].VirtualAddress) ;

        if (0 == ExportDirectoryOffset)
        {
                OutputDebugString("导出表VirtualAddress错误了吧!\r\n") ;
                return 0 ;
        }

        // 算出绝对地址
        ExportDirectoryOffset += dwDllBase ;

        IMAGE_EXPORT_DIRECTORY  ExportDirectory = {0} ;

        if (FALSE == ReadProcessMemory( m_hProcess, 
                                        (LPVOID)ExportDirectoryOffset,
                                        &ExportDirectory, 
                                        sizeof(IMAGE_EXPORT_DIRECTORY),
                                        &dwNumberOfByte))
        {
                OutputDebugString("读导出表出错!\r\n") ;
                return 0 ;
        }

        int NumberOfNames = ExportDirectory.NumberOfNames ;

        char *pDllName = (char *)(ExportDirectory.Name) ;

        pDllName = (char *)((int)pDllName + dwDllBase) ;
        char szDllName[MAXBYTE] = {0} ;
        DWORD dwNothing = 0 ;

        if(FALSE == ReadProcessMemory(m_hProcess, 
                        (LPVOID)pDllName, 
                        szDllName, 
                        sizeof(char) * MAXBYTE, 
                        &dwNothing) 
                        )
        {
                OutputDebugString("读取Dll名出错了!\r\n") ;
                return 0 ;
        }

        // 将dll在链表的位置保存起来
        position p = m_DllNameList.Find(szDllName) ;
        if (NULL != p)
        {
                p->SetLoad() ;
                return 1 ;
        }

        int posDll = m_DllNameList.Insert(szDllName, TRUE) ;

        /*
        // 指向名字数组
        int *pNameAry = (int *)(ExportDirectory.AddressOfNames + (int)hModule) ;
        // 指向函数地址数组
        int *pFunAddrAry = (int *)(ExportDirectory.AddressOfFunctions + (int)hModule) ;
        // 指向序号数组
        short *pOrdinals = (short *)(ExportDirectory.AddressOfNameOrdinals + (int)hModule) ;
        */

        // 指向名字数组
        int NameAryOffset = (int)(ExportDirectory.AddressOfNames + dwDllBase) ;
        // 指向函数地址数组
        int FunAddrAryOffset = (int)(ExportDirectory.AddressOfFunctions + dwDllBase) ;
        // 指向序号数组
        short OrdinalsAryOffset = (short)(ExportDirectory.AddressOfNameOrdinals + dwDllBase) ;
        

        for (int i = 0; i < NumberOfNames; ++i)
        {
                // 取出函数名
                //int nFunName = (pNameAry[i] + (int)hModule) ;
                int nFunNameOffset = 0 ;
                ReadProcessMemory(m_hProcess, (LPVOID)(NameAryOffset + (i*4)), 
                             &nFunNameOffset, sizeof(int), &dwNumberOfByte) ;
                nFunNameOffset += dwDllBase ;
                // 取出函数的地址
                //int nFunAddr = (pFunAddrAry[i] + (int)hModule) ;

                int nFunAddrOffset = 0 ;
                ReadProcessMemory(m_hProcess, (LPVOID)(FunAddrAryOffset + (i*4)), 
                             &nFunAddrOffset, sizeof(int), &dwNumberOfByte) ;
                nFunAddrOffset += dwDllBase ;
                m_rbt.Insert(nFunAddrOffset, nFunNameOffset, posDll) ;

                // 以下代码是供测试用的 
#ifdef _MICROQ
                char szBuffer[MAXBYTE] = {0} ;
                ReadProcessMemory(m_hProcess, (LPVOID)nFunNameOffset, szBuffer,
                                sizeof(char)*MAXBYTE, &dwNumberOfByte) ;
                printf("DllName: %s  FunName: %s ", szDllName, szBuffer) ;
                printf("FunAddress: %p\r\n",nFunAddrOffset) ;
#endif
        }

        ++m_nDllCount ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : RemoveDll
*  功能描述 : UnloadDll
*  参数列表 : 
*  说    明 : 移除没有加载这么严格，只是单纯的置下标志位，api信息并没有在树中删除
*  返回结果 : 如果成功返回1,不能处理返回0
*
*******************************************************************************/
int DllExportFun::RemoveDll(HMODULE  hModule)
{
        if (NULL == hModule || INVALID_HANDLE_VALUE == hModule)
        {
                return 0 ;
        }

        DWORD dwDllBase = (unsigned long)hModule ;

        IMAGE_DOS_HEADER *pdosHeader = (IMAGE_DOS_HEADER*)dwDllBase;
        
        DWORD dwNTHeaderOffset = dwDllBase + pdosHeader->e_lfanew ;

        IMAGE_NT_HEADERS *pNtHeaders = (IMAGE_NT_HEADERS*)(dwDllBase + pdosHeader->e_lfanew) ;
        IMAGE_EXPORT_DIRECTORY *pExportDirectory = (IMAGE_EXPORT_DIRECTORY *)(dwDllBase \
                + pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);

        int NumberOfNames = pExportDirectory->NumberOfNames ;

        char *pDllName = (char *)(dwDllBase + pExportDirectory->Name) ;

        position p = m_DllNameList.Find(pDllName) ;

        if (NULL != p)
        {
                p->SetUnLoad() ;
        }
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : GetFunName
*  功能描述 : 取得dll名字和函数名(dll名字是存在调试器进程类中的，
*                                       函数名传出的是调试进程中一个地址值)
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回1, 失败返回0
*
*******************************************************************************/
int DllExportFun::GetFunName(IN unsigned int _nAddr, OUT char *&pDllName, OUT char *&pFunName)
{
        TreeNode *p = m_rbt.Find(_nAddr) ;

        if (NULL == p)
        {
                return 0 ;
        }

        // 判断dll有没有移除
        if ( FALSE == m_DllNameList.GetNode(p->nBelongDll)->GetState())
        {
                return 0 ;
        }
        pDllName = m_DllNameList.GetNode(p->nBelongDll)->GetDllName() ;
        pFunName = (char *)p->pFunName ;

        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetProcessValue
*  功能描述 : 设置被调戏程序的进程句柄
*  参数列表 : handle    ---  被调戏程序句柄
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
void DllExportFun::SetProcessValue(HANDLE handle)
{
        if (INVALID_HANDLE_VALUE == handle)
        {
                return ;
        }
        m_hProcess = handle ;
        return ;
}

/*******************************************************************************
*
*  函 数 名 : AddressToFunName
*  功能描述 : 地址转换成函数名(这里存在地址的解析)
*  参数列表 : da                ---     t_disasm结构
              pszDllName        ---     用来存储dll名字
              pszFunName        ---     用来存储函数名字
*  说    明 : 
*  返回结果 : 成功返回api的地址, 失败返回0
*
*******************************************************************************/
int DllExportFun::AddressToFunName(t_disasm &da, char *pszDllName, char *pszFunName)
{
        // 判断是不是call
        if ((0x70 != da.cmdtype && 0x40 != da.cmdtype) && 0 == da.cmdtype)
        {
                return 0 ;
        }

        if (NULL == m_hProcess || INVALID_HANDLE_VALUE == m_hProcess)
        {
                return 0 ;
        }

        int      pAddrFun = 0 ;
        DWORD    dwRet    = 0 ;
        char    *pDllName = NULL ;
        char    *pFunName = NULL ;
        int     *p        = NULL ;

        if (4 == da.memtype)
        {
                p = (int *)da.adrconst ;
                if (NULL == p)
                {
                        return 0 ;
                }

                // 读调用地址
                if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)p, &pAddrFun, 
                                                sizeof(int), &dwRet) )
                {
                        OutputDebugString("读取调用地址出错了!\r\n") ;
                        return 0 ;
                }
#ifdef _MICROQ
                printf("%p\r\n", pAddrFun) ;
#endif
        }
        else
        {
                p = (int *)(da.jmpconst) ;
                if (NULL == p)
                {
                        return 0 ;
                }
                p = (int *)(int(p) + 2) ;

                // 读调用地址
                if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)p, 
                                                &pAddrFun, sizeof(int), &dwRet) )
                {
                        OutputDebugString("读取目标函数名") ;
                        return 0 ;
                }

                if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)pAddrFun, 
                                        &pAddrFun, sizeof(int), &dwRet))
                {
                        OutputDebugString("读取调用地址出错了!\r\n") ;
                        return 0 ;
                }
#ifdef _MICROQ
                printf("%p\r\n", pAddrFun) ;
#endif
        }

        if (1 == GetFunName(pAddrFun, pDllName, pFunName))
        
        {
                char *szBuf[MAXBYTE] = {0} ;

                DWORD dwNothing = 0 ;
                if (FALSE == ReadProcessMemory(m_hProcess, (LPVOID)pFunName, szBuf, 
                                        sizeof(char) * MAXBYTE, &dwNothing))
                {
                        OutputDebugString("读取API函数名出错!\r\n") ;
                        return 0 ;
                }

                // 外面要保证buffer的长度为MAXBYTE
                if (NULL != pszDllName)
                {
                        memcpy(pszDllName, pDllName, sizeof(char)*MAXBYTE) ;
                }
                if (NULL != pszFunName)
                {
                        memcpy(pszFunName, szBuf, sizeof(char)*MAXBYTE) ;
                }
                return pAddrFun ;
        }
        return 0 ;
}

int DllExportFun::DisplayLoadDllName()
{
       return m_DllNameList.DisplayLoadDllName() ;
}
