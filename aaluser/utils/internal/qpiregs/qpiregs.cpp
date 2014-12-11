// INTEL CONFIDENTIAL

// FOR INTEL INTERNAL USE ONLY

#include <iostream>
#include <iomanip>

#include <cstdlib>

extern "C" {
#include <pci/pci.h>
}

using namespace std;

#ifndef PCI_VENDOR_ID_INTEL
#define PCI_VENDOR_ID_INTEL          0x8086
#endif // PCI_VENDOR_ID_INTEL

#ifndef PCI_DEVICE_ID_INTEL_QPILINK0
#define PCI_DEVICE_ID_INTEL_QPILINK0 0x3c80
#endif // PCI_DEVICE_ID_INTEL_QPILINK0

#ifndef QPILINK0_BUS
#define QPILINK0_BUS 0x7f
#endif // QPILINK0_BUS
#ifndef QPILINK0_DEV
#define QPILINK0_DEV    8
#endif // QPILINK0_DEV
#ifndef QPILINK0_FN
#define QPILINK0_FN     0
#endif // QPILINK0_FN

#define QPILINK0 QPILINK0_BUS,QPILINK0_DEV,QPILINK0_FN

#ifndef QPILS_CFG_OFFSET
#define QPILS_CFG_OFFSET      0x48
#endif // QPILS_CFG_OFFSET

#ifndef QPIERRCNT0_CFG_OFFSET
#define QPIERRCNT0_CFG_OFFSET 0x78
#endif // QPIERRCNT0_CFG_OFFSET

#ifndef QPIERRCNT1_CFG_OFFSET
#define QPIERRCNT1_CFG_OFFSET 0x7c
#endif // QPIERRCNT1_CFG_OFFSET


ostream & PrintDev(ostream & , struct pci_access * , struct pci_dev * );
ostream & ShowQPILS(ostream & , u32 );


int main(int argc, char *argv[])
{
   struct pci_access *pcia   = NULL;
   struct pci_dev    *pcidev = NULL;
   int                res    = 0;
   u32                lreg;

   if ( NULL == (pcia = pci_alloc()) ) {
      cerr << "pci_alloc() failed." << endl;
      res = 1;
      goto ERROR;
   }

   pci_init(pcia);

   if ( NULL == ( pcidev = pci_get_dev(pcia, 0, QPILINK0) ) ) {
      cerr << "pci_get_dev() failed." << endl;
      res = 2;
      goto ERROR;
   }

   res = pci_fill_info(pcidev, PCI_FILL_IDENT     |
//                               PCI_FILL_IRQ       |
                               PCI_FILL_BASES     |
//                               PCI_FILL_ROM_BASE  |
                               PCI_FILL_SIZES     |
                               PCI_FILL_CLASS     |
                               PCI_FILL_CAPS      |
                               PCI_FILL_EXT_CAPS
//                               PCI_FILL_PHYS_SLOT
                      );
   if ( res < 0 ) {
      cerr << "pci_fill_info() failed." << endl;
      res = 3;
      goto ERROR;
   }

   if ( ( PCI_VENDOR_ID_INTEL          != pcidev->vendor_id ) ||
        ( PCI_DEVICE_ID_INTEL_QPILINK0 != pcidev->device_id ) ) {
      cerr.flags(ios_base::right | ios_base::hex);
      cerr.fill('0');

      cerr << "VID/DID mismatch. actual=" <<
              "0x"  << setw(4) << (u16)pcidev->vendor_id <<
              "/0x" << setw(4) << (u16)pcidev->device_id <<
              " expected=" <<
              "0x"  << setw(4) << (u16)PCI_VENDOR_ID_INTEL <<
              "/0x" << setw(4) << (u16)PCI_DEVICE_ID_INTEL_QPILINK0 << endl;

      res = 4;
      goto ERROR;
   }

   PrintDev(cout, pcia, pcidev) << endl;

   // read/print QPILS
   lreg = pci_read_long(pcidev, QPILS_CFG_OFFSET);
   ShowQPILS(cout, lreg) << endl;

   // read/print QPIERRCNT0
   lreg = pci_read_long(pcidev, QPIERRCNT0_CFG_OFFSET);
   cout << "QPIERRCNT0=" << lreg << endl;

   // read/print QPIERRCNT1
   lreg = pci_read_long(pcidev, QPIERRCNT1_CFG_OFFSET);
   cout << "QPIERRCNT1=" << lreg << endl;

ERROR:

   if ( NULL != pcidev ) {
      pci_free_dev(pcidev);
   }

   if ( NULL != pcia ) {
      pci_cleanup(pcia);
   }

   return res;
}

ostream & PrintDev(ostream &os, struct pci_access *pcia, struct pci_dev *pcidev)
{
   char          name[256];
   ios::fmtflags fl = os.flags();
   char          fi = os.fill();

   os.flags(ios_base::right | ios_base::hex);
   os.fill('0');

   os << "0x" << setw(4) << (u16)pcidev->domain << ':' <<
         "0x" << setw(2) << (u16)pcidev->bus    << ':' <<
         "0x" << setw(2) << (u16)pcidev->dev    << '.';

   os.flags(ios_base::left | ios_base::dec);
   os.fill(' ');

   os << (u16)pcidev->func;

   os.flags(ios_base::right | ios_base::hex);
   os.fill('0');

   os << "  VID/DID=" <<
      "0x" << setw(4) << (u16)pcidev->vendor_id << '/' <<
      "0x" << setw(4) << (u16)pcidev->device_id;

   *name = 0;
   pci_lookup_name(pcia, name, sizeof(name), PCI_LOOKUP_DEVICE, pcidev->vendor_id, pcidev->device_id);

   os << "  name=\"" << name << "\"";

   os.fill(fi);
   os.flags(fl);
   return os;
}

// Link Layer Retry Queue Consumed
#define QPILS_LLRQC_SHIFT               28
#define QPILS_LLRQC_MASK                0x70000000
#define QPILS_LLRQC_VALUES \
"0 to 7",                  \
"8 to 15",                 \
"16 to 31",                \
"32 to 63",                \
"64 to 95",                \
"96 to 127",               \
"128 to 191",              \
"192 to 255"
// Link Initialization Status
#define QPILS_LIS_SHIFT                 24
#define QPILS_LIS_MASK                  0x0f000000
#define QPILS_LIS_VALUES              \
"Waiting for Physical Layer Ready",   \
"Internal Stall Link Initialization", \
"Sending ReadyForInit",               \
"Parameter Exchange",                 \
"Sending ReadyForNormalOperation",    \
NULL,                                 \
"Normal Operation",                   \
"Link Level Retry (LRSM)",            \
"Link Abort (LRSM)",                  \
"Param_Ex_Done Stalled",              \
"Param_Ex_Done Proceeding",           \
"WaitForNormal",                      \
"LocalLinkReset"
// Link Initialization Failure Count
#define QPILS_LIFC_SHIFT                22
#define QPILS_LIFC_MASK                 0x00c00000
#define QPILS_LIFC_VALUES \
"0",                      \
"1",                      \
"2-15",                   \
">15"
// Last Link Level Retry NUM_PHY_REINIT
#define QPILS_LLLR_NUM_PHY_REINIT_SHIFT 21
#define QPILS_LLLR_NUM_PHY_REINIT_MASK  0x00200000
#define QPILS_LLLR_NUM_PHY_REINIT_VALUES \
"0",                                     \
"1+"
// Last Link Level Retry NUM_RETRY
#define QPILS_LLLR_NUM_RETRY_SHIFT      19
#define QPILS_LLLR_NUM_RETRY_MASK       0x00180000
#define QPILS_LLLR_NUM_RETRY_VALUES \
"0",                                \
"1",                                \
"2-15",                             \
">15"
// VNA Credits at Receiver
#define QPILS_VNACAR_SHIFT              16
#define QPILS_VNACAR_MASK               0x00070000
#define QPILS_VNACAR_VALUES \
"0 credits",                \
"1-7 credits",              \
"8-11 credits",             \
"12-15 credits",            \
"16-31 credits",            \
"32-63 credits",            \
"64-127 credits",           \
"128+ credits"
// VN0 Snp Credits at Receiver
#define QPILS_VN0SCAR_SHIFT             15
#define QPILS_VN0SCAR_MASK              0x00008000
#define QPILS_VN0SCAR_VALUES \
"0 Credits",                 \
"1+ Credits"
// VN0 Hom Credits at Receiver
#define QPILS_VN0HCAR_SHIFT             14
#define QPILS_VN0HCAR_MASK              0x00004000
#define QPILS_VN0HCAR_VALUES \
"0 Credits",                 \
"1+ Credits"
// VN0 NDR Credits at Receiver
#define QPILS_VN0NDRCAR_SHIFT           13
#define QPILS_VN0NDRCAR_MASK            0x00002000
#define QPILS_VN0NDRCAR_VALUES \
"0 Credits",                   \
"1+ Credits"
// VN0 DRS Credits at Receiver
#define QPILS_VN0DRSCAR_SHIFT           12
#define QPILS_VN0DRSCAR_MASK            0x00001000
#define QPILS_VN0DRSCAR_VALUES \
"0 Credits",                   \
"1+ Credits"
// VN0 NCS Credits at Receiver
#define QPILS_VN0NCSCAR_SHIFT           11
#define QPILS_VN0NCSCAR_MASK            0x00000800
#define QPILS_VN0NCSCAR_VALUES \
"0 Credits",                   \
"1+ Credits"
// VN0 NCB Credits at Receiver
#define QPILS_VN0NCBCAR_SHIFT           10
#define QPILS_VN0NCBCAR_MASK            0x00000400
#define QPILS_VN0NCBCAR_VALUES \
"0 Credits",                   \
"1+ Credits"
// VN1 Snp Credits at Receiver
#define QPILS_VN1SCAR_SHIFT             7
#define QPILS_VN1SCAR_MASK              0x00000080
#define QPILS_VN1SCAR_VALUES \
"0 Credits",                 \
">0 Credits"
// VN1 Hom Credits at Receiver
#define QPILS_VN1HCAR_SHIFT             6
#define QPILS_VN1HCAR_MASK              0x00000040
#define QPILS_VN1HCAR_VALUES \
"0 Credits",                 \
">0 Credits"
// VN1 NDR Credits at Receiver
#define QPILS_VN1NDRCAR_SHIFT           5
#define QPILS_VN1NDRCAR_MASK            0x00000020
#define QPILS_VN1NDRCAR_VALUES \
"0 Credits",                   \
">0 Credits"
// VN1 DRS Credits at Receiver
#define QPILS_VN1DRSCAR_SHIFT           4
#define QPILS_VN1DRSCAR_MASK            0x00000010
#define QPILS_VN1DRSCAR_VALUES \
"0 Credits",                   \
">0 Credits"
// VN1 NCS Credits at Receiver
#define QPILS_VN1NCSCAR_SHIFT           3
#define QPILS_VN1NCSCAR_MASK            0x00000008
#define QPILS_VN1NCSCAR_VALUES \
"0 Credits",                   \
">0 Credits"
// VN1 NCB Credits at Receiver
#define QPILS_VN1NCBCAR_SHIFT           2
#define QPILS_VN1NCBCAR_MASK            0x00000004
#define QPILS_VN1NCBCAR_VALUES \
"0 Credits",                   \
">0 Credits"

#define DISPLAY_FIELD(ostr, reg, field, descr)                  \
do                                                              \
{  const char *_values[] = { field##_VALUES };                  \
   const u32 _uval = ( (reg) & field##_MASK ) >> field##_SHIFT; \
   ostr << descr << " = ";                                      \
   if ( _uval < ( sizeof(_values) / sizeof(_values[0]) ) ) {    \
      if ( NULL == _values[_uval] ) {                           \
         ostr << "<reserved>";                                  \
      } else {                                                  \
         ostr << _values[_uval];                                \
      }                                                         \
   } else {                                                     \
      ostr << "<invalid>";                                      \
   }                                                            \
}while(0)

ostream & ShowQPILS(ostream &os, u32 reg)
{
   ios::fmtflags fl = os.flags();
   char          fi = os.fill();

   os.flags(ios_base::right | ios_base::hex);
   os.fill('0');

   os << "QPILS=0x" << setw(8) << reg << endl;

   os.flags(ios_base::left | ios_base::dec);
   os.fill(' ');

   DISPLAY_FIELD(os, reg, QPILS_LLRQC,               " Link Layer Retry Queue Consumed");      os << endl;
   DISPLAY_FIELD(os, reg, QPILS_LIS,                 " Link Initialization Status");           os << endl;
   DISPLAY_FIELD(os, reg, QPILS_LIFC,                " Link Initialization Failure Count");    os << endl;
   DISPLAY_FIELD(os, reg, QPILS_LLLR_NUM_PHY_REINIT, " Last Link Level Retry NUM_PHY_REINIT"); os << endl;
   DISPLAY_FIELD(os, reg, QPILS_LLLR_NUM_RETRY,      " Last Link Level Retry NUM_RETRY");      os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VNACAR,              " VNA Credits at Receiver");              os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN0SCAR,             " VN0 Snp Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN0HCAR,             " VN0 Hom Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN0NDRCAR,           " VN0 NDR Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN0DRSCAR,           " VN0 DRS Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN0NCSCAR,           " VN0 NCS Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN0NCBCAR,           " VN0 NCB Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN1SCAR,             " VN1 Snp Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN1HCAR,             " VN1 Hom Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN1NDRCAR,           " VN1 NDR Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN1DRSCAR,           " VN1 DRS Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN1NCSCAR,           " VN1 NCS Credits at Receiver");          os << endl;
   DISPLAY_FIELD(os, reg, QPILS_VN1NCBCAR,           " VN1 NCB Credits at Receiver");          os << endl;

   os.fill(fi);
   os.flags(fl);
   return os;
}

