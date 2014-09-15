/******************************************************************************
* Copyright (c) 2010, ���� ,All right reserved.
*
* �� �� ��: entry.c
*
* ��������: ��������ں���
*
* �� �� ��: ����,  2010�� 3�� 4��
*
* ��������: 2010. 3. 4
*
* �� �� ��: 1.0
*
* �޸ļ�¼: 
*
* ���ﻷ����Windows XP SP2�� VC 6.0 sp6
* 
*******************************************************************************/

#include "debug.h"
#include <stdio.h>
#include <windows.h>

int main(int argc, char *argv[])
{
        Debug dbg ;
        if (2 == argc)
        {
                dbg.SetExePathName(argv[1]) ;
        }
        else
        {
                char szFileName[256] = "" ;
                OPENFILENAME file = {0} ;
                file.lpstrFile = szFileName ;
                file.lStructSize = sizeof(OPENFILENAME) ;
                file.nMaxFile = 256 ;
                file.lpstrFilter = "Executables\0*.exe\0All Files\0*.*\0\0" ;
                file.nFilterIndex = 1 ;
                if(::GetOpenFileName(&file))
                {
                        dbg.SetExePathName(szFileName) ;
                }

        }
        dbg.StartDebug() ;
        system("pause") ;
        return 0 ;
}
