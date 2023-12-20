///@system  Dstar V10 api demo
///@file    UdpClient.cpp
///@author  Hao Lin 2021-01-20

#ifndef TUDPCLIENT_H
#define TUDPCLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>


class TUdpClient
{
public:
    TUdpClient() : m_Socket(-1) { };
    ~TUdpClient() { Close(); };

    int  Init(const char *toip, const unsigned short toport) {
        //检查当前socket
        if (-1 != m_Socket)
            return 1;

        //创建socket, 无效则返回
        int s = ::socket(AF_INET,SOCK_DGRAM,0);
        if (-1 == s)
            return 2;

        unsigned value = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

        m_Socket = s;

        memset(&m_ToAddr, 0, sizeof(sockaddr_in));
        m_ToAddr.sin_family = AF_INET;
        m_ToAddr.sin_addr.s_addr = inet_addr(toip);
        m_ToAddr.sin_port = htons(toport);
        
        int result = connect(m_Socket, (struct sockaddr *) &m_ToAddr, sizeof(sockaddr_in));
        if (result == -1) {
            ::close(m_Socket); // 释放创建的套接字
            m_Socket = -1;
            return 3;
        }
        return 0;
    };

    void Close() {
        if (-1 == m_Socket)
            return;

        m_Socket = -1;
    };

    int Send(const char* buf, const int len) {
        //int ret = ::sendto(m_Socket, buf, len, 0, (struct sockaddr*)&m_ToAddr, sizeof(m_ToAddr));
        int ret = ::send(m_Socket, buf, len, 0);
        if (len != ret)
        {
            return 1;
        }
        return 0;
    };//返回0为成功

private:
    int         m_Socket;
    sockaddr_in m_ToAddr;
};

#endif // TUDPCLIENT_H
