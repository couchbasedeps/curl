/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2019, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#include "curl_setup.h"

#ifdef WIN32
/*
 * This is a socketpair() implementation for Windows.
 */
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>

int socketpair(int domain, int type, int protocol, curl_socket_t socks[2])
{
  union {
    struct sockaddr_in inaddr;
    struct sockaddr addr;
  } a;
  curl_socket_t listener;
  socklen_t addrlen = sizeof(a.inaddr);
  int reuse = 1;
  (void)domain;
  (void)type;
  (void)protocol;

  listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(listener == CURL_SOCKET_BAD)
    return CURL_SOCKET_BAD;

  memset(&a, 0, sizeof(a));
  a.inaddr.sin_family = AF_INET;
  a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  a.inaddr.sin_port = 0;

  socks[0] = socks[1] = CURL_SOCKET_BAD;

  if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
                (char *)&reuse, (socklen_t)sizeof(reuse)) == -1)
    goto error;
  if(bind(listener, &a.addr, sizeof(a.inaddr)) == CURL_SOCKET_BAD)
    goto error;
  if(getsockname(listener, &a.addr, &addrlen) == CURL_SOCKET_BAD)
    goto error;
  if(listen(listener, 1) == CURL_SOCKET_BAD)
    goto error;
  socks[0] = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
  if(socks[0] == CURL_SOCKET_BAD)
    goto error;
  if(connect(socks[0], &a.addr, sizeof(a.inaddr)) == CURL_SOCKET_BAD)
    goto error;
  socks[1] = accept(listener, NULL, NULL);
  if(socks[1] == CURL_SOCKET_BAD)
    goto error;

  closesocket(listener);
  return 0;

  error:
  closesocket(listener);
  closesocket(socks[0]);
  closesocket(socks[1]);
  return CURL_SOCKET_BAD;
}
#endif /* WIN32 */
