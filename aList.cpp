// List.cpp: implementation of the List class.
//
//////////////////////////////////////////////////////////////////////

#include "aList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*******************************************************************************
*
*  �� �� �� : aList
*  �������� : �����캯��
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
aList::aList():pHead(NULL),pLast(NULL),nSize(0)
{
}

/*******************************************************************************
*
*  �� �� �� : ~aList
*  �������� : ������������
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
aList::~aList()
{
        DistList p ;
        while (pHead != NULL)
        {
                p = pHead ;
                pHead = pHead->next ;
                delete p ;
        }
        pHead = pLast = NULL ;
}

/*******************************************************************************
*
*  �� �� �� : operator [] 
*  �������� : ����[]�����
*  �����б� : index     --      �����±�
*  ˵    �� : 
*  ���ؽ�� : DistPosition
*
*******************************************************************************/
DistPosition  aList::operator [] (IN unsigned int index)
{
        if (index < 0 || index > nSize)
        {
                exit (EXIT_FAILURE);
        }

        unsigned int i(0) ;
        DistList p = pHead;
        for (; i < index; ++i)
        {
                p = p->next ;
        }
        return p ;
}

/*******************************************************************************
*
*  �� �� �� : IsEmpty 
*  �������� : �ڽӱ��Ƿ�Ϊ��
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ���Ϊ�շ���1,���򷵻�0
*
*******************************************************************************/
int aList::IsEmpty()
{
        return nSize == 0 ? 1 : 0 ;
}

/*******************************************************************************
*
*  �� �� �� : Find 
*  �������� : ����
*  �����б� : data         --           Ҫ���ҵĹؼ���
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�������ָ��ýڵ��ָ��,���򷵻�NULL
*
*******************************************************************************/
DistPosition aList::Find(IN unsigned int data)
{
        if (pHead == NULL)
        {
                return NULL ;
        }

        else if (pHead != NULL && pHead->data == data)
        {
                return pHead ;
        }

        DistPosition p = NULL ;
        p = FindPrev(data) ;
        if( p != NULL)
        {
                return p->next ;
        }

        return NULL ;
}

/*******************************************************************************
*
*  �� �� �� : FindPrev 
*  �������� : ����ǰ��
*  �����б� : data         --           Ҫ���ҵĹؼ���
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�������ָ��ýڵ��ǰ������ָ��,���򷵻�NULL
*
*******************************************************************************/
DistPosition aList::FindPrev(IN unsigned int data)
{
        DistPosition p = pHead ;
        if (p == NULL || p->next == NULL)
        {
                return NULL ;
        }

        for(; p->next != NULL && p->next->data != data; p = p->next)
                ;

        return p ;
}

/*******************************************************************************
*
*  �� �� �� : Insert 
*  �������� : ����ڵ�
*  �����б� : data         --           Ҫ��ӵĹؼ���
              offset       --           δʹ��
              nIndex       --           ���뵽����
*  ˵    �� : 
*  ���ؽ�� : �������ɹ�������1,���򷵻�0
*
*******************************************************************************/
int aList::Insert(IN unsigned int data, IN unsigned int offset, IN int nIndex)
{
        int i = 0;
        DistPosition p = pHead ;
        DistPosition tmp = new DistNode(data, offset) ;
    
        ++nSize ;
        // ��������ǿյ������
        if (pHead == NULL)
        {
                pHead = tmp ;
                pLast = tmp ;
                return 1 ;
        }

        for (; i < nIndex && p->next != NULL; p = p->next)
                ;

        tmp->next = p->next ;
        p->next = tmp ;
        if (tmp->next == NULL)
        {
                pLast = NULL ;
        }

        if (NULL == pHead)
        {
                pHead = tmp ;
        }
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : Insert 
*  �������� : ������ͷ������ڵ�
*  �����б� : data         --           Ҫ��ӵĹؼ���
              offset       --           ����
*  ˵    �� : 
*  ���ؽ�� : �������ɹ�������1,���򷵻�0
*
*******************************************************************************/
int aList::InsertPrev(IN unsigned int data, IN unsigned int offset)
{
        DistPosition tmp = new DistNode(data,offset) ;

        ++nSize ;

        tmp->next = pHead ;
        pHead = tmp ;

        if (pLast == NULL)
        {
                pLast = tmp ;
        }

        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : Insert 
*  �������� : ������β������ڵ�
*  �����б� : data         --           Ҫ��ӵĹؼ���
              offset       --           ����
*  ˵    �� : 
*  ���ؽ�� : �������ɹ�������1,���򷵻�0
*
*******************************************************************************/
int aList::InsertLast(IN unsigned int data, IN unsigned int offset)
{
        DistPosition tmp = new DistNode(data, offset) ;

        ++nSize ;

        // �������Ϊ�յ����
        if (pLast == NULL && pHead == NULL)
        {
                pLast = pHead = tmp ;
        }
        else
        {
                pLast->next = tmp ;
                pLast = tmp ;
        }
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : Delete 
*  �������� : ɾ�������е�ָ�����
*  �����б� : data         --           Ҫɾ�����Ĺؼ���
*  ˵    �� : 
*  ���ؽ�� : ���ɾ���ɹ�������1�����򷵻�0
*
*******************************************************************************/
int aList::Delete(IN unsigned int data)
{
        if (NULL == pHead)
        {
                return 0 ;
        }

        if (pHead->data == data)
        {
                DistPosition tmp = pHead->next ;
                delete pHead ;
                pHead = tmp ;
                --nSize ;
                if (0 == nSize)
                {
                        pHead = pLast = NULL ;
                }
                return 1 ;
        }

        DistPosition p = FindPrev(data) ;

        if (p != NULL)
        {
                if (NULL == p->next->next)
                {
                        delete p->next ;
                        --nSize ;
                        pLast = p ;
                        return 1 ;
                }
                else
                {
                        DistPosition tmp = p->next ;
                        p->next = tmp->next ;
                        --nSize ;
                        delete tmp ;
                        return 1 ;
                }
        }

        return 0 ;
}

int aList::Push(IN unsigned int data, IN unsigned int offset)
{
        return InsertPrev(data, offset) ;
}

unsigned int aList::Pop()
{
        if (pHead != NULL)
        {
                int data(0) ;
                DistPosition tmp = pHead ;
                pHead = pHead->next ;
                data = tmp->data ;
                delete tmp ;
                --nSize ;
                if (0 == nSize)
                {
                        pHead = pLast = NULL ;
                }
                return data ;        
        }
        return 0 ;
}

DistNode * aList::Top()
{
        return (pHead != NULL)?pHead:NULL;
}

int aList::GetSize()
{
        return nSize ;
}

int aList::Clear()
{
        if (NULL == this)
        {
                return 0 ;
        }
        DistList p = pHead ;
        DistList q = NULL ;
        while (NULL != p)
        {
                q = p ;
                p = p->next ;
                delete q ;
        }
        pHead = pLast = NULL ;
        nSize = 0 ;
        return 1 ;
}
