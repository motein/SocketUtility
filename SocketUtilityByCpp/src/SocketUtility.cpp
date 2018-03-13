/*
 * SocketUtility.cpp
 *
 *  Created on: Mar 13, 2018
 *      Author: motein
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <string>

#include "SessionManager.h"
#include "Socket.h"

Socket* socket_getInstance(std::string sock, int flag = 0) {
	static std::map<std::string, Socket*> sockets;
	std::map<std::string, Socket*>::iterator it = sockets.find(sock);
	if (it == sockets.end()) {
		it = sockets.insert(
						std::map<std::string, Socket*>::value_type(sock,
								new Socket())).first;
	}
	if (flag < 0) {
		delete it->second;
		sockets.erase(it);
		return 0;
	}
	return it->second;
}

extern "C" {
int socket_close(const char* sock) {
	socket_getInstance(sock)->close();
	socket_getInstance(sock, -1);
	return 1;
}

int socket_connect(const char* sock, const char* host, int port) {
	return socket_getInstance(sock)->connect(host, port);
}

int socket_accept(const char* sock, int port) {
	return socket_getInstance(sock)->accept(port);
}

int socket_gets(const char* sock, char* str, int size) {
	std::string s;
	int ret = socket_getInstance(sock)->gets(s);
	if (!ret) {
		return ret;
	}
	if (size < (int) s.size() + 1) {
		return 0;
	}
	sprintf(str, s.c_str());
	return 1;
}
int socket_puts(const char* sock, const char* str) {
	return socket_getInstance(sock)->print(str);
}
int socket_read(const char* sock, unsigned char* data, int dataSize) {
	return socket_getInstance(sock)->read(data, dataSize);
}
int socket_write(const char* sock, const unsigned char* data, int dataSize) {
	return socket_getInstance(sock)->write(data, dataSize);
}
}

const char* getHost() {
	const char* host = "localhost";
	return host;
}

int getSessionId(const char* envVarName) {
	int sessionId = 0;
	char* p = getenv(envVarName);
	if (p != 0) {
		sessionId = atoi(p);
	}
	return sessionId;
}




