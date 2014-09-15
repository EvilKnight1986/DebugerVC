/*******************************************************************************
*  
*  Copyright (c) all 2009 ���� All rights reserved
*  FileName : RedBlackTree.h
*  D a t e  : 2010.3.21
*  ��   ��  : ����������ļ�
*  ˵   ��  : 
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
*  �� �� �� : RedBlackTree
*  �������� : RedBlackTree���캯��������һ�ÿյĺ����
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ��
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
*  �� �� �� : ~RedBlackTre
*  �������� : RedBlackTree��������������һ�ÿյĺ����
*  �����б� : ��
*  ˵    �� : 
*  ���ؽ�� : ��
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
*  �� �� �� : Insert
*  �������� : ����������һ���ڵ�
*  �����б� : _nAddr    -- Ҫ�����ֵ
              _pFunName -- ָ�������ĵ�ַ
              _nBelongDll -- ������dll���
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
int RedBlackTree::Insert(unsigned int _nAddr, int _pFunName, int _nBelongDll)
{
        TreeNode * pNode = m_pRoot ;
        TreeNode * tmp = NULL ;

        // �����Ϊ�յĻ�
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
        // ����߲���
        if (_nAddr < pNode->nAddress && pNode->left == m_pSentinel)
        {
                pNode->left = tmp ;
        }
        // ���ұ߲���
        else if (_nAddr > pNode->nAddress && pNode->right == m_pSentinel)
        {
                pNode->right = tmp ;
        }
        insertFixup(tmp) ;
        return  1;
}

/*******************************************************************************
*
*  �� �� �� : Remove
*  �������� : �Ƴ����е�ָ���ڵ�
*  �����б� : _nAddr    -- Ҫ�Ƴ���ֵ
*  ˵    �� : 
*  ���ؽ�� : ����Ƴ��ˣ�����1�����򷵻�0
*
*******************************************************************************/
int RedBlackTree::Remove(unsigned int _nAddr)
{
        return Delete(m_pRoot, _nAddr) ;

}

/*******************************************************************************
*
*  �� �� �� : Destory
*  �������� : ����������
*  �����б� : T         -- ���ĸ����
*  ˵    �� : 
*  ���ؽ�� : 
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
*  �� �� �� : FindMin
*  �������� : ����������С�Ľڵ�
*  �����б� : T         -- ���ĸ����
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����ָ���������С�ڵ��ָ�룬���򷵻�NULL
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
*  �� �� �� : FindMax
*  �������� : �����������Ľڵ�
*  �����б� : T         -- ���ĸ����
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����ָ����������ڵ��ָ�룬���򷵻�NULL
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
*  �� �� �� : getGrandParent
*  �������� : ����ָ���ڵ���游�ڵ�
*  �����б� : pNode         -- ���Ľڵ�
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����ָ��ýڵ��游���
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
*  �� �� �� : getUncle
*  �������� : ����ָ���ڵ��������
*  �����б� : pNode         -- ���Ľڵ�
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����ָ��ýڵ��������
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
*  �� �� �� : getSibling
*  �������� : ����ָ���ڵ���ֵܽ��
*  �����б� : pNode         -- ���Ľڵ�
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����ָ��ýڵ���ֵܽ��
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

// ��
void RedBlackTree::Preorder(void)
{
    preorder(m_pRoot) ;
}

// ��
void RedBlackTree::Postorder(void)
{
    postorder(m_pRoot) ;
}

// ��
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
*  �� �� �� : Delete
*  �������� : ɾ��ָ���ڵ�
*  �����б� : T                -- ָ������ָ��
              nAddress         -- Ҫɾ���ڵ��ֵ
*  ˵    �� : 
*  ���ؽ�� : �������ִ�гɹ�����1�����򷵻�0
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

        // ֻ�������
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
