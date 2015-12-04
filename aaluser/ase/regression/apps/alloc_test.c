#include "ase_common.h"

int main()
{
  session_init();

  mmio_write32(0x1000, 0xCAFEBABE);

  mmio_write32(0x1100, 0xCAFEBABE);

  mmio_write32(0x1200, 0xCAFEBABE);

  mmio_write64(0x1008, 0xFEEDFACEDEBAFCAD);

  session_deinit();

  return 0;
}
