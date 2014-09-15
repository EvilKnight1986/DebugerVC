/*******************************************************************************
*  
*  Copyright (c) all 2010 ���� All rights reserved
*  FileName : RedBlackTree.h
*  D a t e  : 2010.3.21
*  ��   ��  : ͷ�ļ�
*  ˵   ��  : ������¼ָ��
*
*
*******************************************************************************/

#if !defined(ABERRANCETREE_H__36010BDB_D651_426D_94D2_2CED692385AD__INCLUDED_)
#define ABERRANCETREE_H__36010BDB_D651_426D_94D2_2CED692385AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Disasm/disasm.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#define IN
#define OUT

enum btn_color { b_red, b_black} ;
#define DUMPSIZE        16

class AberrTreeNode
{
public:
	int SetDump(IN char *pDump, IN int nLen) ;

public:
        AberrTreeNode():left(NULL),right(NULL),parent(NULL),pNext(NULL),color(b_red),
                nAddress(0), jmpconst(0),nCount(1)
        {
        }
        AberrTreeNode(int _nAddr):left(NULL),right(NULL),parent(NULL),pNext(NULL),
                                color(b_red), nAddress(_nAddr), jmpconst(0),nCount(1)
        {
        }
        ~AberrTreeNode()
        {
        }
public:
        AberrTreeNode *          left     ;                 // ���������
        AberrTreeNode *          right    ;                 // �����Ҷ���
        AberrTreeNode *          parent   ;                 // ���游���
        AberrTreeNode *          pNext    ;                 // ����������һ�����
        btn_color                color    ;                 // ������ɫֵ
        unsigned int             nAddress ;                 // ���溯�������ĵ�ַ
        char                     szDump[DUMPSIZE] ;         // Hexadecimal dump of the command
        void *                   jmpconst ;                 // Constant jump address
        unsigned int             nCount   ;                 // ��¼ָ���ܹ��Ĵ���
};

class AberranceTree
{
public:
        int             InsertLink(IN AberrTreeNode *p) ;
        int             DisplayLink(void) ;
	int             SetDump(IN char *pDump) ;
        int             Clear(IN void) ;
        void            RecursionPreorder(IN AberrTreeNode * T) ;
        void            Inorder(void) ;
        void            Postorder(void) ;
        void            Preorder(void) ;
        int             Remove(IN unsigned int _nAddr) ;
        AberrTreeNode * GetListHead(void);
        AberrTreeNode * Insert(IN unsigned int _nAddr, IN char *pDump, IN int nLen) ;
        AberrTreeNode * Find(IN unsigned int _nAddr) ;
        AberrTreeNode * FindEx(IN unsigned int _nAddr, IN char *pSzDump, IN int nInstrLen) ;
        AberrTreeNode * FindMax(IN AberrTreeNode * T) ;
        AberrTreeNode * FindMin(IN AberrTreeNode * T) ;
public:
        AberranceTree()  ;
        ~AberranceTree() ;
private:
        int             Delete(IN AberrTreeNode *T, IN unsigned int _nAddr) ;
        void            rotateRight(IN AberrTreeNode * pNode) ;
        void            rotateLeft(IN AberrTreeNode  * pNode) ;
        void            removeFixup(IN AberrTreeNode * pNode) ;
        void            insertFixup(IN AberrTreeNode * pNode) ;
        void            Destory(IN AberrTreeNode *T)   ;
        void            inorder(IN AberrTreeNode * T)  ;
        void            postorder(IN AberrTreeNode * T);
        void            preorder(IN AberrTreeNode * T) ;
        AberrTreeNode * getGrandParent (IN const AberrTreeNode *pNode) ;
        AberrTreeNode * getSibling(IN const AberrTreeNode * pNode) ;
        AberrTreeNode * getUncle(IN const AberrTreeNode * pNode) ;
        AberrTreeNode & operator = (IN const AberrTreeNode &obj){} ;
private:
        AberrTreeNode * m_pSentinel ;
        AberrTreeNode * m_pRoot ;               // ָ�����ĸ����
        AberrTreeNode * m_pHead ;               // ָ������ͷ
        AberrTreeNode * m_pLast ;               // ָ������β
};

#endif // !defined(ABERRANCETREE_H__36010BDB_D651_426D_94D2_2CED692385AD__INCLUDED_)
