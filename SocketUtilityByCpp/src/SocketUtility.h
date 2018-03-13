/*
 * SocketUtility.h
 *
 *  Created on: Mar 13, 2018
 *      Author: motein
 */

#ifndef SRC_SOCKETUTILITY_H_
#define SRC_SOCKETUTILITY_H_

void socket_close(const char* sock);
void socket_connect(const char* sock, const char* host, int port);
void socket_accept(const char* sock, int port);
int socket_gets(const char* sock, char* str, int size);
int socket_print(const char* sock, const char* str);
int socket_read(const char* sock, unsigned char* data, int dataSize);
int socket_write(const char* sock, const unsigned char* data, int dataSize);

#endif /* SRC_SOCKETUTILITY_H_ */
