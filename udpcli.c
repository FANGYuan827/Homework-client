
/***************************************************************************************
****************************************************************************************
* FILE     : udpcli.c
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
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "common.h"
#include "udpcli.h"
#include "sha1.h"



/* Private define ------------------------------------------------------------*/

#define OPEN_FAIL -1
#define SENDTO_FAIL -1
#define RECVFROM_FAIL -1


/*==================================================================
* Function      : UDPService   
* Description   : 对外提供UDP服务
* Input Para    : p_Server:服务器指针 指向可用服务器
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void UDPService(stServerNode *p_Server) 
{
    assert(p_Server != NULL);

    struct sockaddr_in stServerAddr;
    socklen_t uliSerAddrLen = sizeof(stServerAddr);
    int sockfd;    
    char cBuf;//临时变量，清输入缓存用
	uint16_t OperateNum = 0;
	
    char *p_PathName = (char*)malloc(PATH_MAX);
	
    if(NULL == p_PathName)
    {
        printf("内存申请失败！\n");
        return;
    }
    
    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "%s\n",strerror(errno));
        return;
    }
    
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_addr.s_addr = inet_addr(p_Server->pszIP);
    stServerAddr.sin_port = htons(p_Server->usiPort);
    
    while(1)
    {
		/* 操作选择界面 */
        OperateMenu();
		
        scanf("%hu", &OperateNum);
        while (((cBuf = getchar()) != EOF) && (cBuf != '\n'));

        /* 上传操作 */
        if (1 == OperateNum)
        {
            OperateNum = 0;
            g_enTransState = TRANS_UPLOAD;   //置为上传状态
			UDP_UploadFile(sockfd, p_PathName, stServerAddr, uliSerAddrLen);  
        }
		
        /* 下载操作 */
        else if(2 == OperateNum)
        {
            OperateNum = 0;
            g_enTransState = TRANS_DOWNLOAD;   //置为下载状态
			UDP_DownloadFile(sockfd, p_PathName, stServerAddr, uliSerAddrLen);   
        }
		
		/* 退出程序 */
		else
		{
			if(NULL == p_PathName)
			{
				free(p_PathName);
				p_PathName = NULL;
			}
			return;
		}
    }

	/* 释放内存 */
    if(NULL == p_PathName)
	{
		free(p_PathName);
		p_PathName = NULL;
	}
	
	return;

}


/*==================================================================
* Function      : UDP_UploadFile   
* Description   : 文件上传
* Input Para    : sockfd:通信套接字, p_PathName:文件路径
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void UDP_UploadFile(int sockfd, char *p_PathName, struct sockaddr_in stServerAddr, socklen_t uliSerAddrLen)   
{
	int rev = 0;  
	
	rev = sendto(sockfd, (TRANS_STATE_E*)&g_enTransState, sizeof(TRANS_STATE_E), 0, (struct sockaddr *)&stServerAddr, uliSerAddrLen);
	
	if(SENDTO_FAIL == rev) 
	{  
		fprintf(stderr, "%s\n",strerror(errno));
		return;
	}
	
	else
	{
		/* 接收本次发送应答 */
		if((recvfrom(sockfd, g_szAckBuf, ACK_SIZE, 0, (struct sockaddr*)&stServerAddr, &uliSerAddrLen)) < 0)          
		{            
			fprintf(stderr, "%s\n", strerror(errno));
			return;
		}
		if(0 == strncmp(g_szAckBuf, "ok", 2))
		{
			printf("服务器已准备好接收文件...\n");
		}
	}

	PrintWorkDir(); //打印当前工作目录
	printf("请输入需要查看的目录绝对路径: ");
	
	/* 获取输入目录 */
	gets_s(p_PathName, PATH_MAX, stdin);  
	PrintDirFile(p_PathName);  //输出输入目录下的文件及目录
	
	printf("输入需要上传的文件名: \n");
	
	/* //获取上传文件名 */
	gets_s(g_pstComTransInfo->szFilename, NAME_MAX, stdin);   
	strncat(p_PathName, "/", 1);
	strncat(p_PathName, g_pstComTransInfo->szFilename, sizeof(g_pstComTransInfo->szFilename));
	printf("文件绝对路径: %s\n", p_PathName);

	/* 上传之前获取文件SHA1值 */
	SHA1File(p_PathName, g_pstComTransInfo->szSHA1);
	printf("待上传文件SHA1: %s\n", g_pstComTransInfo->szSHA1);
	
	/* 获取文件大小 */
	g_pstComTransInfo->iFileSize = GetFileSize(p_PathName);
	printf("待上传文件大小: %d\n", g_pstComTransInfo->iFileSize);


	/* 发送文件相关信息 */
	rev = sendto(sockfd, (COM_TRANS_INFO_S*)g_pstComTransInfo, sizeof(COM_TRANS_INFO_S), 0, (struct sockaddr *)&stServerAddr, uliSerAddrLen);

	if(SENDTO_FAIL == rev) 
	{  
		fprintf(stderr, "%s\n",strerror(errno));
		return;
	}

	else
	{
		/* 接收本次发送应答 */
		if((recvfrom(sockfd, g_szAckBuf, ACK_SIZE, 0, (struct sockaddr*)&stServerAddr, &uliSerAddrLen)) < 0)          
		{            
			fprintf(stderr, "%s\n", strerror(errno));
			return;
		}
		if(0 == strncmp(g_szAckBuf, "ok", 2))
		{
			printf("服务器已准备接收文件\n");
		}
		printf("服务器已接收到%d字节文件相关信息\n", rev);
	}

	UDP_SendFile(sockfd, p_PathName, &stServerAddr);

}


/*==================================================================
* Function      : TCP_DownloadFile   
* Description   : 文件下载
* Input Para    : sockfd:通信套接字, p_PathName:文件路径
* Output Para   : 无
* Return Value  : 无
==================================================================*/ 
void UDP_DownloadFile(int sockfd, char *p_PathName, struct sockaddr_in stServerAddr, socklen_t uliSerAddrLen)
{
	int rev = 0;
	
	rev = sendto(sockfd, (TRANS_STATE_E*)&g_enTransState, sizeof(TRANS_STATE_E), 0, (struct sockaddr *)&stServerAddr, uliSerAddrLen);
	
	if(SENDTO_FAIL == rev) 
	{  
		fprintf(stderr, "%s\n",strerror(errno));
		return;
	}
	else
	{
		/* 接收本次发送应答 */
		if((recvfrom(sockfd, g_szAckBuf, ACK_SIZE, 0, (struct sockaddr*)&stServerAddr, &uliSerAddrLen)) < 0)          
		{            
			fprintf(stderr, "%s\n", strerror(errno));
			return;
		}
		if(0 == strncmp(g_szAckBuf, "ok", 2))
		{
			printf("服务器已准备好查看\n");
		}
	}

	/* 接收当前工作目录 */
	if((recvfrom(sockfd, p_PathName, PATH_MAX, 0, (struct sockaddr*)&stServerAddr, &uliSerAddrLen)) < 0)          
	{            
		fprintf(stderr, "%s\n", strerror(errno));
		return;
	}
	
	else
	{
		printf("服务器当前工作目录: %s\n", p_PathName);
		if(SENDTO_FAIL == sendto(sockfd, "ok", 2, 0, (struct sockaddr *)&stServerAddr, uliSerAddrLen))
		{
			fprintf(stderr, "%s\n", strerror(errno));
			return;
		}
	}

	/* 查看远端服务器文件 */
	printf("请输入需要查看的目录绝对路径: ");
	
	/* 获取输入目录 */
	gets_s(p_PathName, PATH_MAX, stdin);  
	sendto(sockfd, p_PathName, PATH_MAX, 0, (struct sockaddr *)&stServerAddr, uliSerAddrLen);
	printf("服务器该目录文件列表如下:\n");
	printf("*******目录文件列表*******\n");
	
	while(1)
	{
		if((recvfrom(sockfd, g_pstComTransInfo->szFilename, NAME_MAX, 0, (struct sockaddr*)&stServerAddr, &uliSerAddrLen)) < 0)          
		{            
			fprintf(stderr, "%s\n", strerror(errno));
			return;
		}
		else
		{
			if(0 == strncmp(g_pstComTransInfo->szFilename, "**", 2))
			{
				break;
			}
			printf("%s\n", g_pstComTransInfo->szFilename);
		}
	}
	printf("*******目录文件列表打印完毕*******\n");
	
	/* 选择服务器文件 */
	printf("输入需要下载的文件名:\n");
	
	/* 获取下载文件名 */
	gets_s(g_pstComTransInfo->szFilename, NAME_MAX, stdin);   
	strncat(p_PathName, "/", 1);
	strncat(p_PathName, g_pstComTransInfo->szFilename, sizeof(g_pstComTransInfo->szFilename));
	printf("文件绝对路径: %s\n", p_PathName);
	sendto(sockfd, p_PathName, PATH_MAX, 0, (struct sockaddr *)&stServerAddr, uliSerAddrLen); //将待下载文件绝对路径发给服务器

	/* 接收文件相关信息 */
	if(RECVFROM_FAIL == recvfrom(sockfd, (COM_TRANS_INFO_S*)g_pstComTransInfo, sizeof(COM_TRANS_INFO_S), 0, (struct sockaddr*)&stServerAddr, &uliSerAddrLen))
	{
		fprintf(stderr, "%s\n",strerror(errno));
		return;
	}

	printf("SHA1: %s\n", g_pstComTransInfo->szSHA1);
	printf("FileName: %s\n", g_pstComTransInfo->szFilename);
	printf("FileSize: %d\n", g_pstComTransInfo->iFileSize);

	/* 接收文件内容 */
	UDP_RcvFile(sockfd, &stServerAddr, uliSerAddrLen);
	
}



 /**@fn 
 *  @brief  UDP发送函数
 *  @param c 参数描述.
 *  @param n 参数描述.
 *  @return 返回描述
 */

/*==================================================================
* Function      : UDP_SendFile
* Description   : 实现文件接收
* Input Para    : sockfd:通信套接字
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void UDP_SendFile(int sockfd, const char *p_PathName, struct sockaddr_in *pstServerAddr)
{
	
    if((NULL == p_PathName) || (NULL == pstServerAddr))
    {
        printf("传入参数错误！\n");
        return;
    }

    int length = 0;
    int rev = 0;   
    socklen_t uliAddrLen = sizeof(struct sockaddr_in);
    
	int fd = open(p_PathName, O_RDONLY);   
    
	if (OPEN_FAIL == fd)
    {
        fprintf(stderr, "%s\n",strerror(errno));
        return;
    }

    while ((length = read(fd, g_pszTransBuf, BUFFER_SIZE)) > 0)
    {

        if((rev = (sendto(sockfd, g_pszTransBuf, length, 0, (struct sockaddr*)pstServerAddr, sizeof(struct sockaddr)))) < 0)          
        {           
            close(fd);
            printf("%s文件发送失败!\n", p_PathName);
            break; 
        }

        /* 接收本次发送应答 */
        if((recvfrom(sockfd, g_szAckBuf, ACK_SIZE, 0, (struct sockaddr*)pstServerAddr, &uliAddrLen)) < 0)          
        {       
            close(fd);     
            fprintf(stderr, "%s\n", strerror(errno));
            break;
        }
        if(0 == strncmp(g_szAckBuf, "ok", 2))
        {
            ;
        }
    }
   
    printf("%s文件发送成功!\n", p_PathName); 

    close(fd);
}

 
 /**@fn 
 *  @brief  UDP接收函数
 *  @param c 参数描述.
 *  @param n 参数描述.
 *  @return 返回描述
 */

 /*==================================================================
* Function      : UDP_RcvFile
* Description   : 实现文件接收
* Input Para    : sockfd:通信套接字
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void UDP_RcvFile(int sockfd, struct sockaddr_in *pstServerAddr, socklen_t uliSerAddrLen)
{
    fd_set ReadFds;
	
    struct timeval sttv = {0, 0};
    int TimeoutCount = 0;  
    int rev = 0;   
    int filesize = 0;  
	
    int fd = open(g_pstComTransInfo->szFilename, O_RDWR | O_CREAT, 0664);
	
    if(OPEN_FAIL == fd)
    {
        fprintf(stderr, "%s\n",strerror(errno));
        return;
    }

    while (1)
    {
        FD_ZERO(&ReadFds);
        FD_SET(sockfd, &ReadFds);
        sttv.tv_sec = 3;
        sttv.tv_usec = 0;
        select(sockfd+1, &ReadFds, NULL, NULL, &sttv);
        
        if(FD_ISSET(sockfd, &ReadFds))
        {
			
            TimeoutCount = 0;  
			
            rev = recvfrom(sockfd, (char*)g_pszTransBuf, BUFFER_SIZE, 0, (struct sockaddr*)pstServerAddr, &uliSerAddrLen);
			
            if(RECVFROM_FAIL == rev)
            {
                close(fd);
                fprintf(stderr, "%s\n",strerror(errno));
                return;
            }

            if(-1 == write(fd, g_pszTransBuf, rev))
            {
                close(fd);
                fprintf(stderr, "%s\n",strerror(errno));
                return;
            }

            filesize += rev;
            
            /* 发送响应 */
            rev = sendto(sockfd, "ok", 2, 0, (struct sockaddr*)pstServerAddr, uliSerAddrLen);
			
            if(SENDTO_FAIL == rev)
            {
                close(fd);
                fprintf(stderr, "%s\n",strerror(errno));
                return;
            }

            /* 文件接收计数达到文件大小则接收结束 */
            if(filesize == g_pstComTransInfo->iFileSize)
            {
                break;
            }
        }
		
        else
        {
            printf("超时%d次,超时3次本次传输将退出!\n", ++TimeoutCount);
            if(TimeoutCount == 3)
            {
                printf("通信异常，本次文件传输未完成!\n");
                close(fd);
                break;
            }
        }
    }
    
    close(fd);
    SHA1File(g_pstComTransInfo->szFilename, g_pszSha1Digest);
    printf("从服务器下载文件SHA1: %s\n", g_pszSha1Digest);
	
    if(0 == strncmp(g_pstComTransInfo->szSHA1, g_pszSha1Digest, 40))
    {
        printf("SHA1相同，文件下载正常\n");
    }
	
	return;
}

/************************ (C) COPYRIGHT HIKVISION *****END OF FILE****/

