#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define MAXBUFLEN 256

struct DataPacket
{
	void* m_pData = nullptr;
	size_t m_uiSize = 0;

	~DataPacket()
	{
		if (m_pData)
			free(m_pData);
	}
};

class NetworkListen
{
public:
	NetworkListen();
	~NetworkListen();

	void Init();
	bool GetData(DataPacket& data);

private:
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
};

