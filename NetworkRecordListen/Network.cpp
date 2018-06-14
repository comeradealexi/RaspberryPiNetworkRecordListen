#include "Network.h"
#include "RaspberryPiShared.h"

NetworkListen::NetworkListen()
{
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, RaspberryPi::VideoSurveillance::k_Port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return ;
	}


	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	fcntl(sockfd, F_SETFL, O_NONBLOCK);
}


NetworkListen::~NetworkListen()
{
}

void NetworkListen::Init()
{

}

bool NetworkListen::GetData(DataPacket& data)
{
	int numbytes = -1;
	char buf[MAXBUFLEN];

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
		(struct sockaddr *)&their_addr, &addr_len)) > 0) 
	{
		data.m_pData = malloc(numbytes);
		memcpy(data.m_pData, buf, numbytes);
		data.m_uiSize = numbytes;
		return true;
	}

	return false;
}
