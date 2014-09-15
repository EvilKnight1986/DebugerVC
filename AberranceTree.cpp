/*******************************************************************************
*  
*  Copyright (c) all 2009 黄奇 All rights reserved
*  FileName : AberranceTree.h
*  D a t e  : 2010.3.21
*  功   能  : 红黑树定义文件
*  说   明  : 
*
*
*******************************************************************************/

#include "AberranceTree.h"
#include "Tools.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
*  函 数 名 : AberranceTree
*  功能描述 : AberranceTree构造函数
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
AberranceTree::AberranceTree():m_pSentinel(NULL), m_pRoot(NULL),m_pHead(NULL), 
                m_pLast(NULL)
{
        m_pSentinel = new AberrTreeNode ;

        if (m_pSentinel == NULL)
        {
                exit(EXIT_FAILURE) ;
        }

        m_pSentinel->color  = b_black ;
        m_pSentinel->left   = m_pSentinel ;
        m_pSentinel->right  = m_pSentinel ;
        m_pSentinel->parent = m_pSentinel ;
        m_pRoot = m_pSentinel ;
}

/*******************************************************************************
*
*  函 数 名 : ~AberranceTree
*  功能描述 : AberranceTree析构函数
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
AberranceTree::~AberranceTree()
{
        if (m_pRoot != m_pSentinel)
        {
                Destory(m_pRoot) ;
                m_pRoot = NULL ;
        }
        if (m_pSentinel != NULL)
        {
                delete m_pSentinel ;
                m_pSentinel = NULL ;
        }
        m_pHead = NULL ;
        m_pLast = NULL ;
}

/*******************************************************************************
*
*  函 数 名 : Insert
*  功能描述 : 向红黑树插入一个节点
*  参数列表 : _nAddr    -- 要指令的地址值
*  说    明 : 
*  返回结果 : 如果成功，返回指向节点的指针
*
*******************************************************************************/
AberrTreeNode* AberranceTree::Insert(IN unsigned int _nAddr, IN char *pDump, int nLen)
{
        AberrTreeNode * pNode = m_pRoot ;
        AberrTreeNode * tmp =   NULL ;
        // 这里还要判断pDump是不是为空

        if (NULL == pDump)
        {
                OutputDebugString("Dump为空!\r\n") ;
                return 0 ;
        }

        if (nLen <= 0 || nLen >= DUMPSIZE)
        {
                OutputDebugString("DumpSize不对!\r\n") ;
                return 0 ;
        }

        // 先判断树中有没有相同的结点，因为一条指令可能会跑很多次
        if (NULL != m_pRoot)
        {
                tmp = FindEx(_nAddr, pDump, nLen) ;
                if (NULL != tmp) 
                {
                        ++(tmp->nCount) ;
                }
        }

        // 如果树为空的话
        if (m_pRoot == m_pSentinel)
        {
                tmp = new AberrTreeNode(_nAddr) ;
                if (tmp == NULL)
                {
                        exit(EXIT_FAILURE) ;
                }

                memset(tmp->szDump, 0, sizeof(char) * DUMPSIZE) ;
                tmp->SetDump(pDump, sizeof(char) * nLen) ;

                tmp->left = tmp->right = tmp->parent = m_pSentinel ;
                tmp->color = b_red ;

                m_pRoot = tmp ;

                if (InsertLink(tmp))
                {
                        return tmp ;
                }
                else
                {
                        return 0 ;
                }
        }

        for (; pNode != m_pSentinel;)
        {
                if (_nAddr <= pNode->nAddress && pNode->left != m_pSentinel)
                {
                        pNode = pNode->left ;
                }
                else if (_nAddr > pNode->nAddress && pNode->right != m_pSentinel)
                {
                        pNode = pNode->right ;
                }
                else if (_nAddr == pNode->nAddress)
                {
                        pNode = m_pSentinel ;
                }
                else
                {
                        break ;
                }
        }

        if (pNode == m_pSentinel)
        {
                return 0;
        }

        tmp = NULL ;
        tmp = new AberrTreeNode(_nAddr) ;
        if (tmp == NULL)
        {
                exit(EXIT_FAILURE) ;
        }

        memset(tmp->szDump, 0, sizeof(char) * DUMPSIZE) ;
        tmp->SetDump(pDump, sizeof(char) * nLen) ;
        tmp->left = tmp->right = m_pSentinel ;
        tmp->color = b_red ;
        tmp->parent = pNode ;

        // 往左边插入
        if (_nAddr <= pNode->nAddress && pNode->left == m_pSentinel)
        {
                pNode->left = tmp ;
        }
        // 入右边插入
        else if (_nAddr > pNode->nAddress && pNode->right == m_pSentinel)
        {
                pNode->right = tmp ;
        }
        insertFixup(tmp) ;

        if (InsertLink(tmp))
        {
                return tmp ;
        }
        else
        {
                return 0 ;
        }
}

/*******************************************************************************
*
*  函 数 名 : Remove
*  功能描述 : 移除树中的指定节点
*  参数列表 : _nAddr    -- 要移除的值
*  说    明 : 
*  返回结果 : 如果移除了，返回1，否则返回0
*
*******************************************************************************/
int AberranceTree::Remove(IN unsigned int _nAddr)
{
        return Delete(m_pRoot, _nAddr) ;
}

/*******************************************************************************
*
*  函 数 名 : Destory
*  功能描述 : 销毁整棵树
*  参数列表 : T         -- 树的根结点
*  说    明 : 
*  返回结果 : 
*
*******************************************************************************/
void AberranceTree::Destory(IN AberrTreeNode *T)
{
    if (T != m_pSentinel && T != NULL )
    {
        Destory(T->left) ;
        Destory(T->right) ;
    }
}

/*******************************************************************************
*
*  函 数 名 : FindMin
*  功能描述 : 查找树中最小的节点
*  参数列表 : T         --      树的根结点
*  说    明 : 
*  返回结果 : 如果成功返回指向该树中最小节点的指针，否则返回NULL
*
*******************************************************************************/
AberrTreeNode * AberranceTree::FindMin(IN AberrTreeNode * T)
{
    if (m_pRoot == m_pSentinel || T == m_pSentinel)
    {
        return NULL ;
    }

    while (T->left != m_pSentinel )
    {
        T = T->left ;
    }

    return T ;
}

/*******************************************************************************
*
*  函 数 名 : FindMax
*  功能描述 : 查找树中最大的节点
*  参数列表 : T         -- 树的根结点
*  说    明 : 
*  返回结果 : 如果成功返回指向该树中最大节点的指针，否则返回NULL
*
*******************************************************************************/
AberrTreeNode * AberranceTree::FindMax(IN AberrTreeNode * T)
{
    if (m_pRoot == m_pSentinel || T == m_pSentinel)
    {
        return NULL ;
    }

    while (T->right != m_pSentinel )
    {
        T = T->right ;
    }

    return T ;
}

/*******************************************************************************
*
*  函 数 名 : getGrandParent
*  功能描述 : 查找指定节点的祖父节点
*  参数列表 : pNode         -- 树的节点
*  说    明 : 
*  返回结果 : 如果成功返回指向该节的祖父结点
*
*******************************************************************************/
AberrTreeNode * AberranceTree::getGrandParent(IN const AberrTreeNode * pNode)
{
    if (pNode != m_pSentinel && pNode->parent != m_pSentinel)
    {
        return pNode->parent->parent ;
    }

    return m_pSentinel ;
}

/*******************************************************************************
*
*  函 数 名 : getUncle
*  功能描述 : 查找指定节点的叔叔结点
*  参数列表 : pNode         -- 树的节点
*  说    明 : 
*  返回结果 : 如果成功返回指向该节点的叔叔结点
*
*******************************************************************************/
AberrTreeNode * AberranceTree::getUncle(IN const AberrTreeNode * pNode)
{
    AberrTreeNode * p = getGrandParent(pNode) ;

    if (p == m_pSentinel)
    {
        return m_pSentinel ;
    }

    if (p->left == pNode->parent)
    {
        return p->right ;
    }
    else
    {
        return p->left ;
    }

    return m_pSentinel ;
}

/*******************************************************************************
*
*  函 数 名 : getSibling
*  功能描述 : 查找指定节点的兄弟结点
*  参数列表 : pNode         -- 树的节点
*  说    明 : 
*  返回结果 : 如果成功返回指向该节点的兄弟结点
*
*******************************************************************************/
AberrTreeNode * AberranceTree::getSibling(IN const AberrTreeNode * pNode)
{
    if (pNode == m_pSentinel || pNode->parent == m_pSentinel || pNode == m_pRoot)
    {
        return m_pSentinel ;
    }

    if (pNode->parent->left == pNode)
    {
        return pNode->parent->right ;
    }
    else
    {
        return pNode->parent->left ;
    }

    return m_pSentinel ;
}

/*******************************************************************************
*
*  函 数 名 : insertFixup
*  功能描述 : 插入着色
*  参数列表 : pNode         -- 树的节点
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void AberranceTree::insertFixup(IN AberrTreeNode * pNode)
{
        if (pNode == m_pSentinel)
        {
            return; // impossible actually.   
        }

        AberrTreeNode * pUncle = m_pSentinel;    
        AberrTreeNode * pGrandParent = NULL;   
        while (pNode != m_pRoot && b_red == pNode->parent->color)    
        {   
            pUncle = getUncle(pNode);   
            pGrandParent = getGrandParent(pNode);   
            if (pUncle != NULL && pUncle != m_pSentinel && pUncle->color == b_red)   
            {   
                pNode->parent->color = b_black;   
                pUncle->color = b_black;   
                pGrandParent->color = b_red;   
                pNode = pGrandParent;   
            }   
            else  
            {   
                if (pNode->parent == pGrandParent->left)     
                {   
                    if (pNode == pNode->parent->right)   
                    {   
                        pNode = pNode->parent;   
                        rotateLeft(pNode);   
                    }   
                    pNode->parent->color = b_black;   
                    pGrandParent->color = b_red;   
                    rotateRight(pGrandParent);   
                }   
                else  
                {   
                    if (pNode == pNode->parent->left)   
                    {   
                        pNode = pNode->parent;   
                        rotateRight(pNode);   
                    }   
                    pNode->parent->color = b_black;   
                    pGrandParent->color = b_red;   
                    rotateLeft(pGrandParent);   
                }   
            }   
        }   
        m_pRoot->color = b_black;   
}

/*******************************************************************************
*
*  函 数 名 : removeFixup
*  功能描述 : 移除着色
*  参数列表 : pNode         -- 树的节点
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void AberranceTree::removeFixup(IN AberrTreeNode * pNode)
{
    AberrTreeNode * pSibling = NULL;   
    while ((pNode != m_pRoot) && (pNode->color == b_black))   
    {   
        pSibling = getSibling(pNode);   
        if (pNode == pNode->parent->left) // left child node   
        {   
            if (pSibling->color == b_red)   
            {   
                // case 1, can change to case 2, 3, 4   
                pNode->parent->color = b_red;   
                pSibling->color = b_black;   
                rotateLeft(pNode->parent);   
                // change to new sibling,    
                pSibling = pNode->parent->right;   
            }   
            // case 2;   
            if ((b_black == pSibling->left->color) && (b_black == pSibling->right->color))    
            {    
                pSibling->color = b_red;    
                pNode = pNode->parent;    
            }   
            else  
            {   
                if (b_black == pSibling->right->color)    
                {    
                    pSibling->color = b_red;    
                    pSibling->left->color = b_black;    
                    rotateRight(pSibling);    
                    pSibling = pNode->parent->right;    
                }   
                pSibling->color = pNode->parent->color;   
                pNode->parent->color = b_black;   
                pSibling->right->color = b_black;   
                rotateLeft(pNode->parent);   
                break;    
            }   
        }   
        else  
        {   
            if (pSibling->color == b_red)   
            {   
                // case 1, can change to case 2, 3, 4   
                pNode->parent->color = b_red;   
                pSibling->color = b_black;   
                rotateRight(pNode->parent);   
                // change to new sibling,    
                pSibling = pNode->parent->left;   
            }   
            // case 2;   
            if ((b_black == pSibling->left->color) && (b_black == pSibling->right->color))    
            {    
                pSibling->color = b_red;    
                pNode = pNode->parent;    
            }   
            else  
            {   
                if (b_black == pSibling->left->color)    
                {    
                    pSibling->color = b_red;    
                    pSibling->right->color = b_black;    
                    rotateLeft(pSibling);    
                    pSibling = pNode->parent->left;    
                }   
                pSibling->color = pNode->parent->color;   
                pNode->parent->color = b_black;   
                pSibling->left->color = b_black;   
                rotateRight(pNode->parent);   
                break;    
            }   
        }   
    }   
    pNode->color = b_black;   
}

/*******************************************************************************
*
*  函 数 名 : rotateLeft
*  功能描述 : 左旋转
*  参数列表 : pNode         -- 树的节点
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void AberranceTree::rotateLeft(IN AberrTreeNode * pNode)
{
        if (pNode == m_pSentinel || pNode == NULL || pNode->right == m_pSentinel \
            || pNode->right == NULL)
        {
                return ;
        }

        AberrTreeNode * x = pNode ;
        AberrTreeNode * y = pNode->right ;

        x->right = y->left ;

        if (y->left != m_pSentinel)
        {
                y->left->parent = x ;
        }

        y->left = x ;
        y->parent = x->parent ;
        x->parent = y ;

        if (x == y->parent->left)
        {
                y->parent->left = y ;
        }
        else
        {
                y->parent->right = y ;
        }

        if (pNode == m_pRoot)
        {
                m_pRoot = y ;
        }
        return ;
}

/*******************************************************************************
*
*  函 数 名 : rotateRight
*  功能描述 : 右旋转
*  参数列表 : pNode         -- 树的节点
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
void AberranceTree::rotateRight(IN AberrTreeNode * pNode)
{
        if (pNode == m_pSentinel || pNode->left == m_pSentinel)
        {
                return ;
        }

        AberrTreeNode * x = pNode->left ;
        AberrTreeNode * y = pNode ;
        y->left = x->right ;

        if (x->right != m_pSentinel)
        {
                x->right->parent = y ;
        }

        x->right = y ;
        x->parent = y->parent ;
        y->parent = x ;

        if (y == x->parent->left)
        {
                x->parent->left = x ;
        }
        else
        {
                x->parent->right = x ;
        }

        if (pNode == m_pRoot)
        {
                m_pRoot = x ;
        }
        return ;
}

// 先
void AberranceTree::Preorder(void)
{
    preorder(m_pRoot) ;
}

// 后
void AberranceTree::Postorder(void)
{
    postorder(m_pRoot) ;
}

// 中
void AberranceTree::Inorder(void)
{
    inorder(m_pRoot) ;
}

void AberranceTree::inorder(IN AberrTreeNode * T)
{
    if (T != m_pSentinel)
    {
        inorder(T->left) ;
        printf("%p  ", T->nAddress) ;
        for (int i = 0; i < 12; ++i)
        {
                printf("%02X", T->szDump[i]) ;
        }
        printf(" %6s  %p\r\n", ((T->color == b_red)? "red":"black"), T->jmpconst) ;
        inorder(T->right) ;
    }
}

void AberranceTree::postorder(IN AberrTreeNode * T)
{
    if (T != m_pSentinel)
    {
        postorder(T->left) ;
        postorder(T->right) ;
        printf("%p  ", T->nAddress) ;
        for (int i = 0; i < 12; ++i)
        {
                printf("%02X", T->szDump[i]) ;
        }
        printf(" %6s  %p\r\n", ((T->color == b_red)? "red":"black"), T->jmpconst) ;
    }
}

void AberranceTree::preorder(IN AberrTreeNode * T)
{
    if (T != m_pSentinel)
    {
        printf("%p  ", T->nAddress) ;
        for (int i = 0; i < 12; ++i)
        {
                printf("%02X", T->szDump[i]) ;
        }
        printf(" %6s  %p\r\n", ((T->color == b_red)? "red":"black"), T->jmpconst) ;
        preorder(T->left) ;
        preorder(T->right) ;
    }
}

/*******************************************************************************
*
*  函 数 名 : Delete
*  功能描述 : 删除指定节点
*  参数列表 : T                -- 指向树的指针
              nAddress         -- 要删除节点的值
*  说    明 : 
*  返回结果 : 如果函数执行成功返回1，否则返回0
*
*******************************************************************************/
int AberranceTree::Delete(IN AberrTreeNode *T, IN unsigned int nAddress)
{
        if (T == NULL || T == m_pSentinel)
        {
                return 0;
        }

        while(T != m_pSentinel)
        {
                if (nAddress < T->nAddress)
                {
                        T = T->left ;
                }
                else if (nAddress > T->nAddress)
                {
                        T = T->right ;
                }
                else
                {
                        break ;
                }
        }

        if (T == m_pSentinel)
        {
                return 0;
        }

        if (T->left != m_pSentinel && T->right != m_pSentinel)
        {
                AberrTreeNode * p = FindMin(T->right) ;
                if (p != m_pSentinel && p != NULL)
                {
                        T->nAddress = p->nAddress;
                        Delete(T->right, T->nAddress) ;
                }
        }

        // 只有左儿子
        else if (T->left)
        {
                AberrTreeNode * p = T ;

                if (m_pRoot == T)
                {
                        m_pRoot = T->left ;
                        m_pRoot->parent = m_pSentinel ;
                        delete T ;
                }
                else
                {
                        T->left->parent = T->parent ;
                        if (T->parent->left == T)
                        {
                                T->parent->left = T->left ;
                        }
                        else
                        {
                                T->parent->right = T->left ;
                        }
                        delete p ;
                 }
        }

        else if (T->right)
        {
                if (m_pRoot == T)
                {
                        m_pRoot = T->left ;
                        m_pRoot->parent = m_pSentinel ;
                        delete T ;
                }
                else
                {
                        AberrTreeNode * p = T ;
                        T->right->parent = T->parent ;
                        if (T->parent->left == T)
                        {
                                T->parent->left = T->right ;
                        }
                        else
                        {
                                T->parent->right = T->right ;
                        }
                        delete p ;
                }
        }
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : Clear
*  功能描述 : 清空树
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 成功返回1，失败返回0
*
*******************************************************************************/
int AberranceTree::Clear()
{
        if (m_pRoot != m_pSentinel)
        {
                Destory(m_pRoot) ;
                m_pRoot = NULL ;
        }
        if (m_pSentinel != NULL)
        {
                delete m_pSentinel ;
                m_pSentinel = NULL ;
        }

        m_pSentinel = new AberrTreeNode ;
        if (m_pSentinel == NULL)
        {
                exit(EXIT_FAILURE) ;
        }
        m_pSentinel->color = b_black ;
        m_pSentinel->left   = m_pSentinel ;
        m_pSentinel->right  = m_pSentinel ;
        m_pSentinel->parent = m_pSentinel ;
        m_pRoot = m_pSentinel ;
        m_pHead = NULL ;
        m_pLast = NULL ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : Find
*  功能描述 : 查找
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 成功返回树的节点，失败返回NULL
*
*******************************************************************************/
AberrTreeNode * AberranceTree::Find(IN unsigned int _nAddr)
{
        AberrTreeNode * p = m_pRoot ;
        for(;m_pSentinel != p;)
        {
                if (_nAddr == p->nAddress)
                {
                        return p ;
                }
                else if (_nAddr < p->nAddress)
                {
                        p = p->left ;
                }
                else
                {
                        p = p->right ;
                }
        }
        return NULL ;
}

/*******************************************************************************
*
*  函 数 名 : FindEx
*  功能描述 : 扩展查找
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 成功返回树的节点，失败返回NULL
*
*******************************************************************************/
AberrTreeNode * AberranceTree::FindEx(IN unsigned int _nAddr, 
                                      IN char *pSzDump,
                                      IN int nInstrLen)
{
        AberrTreeNode * p = m_pRoot ;
        for(;m_pSentinel != p;)
        {
                if (_nAddr == p->nAddress && 0 == memcmp(p->szDump, pSzDump, 
                                                        sizeof(char) * nInstrLen))
                {
                        return p ;
                }
                else if (_nAddr < p->nAddress)
                {
                        p = p->left ;
                }
                else
                {
                        p = p->right ;
                }
        }
        return NULL ;
}

/*******************************************************************************
*
*  函 数 名 : SetDump
*  功能描述 : 设置Dump数据
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 成功返回树的节点，失败返回NULL
*
*******************************************************************************/
int AberrTreeNode::SetDump(IN char *pDump, IN int nLen)
{
        if (NULL == this)
        {
                return 0 ;
        }
        if (NULL == pDump)
        {
                return 0 ;
        }

        memset(szDump, 0, sizeof(char)*DUMPSIZE) ;
        memcpy(szDump, pDump, sizeof(char) * nLen ) ;
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : DisplayLink
*  功能描述 : 输出链表
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 成功返回树的节点，失败返回NULL
*
*******************************************************************************/
int AberranceTree::DisplayLink()
{
        if (NULL == m_pHead)
        {
                return 0 ;
        }

        AberrTreeNode *p = m_pHead ;
        for (; p != NULL; p = p->pNext)
        {
                printf("%p  ", p->nAddress) ;
                for (int i = 0; i < 12; ++i)
                {
                        printf("%02X", p->szDump[i]) ;
                }
                printf(" %6s  %p\r\n", ((p->color == b_red)? "red":"black"), p->jmpconst) ;
        }

        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : InsertLink
*  功能描述 : 将树的结点加到链表中去
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 成功返回树的节点，失败返回NULL
*
*******************************************************************************/
int AberranceTree::InsertLink(IN AberrTreeNode *p)
{
        if (NULL == p)
        {
                return 0 ;
        }
        // 往链表里插
        // 如果链表是空的话
        if (NULL == m_pHead)
        {
                m_pHead = p ;
                m_pLast = p ;
        }
        // 否则往尾部插
        else
        {
                m_pLast->pNext = p ;
                m_pLast = p ;
        }
        return 1 ;
}

/*******************************************************************************
*
*  函 数 名 : GetListHead
*  功能描述 : 取得链表头结点
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 链表的头结点
*
*******************************************************************************/
AberrTreeNode * AberranceTree::GetListHead()
{
        return m_pHead ;
}
