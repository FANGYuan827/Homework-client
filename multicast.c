/** @file  mulitcast.c
 *  @note 
 *  @brief 文件传输客户端
 *
 *  @author     
 *  @date       
 *
 *  @note 
 *
 *  @warning 
 */

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
#include "multicast.h"

 /**@fn     CountTime
 *  @brief  服务器发现计时线程函数
 *  @param c 参数描述
 *  @param n 参数描述
 *  @return 返回描述
 */
void *CountTime(void *arg)
{
    timeout = false;         //timeout:服务器发现计时标志
    sleep(10);
    printf("超时时间到!\n");
    timeout = true;
}

 /**@fn     ServerSearch
 *  @brief  服务器发现函数
 *  @param c 参数描述
 *  @param n 参数描述
 *  @return 返回描述
 */
 
int16_t ServerSearch(stServerNode *pHead)
{
    int sockfd;
    struct sockaddr_in stMcastAddr = {0}, stServerAddr = {0};
    char szBuf[128] = {0};
    char szServerAddr[INET_ADDRSTRLEN] = {0};   //INET_ADDRSTRLEN IP地址长度  
 
    int iLenServerAddr = sizeof(stServerAddr);   //0801

    uint16_t usiServerPort = 0;

    
    /* 设置socket为非阻塞 */
    sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0); //AF_INET ipv4 SOCK_DGRAM tcp 
    
    if (-1 == sockfd)
    { 
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }
    
    /* 填充多播地址 */
    stMcastAddr.sin_family = AF_INET;
    stMcastAddr.sin_addr.s_addr = inet_addr(MCAST_ADDR);
    stMcastAddr.sin_port = htons(MCAST_PORT);
    

    /* 向局域网内多播地址发送多播内容 */
    printf("开始发送多播数据，探测服务器...\n");
    int iRet = sendto(sockfd, MCAST_DATA, sizeof(MCAST_DATA), 0, (struct sockaddr*)&stMcastAddr, sizeof(stMcastAddr));

    if(0 > iRet)
    {
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }
        
    else
    {
        /* 10s内未收到服务器端回复信息视为未找到服务器 */
        pthread_t tid;  ///计时线程id
        pthread_create(&tid, NULL, CountTime, NULL);
        pthread_detach(tid);///分离线程
        timeout = false;
            
        /* 计时10s内一直查收服务器发来的信息 */
        while (false == timeout)
        {
            iRet = recvfrom(sockfd, szBuf, sizeof(szBuf), 0, (struct sockaddr*)&stServerAddr, &iLenServerAddr);   //接收成功返回字节数 否则返回-1

            //接收成功
            if (-1 != iRet)
            {
                memset(szServerAddr, 0, sizeof(szServerAddr));  
                    
                inet_ntop(AF_INET, &stServerAddr.sin_addr, szServerAddr, sizeof(szServerAddr));
                usiServerPort = ntohs(stServerAddr.sin_port);

                printf("服务器IP为: %s 端口号port为: %hu\n", szServerAddr, usiServerPort);

                if (false == IsExist(pHead, szServerAddr))  //检查链表中IP是否存在
                {
                    AddNode(pHead, szServerAddr, usiServerPort);
                }
            }
        }
    }
	
    close(sockfd);
    return CountNodes(pHead);
}

