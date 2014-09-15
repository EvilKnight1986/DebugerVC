/******************************************************************************
* Copyright (c) 2010, 黄奇 ,All right reserved.
*
* 文 件 名: entry.c
*
* 功能描述: 调试器入口函数
*
* 创 建 人: 黄奇,  2010年 3月 4日
*
* 创建日期: 2010. 3. 4
*
* 版 本 号: 1.0
*
* 修改记录: 
*
* 编绎环境：Windows XP SP2、 VC 6.0 sp6
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
