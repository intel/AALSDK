
#ifndef MM_DEBUG_LINK_LINUX_H
#define MM_DEBUG_LINK_LINUX_H

#include <aalsdk/AAL.h>
#include <unistd.h>

#include "mm_debug_link_interface.h"

using namespace AAL;

class mm_debug_link_linux: public mm_debug_link_interface
{
private:
  int m_fd;
  static const size_t BUFSIZE = 3000; // size of buffer for t2h data
  char m_buf[BUFSIZE];
  size_t m_buf_end;
  int m_write_fifo_capacity;
  btVirtAddr map_base;

public:
  mm_debug_link_linux() { m_fd = -1; m_buf_end = 0; m_write_fifo_capacity = 0;}
  int open(btVirtAddr stpAddr);
  void* read_mmr(btCSROffset target, int access_type);
  void write_mmr(off_t target, int access_type, unsigned int write_val);
  ssize_t read();
  ssize_t write( const void *buf, size_t count);
  void close(void);
  void ident(int id[4]);
  void write_ident(int val);
  void reset(bool val);
  void enable(int channel, bool state);
  int get_fd(void) { return m_fd; }

  char *buf(void) { return m_buf; }
  bool is_empty(void) { return m_buf_end == 0; }
  bool flush_request(void);
  size_t buf_end(void) { return m_buf_end; }
  void buf_end(int index) { m_buf_end = index; }
};

#endif

