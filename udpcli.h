
/***************************************************************************************
****************************************************************************************
* FILE     : udpcli.h
* Description  : 
*            
* Copyright (c) 2019 by Hikvision. All Rights Reserved.
* 
* History:
* Version      Name        Date                Description
   0.1         fangyuan9   2019/07/31          Initial Version 1.0.0
   
****************************************************************************************
****************************************************************************************/
#ifndef _UDPCLI_H
#define _UDPCLI_H 1 

void UDPService(stServerNode *p_Server);

void UDP_UploadFile(int sockfd, char *p_PathName, struct sockaddr_in stServerAddr, socklen_t uliSerAddrLen);
void UDP_DownloadFile(int sockfd, char *p_PathName, struct sockaddr_in stServerAddr, socklen_t uliSerAddrLen);

void UDP_SendFile(int sockfd, const char *p_PathName, struct sockaddr_in *pstServerAddr);
void UDP_RcvFile(int sockfd, struct sockaddr_in *pstServerAddr, socklen_t uliSerAddrLen);

#endif

/************************ (C) COPYRIGHT HIKVISION *****END OF FILE****/
