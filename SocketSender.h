#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <iostream>

class SocketSender {
private:

	int port;
	char buffer[1024];
	struct sockaddr_in dst_addr;
	SOCKET dst_socket;

public:

	SocketSender(int port, const char serverIpAddr[]);

	int SS_startup();

	bool SS_connect();

	int SS_send(std::string send_str);

	void SS_setBlockingMode(bool blockingMode);

	int SS_receive();

	std::string SS_getBuffer();

	int SS_close();

	void SS_clean();

};