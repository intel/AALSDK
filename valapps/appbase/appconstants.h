#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

// the configured options and settings for APPCONSTANTS
#define APPCONSTANTS_VERSION_MAJOR @APPCONSTANTS_VERSION_MAJOR@
#define APPCONSTANTS_VERSION_MINOR @APPCONSTANTS_VERSION_MINOR@

#include <aalsdk/AALTypes.h>

using namespace AAL;

extern const btCSROffset PORT_CAPABILITY;
extern const btCSROffset PORT_CONTROL;
extern const btCSROffset PORT_STATUS;
extern const btWSSize    LPBK1_DSM_SIZE;
extern const btWSSize    LPBK1_BUFFER_SIZE;
extern const btCSROffset CSR_SRC_ADDR;
extern const btCSROffset CSR_DST_ADDR;
extern const btCSROffset CSR_CTL;
extern const btCSROffset CSR_CFG;
extern const btCSROffset CSR_NUM_LINES;
extern const btCSROffset CSR_AFU_DSM_BASEL;
extern const btCSROffset CSR_AFU_DSM_BASEH;
extern const btCSROffset DSM_STATUS_TEST_COMPLETE;
extern const btPhysAddr  NLB_TEST_MODE_PCIE0;

static inline btWSSize appCL(btWSSize x) { return (x) * 64; }
static inline btWSSize appMB(btWSSize x) { return (x) * 1024 * 1024; }

#endif
