#include "SocketSender.h"

SocketSender::SocketSender(int port, const char serverIpAddr[]) {
	this->port = port;

	memset(&buffer, 0, sizeof(buffer));
	memset(&dst_addr, 0, sizeof(dst_addr));

	dst_addr.sin_port = htons(port);
	dst_addr.sin_family = AF_INET;
	inet_pton(dst_addr.sin_family, serverIpAddr, &dst_addr.sin_addr.s_addr);
}

int SocketSender::SS_startup() {
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	dst_socket = socket(AF_INET, SOCK_STREAM, 0);
	return ret;
}

bool SocketSender::SS_connect() {
	int ret = connect(dst_socket, (struct sockaddr*) & dst_addr, sizeof(dst_addr));
	return ret;
}

int SocketSender::SS_send(std::string send_str) {
	const char* buffer = send_str.c_str();
	int d_size = send_str.size();
	int ret = send(dst_socket, buffer, d_size, 0);
	return ret;
}

int SocketSender::SS_receive() {
	int ret = recv(this->dst_socket, this->buffer, sizeof(this->buffer), 0);
	return ret;
}

std::string SocketSender::SS_getBuffer() {
	std::string bufStr = this->buffer;
	return bufStr;
}

void SocketSender::SS_setBlockingMode(bool blockingMode) {
	u_long val = 1;
	if (blockingMode) val = 0;
	
	ioctlsocket(this->dst_socket, FIONBIO, &val);
}

int SocketSender::SS_close() {
	return closesocket(dst_socket);
}

void SocketSender::SS_clean() {
	WSACleanup();
}