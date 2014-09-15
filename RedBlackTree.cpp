/*******************************************************************************
*  
*  Copyright (c) all 2009 黄奇 All rights reserved
*  FileName : RedBlackTree.h
*  D a t e  : 2010.3.21
*  功   能  : 红黑树定义文件
*  说   明  : 
*
*
*******************************************************************************/

#include "RedBlackTree.h"
#include "Tools.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
*  函 数 名 : RedBlackTree
*  功能描述 : RedBlackTree构造函数，构造一棵空的红黑树
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
RedBlackTree::RedBlackTree():m_pSentinel(NULL), m_pRoot(NULL)
{
    m_pSentinel = new TreeNode ;

    if (m_pSentinel == NULL)
    {
        exit(EXIT_FAILURE) ;
    }

    m_pSentinel->color  = black ;
    m_pSentinel->left   = m_pSentinel ;
    m_pSentinel->right  = m_pSentinel ;
    m_pSentinel->parent = m_pSentinel ;
    m_pRoot = m_pSentinel ;
}

/*******************************************************************************
*
*  函 数 名 : ~RedBlackTre
*  功能描述 : RedBlackTree析构函数，构造一棵空的红黑树
*  参数列表 : 无
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
RedBlackTree::~RedBlackTree()
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
}

/*******************************************************************************
*
*  函 数 名 : Insert
*  功能描述 : 向红黑树插入一个节点
*  参数列表 : _nAddr    -- 要插入的值
              _pFunName -- 指向函数名的地址
              _nBelongDll -- 所属的dll序号
*  说    明 : 
*  返回结果 : 无
*
*******************************************************************************/
int RedBlackTree::Insert(unsigned int _nAddr, int _pFunName, int _nBelongDll)
{
        TreeNode * pNode = m_pRoot ;
        TreeNode * tmp = NULL ;

        // 如果树为空的话
        if (m_pRoot == m_pSentinel)
        {
                tmp = new TreeNode(_nAddr, _pFunName, _nBelongDll) ;
                if (tmp == NULL)
                {
                exit(EXIT_FAILURE) ;
                }

                tmp->left = tmp->right = tmp->parent = m_pSentinel ;
                tmp->color = red ;

                m_pRoot = tmp ;
                return 1 ;
        }

        for (; pNode != m_pSentinel;)
        {
                if (_nAddr < pNode->nAddress && pNode->left != m_pSentinel)
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
        tmp = new TreeNode(_nAddr, _pFunName, _nBelongDll) ;
        if (tmp == NULL)
        {
                exit(EXIT_FAILURE) ;
        }
        tmp->left = tmp->right = m_pSentinel ;
        tmp->color = red ;
        tmp->parent = pNode ;
        // 往左边插入
        if (_nAddr < pNode->nAddress && pNode->left == m_pSentinel)
        {
                pNode->left = tmp ;
        }
        // 入右边插入
        else if (_nAddr > pNode->nAddress && pNode->right == m_pSentinel)
        {
                pNode->right = tmp ;
        }
        insertFixup(tmp) ;
        return  1;
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
int RedBlackTree::Remove(unsigned int _nAddr)
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
void RedBlackTree::Destory(TreeNode *T)
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
*  参数列表 : T         -- 树的根结点
*  说    明 : 
*  返回结果 : 如果成功返回指向该树中最小节点的指针，否则返回NULL
*
*******************************************************************************/
TreeNode * RedBlackTree::FindMin(TreeNode * T)
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
TreeNode * RedBlackTree::FindMax(TreeNode * T)
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
TreeNode * RedBlackTree::getGrandParent(const TreeNode * pNode)
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
TreeNode * RedBlackTree::getUncle(const TreeNode * pNode)
{
    TreeNode * p = getGrandParent(pNode) ;
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
TreeNode * RedBlackTree::getSibling(const TreeNode * pNode)
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

void RedBlackTree::insertFixup(TreeNode * pNode)
{
        if (pNode == m_pSentinel)
        {
            return; // impossible actually.   
        }

        TreeNode * pUncle = m_pSentinel;    
        TreeNode * pGrandParent = NULL;   
        while (pNode != m_pRoot && red == pNode->parent->color)    
        {   
            pUncle = getUncle(pNode);   
            pGrandParent = getGrandParent(pNode);   
            if (pUncle != NULL && pUncle != m_pSentinel && pUncle->color == red)   
            {   
                pNode->parent->color = black;   
                pUncle->color = black;   
                pGrandParent->color = red;   
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
                    pNode->parent->color = black;   
                    pGrandParent->color = red;   
                    rotateRight(pGrandParent);   
                }   
                else  
                {   
                    if (pNode == pNode->parent->left)   
                    {   
                        pNode = pNode->parent;   
                        rotateRight(pNode);   
                    }   
                    pNode->parent->color = black;   
                    pGrandParent->color = red;   
                    rotateLeft(pGrandParent);   
                }   
            }   
        }   
        m_pRoot->color = black;   
}

void RedBlackTree::removeFixup(TreeNode * pNode)
{
    TreeNode * pSibling = NULL;   
    while ((pNode != m_pRoot) && (pNode->color == black))   
    {   
        pSibling = getSibling(pNode);   
        if (pNode == pNode->parent->left) // left child node   
        {   
            if (pSibling->color == red)   
            {   
                // case 1, can change to case 2, 3, 4   
                pNode->parent->color = red;   
                pSibling->color = black;   
                rotateLeft(pNode->parent);   
                // change to new sibling,    
                pSibling = pNode->parent->right;   
            }   
            // case 2;   
            if ((black == pSibling->left->color) && (black == pSibling->right->color))    
            {    
                pSibling->color = red;    
                pNode = pNode->parent;    
            }   
            else  
            {   
                if (black == pSibling->right->color)    
                {    
                    pSibling->color = red;    
                    pSibling->left->color = black;    
                    rotateRight(pSibling);    
                    pSibling = pNode->parent->right;    
                }   
                pSibling->color = pNode->parent->color;   
                pNode->parent->color = black;   
                pSibling->right->color = black;   
                rotateLeft(pNode->parent);   
                break;    
            }   
        }   
        else  
        {   
            if (pSibling->color == red)   
            {   
                // case 1, can change to case 2, 3, 4   
                pNode->parent->color = red;   
                pSibling->color = black;   
                rotateRight(pNode->parent);   
                // change to new sibling,    
                pSibling = pNode->parent->left;   
            }   
            // case 2;   
            if ((black == pSibling->left->color) && (black == pSibling->right->color))    
            {    
                pSibling->color = red;    
                pNode = pNode->parent;    
            }   
            else  
            {   
                if (black == pSibling->left->color)    
                {    
                    pSibling->color = red;    
                    pSibling->right->color = black;    
                    rotateLeft(pSibling);    
                    pSibling = pNode->parent->left;    
                }   
                pSibling->color = pNode->parent->color;   
                pNode->parent->color = black;   
                pSibling->left->color = black;   
                rotateRight(pNode->parent);   
                break;    
            }   
        }   
    }   
    pNode->color = black;   
}

void RedBlackTree::rotateLeft(TreeNode * pNode)
{
    if (pNode == m_pSentinel || pNode == NULL || pNode->right == m_pSentinel \
            || pNode->right == NULL)
    {
        return ;
    }
    TreeNode * x = pNode ;
    TreeNode * y = pNode->right ;

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
}

void RedBlackTree::rotateRight(TreeNode * pNode)
{
    if (pNode == m_pSentinel || pNode->left == m_pSentinel)
    {
        return ;
    }
    
    TreeNode * x = pNode->left ;
    TreeNode * y = pNode ;
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
}

// 先
void RedBlackTree::Preorder(void)
{
    preorder(m_pRoot) ;
}

// 后
void RedBlackTree::Postorder(void)
{
    postorder(m_pRoot) ;
}

// 中
void RedBlackTree::Inorder(void)
{
    inorder(m_pRoot) ;
}

void RedBlackTree::inorder(TreeNode * T)
{
    if (T != m_pSentinel)
    {
        inorder(T->left) ;
        printf("%p %-40s %6s\r\n", T->nAddress, T->pFunName,((T->color == red)? "red":"black")) ;
        inorder(T->right) ;
    }
}

void RedBlackTree::postorder(TreeNode * T)
{
    if (T != m_pSentinel)
    {
        postorder(T->left) ;
        postorder(T->right) ;
        printf("%p %-40s %6s\r\n", T->nAddress, T->pFunName,((T->color == red)? "red":"black")) ;
    }
}

void RedBlackTree::preorder(TreeNode * T)
{
    if (T != m_pSentinel)
    {
        //cout << T->nAddress << " " << ((T->color == red)? "red":"black") << endl ;
        printf("%p %-40s %6s\r\n", T->nAddress, T->pFunName,((T->color == red)? "red":"black")) ;
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
int RedBlackTree::Delete(TreeNode *T, unsigned int nAddress)
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
                TreeNode * p = FindMin(T->right) ;
                if (p != m_pSentinel && p != NULL)
                {
                        T->nAddress = p->nAddress;
                        Delete(T->right, T->nAddress) ;
                }
        }

        // 只有左儿子
        else if (T->left)
        {
                TreeNode * p = T ;

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
                        TreeNode * p = T ;
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

int RedBlackTree::Clear()
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

        m_pSentinel = new TreeNode ;
        if (m_pSentinel == NULL)
        {
                exit(EXIT_FAILURE) ;
        }
        m_pSentinel->color = black ;
        m_pSentinel->left   = m_pSentinel ;
        m_pSentinel->right  = m_pSentinel ;
        m_pSentinel->parent = m_pSentinel ;
        m_pRoot = m_pSentinel ;
        return 1 ;
}

TreeNode * RedBlackTree::Find(unsigned int _nAddr)
{
        TreeNode * p = m_pRoot ;
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
