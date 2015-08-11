#ifndef __DIAG_DEFAULTS_H__
#define __DIAG_DEFAULTS_H__

#include "nlb-specific.h"

#define DEFAULT_BEGINCL     	NLB_MIN_CL
#define DEFAULT_ENDCL       	NLB_MIN_CL
#define DEFAULT_DSMPHYS     	0
#define DEFAULT_SRCPHYS     	0
#define DEFAULT_DSTPHYS     	0
#define DEFAULT_PREFILLHITS 	"no"
#define DEFAULT_PREFILLMISS 	"no"
#define DEFAULT_NOBW        	"no"
#define DEFAULT_TABULAR     	"no"
#define DEFAULT_SUPPRESSHDR 	"no"
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
#define DEFAULT_POLL       		"yes"
#define DEFAULT_CSR_WRITE  		"no"
#define DEFAULT_UMSG_DATA  		"no"
#define DEFAULT_UMSG_HINT  		"no"
//#define DEFAULT_FPGA_CLK_FREQ 	200000000ULL

#endif
