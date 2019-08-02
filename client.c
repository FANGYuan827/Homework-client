
/***************************************************************************************
****************************************************************************************
* FILE     : client.c
* Description  : 
*            
* Copyright (c) 2019 by Hikvision. All Rights Reserved.
* 
* History:
* Version      Name        Date                Description
   0.1         fangyuan9   2019/07/31          Initial Version 1.0.0
   
****************************************************************************************
****************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "common.h"
#include "tcpcli.h"
#include "udpcli.h"
#include "multicast.h"


/*==================================================================
* Function      : main   
* Description   : 主函数
* Input Para    : 
* Output Para   : 无
* Return Value  : 无
==================================================================*/
int main(int argc,const char *argv[])
{
    char UserInput = '\0';
    char cBuf = 0;              //用于清空输入尾部的换行符
	uint16_t UserInputNum = 0;
    uint16_t ServerNum = 0;        //选择服务编号变量
    uint16_t ServerCount = 0;   //在线服务器总数
    stServerNode *p_ServerInUse = NULL; //用于存储待连接的服务器信息  pstServer

    stServerNode *p_ServerListHead = (stServerNode*)malloc(sizeof(stServerNode));   //pHead:新建服务器链表头结点
	
    if (NULL == p_ServerListHead)
    {
        printf("内存申请失败\n");
        return -1;
    }

    p_ServerListHead->pstNext = NULL;
	
    while (0 == (ServerCount = ServerSearch(p_ServerListHead)))
    {
        printf("无可用服务器,是否继续搜索?\n输入Y继续,输入其他值结束搜索并退出程序\n");
        scanf("%c", &UserInput);
		
        while (((cBuf = getchar()) != EOF) && (cBuf != '\n'));//使用getchar()获取输入缓冲区字符，直到获取的c是'\n'或文件结尾符EOF为止

        if ('Y' == UserInput)
        {
            UserInput = '\0';
            continue;
        }
		
        else
        {
            printf("无可用服务器，程序退出\n");
            return 0;
        }
    }

    printf("**********可用服务器列表如下************\n");
    PrintNode(p_ServerListHead);
    printf("**********可用服务器列表打印完毕***************\n");
    
    printf("选择需要连接服务器序号:\n");
    scanf("%hu", &ServerNum);
	
    while (ServerNum <= 0 || ServerNum > ServerCount)
    {
        printf("服务器序号输入错误，请重新输入:\n");
        scanf("%hu", &ServerNum);
        while (((cBuf = getchar()) != EOF) && (cBuf != '\n'));
    }

    p_ServerInUse = FindNode(p_ServerListHead, ServerNum);
    printf("准备连接服务器[%s:%hu]\n", p_ServerInUse->pszIP, p_ServerInUse->usiPort);

    /* 全局变量内存申请 */
	
    /* 文件信息结构 */
    g_pstComTransInfo = (COM_TRANS_INFO_S*)malloc(sizeof(COM_TRANS_INFO_S));
    
	if (NULL == g_pstComTransInfo)
    {
        printf("内存申请失败\n");
        return -1;
    }

    /* 发送或接收缓存 */
    g_pszTransBuf = (char*)malloc(BUFFER_SIZE);
    if(NULL == g_pszTransBuf)
    {
        printf("内存申请失败！\n");
        return -1;
    }

    /*用于存储计算接收后文件的摘要 */
    g_pszSha1Digest = (char*)malloc(COM_SHA1DIGEST_LEN);
    if(NULL == g_pszSha1Digest)
    {
        printf("内存申请失败！\n");
        return -1;
    }

    /* 全局变量内存初始化 */
    memset((void*)g_pstComTransInfo, 0, sizeof(COM_TRANS_INFO_S));
    memset(g_pszTransBuf, 0, BUFFER_SIZE);
    memset(g_pszSha1Digest, 0, COM_SHA1DIGEST_LEN);

	
    ProtocolMenu();
    scanf("%hu", &UserInputNum);
	
	/* 根据输入序号选择不同协议 */
    if (1 == UserInputNum)
    {
        UDPService(p_ServerInUse);
    }
	
    else if(2 == UserInputNum)
    {
        TCPService(p_ServerInUse);
    }

	
	/* 程序结束之前，释放所有资源 */
    DeleteList(p_ServerListHead);  

	if(NULL != g_pstComTransInfo)
    {
		free(g_pstComTransInfo);
		g_pstComTransInfo = NULL;
	}
	
	if(NULL != g_pszTransBuf)
    {
		free(g_pszTransBuf);
		g_pszTransBuf = NULL;
	}
    
	if(NULL != g_pszSha1Digest)
    {
		free(g_pszSha1Digest);
		g_pszSha1Digest = NULL;
	}
	
    return 0;
}

/************************ (C) COPYRIGHT HIKVISION *****END OF FILE****/

