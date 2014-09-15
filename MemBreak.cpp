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
*  �� �� �� : RemoveAll 
*  �������� : �Ƴ��������н��
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ���ɾ���ɹ�������1�����򷵻�0
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
*  �� �� �� : Remove 
*  �������� : ͨ���ؼ����Ƴ�����ָ�����
*  �����б� : nAddr     --    ��ֵַ
*  ˵    �� : 
*  ���ؽ�� : ���ɾ���ɹ�������1�����򷵻�0
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
*  �� �� �� : RemoveById 
*  �������� : ͨ��id�Ƴ�����ָ�����
*  �����б� : id       --        ���
*  ˵    �� : 
*  ���ؽ�� : ���ɾ���ɹ�������1�����򷵻�0
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
*  �� �� �� : FindPrev 
*  �������� : ����ָ���ؼ��ֵ�ǰ�����
*  �����б� : nAddr       --        �ؼ���
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�����ָ���ؼ��ֵ�ǰ����㣬���򷵻�NULL
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
*  �� �� �� : FindPrevById 
*  �������� : ����ָ����ŵ�ǰ�����
*  �����б� : nAddr       --        �ؼ���
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�����ָ����ŵ�ǰ����㣬���򷵻�NULL
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
*  �� �� �� : Find 
*  �������� : ͨ���ؼ��ֲ��ҽ��
*  �����б� : nAddr       --        �ؼ���
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�����ָ��ý���ָ�룬���򷵻�NULL
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
*  �� �� �� : FindById 
*  �������� : ͨ����Ų��ҽ��
*  �����б� : id       --        ���
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�����ָ��ý���ָ�룬���򷵻�NULL
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
*  �� �� �� : IsHaving 
*  �������� : �Ƿ����
*  �����б� : nAddr       --        �ؼ���
*  ˵    �� : 
*  ���ؽ�� : ����˵�ַ��������ĳ���������������򷵻�1�����򷵻�0
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
*  �� �� �� : Insert 
*  �������� : ������
*  �����б� : lpBreakBase       --        ����ַ
              BreakLen          --        ����
              Protect           --        ����
*  ˵    �� : 
*  ���ؽ�� : �������ɹ�����1,���򷵻�0
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
*  �� �� �� : Display 
*  �������� : ��ӡ������
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : �������ɹ�����1,���򷵻�0
*
*******************************************************************************/
void CMemBreak::Display()
{
        PMemBreakNode p = m_pHead ;
        if (NULL == m_pHead)
        {
                printf("���ڴ�ϵ���Ϣ\r\n") ;
                return ;
        }
        
        printf("�ڴ�ϵ���Ϣ!\r\n") ;
        for(;NULL != p; p = p->pNext)
        {
                printf("�ϵ�ID: %3d ��ʼ��ַ:%p ����:%p ", 
                                p->nID, p->lpBreakBase, p->BreakLen) ;

                switch(p->Protect)
                {
                case 1:
                        printf("����: Read") ;
                        break ;
                case 2:
                        printf("����: Write") ;
                        break ;
                case 3:
                        printf("����: Execute") ;
                        break ;
                }
                printf("  ״̬: %s\r\n", p->isActive?"Active":"NoActive") ;
        }
}

/*******************************************************************************
*
*  �� �� �� : HaveIn
*  �������� : ����һ���ڴ��ַ���ж�����ڴ��Ƿ�����Ч�ϵ㵱�У�ͬʱȡ�öϵ�id
*  �����б� : nAddr     --      �ڴ��ַ
              nPage     --      �������������ڱ��洦���ڴ�ҳ��id��
*  ˵    �� : 
*  ���ؽ�� : ������ڷ���1�������ڷ���0
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
*  �� �� �� : GetCount
*  �������� : �����ڴ�ϵ�����
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : �����ڴ�ϵ�����
*
*******************************************************************************/
int CMemBreak::GetCount()
{
        return m_nCount ;
}
