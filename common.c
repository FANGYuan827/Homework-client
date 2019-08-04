
/***************************************************************************************
****************************************************************************************
* FILE     : common.c
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
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#include "common.h"


/* 全局变量定义区 */
TRANS_STATE_E       g_enTransState;                 //传输状态
COM_TRANS_INFO_S    *g_pstComTransInfo  = NULL;     //保存文件信息结构
char                *g_pszTransBuf      = NULL;     //发送或接收缓存
char                g_szAckBuf[ACK_SIZE];           //接收应答缓存
char                *g_pszSha1Digest    = NULL;     //用于存储计算接收后文件的摘要
volatile bool       timeout = false;                //服务器发现计时标志
/* 全局变量定义区 */


 
/*==================================================================
* Function      : gets_s   
* Description   : 字符串获取函数
* Input Para    : p_Server:服务器指针 指向可用服务器
* Output Para   : 无
* Return Value  : 成功返回字符串地址，失败返回空
==================================================================*/
char *gets_s(char *str, size_t num, FILE *stream)
{
    if (0 != fgets(str, num, stream))
    {
        size_t len = strlen(str);
        if (len > 0 && str[len-1] == '\n')
            str[len-1] = '\0';
        return str;
    }
    return NULL;
}
 
/*==================================================================
* Function      : IsExist   
* Description   : 检测链表中是否存在指定IP
* Input Para    : pHead:可用服务器链表头指针 p_IP:指定服务器IP
* Output Para   : 无
* Return Value  : 成功返回true，失败返回false
==================================================================*/
bool IsExist(stServerNode *pHead, char* p_IP)
{
    assert(pHead != NULL);

    stServerNode *pTemp = pHead->pstNext;

    while(pTemp != NULL)
    {
        if(1 == strncmp(pTemp->pszIP, p_IP, sizeof(p_IP)))
        {
            return true;
        }
        pTemp = pTemp->pstNext;
    }

    return false;
}


/*==================================================================
* Function      : AddNode   
* Description   : 将指定服务器插入可用服务器链表
* Input Para    : pHead:服务器链表头指针 p_IP:服务器指针 PortNum:端口号
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void AddNode(stServerNode *pHead, char *p_IP, uint16_t PortNum)  
{
    assert(pHead != NULL);

    stServerNode *pTemp = pHead->pstNext;//保存原链表第一个节点指针

    /* 为新节点开辟内存 */
    stServerNode *pNode = (stServerNode *)malloc(sizeof(stServerNode));

    if(NULL == pNode)
    {
        printf("内存申请失败\n");
        return;
    }

    memset(pNode, 0, sizeof(stServerNode));
    
    pNode->pszIP = (char*)malloc(strlen(p_IP)+1);

    if(NULL == pNode->pszIP)
    {
        printf("内存申请失败\n");
        return;
    }

    memset(pNode->pszIP, 0, strlen(p_IP) + 1);
    strncpy(pNode->pszIP, p_IP, strlen(p_IP));

    pNode->usiPort = PortNum;
    pNode->pstNext = pTemp;
    pHead->pstNext = pNode;

}

/*==================================================================
* Function      : FindNode   
* Description   : 将指定服务器插入可用服务器链表
* Input Para    : pHead:服务器链表头指针 ServerNum:可用服务器序号
* Output Para   : 无
* Return Value  : 返回可用服务器节点指针
==================================================================*/
stServerNode *FindNode(stServerNode *pHead, uint16_t ServerNum)
{
    assert(pHead != NULL);

    stServerNode *pTemp = pHead->pstNext;

    while(--ServerNum)
    {
        pTemp = pTemp->pstNext;
    }

    return pTemp;
}


/*==================================================================
* Function      : CountNodes   
* Description   : 统计指定服务器链表中可用服务器总数
* Input Para    : pHead:服务器链表头指针 
* Output Para   : 无
* Return Value  : 返回可用服务器总数
==================================================================*/
uint16_t CountNodes(stServerNode *pHead)
{
    assert(pHead != NULL);

    stServerNode *pTemp = pHead->pstNext;
    uint16_t usiNum = 0;

    while(pTemp)
    {
        ++usiNum;
        pTemp = pTemp->pstNext;
    }

    return usiNum;
}


/*==================================================================
* Function      : PrintNode   
* Description   : 打印链表中所有服务器信息
* Input Para    : pHead:服务器链表头指针 
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void PrintNode(stServerNode *pHead)
{
    assert(pHead != NULL);

    stServerNode *pTemp = pHead->pstNext;
    int iNum = 0;

    while(pTemp != NULL)
    {
        iNum++;
        printf("服务器%3d [%s:%hu]\n", iNum, pTemp->pszIP, pTemp->usiPort);
        pTemp = pTemp->pstNext;
    }
}

/*==================================================================
* Function      : DeleteList   
* Description   : 删除链表中所有服务器节点
* Input Para    : pHead:服务器链表头指针 
* Output Para   : 无
* Return Value  : 无
==================================================================*/
 void DeleteList(stServerNode *pHead)
 {
    assert(pHead != NULL);

    stServerNode *pTemp = pHead;
 
    while(NULL != pHead)
    {
        pTemp = pHead->pstNext;
        free(pHead);
        pHead = pTemp;
    }
 }


/*==================================================================
* Function      : ProtocolMenu   
* Description   : 协议选择菜单
* Input Para    : 无 
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void ProtocolMenu(void)
{
    printf("可用传输协议有:\n");
    printf("1-UDP\n2-TCP\n");
	printf("输入序号选择传输协议:\n");
}


/*==================================================================
* Function      : OperateMenu   
* Description   : 操作选择菜单
* Input Para    : 无 
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void OperateMenu(void)
{
    printf("\n");

    printf("1-Uploadfile(文件上传)\n");
    printf("2-Downloadfile(文件下载)\n");
    printf("3-Exit(退出程序)\n");
    
    printf("\n请输入所需操作序号:\n");
}

 
/*==================================================================
* Function      : GetFileSize   
* Description   : 获取文件大小函数
* Input Para    : 无 
* Output Para   : 无
* Return Value  : 失败返回-1，成功返回文件大小
==================================================================*/
int GetFileSize(const char* p_FilePath)   
{
    struct stat stStat;
    if(0 == stat(p_FilePath, &stStat))
    {
        return stStat.st_size;
    }
    else
    {
        return -1;
    }
}


/*==================================================================
* Function      : PrintWorkDir   
* Description   : 打印当前工作路径信息
* Input Para    : 无 
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void PrintWorkDir(void)
{
    char *p_PathBuff = (char*)malloc(PATH_MAX);    //存储目录变量
    
	if(NULL == p_PathBuff)
    {
        printf("内存申请失败\n");
        return;
    }

    if(NULL == getcwd(p_PathBuff, PATH_MAX))   //获取当前目录
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    printf("当前工作目录为:%s\n", p_PathBuff);
	
	/* 释放路径内存 */
	if(NULL != p_PathBuff)
	{
		free(p_PathBuff);
		p_PathBuff = NULL;
	} 
}


/*==================================================================
* Function      : PrintDirFile   
* Description   : 打印指定目录中文件列表
* Input Para    : p_PathName：指向需要打印目录的路径的指针 
* Output Para   : 无
* Return Value  : 无
==================================================================*/
void PrintDirFile(const char* p_PathName)
{
    if(NULL == p_PathName)
    {
        printf("参数错误！\n");
        return;
    }
 	
    struct dirent* p_stdir = NULL;
    DIR *p_DirPath = opendir(p_PathName);
	
    if (NULL == p_DirPath)
    {
        fprintf(stderr, "%s\n",strerror(errno));
        return;
    }

    printf("该目录文件列表为:\n");
    printf("*******目录文件列表*******\n");
	
    for (p_stdir = readdir(p_DirPath); p_stdir; p_stdir = readdir(p_DirPath))
    {
        /* 不是目录文件直接输出 */ 
        if (DT_DIR != p_stdir->d_type)
        {
            printf ("%s\n", p_stdir->d_name);
        }
		
        /* 不输出. 和..目录文件 */ 
        else
        {
            if ((0 != strncmp(p_stdir->d_name, ".", 1))
                && (0 != strncmp(p_stdir->d_name, "..", 2)))
            {
                printf ("./%s/\n", p_stdir->d_name);
            }
        }
    }
	
    printf("\n*******目录文件列表打印完毕*******\n");
    closedir(p_DirPath);
}

/************************ (C) COPYRIGHT HIKVISION *****END OF FILE****/

