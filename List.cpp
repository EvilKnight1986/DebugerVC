// List.cpp: implementation of the List class.
//
//////////////////////////////////////////////////////////////////////

#include "List.h"

/*******************************************************************************
*
*  �� �� �� : SetState
*  �������� : ����State
*  �����б� : state:            ��״̬
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void Node::SetState(char state)
{
        if (1 != state && 0 != state)
        {
#ifdef _DEBUG
                printf("״̬���԰ɣ�\r\n") ;
#endif
                return ;
        }
        bIsActive = state ;
}

/*******************************************************************************
*
*  �� �� �� : StdState
*  �������� : State ��λ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void Node::StdState()
{
        bIsActive = 1 ;    
}

/*******************************************************************************
*
*  �� �� �� : StdState
*  �������� : State��0
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void Node::ClsState()
{
        bIsActive = 0 ;
}

List::~List()
{
        if (NULL != pHead)
        {
                for (Node *p = pHead ;NULL != p; p = pHead)
                {
                        pHead = pHead->pNext ;
                        delete p;
                } 
        }
}

/*******************************************************************************
*
*  �� �� �� : InsertPrev
*  �������� : �����ͷ�����Ԫ��
*  �����б� : _nAddr:           �ϵ�ĵ�ַ
*             _byOldValue:      ԭ����ֵ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int List::InsertPrev(unsigned int _nAddr, char _byOldValue)
{
        Node *p = new Node ;
        
        if (NULL == p)
        {
#ifdef _DEBUG
                printf("Malloc failed\r\n") ;
#endif
                return 0 ;
        }
        
        p->bIsActive = 1 ;
        p->byOldValue = _byOldValue ;
        p->nAddr = _nAddr ;   
        p->pNext = pHead ;
        pHead = p ;
        ++nSum ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : InsertTail
*  �������� : �����β�����Ԫ��
*  �����б� : _nAddr:           �ϵ�ĵ�ַ
*             _byOldValue:      ԭ����ֵ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int List::InsertTail(unsigned int _nAddr, char _byOldValue)
{
        Node *p = new Node ;
        
        if (NULL == p)
        {
#ifdef _DEBUG
                printf("Malloc failed\r\n") ;
#endif
                return 0 ;
        }
        
        p->bIsActive = 1 ;
        p->byOldValue = _byOldValue ;
        p->nAddr = _nAddr ;
        p->pNext = NULL ;
        
        if (NULL == pHead)
        {
                pHead = p ;
                ++nSum ;
        }
        else
        {
                // Ѱ�����һ�����
                for (Node *ptr = pHead; ptr->pNext != NULL; ptr = ptr->pNext)
                {
                }
                ptr->pNext = p ;
                ++nSum ;
        }
        
        return 1 ;      
}

/*******************************************************************************
*
*  �� �� �� : Insert
*  �������� : �����β�����Ԫ��
*  �����б� : _nAddr:           �ϵ�ĵ�ַ
*             _byOldValue:      ԭ����ֵ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int List::Insert(unsigned int _nAddr, char _byOldValue)
{
       return InsertTail(_nAddr, _byOldValue) ; 
}

/*******************************************************************************
*
*  �� �� �� : Delete
*  �������� : ɾ��������ָ��Ԫ��
*  �����б� : _nAddr:           Ҫɾ���ϵ�ĵ�ַ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
*
*******************************************************************************/
int List::Delete(unsigned int _nAddr)
{
        if (NULL == pHead)
        {
#ifdef _DEBUG
                printf("����Ϊ��\r\n") ;
#endif
                return 0 ;
        }

        Node *p = FindPrev(_nAddr) ;

        if (NULL != p)
        {
                if (NULL != p->pNext)
                {
                        Node *q = p->pNext ;
                        p->pNext = q->pNext ;
                        delete q ;
                        --nSum ;
                        return 1 ;
                }
                else
                {
                        Node *q = p->pNext ;
                        p->pNext = NULL ;
                        delete q ;
                        --nSum ;
                        return 1 ;
                }
        }

        if (_nAddr == pHead->nAddr)
        {
                p = pHead ;
                pHead = pHead->pNext ;
                delete p ;
                --nSum ;
                return 1 ;
        }

#ifdef _DEBUG
        printf("û���ҵ�Ҫɾ����Ԫ��\r\n") ;
#endif
        return 0 ;
}

/*******************************************************************************
*
*  �� �� �� : Find
*  �������� : ����ָ��Ԫ�ص�ǰ���ڵ�
*  �����б� : _nAddr:           Ҫ���ҵĶϵ�ĵ�ַ
*  ˵    �� : 
*  ���ؽ�� : ����ҵ����ؽ��ָ�룬���򷵻�NULL
*
*******************************************************************************/
Node * List::Find(unsigned int _nAddr)
{
        if (NULL == pHead)
        {
                return NULL ;
        }

        if (_nAddr == pHead->nAddr)
        {
                return pHead ;
        }

        Node *p = FindPrev(_nAddr) ;
        if (NULL != p)
        {
                return p->pNext ;
        }
        return NULL;
        
}

/*******************************************************************************
*
*  �� �� �� : FindPrev
*  �������� : ����ָ��Ԫ�ص�ǰ���ڵ�
*  �����б� : _nAddr:           Ҫ���ҵĶϵ�ĵ�ַ
*  ˵    �� : 
*  ���ؽ�� : ����ҵ����ؽ��ָ�룬���򷵻�NULL
*
*******************************************************************************/
Node * List::FindPrev(unsigned int _nAddr)
{
        Node * p = pHead ;
        if (NULL == p || NULL == p->pNext)
        {
                return NULL ;
        }

        for (; NULL != p->pNext && _nAddr != p->pNext->nAddr; p = p->pNext)
        {
        }

        if (NULL == p->pNext)
        {
                return NULL ;
        }
        
        if (_nAddr == p->pNext->nAddr)
        {
                return p ;
        }
        return NULL ;
}

/*******************************************************************************
*
*  �� �� �� : SetState
*  �������� : ����ĳ���ڵ��State
*  �����б� : _nAddr:           Ҫ����State�ĵ�ֵַ
*             state:            ��״̬
*  ˵    �� : 
*  ���ؽ�� : �����������0,�ɹ�����1
*
*******************************************************************************/
int List::SetState(unsigned int _nAddr, char state)
{
        if (NULL == this)
        {
                return 0 ;
        }
        if (state != 0 && state != 1)
        {
#ifdef _DEBUG
                printf("�����״̬���԰�\r\n") ;
#endif
                return 0 ;
        }

        Node * p = Find(_nAddr) ;
        if (NULL == p)
        {
                return 0 ;
        }

        p->SetState(state) ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : StdState
*  �������� : ��ĳ���ڵ��State��λ
*  �����б� : _nAddr:           Ҫ��λState�ĵ�ֵַ
*  ˵    �� : 
*  ���ؽ�� : �����������0,�ɹ�����1
*
*******************************************************************************/
int List::StdState(unsigned int _nAddr)
{
        return SetState(_nAddr, 1) ;
}

/*******************************************************************************
*
*  �� �� �� : ClsState
*  �������� : ��ĳ���ڵ��State����
*  �����б� : _nAddr:           Ҫ��λState�ĵ�ֵַ
*  ˵    �� : 
*  ���ؽ�� : �����������0,�ɹ�����1
*
*******************************************************************************/
int List::ClsState(unsigned int _nAddr)
{
        return SetState(_nAddr, 0) ;
}

/*******************************************************************************
*
*  �� �� �� : GetValue
*  �������� : ��ĳ���ڵ��ֵ(�ϵ�ԭ�ֽ�����)
*  �����б� : _nAddr:           �ϵ��ַ
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1��ʧ�ܷ���0
*
*******************************************************************************/
int List::GetValue(unsigned int _nAddr, char *pValue)
{
        Node * p = Find(_nAddr) ;
        if (NULL == p)
        {
                return 0 ;
        }
        *pValue = p->byOldValue ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : Display
*  �������� : ����ϵ���Ϣ
*  �����б� : _nAddr:           �ϵ��ַ
*  ˵    �� : 
*  ���ؽ�� : ����Ļ�����1��ʧЧ�Ļ�����0
*
*******************************************************************************/
void List::Display()
{
        Node * p = pHead ;

        if (0 == nSum)
        {
                printf("����ϵ���Ϣ\r\n") ;
        }

        for (;NULL != p; p = p->pNext)
        {
                printf("�ϵ��ַ:%p, ԭ����ָ��: %X, %s\r\n", 
                                p->nAddr, p->byOldValue, 
                                        (p->bIsActive)?"Active":"NoActive") ;
        }
}

/*******************************************************************************
*
*  �� �� �� : IsActive
*  �������� : ���ص�ǰ״̬
*  �����б� : _nAddr:           �ϵ��ַ
*  ˵    �� : 
*  ���ؽ�� : ����Ļ�����1��ʧЧ�Ļ�����0
*
*******************************************************************************/
int List::IsActive(unsigned int _nAddr)
{
        Node *p = Find(_nAddr) ;
        if (NULL == p)
        {
                return 0 ;
        }
        return p->bIsActive ;
}

/*******************************************************************************
*
*  �� �� �� : Clean
*  �������� : �������
*  �����б� : _��
*  ˵    �� : 
*  ���ؽ�� : ����1
*
*******************************************************************************/
int List::Clean()
{
        Node *p = pHead ;
        Node *q = NULL ;
        while (NULL != p)
        {
                q = p ;
                p = p->pNext ;
                delete q ;
        }
        nSum = 0 ;
        pHead = NULL ;
        return 1 ;
}

Node * List::GetListHead()
{
        return pHead ;
}

int List::GetNodeCount()
{
        return nSum ;
}