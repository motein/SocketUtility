/*
 * Socket.h
 *
 *  Created on: Mar 13, 2018
 *      Author: motein
 */

#ifndef SRC_SOCKET_H_
#define SRC_SOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <string>

#ifdef MINGW
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/time.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#endif

class Socket {
private:
	int fd;
	int server;
	int bufferedSize;
	char buffer[256];
	static const int BUFFER_SIZE = 256;
	static const int SOCKET_MAX = 5;
public:
	Socket() :
			fd(-1), server(-1), bufferedSize(0) {
#ifdef MINGW
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0)
			perror("initial error");
#endif
	}

	virtual ~Socket() {
		close();
	}

	void close() {
		if (fd != -1) {
#ifdef MINGW
			shutdown(fd, SD_BOTH);
#else
			shutdown(fd, SHUT_RDWR);
#endif
			::close(fd);
			fd = -1;
		}
		if (server != -1) {
			::close(server);
			server = -1;
		}
		bufferedSize = 0;
	}

#ifdef MINGW
	int accept(int port) {
		struct sockaddr caddr;
#else
		struct sockaddr_un caddr;
#endif
		socklen_t len;
		struct sockaddr_in saddr;
		// create a socket
		if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return 0;
		}

		// initialize an address and bind
#ifdef MINGW
		char on = 1;
		memset((char *) &saddr,0,sizeof(saddr));
#else
		int on = 1;
		bzero((char *) &saddr, sizeof(saddr));
#endif

		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = htonl(INADDR_ANY);
		saddr.sin_port = htons(port);

		setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		if ((bind(server, (struct sockaddr *) &saddr, sizeof(saddr))) == -1) {
			perror("bind");
			return 0;
		}

		// listen
		if ((listen(server, SOCKET_MAX)) == -1) {
			perror("listen");
			return 0;
		}

		// accept
		len = sizeof(caddr);
		fd = ::accept(server, (struct sockaddr *) &caddr, &len);
		if (fd == -1) {
			perror("accept");
			return 0;
		}

		return 1;
	}

	int connect(const char* host, int port) {
		struct sockaddr_in addr;
		struct hostent *hp;

		// create a socket
		if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return 0;
		}

		// initialize an address and connect
#ifdef MINGW
		memset((char *) &addr,0,sizeof(addr));
#else
		bzero((char *) &addr, sizeof(addr));
#endif
		if ((hp = gethostbyname(host)) == NULL) {
			perror("get host by name");
			return 0;
		}

#ifdef MINGW
		memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
#else
		bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
#endif
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		if (::connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			perror("connect");
			return 0;
		}

		return 1;
	}

	int gets(std::string& s) {
		s.clear();
		char rec_temp2[BUFFER_SIZE + 1];;

		while (1) {
			if (bufferedSize == 0) {
#ifdef MINGW
				bufferedSize = ::recv(fd, buffer, BUFFER_SIZE - 3, 0);
#else
				bufferedSize = ::read(fd, buffer, BUFFER_SIZE - 3);
#endif
				if (bufferedSize == 0) {
					return 0;
				} else if (bufferedSize < 0) {
					if (s.size() <= 0) {
						return 0;
					} else {
						return 1;
					}
				}
				buffer[bufferedSize] = '\0';
			} else if (bufferedSize < 0) {
				return 0;
			}

			int b_text_end = 0;

			for (int i = 0; i <= bufferedSize; i++) {
				if (i == bufferedSize) {
					memcpy(rec_temp2, buffer, bufferedSize);
					bufferedSize = 0;
					break;
				}
				if (buffer[i] == '\0' || buffer[i] == '\n') {
					memcpy(rec_temp2, buffer, i + 1);
					rec_temp2[i + 1] = '\0';

					bufferedSize -= i + 1;
					memmove(buffer, &buffer[i + 1], bufferedSize + 1);
					b_text_end = 1;
					break;
				}
			}
			s.append(rec_temp2);
			if (b_text_end) {
				break;
			}
		}
		return 1;
	}

	int print(const char* str) {
		int size = 0;
		while (1) {
#ifdef MINGW
			size = ::send(fd, str, strlen(str) + 1, 0);
#else
			size = ::write(fd, str, strlen(str) + 1);
#endif
			if (size < 0) {
				perror("write");
				return 0;
			}
			if (str[size - 1] == '\0') {
				break;
			}
			str += size;
		}
		return 1;
	}

	int read(unsigned char* data, int dataSize) {
		int size = 0;
		if (bufferedSize > 0) {
			int copySize = (dataSize < bufferedSize ? dataSize : bufferedSize);
			memcpy(data, buffer, copySize);
			bufferedSize -= copySize;
			size += copySize;
			memmove(buffer, &buffer[copySize], bufferedSize);
		}

		while (1) {
			if (size < dataSize) {

#ifdef MINGW
				int length = ::recv(fd, (char*) &data[size], dataSize - size, 0);
#else
				int length = ::read(fd, (char*) &data[size], dataSize - size);
#endif
				if (length < 0) {
					return 0;
				}
				size += length;
			} else {
				break;
			}
		}
		return 1;
	}

	int write(const unsigned char* data, int dataSize) {
		int size = 0;
		while (1) {
#ifdef MINGW
			int length = ::send(fd, (const char*) &data[size], dataSize - size, 0);
#else
			int length = ::write(fd, (const char*) &data[size], dataSize - size);
#endif
			if (length < 0) {
				return 0;
			}

			size += length;
			if (size >= dataSize) {
				break;
			}
		}
		return 1;
	}
};

#endif /* SRC_SOCKET_H_ */
