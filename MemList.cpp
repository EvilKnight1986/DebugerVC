// MemList.cpp: implementation of the CMemList class.
//
//////////////////////////////////////////////////////////////////////

#include "MemList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemList::CMemList():m_pHead(NULL),m_nCount(0),m_nID(0)
{

}

CMemList::~CMemList()
{
        RemoveAll() ;
}

int CMemList::RemoveAll()
{
        if (NULL == this)
        {
                return 0 ;
        }
        if (0 == m_nCount && NULL == m_pHead)
        {
                return 0 ;
        }
        
        PMemNode p = m_pHead ;
        PMemNode q = p ;
        while (NULL != p)
        {
                q = p ;
                p = p->pNext ;
                delete q ;
        }
        m_pHead = NULL ;
        m_nCount = 0 ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : Insert
*  功能描述 : 插入节点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果插入成功，返回id,否则返回0
*
*******************************************************************************/
int CMemList::Insert(PMEMORY_BASIC_INFORMATION pMemBaseInfo)
{
        if (NULL == this)
        {
                return 0 ;
        }
        if (NULL == pMemBaseInfo)
        {
                return 0 ;
        }
        
        PMemNode p = new MemNode ;
        if (NULL == p)
        {
                return 0 ;
        }
        memset(p, 0, sizeof(MemNode)) ;
        ++m_nID ;

        // 进行赋值操作
        p->AllocationBase = pMemBaseInfo->AllocationBase ;
        p->AllocationProtect = pMemBaseInfo->AllocationProtect ;
        p->BaseAddress = pMemBaseInfo->BaseAddress ;
        p->Protect = pMemBaseInfo->Protect ;
        p->newProtect = pMemBaseInfo->Protect ;
        p->RegionSize = pMemBaseInfo->RegionSize ;
        p->State = pMemBaseInfo->State ;
        p->Type = pMemBaseInfo->Type ;
        p->nID = m_nID ;

        if (NULL == m_pHead)
        {
                m_pHead = p ;                
        }
        else
        {
                // 先找到尾部
                PMemNode q = m_pHead ;
                while (q->pNext != NULL)
                {
                        q = q->pNext ;
                }
                if (NULL == q)
                {
                        return 0 ;
                }
                q->pNext = p ;
        }
        ++m_nCount ;
        return p->nID  ; 
}

/*******************************************************************************
*
*  函 数 名 : IsHaving
*  功能描述 : 判断有没有重复
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果存在返回1，否则返回0
*
*******************************************************************************/
int CMemList::IsHaving(PMEMORY_BASIC_INFORMATION pMemBaseInfo)
{
        if (NULL == this)
        {
                return 0 ;
        }
        if (pMemBaseInfo == NULL)
        {
                return 0 ;
        }
        if (0 == m_nCount)
        {
                return 0 ;
        }
        PMemNode p = m_pHead;
        while (NULL != p)
        {
                if (pMemBaseInfo->BaseAddress == p->BaseAddress)
                {
                        return 1 ;
                }
                p = p->pNext ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : Remove
*  功能描述 : 删除指定的结点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果存在返回1，否则返回0
*
*******************************************************************************/
int CMemList::Remove(PMEMORY_BASIC_INFORMATION pMemBaseInfo)
{
        if (NULL == this)
        {
                return 0 ;
        }
        PMemNode q = NULL ;
        if (m_pHead->BaseAddress == pMemBaseInfo->BaseAddress
                &&m_pHead->RegionSize == pMemBaseInfo->RegionSize)
        {
                q = m_pHead->pNext ;
                delete m_pHead ;
                m_pHead = q ;
                --m_nCount ;
                return 1 ;
        }
        PMemNode p = FindPrev(pMemBaseInfo) ;
        if (NULL == p)
        {
                return 0 ;
        }
        q = p->pNext ;
        p->pNext = q->pNext ;
        delete q ;
        --m_nCount ;
        return 1 ;
}

int     CMemList::RemoveById(int id)
{
        if (NULL == this)
        {
                return 0 ;
        }

        if (NULL == m_pHead)
        {
                return 0 ;
        }

        PMemNode q = NULL ;
        if (m_pHead->nID == id)
        {
                q = m_pHead->pNext ;
                delete m_pHead ;
                m_pHead = q ;
                --m_nCount ;
                return 1 ;
        }

        PMemNode p = FindPrevById(id) ;

        if (NULL == p)
        {
                return 0 ;
        }
        q = p->pNext ;
        p->pNext = q->pNext ;
        delete q ;
        --m_nCount ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : FindPrev
*  功能描述 : 查找指定节点的前驱
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果存在返回1，否则返回0
*
*******************************************************************************/
PMemNode CMemList::FindPrev(PMEMORY_BASIC_INFORMATION pMemBaseInfo)
{
        if (NULL == this)
        {
                return 0 ;
        }

        if (NULL == m_pHead)
        {
                return NULL ;
        }

        PMemNode p = m_pHead ;
        while (NULL != p->pNext )
        {
                if (p->pNext->BaseAddress == pMemBaseInfo->BaseAddress)
                {
                        return p ;
                }
                p = p->pNext ;
        }
        
        return NULL ;
}

PMemNode CMemList::FindPrevById(int id)
{
        if (NULL == this)
        {
                return 0 ;
        }
        
        if (NULL == m_pHead)
        {
                return NULL ;
        }
        
        PMemNode p = m_pHead ;
        for (;NULL != p->pNext; p = p->pNext)
        {
                if (p->pNext->nID == id)
                {
                        return p ;
                }
        }
        
        return NULL ;      
}


PMemNode CMemList::Find(PMEMORY_BASIC_INFORMATION pMemBaseInfo)
{
        if (NULL == this)
        {
                return NULL ;
        }

        if (NULL == pMemBaseInfo)
        {
                return NULL ;
        }

        if (NULL == m_pHead)
        {
                return NULL ;
        }

        if (m_pHead->BaseAddress == pMemBaseInfo->BaseAddress)
        {
                return m_pHead ;
        }
        
        PMemNode p = FindPrev(pMemBaseInfo) ;
        if (NULL != p)
        {
                return p->pNext ;
        }
        return NULL ;
}


PMemNode CMemList::FindById(int id)
{
        if (NULL == this)
        {
                return NULL ;
        }

        if (NULL == m_pHead)
        {
                return NULL ;
        }

        if (id == m_pHead->nID)
        {
                return m_pHead ;
        }
        
        PMemNode p = FindPrevById(id) ;
        if (NULL != p)
        {
                return p->pNext ;
        }
        return NULL ;
}

void CMemList::Display()
{
        if (NULL == this)
        {
                return ;
        }

        if (0 == m_nCount)
        {
                return ;
        }

        else
        {
                printf("Int3断点信息:\r\n") ;
        }

        PMemNode p = m_pHead ;
        while (NULL != p)
        {
                printf("ID:%d \tBaseAddress:%p \tRegionSize:%p \tProtect:%p\r\n", 
                                p->nID,
                                p->BaseAddress,
                                p->RegionSize, 
                                p->Protect) ;
                p = p->pNext ;
        }
        return ;
}

/*******************************************************************************
*
*  函 数 名 : EnumDestProcessEffectivePage
*  功能描述 : 枚举目标进程有效内存分页
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回1，否则返回0
*
*******************************************************************************/
int CMemList::EnumDestProcessEffectivePage(HANDLE hProcess)
{
        if (NULL == hProcess)
        {
                return 0 ;
        }

        // 先清空链表中的所有结点
        RemoveAll() ;

        LPVOID nAddr = 0 ;
        MEMORY_BASIC_INFORMATION mbi = {0} ;

        while (TRUE)
        {
                if (sizeof(mbi) != VirtualQueryEx(hProcess, nAddr, &mbi, sizeof(mbi)))
                {
                        break ;
                }
                
                if (MEM_COMMIT == mbi.State)
                {
                        Insert(&mbi) ;
                }
                nAddr = (LPVOID)((DWORD)nAddr + mbi.RegionSize) ;
        }
        
        return 1 ;
}

int CMemList::HaveIn(LPVOID nAddr, int &nPage)
{
        nPage = 0 ;
        if (0 == nAddr)
        {
                return 0 ;
        }
        
        if (NULL == m_pHead)
        {
                return 0 ;
        }
        
        PMemNode p = m_pHead ;
        
        while (NULL != p)
        {
                if (nAddr >= p->BaseAddress 
                        && (DWORD)nAddr < (p->RegionSize + (DWORD)p->BaseAddress))
                {
                        nPage = p->nID ;
                        return 1 ;
                }
                p = p->pNext ;
        }
        
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : RevertMemoryProtectAll
*  功能描述 : 恢复内存页的属性
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回1，否则返回0
*
*******************************************************************************/
int CMemList::RevertMemoryProtectAll(HANDLE hProcess)
{
        if (NULL == hProcess || INVALID_HANDLE_VALUE == hProcess)
        {
                return 0 ;
        }

        for (PMemNode p = m_pHead; NULL != p; p = p->pNext)
        {
                if (p->newProtect != p->Protect)
                {
                        VirtualProtectEx(hProcess, p->BaseAddress, 1, p->Protect, NULL) ;
                }
        }
        return 1 ;
}
