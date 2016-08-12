#include <appconstants.h>

const btWSSize    LPBK1_DSM_SIZE           = appCL(1);
const btWSSize    LPBK1_BUFFER_SIZE        = appMB(4);

// MMIO address space - global
const btCSROffset PORT_CAPABILITY = 0x0030;
const btCSROffset PORT_CONTROL    = 0x0038;
const btCSROffset PORT_STATUS     = 0x0040;

// MMIO address space - AFU specific
const btCSROffset CSR_CTL                  = 0x0138;
const btCSROffset CSR_SRC_ADDR             = 0x0120;
const btCSROffset CSR_DST_ADDR             = 0x0128;
const btCSROffset CSR_CFG                  = 0x0140;
const btCSROffset CSR_NUM_LINES            = 0x0130;
const btCSROffset CSR_AFU_DSM_BASEL        = 0x0110;
const btCSROffset CSR_AFU_DSM_BASEH        = 0x0114;
const btCSROffset DSM_STATUS_TEST_COMPLETE = 0x040;

// Port status register
const btPhysAddr  NLB_TEST_MODE_PCIE0      = 0x2000;
