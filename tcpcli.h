
/***************************************************************************************
****************************************************************************************
* FILE     : tcpcli.h
* Description  : 
*            
* Copyright (c) 2019 by Hikvision. All Rights Reserved.
* 
* History:
* Version      Name        Date                Description
   0.1         fangyuan9   2019/07/31          Initial Version 1.0.0
   
****************************************************************************************
****************************************************************************************/

#ifndef _TCPCLI_H
#define _TCPCLI_H 1

void TCPService(stServerNode *p_Server);

void TCP_UploadFile(int sockfd,char *p_PathName);
void TCP_DownloadFile(int sockfd,char *p_PathName);

void TCP_SendFile(int sockfd, char *p_PathName);
void TCP_RcvFile(int sockfd);   
#endif

/************************ (C) COPYRIGHT HIKVISION *****END OF FILE****/
