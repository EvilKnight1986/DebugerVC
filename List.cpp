// List.cpp: implementation of the List class.
//
//////////////////////////////////////////////////////////////////////

#include "List.h"

/*******************************************************************************
*
*  函 数 名 : SetState
*  功能描述 : 设置State
*  参数列表 : state:            新状态
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void Node::SetState(char state)
{
        if (1 != state && 0 != state)
        {
#ifdef _DEBUG
                printf("状态不对吧！\r\n") ;
#endif
                return ;
        }
        bIsActive = state ;
}

/*******************************************************************************
*
*  函 数 名 : StdState
*  功能描述 : State 置位
*  参数列表 : 
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void Node::StdState()
{
        bIsActive = 1 ;    
}

/*******************************************************************************
*
*  函 数 名 : StdState
*  功能描述 : State置0
*  参数列表 : 
*  说    明 : 
*  返回结果 : 无
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
*  函 数 名 : InsertPrev
*  功能描述 : 向键表头部添加元素
*  参数列表 : _nAddr:           断点的地址
*             _byOldValue:      原来的值
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
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
*  函 数 名 : InsertTail
*  功能描述 : 向键表尾部添加元素
*  参数列表 : _nAddr:           断点的地址
*             _byOldValue:      原来的值
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
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
                // 寻找最后一个结点
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
*  函 数 名 : Insert
*  功能描述 : 向键表尾部添加元素
*  参数列表 : _nAddr:           断点的地址
*             _byOldValue:      原来的值
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int List::Insert(unsigned int _nAddr, char _byOldValue)
{
       return InsertTail(_nAddr, _byOldValue) ; 
}

/*******************************************************************************
*
*  函 数 名 : Delete
*  功能描述 : 删除链表中指定元素
*  参数列表 : _nAddr:           要删除断点的地址
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int List::Delete(unsigned int _nAddr)
{
        if (NULL == pHead)
        {
#ifdef _DEBUG
                printf("链表为空\r\n") ;
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
        printf("没有找到要删除的元素\r\n") ;
#endif
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : Find
*  功能描述 : 查找指定元素的前驱节点
*  参数列表 : _nAddr:           要查找的断点的地址
*  说    明 : 
*  返回结果 : 如果找到返回结点指针，否则返回NULL
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
*  函 数 名 : FindPrev
*  功能描述 : 查找指定元素的前驱节点
*  参数列表 : _nAddr:           要查找的断点的地址
*  说    明 : 
*  返回结果 : 如果找到返回结点指针，否则返回NULL
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
*  函 数 名 : SetState
*  功能描述 : 设置某个节点的State
*  参数列表 : _nAddr:           要设置State的地址值
*             state:            新状态
*  说    明 : 
*  返回结果 : 如果出错，返回0,成功返回1
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
                printf("你设的状态不对吧\r\n") ;
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
*  函 数 名 : StdState
*  功能描述 : 将某个节点的State置位
*  参数列表 : _nAddr:           要置位State的地址值
*  说    明 : 
*  返回结果 : 如果出错，返回0,成功返回1
*
*******************************************************************************/
int List::StdState(unsigned int _nAddr)
{
        return SetState(_nAddr, 1) ;
}

/*******************************************************************************
*
*  函 数 名 : ClsState
*  功能描述 : 将某个节点的State清零
*  参数列表 : _nAddr:           要置位State的地址值
*  说    明 : 
*  返回结果 : 如果出错，返回0,成功返回1
*
*******************************************************************************/
int List::ClsState(unsigned int _nAddr)
{
        return SetState(_nAddr, 0) ;
}

/*******************************************************************************
*
*  函 数 名 : GetValue
*  功能描述 : 将某个节点的值(断点原字节数据)
*  参数列表 : _nAddr:           断点地址
*  说    明 : 
*  返回结果 : 成功返回1，失败返回0
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
*  函 数 名 : Display
*  功能描述 : 输出断点信息
*  参数列表 : _nAddr:           断点地址
*  说    明 : 
*  返回结果 : 激活的话返回1，失效的话返回0
*
*******************************************************************************/
void List::Display()
{
        Node * p = pHead ;

        if (0 == nSum)
        {
                printf("无软断点信息\r\n") ;
        }

        for (;NULL != p; p = p->pNext)
        {
                printf("断点地址:%p, 原来的指令: %X, %s\r\n", 
                                p->nAddr, p->byOldValue, 
                                        (p->bIsActive)?"Active":"NoActive") ;
        }
}

/*******************************************************************************
*
*  函 数 名 : IsActive
*  功能描述 : 返回当前状态
*  参数列表 : _nAddr:           断点地址
*  说    明 : 
*  返回结果 : 激活的话返回1，失效的话返回0
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
*  函 数 名 : Clean
*  功能描述 : 清空链表
*  参数列表 : _无
*  说    明 : 
*  返回结果 : 返回1
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