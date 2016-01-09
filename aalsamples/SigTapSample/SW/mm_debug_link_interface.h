
#ifndef MM_DEBUG_LINK_INTERFACE_H
#define MM_DEBUG_LINK_INTERFACE_H
#include <aalsdk/AAL.h>
#include <unistd.h>

using namespace AAL;

class mm_debug_link_interface
{
public:
  virtual int open(btVirtAddr stpAddr) = 0;
  virtual ssize_t read() = 0;
  virtual ssize_t write(const void *buf, size_t count) = 0;
  virtual void close(void) = 0;
  virtual void ident(int id[4]) = 0;
  virtual void write_ident(int val) = 0;
  virtual void reset(bool val) = 0;
  virtual void enable(int channel, bool state) = 0;
  virtual int get_fd(void) = 0;
  virtual size_t buf_end(void) = 0;
  virtual void buf_end(int index) = 0;
  virtual char *buf(void) = 0;
  virtual bool is_empty(void) = 0;
  virtual bool flush_request(void) = 0;
};

// Concrete classes must implement this routine.
mm_debug_link_interface *get_mm_debug_link(void);

#endif

