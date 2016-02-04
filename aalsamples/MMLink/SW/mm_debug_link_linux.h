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
/// @file mm_debug_link_linux.h
/// @brief Basic AFU interaction.
/// @ingroup SigTap
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
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
  static const size_t BUFSIZE = 1073741824; // size of buffer for t2h data
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

