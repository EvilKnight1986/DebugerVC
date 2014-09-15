/*******************************************************************************
*  
*  Copyright (c) all 2009 黄奇 All rights reserved
*  FileName : RedBlackTree.h
*  D a t e  : 2010.3.21
*  功   能  : 链表申明文件
*  说   明  : 
*
*
*******************************************************************************/
#include "MyList.h"
#define NAMEMAXLEN      128

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
*  函 数 名 : SetDllName
*  功能描述 : 将dll的名字复制到节点的m_pDllName中
*  参数列表 : _pDllName   -- 指向dll名字的字符串指针
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int MyNode::SetDllName(const char *_pDllName)
{
        if (NULL == this)
        {
                return 0 ;
        }

        if (NULL == _pDllName)
        {
                return 0 ;
        }
        
        int nStrLen = strlen(_pDllName) ;
        if (nStrLen > NAMEMAXLEN)
        {
#ifdef _DEBUG
                OutputDebugString("dll的名字超过长度了\r\n") ;
#endif
                return 0 ;
        }

        if (NULL != m_pDllName)
        {
                delete [] m_pDllName ;
                m_pDllName = NULL ;
        }

        ++nStrLen ;
        m_pDllName = new char[nStrLen] ;

        if (NULL == m_pDllName)
        {
#ifdef _DEBUG
                OutputDebugString("申请内存不成功\r\n") ;
#endif
                return 0 ;
        }

        // 将dll的名字复制到自己申请的空间中
        memset(m_pDllName, 0, sizeof(char) * nStrLen) ;
        memcpy(m_pDllName, _pDllName, sizeof(char) * (nStrLen-1)) ;

        // 计算dll名字的crc值
        m_DllNameCrc = cal_crc(m_pDllName) ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetLoadState
*  功能描述 : 设置dll的当前状态
*  参数列表 : _bStat   -- 将要设成的状态值
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int MyNode::SetLoadState(BOOL _bStat)
{
        if (NULL == this)
        {
                return 0 ;
        }
        m_bIsLoad = _bStat ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetLoad
*  功能描述 : 设置当前dll已经加载
*  参数列表 : 
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int MyNode::SetLoad(void)
{
        if (NULL == this)
        {
                return 0 ;
        }
        m_bIsLoad = TRUE ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : SetImageBase
*  功能描述 : 设置dll基址
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回1
*
*******************************************************************************/
int MyNode::SetImageBase(unsigned int _ImageBase)
{
        if (NULL == this)
        {
                return 0 ;
        }
        m_ImageBase = _ImageBase ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : GetImageBase
*  功能描述 : 返回dll基址
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回dll基址
*
*******************************************************************************/
int MyNode::GetImageBase(void)
{
        if (NULL == this)
        {
                return 0 ;
        }
        return m_ImageBase ;
}


/*******************************************************************************
*
*  函 数 名 : SetUnLoad
*  功能描述 : 设置当前dll没有加载
*  参数列表 : 
*  说    明 : 
*  返回结果 : 成功返回1, 失败返回0
*
*******************************************************************************/
int MyNode::SetUnLoad(void)
{
        if (NULL == this)
        {
                return 0 ;
        }
        m_bIsLoad = FALSE ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : GetCrc
*  功能描述 : 取得Dll名字的CRC值
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回CRC值
*
*******************************************************************************/
unsigned int MyNode::GetCrc(void)
{
        if (NULL == this)
        {
                return 0 ;
        }
        return m_DllNameCrc ;
}

/*******************************************************************************
*
*  函 数 名 : GetCrc
*  功能描述 : 取得Dll名字的CRC值
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回指向dll名字的字符串指针，如果没有设置的话，将返回NULL
*
*******************************************************************************/
char * MyNode::GetDllName(void)
{
        if (NULL == this)
        {
                return FALSE ;
        }
        return m_pDllName ;
}

/*******************************************************************************
*
*  函 数 名 : GetState
*  功能描述 : 取得当前Dll状态
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果dll当前已经加载，返回TRUE,否则返回FALSE
*
*******************************************************************************/
BOOL MyNode::GetState(void)
{
        if (NULL == this)
        {
                return FALSE ;
        }
        return m_bIsLoad ;
}


/*******************************************************************************
*
*  函 数 名 : DispNode
*  功能描述 : 输出节点信息
*  参数列表 : 
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void MyNode::DispNode()
{
        if (NULL == this)
        {
                return ;
        }
        printf("%-12s %p %p %d\r\n", m_pDllName, m_DllNameCrc, m_ImageBase, m_bIsLoad) ;
}

/*******************************************************************************
*
*  函 数 名 : MyList
*  功能描述 : MyList构造函数
*  参数列表 : 
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
MyList::MyList():pHead(NULL),pLast(NULL),nSize(0)
{
}

/*******************************************************************************
*
*  函 数 名 : MyList
*  功能描述 : MyList析构函数
*  参数列表 : 
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
MyList::~MyList()
{
        list p ;
        while (pHead != NULL)
        {
                p = pHead ;
                pHead = pHead->next ;
                delete p ;
        }
        pHead = pLast = 0 ;
}

/*******************************************************************************
*
*  函 数 名 : IsEmpty
*  功能描述 : 判断链表是否为空
*  参数列表 : 
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
int MyList::IsEmpty()
{
        return nSize == 0 ? 1 : 0 ;
}

/*******************************************************************************
*
*  函 数 名 : Find
*  功能描述 : 按dll名查找指定节点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果找到返回指向该节点的指针，否则返回NULL
*
*******************************************************************************/
position MyList::Find(const char *_pDllName)
{
        if (NULL == _pDllName)
        {
                return NULL ;
        }

        if (nSize == 0)
        {
                return NULL ;
        }

        unsigned int m_DllNameCrc = cal_crc(_pDllName) ;

        // 当crc值相同的时候才有必要去判断名字是否真的相同
        if (pHead->m_DllNameCrc == m_DllNameCrc)
        {
                if (0 == stricmp(_pDllName,pHead->m_pDllName))
                {
                        return pHead ;
                }
        }
        position p =  FindPrev(_pDllName) ;
        
        if (NULL != p)
        {
                return p->next ;
        }
        return p ;
}

/*******************************************************************************
*
*  函 数 名 : FindPrev
*  功能描述 : 按dll名查找指定节点的前驱
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果找到返回指向该节点前驱的指针，否则返回NULL
*
*******************************************************************************/
position MyList::FindPrev(const char *_pDllName)
{
        if (NULL == _pDllName)
        {
                return NULL ;
        }
        
        unsigned int m_DllNameCrc = cal_crc(_pDllName) ;

        position p = pHead ;
        if (p == NULL || p->next == NULL)
        {
                return NULL ;
        }
    
        // 先按crc值去查找
        for(; p->next != NULL; p = p->next)
        {
                // 如果找到crc值相同的，再用stricmp去比较是否真的相同
                // 因为存在不同的字串产生相同的值
                if (p->next->m_DllNameCrc == m_DllNameCrc)
                {
                        if (0 == stricmp(_pDllName, p->next->m_pDllName))
                        {
                                return p ;
                        }
                }
        }

        return NULL ;
}

/*******************************************************************************
*
*  函 数 名 : Insert
*  功能描述 : 在链表中插入一个新的节点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 成功返回在链表中的位置,失败返回-1
*
*******************************************************************************/
int MyList::Insert(const char *_pDllName, BOOL _isLoad)
{
        if (NULL == _pDllName)
        {
                return -1 ;
        }

        position p = pHead ;
        position tmp = new MyNode ;

        if (NULL == tmp)
        {
                return -1 ;
        }
        tmp->SetDllName(_pDllName) ;
        tmp->SetLoadState(_isLoad) ;

        ++nSize ;
        // 如果链表是空的情况下
        if (pHead == NULL)
        {
                pHead = tmp ;
                pLast = tmp ;
                return 0 ;
        }

        for (int i = 1; p->next != NULL; p = p->next, ++i)
                ;


        tmp->next = p->next ;
        p->next = tmp ;
        if (tmp->next == NULL)
        {
                pLast = NULL ;
        }
        return i + 1 ;
}

/*******************************************************************************
*
*  函 数 名 : Delete
*  功能描述 : 在链表中删除一个指定的节点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 成功返回1,失败返回0
*
*******************************************************************************/
int MyList::Delete(const char *_pDllName)
{
        if (NULL == _pDllName)
        {
                return 0 ;
        }
        position p = FindPrev(_pDllName) ;

        if (NULL != p)
        {
                position tmp = p->next ;
                p->next = tmp->next ;
                --nSize ;
                delete tmp ;
                return 1 ;
        }
        return 0 ;
}

/*******************************************************************************
*
*  函 数 名 : GetSize
*  功能描述 : 取得链表的长度
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回链表的长度
*
*******************************************************************************/
int MyList::GetSize()
{
    return nSize ;
}

/*******************************************************************************
*
*  函 数 名 : operator []
*  功能描述 : 取得链表中第index个元素的crc值
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回链表的长度
*
*******************************************************************************/
unsigned int & MyList::operator [](unsigned int index)
{
        if (index < 0 || index > nSize)
        {
                exit (EXIT_FAILURE);
        }

        unsigned int i(0) ;
        list p = pHead ;

        for (; i < index; ++i)
        {
                p = p->next ;
        }

        return p->m_DllNameCrc ;
}

/*******************************************************************************
*
*  函 数 名 : GetNode
*  功能描述 : 取得链表第nIndex个结点
*  参数列表 : 
*  说    明 : 
*  返回结果 : 返回该节点的指针，否则返回NULL
*
*******************************************************************************/
position MyList::GetNode(unsigned int nIndex)
{
        if (nIndex < 0 || nIndex > nSize)
        {
                return NULL ;
        }

        unsigned int i = 1 ;
        for (position p = pHead; NULL != p && i < nIndex; p = p->next, ++i)
        {
                ;
        }
        return p ;
}

/*******************************************************************************
*
*  函 数 名 : Displa
*  功能描述 : 输出当前已经加载的DLL名称
*  参数列表 : 
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
int MyList::DisplayLoadDllName()
{
        list p = pHead ;
        for (; NULL != p; p = p->next)
        {
                if (p->m_bIsLoad)
                {
                        printf("%s\r\n", p->m_pDllName) ;
                }
        }
        return 1 ;
}
