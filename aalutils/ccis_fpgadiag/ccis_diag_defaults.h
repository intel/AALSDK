#ifndef __CCIS_DIAG_DEFAULTS_H__
#define __CCIS_DIAG_DEFAULTS_H__

#include "nlb-specific.h"

#define DEFAULT_BEGINCL     	NLB_MIN_CL
#define DEFAULT_ENDCL       	NLB_MIN_CL
#define DEFAULT_DSMPHYS     	0
#define DEFAULT_SRCPHYS     	0
#define DEFAULT_DSTPHYS     	0
#define DEFAULT_WARMFPGACACHE 	"off"
#define DEFAULT_COOLFPGACACHE 	"on"
#define DEFAULT_COOLCPUCACHE 	"off"
#define DEFAULT_NOBW        	"off"
#define DEFAULT_TABULAR     	"on"
#define DEFAULT_SUPPRESSHDR 	"off"
#define DEFAULT_WT          	"off"
#define DEFAULT_WB          	"on"
#define DEFAULT_RDS         	"on"
#define DEFAULT_RDI         	"off"
#define DEFAULT_RDO         	"off"
#define DEFAULT_CONT        	"off"
#define DEFAULT_TONSEC      	0
#define DEFAULT_TOUSEC      	0
#define DEFAULT_TOMSEC      	(DEFAULT_NLB_CONT_TIMEOUT_NS / NANOSEC_PER_MILLI(1))
#define DEFAULT_TOSEC       	0
#define DEFAULT_TOMIN       	0
#define DEFAULT_TOHOUR      	0
#define DEFAULT_POLL       		"on"
#define DEFAULT_CSR_WRITE  		"off"
#define DEFAULT_UMSG_DATA  		"off"
#define DEFAULT_UMSG_HINT  		"off"
#define DEFAULT_AUTO_CH			"on"
#define DEFAULT_QPI				"off"
#define DEFAULT_PCIE0			"off"
#define DEFAULT_PCIE1			"off"
//#define DEFAULT_FPGA_CLK_FREQ 	200000000ULL

#endif
