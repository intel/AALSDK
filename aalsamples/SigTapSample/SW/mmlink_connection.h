
#ifndef MMLINK_CONNECTION_H
#define MMLINK_CONNECTION_H

#include <sys/socket.h>
#include <unistd.h>

#include "mm_debug_link_interface.h"
#include "mmlink_server.h"

class mmlink_connection
{
public:
  // m_bufsize is the size of the buffer for h2t data
  mmlink_connection(mmlink_server *server) : m_bufsize(3000) { m_buf = new char[m_bufsize]; init(server); }
  ~mmlink_connection() { close_connection(); delete[] m_buf; }
  bool is_open() { return m_fd >= 0; }
  bool is_data() { return m_is_data; }
  bool is_bound() { return m_is_bound; }
  void set_is_data(void) { m_is_data = true; }

  size_t send(const char *msg, const size_t len);
  void close_connection() { if (is_open()) ::close(m_fd); init(); }
  void bind() { m_is_bound = true; }
  void socket(int socket) { m_fd = socket; }
  int socket() { return m_fd; }
  int handle_receive();
  int handle_management(void);

  char *buf(void) { return m_buf; }
  void buf_end(size_t index) { m_buf_end = index; }
  size_t buf_end(void) { return m_buf_end; }

  static const char *UNKNOWN;
  static const char *OK;

protected:
  int m_fd;
  bool m_is_bound;
  bool m_is_data;
  const int m_bufsize;
  mmlink_server *m_server;

  char *m_buf;
  size_t m_buf_end;

  void init(mmlink_server *server) { m_server = server; init(); }

private:
  int handle_data(void);
  int handle_management_command(char *cmd);
  int handle_unbound_command(char *cmd);
  int handle_bound_command(char *cmd);
  int get_server_id(void) { return m_server->get_server_id(); }
  mm_debug_link_interface *driver(void) { return m_server->get_driver_fd(); }
  void init(void) { m_fd = -1; m_is_bound = false; m_is_data = false; m_buf_end = 0; }
};

#endif
