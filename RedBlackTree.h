/*******************************************************************************
*  
*  Copyright (c) all 2010 ���� All rights reserved
*  FileName : RedBlackTree.h
*  D a t e  : 2010.3.21
*  ��   ��  : �����ͷ�ļ�
*  ˵   ��  : 
*
*
*******************************************************************************/

#if !defined(AFX_REDBLACKTREE_H__36010BDB_D651_426D_94D2_2CED692385AD__INCLUDED_)
#define AFX_REDBLACKTREE_H__36010BDB_D651_426D_94D2_2CED692385AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

enum rg_color { red, black} ;

class TreeNode
{
public:
        TreeNode():left(NULL),right(NULL),parent(NULL),color(red),nAddress(0), 
                pFunName(0),nBelongDll(0)
        {
        }
        TreeNode(int _nAddr, int _pFunName):left(NULL),right(NULL),parent(NULL),
            color(red),nAddress(_nAddr), pFunName(_pFunName),nBelongDll(0)
        {
        }
        TreeNode(int _nAddr, int _pFunName, int _nBelongDll):left(NULL),right(NULL),
                parent(NULL),color(red),nAddress(_nAddr), pFunName(_pFunName),
                nBelongDll(_nBelongDll)
        {
        }
        ~TreeNode()
        {
        }
public:
        TreeNode *          left     ;                 // ���������
        TreeNode *          right    ;                 // �����Ҷ���
        TreeNode *          parent   ;                 // ���游���
        rg_color            color    ;                 // ������ɫֵ
        unsigned int        nAddress ;                 // ���溯�������ĵ�ַ
        int                 pFunName ;                 // ָ������
        int                 nBelongDll ;               // �����ĸ�Dll,����Dll�����
};

class RedBlackTree
{
public:
        int             Clear(void) ;
        void            RecursionPreorder(TreeNode * T) ;
        void            Inorder(void) ;
        void            Postorder(void) ;
        void            Preorder(void) ;
        int             Remove(unsigned int _nAddrt) ;
        int             Insert(unsigned int _nAddr, int _pFunName, int _nBelongDll) ;
        TreeNode *      Find(unsigned int _nAddr);
        TreeNode *      FindMax(TreeNode * T) ;
        TreeNode *      FindMin(TreeNode * T) ;

public:
        RedBlackTree()  ;
        ~RedBlackTree() ;

private:
        int             Delete(TreeNode *T, unsigned int _nAddr) ;
        void            rotateRight(TreeNode * pNode) ;
        void            rotateLeft(TreeNode  * pNode) ;
        void            removeFixup(TreeNode * pNode) ;
        void            insertFixup(TreeNode * pNode) ;
        void            Destory(TreeNode *T)   ;
        void            inorder(TreeNode * T)  ;
        void            postorder(TreeNode * T);
        void            preorder(TreeNode * T) ;
        TreeNode *      getGrandParent (const TreeNode *pNode) ;
        TreeNode *      getSibling(const TreeNode * pNode) ;
        TreeNode *      getUncle(const TreeNode * pNode) ;
        RedBlackTree & operator   =   (const RedBlackTree &obj){} ;
private:
        TreeNode      * m_pSentinel ;
        TreeNode      * m_pRoot ;
};

#endif // !defined(AFX_REDBLACKTREE_H__36010BDB_D651_426D_94D2_2CED692385AD__INCLUDED_)
