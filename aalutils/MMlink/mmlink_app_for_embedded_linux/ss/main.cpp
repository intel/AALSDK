/*
 * Copyright (c) 2014, Altera Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Altera Corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>

#include "mmlink_server.h"
#include "mm_debug_link_interface.h"
#include "udp_log.h"

#define MAX_FILENAME_SIZE (256)

mmlink_server *server;
void int_handler(int sig)
{
  printf("SIGINT: stopping the server\n");
  if (server)
    server->stop();
}

int main(int argc, char **argv)
{
  int ip = INADDR_ANY;
  short port = 3333;
  char * sys_file = (char *)malloc (MAX_FILENAME_SIZE);

  signal(SIGINT, int_handler);
  for (int i = 1; i < argc; ++i)
  {
    sscanf(argv[i], "--ip=%d", &ip);
    sscanf(argv[i], "--port=%d", &port);
    sscanf(argv[i], "--sysfs=%s", sys_file);
  }


  struct sockaddr_in sock;
  sock.sin_family = AF_INET;
  sock.sin_port = htons(port);
  sock.sin_addr.s_addr = htonl(ip);

  mm_debug_link_interface *driver = get_mm_debug_link();
  server = new mmlink_server(&sock, driver);

  int err = server->run(sys_file);

  server->print_stats();

  delete server; server = NULL;
  delete driver; driver = NULL;
  free(sys_file);
  return err;
}


