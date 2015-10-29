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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "dprint.h"
#include "mm_debug_link_interface.h"
#include "udp_log.h"

bool run = true;
void int_handler(int sig)
{
  DPRINT("SIGINT: stopping\n");
  run = false;
}

unsigned char *get_write_buf(size_t byte_count, size_t *buf_size)
{
  size_t size = byte_count + 12;
  *buf_size = 0;
  // if (size > sizeof(my_global_buffer) / sizeof(*my_global_buffer))
  //   return NULL;

  // unsigned char *buf = my_global_buffer;
  unsigned char *buf = (unsigned char *)malloc(size);

  if (!buf)
  	return buf;
  
  buf[0] = 0x7C;
  buf[1] = 0x01;
  buf[2] = 0x7A;
  buf[3] = 0x04;
  buf[4] = 0x00;
  buf[5] = byte_count >> 8;
  buf[6] = byte_count & 0xFF;
  buf[7] = 0x00;
  buf[8] = 0x00;
  buf[9] = 0x00;
  buf[10] = 0x00;

  for (int i = 0; i < byte_count; ++i)
  {
    buf[11 + i] = (i + 1) & 0x3F;
  }
  buf[size - 1] = buf[size - 2];
  buf[size - 2] = 0x7B;

  *buf_size = size;
  return buf;
}

int do_test(mm_debug_link_interface *driver, unsigned char *write_buf, size_t write_buf_size)
{

  bool have_read = false;
  bool have_written = false;
  int lim = 1000;
//  unsigned char write_buf[] = {
//                                0x7C, 0x01, 0x7A, 0x04,
//                                0x00, 0x00, 0x04, 0x00,
//                                0x00, 0x00, 0x00, 0x01,
//                                0x02, 0x03, 0x7B, 0x04,
//                               };
//

  int total_written = 0;
  while (run && (!have_read || !have_written))
  {
    if (!have_read)
    {
      driver->read();
      if (!driver->is_empty())
      {
        for (int printed_bytes = 0; printed_bytes < driver->buf_end(); ++printed_bytes)
          DPRINT_RAW("0x%02X ", driver->buf()[printed_bytes]);
        DPRINT_RAW("\n");
        driver->buf_end(0);
        have_read = true;
        DPRINT("done reading\n");
      }
    }

    if (!have_written)
    {
      ssize_t written = driver->write(write_buf + total_written, write_buf_size - total_written);
      if (written < 0)
      {
        DPRINT("driver->write() returned %d\n", written);
        break;
      }
      if (written == 0)
      {
        DPRINT("driver->write() returned %d\n", written);
        break;
      }
      total_written += written;
      if (total_written == write_buf_size)
      {
        DPRINT("done writing\n");
        have_written = true;
      }
    }

    if (--lim == 0)
    {     
      run = false;
      DPRINT("hit the limit\n");
    }
  }
}

int main(int argc, char **argv)
{
  int err = 0;

  size_t write_byte_count = 1024;
  size_t iterations = 1;
  if (argc >= 2)
  {
    sscanf(argv[1], "%u", &write_byte_count);
    if (argc >= 3)
    {
      sscanf(argv[2], "%u", &iterations);
    }
  }

  printf("write_byte_count: %u\n", write_byte_count);
  printf("iterations: %u\n", iterations);

  unsigned char *write_buf = NULL;
  size_t write_buf_size = 0;
  write_buf = get_write_buf(write_byte_count, &write_buf_size);

  if (!write_buf)
  {
    fprintf(stderr, "failed to allocate memory.\n");
    return -1;
  }

  signal(SIGINT, int_handler);
  mm_debug_link_interface *driver = get_mm_debug_link();

  if (err = driver->open())
  {
    fprintf(stderr, "failed to init driver (%d).\n", err);
    return err;
  }

  driver->reset(true);
  driver->reset(false);
  driver->enable(1, 1);

  for (int i = 0; i < iterations; ++i)
  {
    do_test(driver, write_buf, write_buf_size);
  }

  if (write_buf)
  {
    free(write_buf);
    write_buf = NULL;
  }

  driver->close();
  delete driver; driver = NULL;

  DPRINT("goodbye.\n");
  return 0;
}

