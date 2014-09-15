// MemBreak.cpp: implementation of the CMemBreak class.
//
//////////////////////////////////////////////////////////////////////

#include "MemBreak.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMemBreak::CMemBreak():m_nCount(0),m_pHead(NULL),m_ID(0)
{

}

CMemBreak::~CMemBreak()
{
        RemoveAll() ;
}

/*******************************************************************************
*
*  函 数 名 : RemoveAll 
*  功能描述 : 移除链表所有结点
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果删除成功，返回1，否则返回0
*
*******************************************************************************/
int CMemBreak::RemoveAll()
{
        if (0 == m_nCount && NULL == m_pHead)
        {
                return 0 ;
        }
        PMemBreakNode p = m_pHead ;
        PMemBreakNode q = p ;
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
*  函 数 名 : Remove 
*  功能描述 : 通过关键字移除链表指定结点
*  参数列表 : nAddr     --    地址值
*  说    明 : 
*  返回结果 : 如果删除成功，返回1，否则返回0
*
*******************************************************************************/
int CMemBreak::Remove(LPVOID nAddr)
{
        if (NULL == this)
        {
                return 0 ;
        }
        
        if (NULL == m_pHead)
        {
                return 0 ;
        }
        
        PMemBreakNode p = NULL ;
        if (m_pHead->lpBreakBase == nAddr)
        {
                p = m_pHead ;
                m_pHead = m_pHead->pNext ;
                delete p ;
                --m_nCount ;
                return 1 ;
        }

        p = FindPrev(nAddr) ;

        if (NULL == p)
        {
                return 0 ;
        }

        PMemBreakNode q = p->pNext ;
        p->pNext = q->pNext ;
        delete q ;
        --m_nCount ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : RemoveById 
*  功能描述 : 通过id移除链表指定结点
*  参数列表 : id       --        序号
*  说    明 : 
*  返回结果 : 如果删除成功，返回1，否则返回0
*
*******************************************************************************/
int CMemBreak::RemoveById(int id)
{
        if (NULL == this)
        {
                return 0 ;
        }
        
        if (NULL == m_pHead)
        {
                return 0 ;
        }
        
        PMemBreakNode p = NULL ;
        if (m_pHead->nID == id)
        {
                p = m_pHead ;
                m_pHead = m_pHead->pNext ;
                delete p ;
                --m_nCount ;
                return 1 ;
        }
        p = FindPrevById(id) ;
        if (NULL == p)
        {
                return 0 ;
        }
        PMemBreakNode q = p->pNext ;
        p->pNext = q->pNext ;
        delete q ;
        --m_nCount ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : FindPrev 
*  功能描述 : 查找指定关键字的前驱结点
*  参数列表 : nAddr       --        关键字
*  说    明 : 
*  返回结果 : 如果找到返回指定关键字的前驱结点，否则返回NULL
*
*******************************************************************************/
PMemBreakNode CMemBreak::FindPrev(LPVOID nAddr)
{
        if (NULL == this)
        {
                return NULL ;
        }

        if (NULL == m_pHead)
        {
                return NULL ;
        }

        PMemBreakNode p = m_pHead ;
        while (p->pNext != NULL)
        {
                if (nAddr == p->pNext->lpBreakBase)
                {
                        return p ;
                }
                p = p->pNext ;
        }
        return NULL ;
}

/*******************************************************************************
*
*  函 数 名 : FindPrevById 
*  功能描述 : 查找指定序号的前驱结点
*  参数列表 : nAddr       --        关键字
*  说    明 : 
*  返回结果 : 如果找到返回指定序号的前驱结点，否则返回NULL
*
*******************************************************************************/
PMemBreakNode CMemBreak::FindPrevById(int id)
{
        if (NULL == this)
        {
                return NULL ;
        }
        
        if (NULL == m_pHead)
        {
                return NULL ;
        }

        PMemBreakNode p = m_pHead ;
        while (p->pNext != NULL)
        {
                if (id == p->pNext->nID)
                {
                        return p ;
                }
                p = p->pNext ;
        }
        return NULL ;  
}

/*******************************************************************************
*
*  函 数 名 : Find 
*  功能描述 : 通过关键字查找结点
*  参数列表 : nAddr       --        关键字
*  说    明 : 
*  返回结果 : 如果找到返回指向该结点的指针，否则返回NULL
*
*******************************************************************************/
PMemBreakNode CMemBreak::Find(LPVOID nAddr)
{
        if (NULL == this)
        {
                return NULL ;
        }
        if (NULL == m_pHead)
        {
                return NULL ;
        }

        if (m_pHead->lpBreakBase == nAddr)
        {
                return m_pHead ;
        }
        PMemBreakNode p = FindPrev(nAddr) ;

        if (NULL != p)
        {
                return p->pNext ;
        }
        
        return NULL ;
}

/*******************************************************************************
*
*  函 数 名 : FindById 
*  功能描述 : 通过序号查找结点
*  参数列表 : id       --        序号
*  说    明 : 
*  返回结果 : 如果找到返回指向该结点的指针，否则返回NULL
*
*******************************************************************************/
PMemBreakNode CMemBreak::FindById(int id)
{
        if (NULL == this)
        {
                return NULL ;
        }
        if (NULL == m_pHead)
        {
                return NULL ;
        }
        
        if (m_pHead->nID == id)
        {
                return m_pHead ;
        }

        PMemBreakNode p =  FindPrevById(id) ;
        if (NULL != p)
        {
                return p->pNext ;
        }
        return NULL ;  
}

/*******************************************************************************
*
*  函 数 名 : IsHaving 
*  功能描述 : 是否包含
*  参数列表 : nAddr       --        关键字
*  说    明 : 
*  返回结果 : 如果此地址被链表中某个结点区间包含，则返回1，否则返回0
*
*******************************************************************************/
int CMemBreak::IsHaving(LPVOID nAddr)
{
        if (NULL == m_pHead)
        {
                return 0 ;
        }

        PMemBreakNode p = m_pHead ;
        
        for (; NULL != p; p = p->pNext)
        {
                if (nAddr == p->lpBreakBase)
                {
                        return 1 ;
                }
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : Insert 
*  功能描述 : 插入结点
*  参数列表 : lpBreakBase       --        基地址
              BreakLen          --        长度
              Protect           --        属性
*  说    明 : 
*  返回结果 : 如果插入成功返回1,否则返回0
*
*******************************************************************************/
int CMemBreak::Insert(LPVOID lpBreakBase, SIZE_T BreakLen, DWORD Protect)
{
        PMemBreakNode p = new MemBreakNode ;

        if (NULL == p)
        {
                return 0 ;
        }
        memset(p, 0, sizeof(MemBreakNode)) ;

        ++m_ID ;
        ++m_nCount ;
        p->nID = m_ID ;
        p->lpBreakBase = lpBreakBase ;
        p->BreakLen = BreakLen ;
        p->Protect = Protect ;
        p->isActive = 1 ;

        if (NULL == m_pHead)
        {
                m_pHead = p ;
                return p->nID ;
        }

        PMemBreakNode q = m_pHead ;
        for(;NULL != q->pNext;q = q->pNext)
        {
        }

        q->pNext = p ;
        return p->nID ;
}

/*******************************************************************************
*
*  函 数 名 : Display 
*  功能描述 : 打印出链表
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果插入成功返回1,否则返回0
*
*******************************************************************************/
void CMemBreak::Display()
{
        PMemBreakNode p = m_pHead ;
        if (NULL == m_pHead)
        {
                printf("无内存断点信息\r\n") ;
                return ;
        }
        
        printf("内存断点信息!\r\n") ;
        for(;NULL != p; p = p->pNext)
        {
                printf("断点ID: %3d 起始地址:%p 长度:%p ", 
                                p->nID, p->lpBreakBase, p->BreakLen) ;

                switch(p->Protect)
                {
                case 1:
                        printf("属性: Read") ;
                        break ;
                case 2:
                        printf("属性: Write") ;
                        break ;
                case 3:
                        printf("属性: Execute") ;
                        break ;
                }
                printf("  状态: %s\r\n", p->isActive?"Active":"NoActive") ;
        }
}

/*******************************************************************************
*
*  函 数 名 : HaveIn
*  功能描述 : 传入一个内存地址，判断这个内存是否处于有效断点当中，同时取得断点id
*  参数列表 : nAddr     --      内存地址
              nPage     --      传出参数，用于保存处于内存页的id号
*  说    明 : 
*  返回结果 : 如果存在返回1，不存在返回0
*
*******************************************************************************/
int CMemBreak::HaveIn(LPVOID nAddr, int &nPage)
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

        PMemBreakNode p = m_pHead ;

        while (NULL != p)
        {
                if (nAddr >= p->lpBreakBase 
                        && (DWORD)nAddr < (p->BreakLen + (DWORD)p->lpBreakBase))
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
*  函 数 名 : GetCount
*  功能描述 : 返回内存断点数量
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 返回内存断点数量
*
*******************************************************************************/
int CMemBreak::GetCount()
{
        return m_nCount ;
}
