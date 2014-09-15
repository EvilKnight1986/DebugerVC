/*******************************************************************************
*  
*  Copyright (c) all 2009 ���� All rights reserved
*  FileName : RedBlackTree.h
*  D a t e  : 2010.3.21
*  ��   ��  : ���������ļ�
*  ˵   ��  : 
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
*  �� �� �� : SetDllName
*  �������� : ��dll�����ָ��Ƶ��ڵ��m_pDllName��
*  �����б� : _pDllName   -- ָ��dll���ֵ��ַ���ָ��
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
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
                OutputDebugString("dll�����ֳ���������\r\n") ;
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
                OutputDebugString("�����ڴ治�ɹ�\r\n") ;
#endif
                return 0 ;
        }

        // ��dll�����ָ��Ƶ��Լ�����Ŀռ���
        memset(m_pDllName, 0, sizeof(char) * nStrLen) ;
        memcpy(m_pDllName, _pDllName, sizeof(char) * (nStrLen-1)) ;

        // ����dll���ֵ�crcֵ
        m_DllNameCrc = cal_crc(m_pDllName) ;
        return 1 ;
}

/*******************************************************************************
*
*  �� �� �� : SetLoadState
*  �������� : ����dll�ĵ�ǰ״̬
*  �����б� : _bStat   -- ��Ҫ��ɵ�״ֵ̬
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
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
*  �� �� �� : SetLoad
*  �������� : ���õ�ǰdll�Ѿ�����
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
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
*  �� �� �� : SetImageBase
*  �������� : ����dll��ַ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����1
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
*  �� �� �� : GetImageBase
*  �������� : ����dll��ַ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����dll��ַ
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
*  �� �� �� : SetUnLoad
*  �������� : ���õ�ǰdllû�м���
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1, ʧ�ܷ���0
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
*  �� �� �� : GetCrc
*  �������� : ȡ��Dll���ֵ�CRCֵ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����CRCֵ
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
*  �� �� �� : GetCrc
*  �������� : ȡ��Dll���ֵ�CRCֵ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ָ��dll���ֵ��ַ���ָ�룬���û�����õĻ���������NULL
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
*  �� �� �� : GetState
*  �������� : ȡ�õ�ǰDll״̬
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ���dll��ǰ�Ѿ����أ�����TRUE,���򷵻�FALSE
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
*  �� �� �� : DispNode
*  �������� : ����ڵ���Ϣ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��
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
*  �� �� �� : MyList
*  �������� : MyList���캯��
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
MyList::MyList():pHead(NULL),pLast(NULL),nSize(0)
{
}

/*******************************************************************************
*
*  �� �� �� : MyList
*  �������� : MyList��������
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��
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
*  �� �� �� : IsEmpty
*  �������� : �ж������Ƿ�Ϊ��
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
int MyList::IsEmpty()
{
        return nSize == 0 ? 1 : 0 ;
}

/*******************************************************************************
*
*  �� �� �� : Find
*  �������� : ��dll������ָ���ڵ�
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�����ָ��ýڵ��ָ�룬���򷵻�NULL
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

        // ��crcֵ��ͬ��ʱ����б�Ҫȥ�ж������Ƿ������ͬ
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
*  �� �� �� : FindPrev
*  �������� : ��dll������ָ���ڵ��ǰ��
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ҵ�����ָ��ýڵ�ǰ����ָ�룬���򷵻�NULL
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
    
        // �Ȱ�crcֵȥ����
        for(; p->next != NULL; p = p->next)
        {
                // ����ҵ�crcֵ��ͬ�ģ�����stricmpȥ�Ƚ��Ƿ������ͬ
                // ��Ϊ���ڲ�ͬ���ִ�������ͬ��ֵ
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
*  �� �� �� : Insert
*  �������� : �������в���һ���µĽڵ�
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : �ɹ������������е�λ��,ʧ�ܷ���-1
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
        // ��������ǿյ������
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
*  �� �� �� : Delete
*  �������� : ��������ɾ��һ��ָ���Ľڵ�
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : �ɹ�����1,ʧ�ܷ���0
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
*  �� �� �� : GetSize
*  �������� : ȡ������ĳ���
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��������ĳ���
*
*******************************************************************************/
int MyList::GetSize()
{
    return nSize ;
}

/*******************************************************************************
*
*  �� �� �� : operator []
*  �������� : ȡ�������е�index��Ԫ�ص�crcֵ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ��������ĳ���
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
*  �� �� �� : GetNode
*  �������� : ȡ�������nIndex�����
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ���ظýڵ��ָ�룬���򷵻�NULL
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
*  �� �� �� : Displa
*  �������� : �����ǰ�Ѿ����ص�DLL����
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : 
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
