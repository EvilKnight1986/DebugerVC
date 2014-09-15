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
*  �� �� �� : InsertDll
*  �������� : ����dll�ĵ�����
*  �����б� : 
*  ˵    �� : ����Ѿ����ڣ���������dll״̬
*  ���ؽ�� : ����ɹ�����1,���ܴ�����0
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
        /* ԭ���Ĵ���
        IMAGE_DOS_HEADER *pdosHeader = (IMAGE_DOS_HEADER*)dwDllBase;
        */
        IMAGE_DOS_HEADER DosHeader = {0} ;
        if(FALSE == ReadProcessMemory(m_hProcess, (LPVOID)dwDllBase, &DosHeader,
                                sizeof(IMAGE_DOS_HEADER), &dwNumberOfByte))
        {
                OutputDebugString("��DOSͷ������!\r\n") ;
                return 0 ;
        }

        // NTͷ��λ��
        DWORD dwNTHeaderOffset = dwDllBase + DosHeader.e_lfanew ;

        /*ԭ���Ĵ���
        IMAGE_NT_HEADERS *pNtHeaders = (IMAGE_NT_HEADERS*)(dwDllBase + pdosHeader->e_lfanew);
        */

        IMAGE_NT_HEADERS NtHeaders = {0} ;

        if (FALSE == ReadProcessMemory( m_hProcess,
                                        (LPVOID)dwNTHeaderOffset,
                                        &NtHeaders,
                                        sizeof(IMAGE_NT_HEADERS),
                                        &dwNumberOfByte) )
        {
                OutputDebugString("��NTͷ������!\r\n") ;
                return 0 ;
        }

        // ����Ҫ�ж�һ����û�е�����
        // ���������ڵ�һ��
        if (NtHeaders.OptionalHeader.NumberOfRvaAndSizes < 1)
        {
                OutputDebugString("û�е�����!\r\n") ;
                return 0 ;
        }

        DWORD ExportDirectoryOffset = (DWORD)(NtHeaders.OptionalHeader.DataDirectory[0].VirtualAddress) ;

        if (0 == ExportDirectoryOffset)
        {
                OutputDebugString("������VirtualAddress�����˰�!\r\n") ;
                return 0 ;
        }

        // ������Ե�ַ
        ExportDirectoryOffset += dwDllBase ;

        IMAGE_EXPORT_DIRECTORY  ExportDirectory = {0} ;

        if (FALSE == ReadProcessMemory( m_hProcess, 
                                        (LPVOID)ExportDirectoryOffset,
                                        &ExportDirectory, 
                                        sizeof(IMAGE_EXPORT_DIRECTORY),
                                        &dwNumberOfByte))
        {
                OutputDebugString("�����������!\r\n") ;
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
                OutputDebugString("��ȡDll��������!\r\n") ;
                return 0 ;
        }

        // ��dll�������λ�ñ�������
        position p = m_DllNameList.Find(szDllName) ;
        if (NULL != p)
        {
                p->SetLoad() ;
                return 1 ;
        }

        int posDll = m_DllNameList.Insert(szDllName, TRUE) ;

        /*
        // ָ����������
        int *pNameAry = (int *)(ExportDirectory.AddressOfNames + (int)hModule) ;
        // ָ������ַ����
        int *pFunAddrAry = (int *)(ExportDirectory.AddressOfFunctions + (int)hModule) ;
        // ָ���������
        short *pOrdinals = (short *)(ExportDirectory.AddressOfNameOrdinals + (int)hModule) ;
        */

        // ָ����������
        int NameAryOffset = (int)(ExportDirectory.AddressOfNames + dwDllBase) ;
        // ָ������ַ����
        int FunAddrAryOffset = (int)(ExportDirectory.AddressOfFunctions + dwDllBase) ;
        // ָ���������
        short OrdinalsAryOffset = (short)(ExportDirectory.AddressOfNameOrdinals + dwDllBase) ;
        

        for (int i = 0; i < NumberOfNames; ++i)
        {
                // ȡ��������
                //int nFunName = (pNameAry[i] + (int)hModule) ;
                int nFunNameOffset = 0 ;
                ReadProcessMemory(m_hProcess, (LPVOID)(NameAryOffset + (i*4)), 
                             &nFunNameOffset, sizeof(int), &dwNumberOfByte) ;
                nFunNameOffset += dwDllBase ;
                // ȡ�������ĵ�ַ
                //int nFunAddr = (pFunAddrAry[i] + (int)hModule) ;

                int nFunAddrOffset = 0 ;
                ReadProcessMemory(m_hProcess, (LPVOID)(FunAddrAryOffset + (i*4)), 
                             &nFunAddrOffset, sizeof(int), &dwNumberOfByte) ;
                nFunAddrOffset += dwDllBase ;
                m_rbt.Insert(nFunAddrOffset, nFunNameOffset, posDll) ;

                // ���´����ǹ������õ� 
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
*  �� �� �� : RemoveDll
*  �������� : UnloadDll
*  �����б� : 
*  ˵    �� : �Ƴ�û�м�����ô�ϸ�ֻ�ǵ��������±�־λ��api��Ϣ��û��������ɾ��
*  ���ؽ�� : ����ɹ�����1,���ܴ�����0
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
*  �� �� �� : GetFunName
*  �������� : ȡ��dll���ֺͺ�����(dll�����Ǵ��ڵ������������еģ�
*                                       �������������ǵ��Խ�����һ����ֵַ)
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int DllExportFun::GetFunName(IN unsigned int _nAddr, OUT char *&pDllName, OUT char *&pFunName)
{
        TreeNode *p = m_rbt.Find(_nAddr) ;

        if (NULL == p)
        {
                return 0 ;
        }

        // �ж�dll��û���Ƴ�
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
*  �� �� �� : SetProcessValue
*  �������� : ���ñ���Ϸ����Ľ��̾��
*  �����б� : handle    ---  ����Ϸ������
*  ˵    �� : 
*  ���ؽ�� : 
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
*  �� �� �� : AddressToFunName
*  �������� : ��ַת���ɺ�����(������ڵ�ַ�Ľ���)
*  �����б� : da                ---     t_disasm�ṹ
              pszDllName        ---     �����洢dll����
              pszFunName        ---     �����洢��������
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����api�ĵ�ַ, ʧ�ܷ���0
*
*******************************************************************************/
int DllExportFun::AddressToFunName(t_disasm &da, char *pszDllName, char *pszFunName)
{
        // �ж��ǲ���call
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

                // �����õ�ַ
                if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)p, &pAddrFun, 
                                                sizeof(int), &dwRet) )
                {
                        OutputDebugString("��ȡ���õ�ַ������!\r\n") ;
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

                // �����õ�ַ
                if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)p, 
                                                &pAddrFun, sizeof(int), &dwRet) )
                {
                        OutputDebugString("��ȡĿ�꺯����") ;
                        return 0 ;
                }

                if(FALSE == ReadProcessMemory(m_hProcess, (PVOID)pAddrFun, 
                                        &pAddrFun, sizeof(int), &dwRet))
                {
                        OutputDebugString("��ȡ���õ�ַ������!\r\n") ;
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
                        OutputDebugString("��ȡAPI����������!\r\n") ;
                        return 0 ;
                }

                // ����Ҫ��֤buffer�ĳ���ΪMAXBYTE
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
