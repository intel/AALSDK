// INTEL CONFIDENTIAL - For Intel Internal Use Only

// valapps/PartialReconf_Power/main.cpp

#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aalclp/aalclp.h>

#include <math.h>
#include <sched.h>

using namespace std;
using namespace AAL;

#define  PWR_MSR610                      "/usr/sbin/rdmsr -c0 -p %d 0x610"
#define  PWR_MSR606                      "/usr/sbin/rdmsr -c0 -p %d 0x606"
#define  SKX_CPU_SPLIT_POINT             48


// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl


/// Command Line
BEGIN_C_DECLS

int pr_power_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int pr_power_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

struct  PRPowerCommandLine
{
   btUIntPtr          flags;
#define PR_POWER_CMD_FLAG_HELP      0x00000001
#define PR_POWER_CMD_FLAG_VERSION   0x00000002
#define PR_POWER_CMD_PARSE_ERROR    0x00000003

   char    bitstream_file[200];
   int     socket;


};
struct PRPowerCommandLine configCmdLine = { 0,"",0 };


int pr_power_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption) {
   struct PRPowerCommandLine *cl = (struct PRPowerCommandLine *)user;
   flag_setf(cl->flags, PR_POWER_CMD_PARSE_ERROR);
   printf("Invalid: %s\n", nonoption);
   return 0;
}

int pr_power_on_dash_only(AALCLP_USER_DEFINED user) {
   struct PRPowerCommandLine *cl = (struct PRPowerCommandLine *)user;
   flag_setf(cl->flags, PR_POWER_CMD_PARSE_ERROR);
   printf("Invalid option: -\n");
   return 0;
}

int pr_power_on_dash_dash_only(AALCLP_USER_DEFINED user) {
   struct PRPowerCommandLine *cl = (struct PRPowerCommandLine *)user;
   flag_setf(cl->flags, PR_POWER_CMD_PARSE_ERROR);
   printf("Invalid option: --\n");
   return 0;
}

int pr_power_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct PRPowerCommandLine *cl = (struct PRPowerCommandLine *)user;
   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, PR_POWER_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, PR_POWER_CMD_FLAG_VERSION);
   }else  if(0 != strcmp("--bitstream=", option))  {
      printf("Invalid option  : %s\n", option);
      flag_setf(cl->flags, PR_POWER_CMD_PARSE_ERROR);

   }
   return 0;
}


int pr_power_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct PRPowerCommandLine *pcmdline     = (struct PRPowerCommandLine *)user;

   // Bitstream file name
   if ( 0 == strcmp("--bitstream", option)) {
      strcpy(pcmdline->bitstream_file ,value);
      return 0;
   }

   // socket number
   if ( 0 == strcmp("--socket", option)) {
      char *endptr = NULL;
      pcmdline->socket = strtoul(value, &endptr, 0);
      return 0;
   }


   return 0;
}

aalclp_option_only  pr_power_nix_long_option_only  = { pr_power_on_nix_long_option_only,  };
aalclp_option       pr_power_nix_long_option       = { pr_power_on_nix_long_option,       };
aalclp_non_option   pr_power_non_option            = { pr_power_on_non_option,            };
aalclp_dash_only    pr_power_dash_only             = { pr_power_on_dash_only,             };
aalclp_dash_only    pr_power_dash_dash_only        = { pr_power_on_dash_dash_only,        };

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "PartialReconf_Power",
                             "0",
                             "",
                             help_msg_callback,
                             &configCmdLine)

int ParseCmds(struct PRPowerCommandLine *pconfigcmd, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   pr_power_nix_long_option_only.user = pconfigcmd;
   aalclp_add_nix_long_option_only(&clp, &pr_power_nix_long_option_only);

   pr_power_nix_long_option.user = pconfigcmd;
   aalclp_add_nix_long_option(&clp, &pr_power_nix_long_option);

   pr_power_non_option.user = pconfigcmd;
   aalclp_add_non_option(&clp, &pr_power_non_option);

   pr_power_dash_only.user             = pconfigcmd;
   aalclp_add_dash_only(&clp,             &pr_power_dash_only);

   pr_power_dash_dash_only.user        = pconfigcmd;
   aalclp_add_dash_dash_only(&clp,        &pr_power_dash_dash_only);

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_add_gcs_compliance() failed : " << res << ' ' << strerror(res) << endl;
      goto CLEANUP;
   }

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      cerr << "aalclp_scan_argv() failed : " << res << ' ' << strerror(res) << endl;
   }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      cerr << "aalclp_destroy() failed : " << clean << ' ' << strerror(clean) << endl;
   }

   return res;
}


void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "Usage:\n");
   fprintf(fp, "    PartialReconf_Power [--bitstream=<FILENAME>] [--socket=<SOCKET NUMBER>] \n \
   Example: ./PartialReconf_Power --bitstream=/home/lab/bitstream/test.rbf --socket=0 \n");
   fprintf(fp, "\n");

}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

int verifycmds(struct PRPowerCommandLine *cl)
{
   std::ifstream bitfile(cl->bitstream_file,std::ios::binary);

   if(!bitfile.good()) {
      printf("Invalid File : %s\n", cl->bitstream_file);
      return 3;
   }
   return 0;
}

END_C_DECLS

// Green Bit stream Header
struct __attribute__((__packed__))  CCIP_GBS_HEADER {

   btByte                          m_digst[256];                   // Digital signature
   btByte                          m_mesgDist[32];                 // Message digest
   btByte                          m_intelpub_key[260];            // Intel public key
   btByte                          m_hash_pubkey[32];              // Hash of public key
   btByte                          m_md_afu[16];                   // GB meta Data structure
   btByte                          m_md_slot_type_uuid[16];        // Partial bitstream slot type UUID
   btUnsigned32bitInt              m_md_afu_power;                 // Partial bitstream power
   btByte                          m_md_port;                      // Partial bitstream port
   btUnsigned16bitInt              m_md_latency;                   // Partial bitstream latency
   btUnsigned32bitInt              m_md_clknum;                    // Partial bitstream clock number
   btUnsigned32bitInt              m_md_btlength;                  // Partial bitstream length *

};

// ALI Reconfigure application path
#define ALI_RECONF_APP        "../../aalsamples/ALI_Configure_AFU/SW/aliconfafu --bitstream="
#define SPLITPOINT_SCRIPT     "../../aalutils/cpuset/find_split_point.sh /proc/cpuinfo"

btInt GetBitStreamPower(btcString filename,btInt &GBSPower);
btInt Calculate_NumCoreIdle(btInt FPIWatts, btInt socket,btInt* coreCount);
btInt Calculate_splitpint(btInt* split_point);


int main(int argc, char *argv[])
{
   btInt res                  = 0;
   btInt Fpga_PwrRequired     = 0;
   btInt CoreCount            = 0;
   int split_point            = 0;
   std::string filename ;
   std::string command ;


   if ( argc < 2 ) {
      showhelp(stdout, &_aalclp_gcs_data);
      return 1;
   } else if ( 0!= ParseCmds(&configCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   }else if ( flag_is_set(configCmdLine.flags, PR_POWER_CMD_FLAG_HELP|PR_POWER_CMD_FLAG_VERSION) ) {
      return 0;
   } else if ( verifycmds(&configCmdLine) ) {
      return 3;
   }

   if(getuid()) {
       ERR(" export LD_LIBRARY_PATH in root and Run PartialReconf_Power with root privialge ");
       exit(1);
   }

   std::cout << "Bitstream File Name:" << configCmdLine.bitstream_file <<std::endl;
   std::cout << "Socket:" << configCmdLine.socket <<std::endl;

   // Get Green bitstream power
   res = GetBitStreamPower(configCmdLine.bitstream_file ,Fpga_PwrRequired);
   if( res !=0) {
      ERR("Failed to parse bit stream header\n");
      exit(1);
   }
   std::cout << "Fpga_PwrRequired:" << Fpga_PwrRequired <<"Watts" <<std::endl;

   // Calcualte CPU split pint
   res = Calculate_splitpint(&split_point);
   if( res !=0) {
      ERR("Failed to calculate split pint\n");
      exit(1);
   }

   // Calcualte number of cores to be idle
   res = Calculate_NumCoreIdle(Fpga_PwrRequired, configCmdLine.socket,&CoreCount);
   if( res !=0) {
      ERR("Failed to calculate bitstream required power\n");
      exit(1);
   }

   //std::cout << "Total Number of CPU cores :" << 48 <<std::endl;
   //std::cout << "Total Number of CPU cores IDLE after PR :" << 48 -CoreCount <<std::endl;
   //std::cout << "Total Number of CPU cores ONNLINE after PR :" << CoreCount <<std::endl;

   std::cout << "Run PowerTop tool before after PR ,Check how many cores Idle" <<std::endl;

   // PR Green bitsream
   command.append(ALI_RECONF_APP);
   command.append(configCmdLine.bitstream_file);
   std::cout << "command:" << command <<std::endl;
   res = system(command.c_str());

   return res;
}

btInt GetBitStreamPower(btcString filename,btInt &GBSPower)
{
   btInt res                                = 0 ;
   btByte *bufptr                           = NULL;
   std::streampos filesize                  = 0;
   struct CCIP_GBS_HEADER *pGBS_Header      = NULL;

   std::ifstream bitfile(filename, std::ios::binary );

   if(!bitfile.good()) {

      ERR("File path is not valid\n");
      res = ENOENT;
      return res;
   }

   bitfile.seekg( 0, std::ios::end );
   filesize = bitfile.tellg();
   if(0 == filesize) {

      ERR("File Size is not valid\n");
      res = EBADF;
      return res;
   }

   bitfile.seekg( 0, std::ios::beg );
   bufptr = new(std::nothrow) btByte[filesize];
   if(NULL == bufptr) {

      ERR("faied to allacote buffer \n");
      res = ENOMEM;
      return res;
    }

    bitfile.read(reinterpret_cast<char *>(bufptr), filesize);

    pGBS_Header = (struct CCIP_GBS_HEADER*)   bufptr;

    std::cout << " Green Bitstream Power Required " << pGBS_Header->m_md_afu_power << std::endl;

    GBSPower = pGBS_Header->m_md_afu_power;

    if(bufptr)
       delete bufptr;

   return res;
}


btInt Calculate_splitpint(btInt* split_point)
{
   FILE *fp                    = NULL;
   int ret_val                 = 0;
   char data[1024]             = {0};
   char *endptr                = NULL;

   fp = popen(SPLITPOINT_SCRIPT, "r");
   if (NULL == fp) {
      printf("Failed to open Split pint script");
      ret_val = EINVAL;
      return  ret_val;
   }

   while (fgets(data, sizeof(data)-1, fp) != NULL) {
      printf("%s", data);
   }

   *split_point = strtoll(data, &endptr, 2);
   printf("split_point: %d \n", *split_point);
   ret_val = pclose(fp);

   return ret_val;
}

btInt Calculate_NumCoreIdle(btInt FPIWatts, btInt socket,btInt* coreCount)
{

   FILE *fp                    = NULL;

   int64_t PackPwrLimit1       = 0;
   int64_t PowerUnitValue      = 0;
   int64_t PackagePowerUnit    = 0;
   btInt  CoreCount            = 0;
   int64_t MaxThreadVal        = 0;

   int ret_val                 = 0;
   int split_point             = 0;
   int max_pid_index           = 0;
   int pid                     = 0;
   int i                       = 0;

   long double TotalWatts      = 0;
   long double AvailableWatts  = 0;
   long double FpgaWatts       = 0;

   char MaxThreadValStr[20]    = {0};
   char data[1024]             = {0};
   char data1[1024]            = {0};
   char command610[40]         = {0};
   char command606[40]         = {0};
   char *endptr                = NULL;

   cpu_set_t idle_set, current_set, full_mask_set;

   // Fail if socket not equal to 0 or 1
   if (socket != 0 ) {
      if (socket != 1)
         printf("Bad Socket ID");
         return ali_errnumBadSocket;
   }

   // zero array before building commands.
   for (i = 0; i < 40; i++) {
     command610[i] = 0;
     command606[i] = 0;
   }

   split_point = SKX_CPU_SPLIT_POINT; // force split temporarily, BUGBUG

   // set msr commands based on socket and split_point
   if (socket == 0) {
      sprintf(command610, PWR_MSR610,0);
      sprintf(command606, PWR_MSR606,0);
   } else {
      sprintf(command610, PWR_MSR610, split_point);
      sprintf(command606, PWR_MSR606, split_point);
   }

    FpgaWatts = (double) FPIWatts;
  //
   // Begin MSR retrieval.
   //
   //  fp = popen("/usr/sbin/rdmsr -c0 -p 0 0x610", "r");
   fp = popen(command610, "r");
   if (NULL == fp) {
      printf("Failed to open MSR 610 ");
      return(ali_errnumRdMsrCmdFail);
   }

   while (fgets(data, sizeof(data)-1, fp) != NULL) {
      printf("%s", data);
   }

   PackPwrLimit1 = strtoll(&data[2], &endptr, 16);
   printf("Power Limit converted: %lx \n", PackPwrLimit1);
   ret_val = pclose(fp);

   PackPwrLimit1 = PackPwrLimit1 & 0x07fff;

   //  fp = popen("/usr/sbin/rdmsr -c0 -p 0 0x606", "r");
   fp = popen(command606, "r");
   if (NULL == fp) {
      printf("Failed to run command\n");
      printf("Failed to open MSR 606 ");
      return(ali_errnumRdMsrCmdFail);
   }
   while (fgets(data, sizeof(data)-1, fp) != NULL) {
      printf("%s", data);
   }

   PackagePowerUnit = strtoll(&data[2], &endptr, 16);
   printf("Package Power Unit Value converted: %lx \n", PackagePowerUnit);
   ret_val = pclose(fp);

   //
   // MSR retrvial complete
   // Calculate power budget.
   //

   PowerUnitValue = PackagePowerUnit & 0x0f;
   PowerUnitValue = pow(2, PowerUnitValue);
   printf("Divisor of Raw Limit1:%lx\n", PowerUnitValue);
   TotalWatts = ((double)PackPwrLimit1)/((double)PowerUnitValue);
   printf("Total Watts: %Lf \n", TotalWatts);

   //
   // Check that at least one core will be present.
   //
   if ((TotalWatts - 5 ) <= FpgaWatts)  {
      printf("Invalid PR Power Value");
      return (ali_errnumFPGAPowerRequestTooLarge);
   }

   AvailableWatts = TotalWatts - (double)FpgaWatts;
   printf("Available Watts: %Lf\n", AvailableWatts);
   CoreCount = (btInt) AvailableWatts /  5;
   printf("Core Count: %d\n", CoreCount);

   *coreCount  = CoreCount;

   return 0;
}


