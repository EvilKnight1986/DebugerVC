// PEAnalyse.cpp: implementation of the PEAnalyse class.
//
//////////////////////////////////////////////////////////////////////

#include "PEAnalyse.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
*  �� �� �� : PEAnalyse
*  �������� : ���캯��--����Ա������ʼ��
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : 
*
*******************************************************************************/
PEAnalyse::PEAnalyse():m_isPE(0),m_hFile(INVALID_HANDLE_VALUE),m_pSectionHeader(NULL),
                        m_lpFile(NULL),m_hMap(NULL)
{

}


/*******************************************************************************
*
*  �� �� �� : ~PEAnalyse
*  �������� : ��������--��Դ���ͷ�
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : 
*
*******************************************************************************/
PEAnalyse::~PEAnalyse()
{
        CloseFileHandle() ;
        if (NULL != m_pSectionHeader)
        {
                delete [] m_pSectionHeader ;
                m_pSectionHeader = NULL ;
        }
}

/*******************************************************************************
*
*  �� �� �� : CloseFileHandle
*  �������� : �رմ򿪵��ļ�����������ļ��ڴ�ӳ����
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : 
*
*******************************************************************************/
void PEAnalyse::CloseFileHandle()
{
        if (NULL != m_hMap)
        {
               UnmapViewOfFile(m_hMap) ; 
               m_hMap = NULL ;
        }

        if (NULL != m_hMap)
        {
                CloseHandle(m_hMap) ;
                m_hMap = NULL ;
        }
        if (INVALID_HANDLE_VALUE != m_hFile)
        {
                CloseHandle(m_hFile) ;
                m_hFile = INVALID_HANDLE_VALUE ;
        }
}

/*******************************************************************************
*
*  �� �� �� : Analyse
*  �������� : PE�ļ�����
*  �����б� : �ļ�ȫ·��
*  ˵    �� : ͬʱ����ͨ�������ķ���ֵ�ж��Ƿ�Ϊ��ȷ��PE�ļ�
*  ���ؽ�� : ��������ɹ�������1, ������������򷵻�0,
*
*******************************************************************************/
int PEAnalyse::Analyse(char *pPath)
{
        m_isPE = 0 ;

        if (NULL == pPath)
        {
                OutputDebugString("�ļ�ָ����·������") ;
                return 0 ;
        }

        m_hFile = CreateFile(pPath, GENERIC_ALL, 
                             FILE_SHARE_WRITE | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             FALSE,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL) ;

        if (INVALID_HANDLE_VALUE == m_hFile)
        {
                OutputDebugString("�ļ��򿪳���") ;
                return 0 ;
        }

        m_hMap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, 0, NULL) ;

        if (NULL == m_hMap)
        {
                CloseFileHandle() ;
                OutputDebugString("CreateFileMapping����") ;
                return 0 ;
        }

        m_lpFile = MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0) ;

        if (NULL == m_lpFile)
        {
                CloseFileHandle() ;
                OutputDebugString("CreateFileMapping����") ;
                return 0 ;
        }

        //DWORD dwNumber ; ���ļ�ʱ����������
        /****************************************************************************
        *
        * ���������Ҫ����ԭ�������ļ���ȡ��Ϊֱ�Ӵ��ļ��ڴ�ӳ�����
        *
        *
        ****************************************************************************/
//         if (FALSE == ReadFile(m_hFile, (LPVOID)&m_DosHeader, sizeof(IMAGE_DOS_HEADER), 
//                                                 &dwNumber, NULL))
//         {
//                 CloseFileHandle() ;
//                 OutputDebugString("���ļ�����") ;
//                 return 0 ;
//         }

        // �����ڴ����ຯ����û���ж���Ч�ԣ�һ�㲻�����
        // ֱ�Ӷ��ڴ�
        memcpy((LPVOID)&m_DosHeader, m_lpFile, sizeof(IMAGE_DOS_HEADER)) ;

        // �ж�DOSͷ�ǲ���MZ
        if (IMAGE_DOS_SIGNATURE != m_DosHeader.e_magic)
        {
                CloseFileHandle() ;
                OutputDebugString("����һ����ЧPE�ļ�,DOSͷ����!\r\n") ;
                return 0 ;
        }

        // �ж�e_lfanew��λ��һ����ļ���λһ��Ϊ0,������������ķǷ�����
        if (m_DosHeader.e_lfanew & 0xffff0000)
        {
                CloseFileHandle() ;
                OutputDebugString("����һ����ЧPE�ļ�,ָ����ȷ��ntͷ") ;
                return 0 ;
        }

        /* ͨ���ļ�ָ�����
        // ͨ��DOSͷ��e_lfanewֵȥ��NTͷ
        SetFilePointer(m_hFile, m_DosHeader.e_lfanew, 0, FILE_BEGIN) ;


        if (FALSE == ReadFile(m_hFile, (PVOID)&m_NtHeaders, 
                        sizeof(IMAGE_NT_HEADERS), &dwNumber, NULL))
        {
                CloseFileHandle() ;
              #ifdef _DEBUG
                OutputDebugString("��NTͷ����") ;
              #endif
                return 0 ;
        }
        */

        // ��ȡNTͷ
        memcpy( (PVOID)&m_NtHeaders, 
                (PVOID) ((int)m_lpFile + m_DosHeader.e_lfanew),
                sizeof(IMAGE_NT_HEADERS)) ;


        // �ж�PE��־
        if (IMAGE_NT_SIGNATURE != m_NtHeaders.Signature)
        {
                CloseFileHandle() ;
                OutputDebugString("����һ����ЧPE�ļ�,NTͷ����") ;
                return 0 ;
        }
       
        if (NULL != m_pSectionHeader)
        {
                delete [] m_pSectionHeader ;
                m_pSectionHeader = NULL ;
        }
        
        // ȡ�ýڵ�����
        int nSectionOfNumber = m_NtHeaders.FileHeader.NumberOfSections;
        if (NULL != m_pSectionHeader)
        {
                delete [] m_pSectionHeader ;
                m_pSectionHeader = NULL ;
        }

        if (nSectionOfNumber > 0)
        {
                m_pSectionHeader = new IMAGE_SECTION_HEADER[nSectionOfNumber] ;
                if (NULL == m_pSectionHeader)
                {
                        OutputDebugString("�������Ϣ�ṹ�����") ;
                        CloseFileHandle() ;
                        return 0 ;
                }

                // ȡ�ýڵ�ƫ����
                // ѡ��ͷ�Ĵ�С��sizeOfOptionHeaderΪ�����������bug
                // һ��ı�����������bug,�����˼��ֶ�д��֮��ľͲ�������������
                int nFileOffset =       m_DosHeader.e_lfanew 
                                      + m_NtHeaders.FileHeader.SizeOfOptionalHeader 
                                      + sizeof(IMAGE_FILE_HEADER) + sizeof(DWORD);

                /*
                if (NULL == SetFilePointer(m_hFile, nFileOffset, NULL, FILE_BEGIN))
                {
                        OutputDebugString("�ƶ��ļ�ָ�����") ;
                        CloseFileHandle() ;
                        return 0 ;
                }

                if (FALSE == ReadFile(m_hFile, (LPVOID)m_pSectionHeader, 
                               nSectionOfNumber * sizeof(IMAGE_SECTION_HEADER),
                                &dwNumber, NULL) )
                {
                        OutputDebugString("��ȡ����Ϣ������") ;
                        CloseFileHandle() ;
                        return 0 ;
                }
                */
                memcpy((PVOID)m_pSectionHeader, (PVOID)((int)m_lpFile + nFileOffset), 
                                        nSectionOfNumber * sizeof(IMAGE_SECTION_HEADER)) ;
        }

        m_isPE = 1 ;
        CloseFileHandle() ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : IsPE
*  �������� : �����ļ��Ľ��
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : �����PE�ļ�����1�����򷵻�0
*
*******************************************************************************/
int PEAnalyse::IsPE()
{
        return m_isPE ;
}

/*******************************************************************************
*
*  �� �� �� : GetEntryPoint
*  �������� : ȡ��OEP��ַ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����򿪵���PE�ļ����򷵻�OEP��ַ�����򷵻�0
*
*******************************************************************************/
int PEAnalyse::GetEntryPoint()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.AddressOfEntryPoint ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : GetImageBase
*  �������� : ȡ��ImageBase
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����򿪵���PE�ļ����򷵻�ImageBase�����򷵻�0
*
*******************************************************************************/
int PEAnalyse::GetImageBase()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.ImageBase ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : GetSizeOfImage
*  �������� : ȡ�þ����С
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����򿪵���PE�ļ����򷵻�SizeOfImage�����򷵻�0
*
*******************************************************************************/
int PEAnalyse::GetSizeOfImage()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.SizeOfImage ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : GetBaseOfCode
*  �������� : 
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����򿪵���PE�ļ����򷵻�BaseOfCod�����򷵻�0
*
*******************************************************************************/
int PEAnalyse::GetBaseOfCode()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.BaseOfCode ;
        }

        return 0 ;
}

int PEAnalyse::GetBaseOfData()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.BaseOfData ;
        }

        return 0 ;
}

int PEAnalyse::GetSectionAlignment()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.SectionAlignment ;
        }

        return 0 ;
}

int PEAnalyse::GetFileAlignment()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.FileAlignment ;
        }
        
        return 0 ;
}

int PEAnalyse::GetMagic()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.Magic ;
        }
        
        return 0 ;
}

int PEAnalyse::GetSubSystem()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.Subsystem ;
        }
        
        return 0 ;
}

int PEAnalyse::GetNumberOfSection()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.FileHeader.NumberOfSections ;
        }
        
        return 0 ;
}

int PEAnalyse::GetTimeDataStamp()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.FileHeader.TimeDateStamp ;
        }
        
        return 0 ;
}

int PEAnalyse::GetSizeOfHeaders()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.SizeOfHeaders ;
        }
        
        return 0 ;
}

int PEAnalyse::GetCharacteristics()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.FileHeader.Characteristics ;
        }
        
        return 0 ;
}

int PEAnalyse::GetCheckSum()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.CheckSum ;
        }
        
        return 0 ;
}

int PEAnalyse::GetSizeOfOptionalHeader()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.FileHeader.SizeOfOptionalHeader ;
        }
        
        return 0 ;
}

int PEAnalyse::GetNumberOfRvaAndSizes()
{
        if (1 == m_isPE)
        {
                return m_NtHeaders.OptionalHeader.NumberOfRvaAndSizes ;
        }
        
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : GetSectionHeaderPtr
*  �������� : ���ؽ���Ϣ��Buffer
*  �����б� : 
*  ˵    �� : ���NumberOfSectionһ��ʹ��
*  ���ؽ�� : ����򿪵���pe�ļ����򷵻ر������Ϣ�ĵ�ַ�����򷵻�0
*
*******************************************************************************/
IMAGE_SECTION_HEADER * PEAnalyse::GetSectionHeaderPtr()
{
        if (1 == m_isPE)
        {
                return m_pSectionHeader ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : GetMapToFilePointer
*  �������� : �����ļ����ڴ��е���ʼ��ַ
*  �����б� : 
*  ˵    �� :
*  ���ؽ�� : ����򿪵���pe�ļ����򷵻��ļ����ڴ��е���ʼ��ַ
*
*******************************************************************************/
PVOID PEAnalyse::GetMapToFilePointer()
{
        if (1 == m_isPE)
        {
                return m_lpFile ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : DispPEInformation
*  �������� : ���PE������Ϣ
*  �����б� : 
*  ˵    �� :
*  ���ؽ�� : 0
*
*******************************************************************************/
void PEAnalyse::DispPEInformation()
{
        if (0 == m_isPE)
        {
                return ;
        }
        printf("PE Base Information\r\n") ;
        printf("EntryPoint:   %p\t\t   SubSystem:            %p\r\n", 
                                        GetEntryPoint(), GetSubSystem()) ;
        printf("ImageBase:    %p\t\t   NumberOfSection:      %p\r\n", 
                                        GetImageBase(), GetNumberOfSection()) ;
        printf("SizeOfImage:  %p\t\t   SizeOfOptionalHeader: %p\r\n", 
                                        GetSizeOfImage(), GetSizeOfOptionalHeader()) ;
        printf("BaseOfCode:   %p\t\t   SizeOfHeaders:        %p\r\n", 
                                        GetBaseOfCode(), GetSizeOfHeaders()) ;
        printf("BaseOfData:   %p\t\t   Characteristics:      %p\r\n", 
                                        GetBaseOfData(), GetCharacteristics()) ;

        // �������Ϣ
        printf("\r\nSection Information:\r\n") ;
        printf("SectionName VirtualSize   RVA    SizeOfRawData PointerToRawData Characteristics\r\n") ;
        for (int i = 0; i < GetNumberOfSection(); ++i)
        {
                printf("%-8s     ", m_pSectionHeader[i].Name) ;
 
                printf("%p   ", m_pSectionHeader[i].Misc.VirtualSize) ;

                printf("%p   ", m_pSectionHeader[i].VirtualAddress) ;
 
                printf("%p       ",  m_pSectionHeader[i].SizeOfRawData) ;
 
                printf("%p         ", m_pSectionHeader[i].PointerToRawData) ;
                
                printf("%p\r\n", m_pSectionHeader[i].Characteristics) ;
        }
}
