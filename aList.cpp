// List.cpp: implementation of the List class.
//
//////////////////////////////////////////////////////////////////////

#include "aList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*******************************************************************************
*
*  函 数 名 : aList
*  功能描述 : 链表构造函数
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
aList::aList():pHead(NULL),pLast(NULL),nSize(0)
{
}

/*******************************************************************************
*
*  函 数 名 : ~aList
*  功能描述 : 链表析构函数
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
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
*  函 数 名 : operator [] 
*  功能描述 : 重载[]运算符
*  参数列表 : index     --      数组下标
*  说    明 : 
*  返回结果 : DistPosition
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
*  函 数 名 : IsEmpty 
*  功能描述 : 邻接表是否为空
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 如果为空返回1,否则返回0
*
*******************************************************************************/
int aList::IsEmpty()
{
        return nSize == 0 ? 1 : 0 ;
}

/*******************************************************************************
*
*  函 数 名 : Find 
*  功能描述 : 查找
*  参数列表 : data         --           要查找的关键字
*  说    明 : 
*  返回结果 : 如果找到，返回指向该节点的指针,否则返回NULL
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
*  函 数 名 : FindPrev 
*  功能描述 : 查找前驱
*  参数列表 : data         --           要查找的关键字
*  说    明 : 
*  返回结果 : 如果找到，返回指向该节点的前驱结点的指针,否则返回NULL
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
*  函 数 名 : Insert 
*  功能描述 : 插入节点
*  参数列表 : data         --           要添加的关键字
              offset       --           未使用
              nIndex       --           插入到哪里
*  说    明 : 
*  返回结果 : 如果插入成功，返回1,否则返回0
*
*******************************************************************************/
int aList::Insert(IN unsigned int data, IN unsigned int offset, IN int nIndex)
{
        int i = 0;
        DistPosition p = pHead ;
        DistPosition tmp = new DistNode(data, offset) ;
    
        ++nSize ;
        // 如果链表是空的情况下
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
*  函 数 名 : Insert 
*  功能描述 : 在链表头部插入节点
*  参数列表 : data         --           要添加的关键字
              offset       --           保留
*  说    明 : 
*  返回结果 : 如果插入成功，返回1,否则返回0
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
*  函 数 名 : Insert 
*  功能描述 : 在链表尾部插入节点
*  参数列表 : data         --           要添加的关键字
              offset       --           保留
*  说    明 : 
*  返回结果 : 如果插入成功，返回1,否则返回0
*
*******************************************************************************/
int aList::InsertLast(IN unsigned int data, IN unsigned int offset)
{
        DistPosition tmp = new DistNode(data, offset) ;

        ++nSize ;

        // 如果链表为空的情况
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
*  函 数 名 : Delete 
*  功能描述 : 删除链表中的指定结点
*  参数列表 : data         --           要删除结点的关键字
*  说    明 : 
*  返回结果 : 如果删除成功，返回1，否则返回0
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
