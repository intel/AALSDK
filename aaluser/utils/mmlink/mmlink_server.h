// Copyright(c) 2014, Altera Corporation
// All rights reserved.
// Copyright(c) 2007-2016, Intel Corporation
// All rights reserved.
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
/// @file mmlink_server.h
/// @brief Basic AFU interaction.
/// @ingroup SigTap
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Sadruta Chandrashekar, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/19/2016     SC       Initial version started based on Altera's sample code.@endverbatim
//****************************************************************************

#ifndef MMLINK_SERVER_H
#define MMLINK_SERVER_H

#include <aalsdk/AAL.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/param.h>

class mmlink_connection;
class mm_debug_link_interface;

using namespace AAL;

class mmlink_server
{
public:
  mmlink_server(struct sockaddr_in *sock, mm_debug_link_interface *driver);
  ~mmlink_server();
  int run(btVirtAddr stpAddr);
  void stop(void) { m_running = false; }
  int get_server_id(void) { return m_server_id; }
  mm_debug_link_interface *get_driver_fd(void) { return m_driver; }
  void print_stats(void);

private:
  int m_listen;
  int m_server_id;
  static const size_t MAX_CONNECTIONS = 2;

  int m_num_bound_connections;
  int m_num_connections;

  bool m_t2h_pending;
  bool m_h2t_pending;
  int handle_t2h(mmlink_connection *data_conn, bool can_read_driver, bool can_write_host);
  int handle_h2t(mmlink_connection *data_conn, bool can_read_host, bool can_write_driver);

  struct sockaddr_in m_addr;
  bool m_running;

  mm_debug_link_interface *m_driver;

  class mmlink_stats;
  mmlink_stats *m_t2h_stats;
  mmlink_stats *m_h2t_stats;

  int setup_listen_socket();
  void get_welcome_message(char *msg, size_t msg_len);

  mmlink_connection **m_conn;
  mmlink_connection *get_unused_connection();
  mmlink_connection *handle_accept();
  void close_other_data_connection(mmlink_connection *pc);
  mmlink_connection *get_data_connection(void);

#undef ENABLE_MMLINK_STATS
// #define ENABLE_MMLINK_STATS
  class mmlink_stats {
    public:
#ifdef ENABLE_MMLINK_STATS
      mmlink_stats(const char *name) { m_name = name; init(); }
      void init(void) {
        m_num_bytes = 0;
        m_last_count = 0;
        m_num_packets = 0;
        m_overflow_size = 0;
        m_min_count = UINT_MAX;
        m_max_count = 0;
        for (int i = 0; i < BUFSIZE; i++)
          m_histogram[i] = 0;
      }
      void print(void) {
        DPRINT("%s stats:\n", m_name);
        DPRINT_RAW("total packets transmitted: %u\n", m_num_packets);
        DPRINT_RAW("total bytes transmitted: %u\n", m_num_bytes);
        DPRINT_RAW("last transmission: (%u bytes):\n", m_last_count);
        DPRINT_RAW("min count: %u\n", m_min_count);
        DPRINT_RAW("max count: %u\n", m_max_count);
        for (int i = 0; i < m_last_count; ++i)
          DPRINT_RAW("0x%02X ", m_last_buf[i]);
        DPRINT_RAW("\n");
        DPRINT_RAW("histogram of count values:\n");
        for (int i = 0; i < m_max_count; i++)
          if (m_histogram[i])
            DPRINT_RAW("%d: %u\n", i, m_histogram[i]);

        if (m_overflow_size)
        {
          DPRINT_RAW("Warning: input data was larger than BUFSIZE; packet counts may be inaccurate.\n");
          DPRINT_RAW("Consider changing BUFSIZE to %u (largest input data seen\n", BUFSIZE);
        }
      }

      void update(int count, char *buf) {
        m_num_bytes += count;
        m_last_count = MIN(count, BUFSIZE);
        m_histogram[m_last_count]++;
        memcpy(m_last_buf, buf, m_last_count);

        m_min_count = MIN(m_min_count, count);
        m_max_count = MAX(m_max_count, count);

        for (int i = 0; i < count; ++i)
        {
          if (count > BUFSIZE)
          {
            m_overflow_size = MAX(m_overflow_size, count);
            break;
          }
          if (buf[i] == 0x7B)
            m_num_packets++;
        }
      }
    private:
      const char *m_name;
      unsigned int m_num_bytes;
      static const size_t BUFSIZE = 1073741824;
      unsigned char m_last_buf[BUFSIZE];
      unsigned int m_histogram[BUFSIZE];
      unsigned char m_last_count;
      unsigned int m_num_packets;
      unsigned int m_overflow_size;
      unsigned m_min_count;
      unsigned m_max_count;
#else
      mmlink_stats(const char *name) { }
      void init(void) { }
      void print(void) { }
      void update(int count, char *buf) { }
#endif
  };

};

#endif
