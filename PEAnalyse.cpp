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
*  函 数 名 : PEAnalyse
*  功能描述 : 构造函数--将成员函数初始化
*  参数列表 : 
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
PEAnalyse::PEAnalyse():m_isPE(0),m_hFile(INVALID_HANDLE_VALUE),m_pSectionHeader(NULL),
                        m_lpFile(NULL),m_hMap(NULL)
{

}


/*******************************************************************************
*
*  函 数 名 : ~PEAnalyse
*  功能描述 : 析构函数--资源的释放
*  参数列表 : 
*  说    明 : 
*  返回结果 : 
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
*  函 数 名 : CloseFileHandle
*  功能描述 : 关闭打开的文件句柄，包括文件内存映像句柄
*  参数列表 : 
*  说    明 : 
*  返回结果 : 
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
*  函 数 名 : Analyse
*  功能描述 : PE文件分析
*  参数列表 : 文件全路径
*  说    明 : 同时可以通过函数的返回值判断是否为正确的PE文件
*  返回结果 : 如果函数成功，返回1, 如果函数出错，则返回0,
*
*******************************************************************************/
int PEAnalyse::Analyse(char *pPath)
{
        m_isPE = 0 ;

        if (NULL == pPath)
        {
                OutputDebugString("文件指定的路径不对") ;
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
                OutputDebugString("文件打开出错") ;
                return 0 ;
        }

        m_hMap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, 0, NULL) ;

        if (NULL == m_hMap)
        {
                CloseFileHandle() ;
                OutputDebugString("CreateFileMapping出错") ;
                return 0 ;
        }

        m_lpFile = MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0) ;

        if (NULL == m_lpFile)
        {
                CloseFileHandle() ;
                OutputDebugString("CreateFileMapping出错") ;
                return 0 ;
        }

        //DWORD dwNumber ; 用文件时的遗留代码
        /****************************************************************************
        *
        * 因调试器需要，将原本的由文件读取改为直接从文件内存映射里读
        *
        *
        ****************************************************************************/
//         if (FALSE == ReadFile(m_hFile, (LPVOID)&m_DosHeader, sizeof(IMAGE_DOS_HEADER), 
//                                                 &dwNumber, NULL))
//         {
//                 CloseFileHandle() ;
//                 OutputDebugString("读文件出错") ;
//                 return 0 ;
//         }

        // 对于内存这类函数，没有判断有效性，一般不会出错
        // 直接读内存
        memcpy((LPVOID)&m_DosHeader, m_lpFile, sizeof(IMAGE_DOS_HEADER)) ;

        // 判断DOS头是不是MZ
        if (IMAGE_DOS_SIGNATURE != m_DosHeader.e_magic)
        {
                CloseFileHandle() ;
                OutputDebugString("不是一个有效PE文件,DOS头不对!\r\n") ;
                return 0 ;
        }

        // 判断e_lfanew高位，一般的文件高位一般为0,否则会引起程序的非法访问
        if (m_DosHeader.e_lfanew & 0xffff0000)
        {
                CloseFileHandle() ;
                OutputDebugString("不是一个有效PE文件,指向不正确的nt头") ;
                return 0 ;
        }

        /* 通过文件指针访问
        // 通过DOS头的e_lfanew值去读NT头
        SetFilePointer(m_hFile, m_DosHeader.e_lfanew, 0, FILE_BEGIN) ;


        if (FALSE == ReadFile(m_hFile, (PVOID)&m_NtHeaders, 
                        sizeof(IMAGE_NT_HEADERS), &dwNumber, NULL))
        {
                CloseFileHandle() ;
              #ifdef _DEBUG
                OutputDebugString("读NT头出错") ;
              #endif
                return 0 ;
        }
        */

        // 读取NT头
        memcpy( (PVOID)&m_NtHeaders, 
                (PVOID) ((int)m_lpFile + m_DosHeader.e_lfanew),
                sizeof(IMAGE_NT_HEADERS)) ;


        // 判断PE标志
        if (IMAGE_NT_SIGNATURE != m_NtHeaders.Signature)
        {
                CloseFileHandle() ;
                OutputDebugString("不是一个有效PE文件,NT头不对") ;
                return 0 ;
        }
       
        if (NULL != m_pSectionHeader)
        {
                delete [] m_pSectionHeader ;
                m_pSectionHeader = NULL ;
        }
        
        // 取得节的数量
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
                        OutputDebugString("申请节信息结构体出错") ;
                        CloseFileHandle() ;
                        return 0 ;
                }

                // 取得节的偏移量
                // 选项头的大小以sizeOfOptionHeader为主，否则会有bug
                // 一般的编译器不会有bug,但是人家手动写的之类的就不能正常处理了
                int nFileOffset =       m_DosHeader.e_lfanew 
                                      + m_NtHeaders.FileHeader.SizeOfOptionalHeader 
                                      + sizeof(IMAGE_FILE_HEADER) + sizeof(DWORD);

                /*
                if (NULL == SetFilePointer(m_hFile, nFileOffset, NULL, FILE_BEGIN))
                {
                        OutputDebugString("移动文件指针出错") ;
                        CloseFileHandle() ;
                        return 0 ;
                }

                if (FALSE == ReadFile(m_hFile, (LPVOID)m_pSectionHeader, 
                               nSectionOfNumber * sizeof(IMAGE_SECTION_HEADER),
                                &dwNumber, NULL) )
                {
                        OutputDebugString("读取节信息出错了") ;
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
*  函 数 名 : IsPE
*  功能描述 : 返回文件的结果
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果是PE文件返回1，否则返回0
*
*******************************************************************************/
int PEAnalyse::IsPE()
{
        return m_isPE ;
}

/*******************************************************************************
*
*  函 数 名 : GetEntryPoint
*  功能描述 : 取得OEP地址
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果打开的是PE文件，则返回OEP地址，否则返回0
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
*  函 数 名 : GetImageBase
*  功能描述 : 取得ImageBase
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果打开的是PE文件，则返回ImageBase，否则返回0
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
*  函 数 名 : GetSizeOfImage
*  功能描述 : 取得镜像大小
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果打开的是PE文件，则返回SizeOfImage，否则返回0
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
*  函 数 名 : GetBaseOfCode
*  功能描述 : 
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果打开的是PE文件，则返回BaseOfCod，否则返回0
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
*  函 数 名 : GetSectionHeaderPtr
*  功能描述 : 返回节信息的Buffer
*  参数列表 : 
*  说    明 : 配合NumberOfSection一起使用
*  返回结果 : 如果打开的是pe文件，则返回保存节信息的地址，否则返回0
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
*  函 数 名 : GetMapToFilePointer
*  功能描述 : 返回文件在内存中的起始地址
*  参数列表 : 
*  说    明 :
*  返回结果 : 如果打开的是pe文件，则返回文件在内存中的起始地址
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
*  函 数 名 : DispPEInformation
*  功能描述 : 输出PE基本信息
*  参数列表 : 
*  说    明 :
*  返回结果 : 0
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

        // 输出节信息
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
