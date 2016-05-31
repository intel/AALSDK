
#include "../PR_SingleApp/ALIReconf.h"

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
//#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#define MSG(x)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl

BEGIN_C_DECLS


aalclp_option_only  aliconigafu_nix_long_option_only  = { aliconigafu_on_nix_long_option_only,  };

aalclp_option       aliconigafu_nix_long_option       = { aliconigafu_on_nix_long_option,       };
aalclp_non_option   aliconigafu_non_option            = { aliconigafu_on_non_option,            };
aalclp_dash_only    aliconigafu_dash_only             = { aliconigafu_on_dash_only,             };
aalclp_dash_only    aliconigafu_dash_dash_only        = { aliconigafu_on_dash_dash_only,        };


int aliconigafu_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption) {
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
   printf("Invalid: %s\n", nonoption);
   return 0;
}

int aliconigafu_on_dash_only(AALCLP_USER_DEFINED user) {
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
   printf("Invalid option: -\n");
   return 0;
}

int aliconigafu_on_dash_dash_only(AALCLP_USER_DEFINED user) {
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
   printf("Invalid option: --\n");
   return 0;
}

int aliconigafu_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, ALICONIFG_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, ALICONIFG_CMD_FLAG_VERSION);
   }else  if(0 != strcmp("--bitstream=", option))  {
      printf("Invalid option  : %s\n", option);
      flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);

   }else if(0 != strcmp("--reconftimeout=", option))  {
      printf("Invalid option : %s\n", option);
      flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
      return 0;
   }
   return 0;
}


int aliconigafu_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct ALIConfigCommandLine *pcmdline     = (struct ALIConfigCommandLine *)user;

   // Bitstream file name
   if ( 0 == strcmp("--bitstream", option)) {
      strcpy(pcmdline->bitstream_file ,value);
      return 0;
   }

   // Reconfigure  timeout
   if ( 0 == strcmp("--reconftimeout", option)) {
      char *endptr = NULL;
      pcmdline->reconftimeout = strtoul(value, &endptr, 0);
      return 0;
   }

   // Reconfigure  action
   if ( 0 == strcmp("--reconfaction", option)) {

      if ( 0 == strcmp("ACTION_HONOR_OWNER", value)) {
         pcmdline->reconfAction =AALCONF_RECONF_ACTION_HONOR_OWNER_ID;
      } else  {
      // Default
         pcmdline->reconfAction =AALCONF_RECONF_ACTION_HONOR_REQUEST_ID;
      }
      return 0;
   }

   // Reactive disabled
   if ( 0 == strcmp("--reactivateDisabled", option)) {

      if ( 0 == strcmp("TRUE", value))  {
         pcmdline->reactivateDisabled =true;
      } else {
      // Default
         pcmdline->reactivateDisabled =false;
      }
      return 0;
   }
   //reconfInterface
   if ( 0 == strcmp("--reconfInterface", option)) {
         strcpy(pcmdline->reconfInterface ,value);
        return 0;
     }

   return 0;
}


int ParseCmds(struct ALIConfigCommandLine *pconfigcmd, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   aliconigafu_nix_long_option_only.user = pconfigcmd;
   aalclp_add_nix_long_option_only(&clp, &aliconigafu_nix_long_option_only);

   aliconigafu_nix_long_option.user = pconfigcmd;
   aalclp_add_nix_long_option(&clp, &aliconigafu_nix_long_option);

   aliconigafu_non_option.user = pconfigcmd;
   aalclp_add_non_option(&clp, &aliconigafu_non_option);

   aliconigafu_dash_only.user             = pconfigcmd;
   aalclp_add_dash_only(&clp,             &aliconigafu_dash_only);

   aliconigafu_dash_dash_only.user        = pconfigcmd;
   aalclp_add_dash_dash_only(&clp,        &aliconigafu_dash_dash_only);

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
   fprintf(fp, "   aliconfafu [--bitstream=<FILENAME>] [--reconftimeout=<SECONDS>]  \
                [--reconfaction=<ACTION_HONOR_REQUEST or ACTION_HONOR_OWNER >]        \
                [--reactivateDisabled=< TRUE or FALSE>]\n");
   fprintf(fp, "\n");

}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

END_C_DECLS

AllocatesReconfService::AllocatesReconfService() :
   m_pRuntime(NULL),
   m_pFMEService(NULL),
   m_pALIReconfService(NULL),
   m_Result(0),
   m_errNum(0)
{
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidALI_CONF_Service_Client, dynamic_cast<IALIReconfigure_Client *>(this));
   m_Sem.Create(0, 1);

   m_bIsOK = true;
}

AllocatesReconfService::~AllocatesReconfService()
{
   m_Sem.Destroy();
}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void AllocatesReconfService::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   m_pFMEService = pServiceBase;
   ASSERT(NULL != m_pFMEService);
   if ( NULL == m_pFMEService ) {
      m_bIsOK = false;
      return;
   }

   m_pALIReconfService = dynamic_ptr<IALIReconfigure>(iidALI_CONF_Service, pServiceBase);
   ASSERT(NULL != m_pALIReconfService);
   if ( NULL == m_pALIReconfService ) {
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void AllocatesReconfService::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
   PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void AllocatesReconfService::serviceReleased(TransactionID const &rTranID)
{
   MSG("Service Released");
   m_Sem.Post(1);
}

 void AllocatesReconfService::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
  {
   MSG("AllocatesReconfService->Service unexpected requested back");
   if ( rEvent.Has(iidReleaseRequestEvent) ) {

      std::cerr << "Description: " << dynamic_ref<IReleaseRequestEvent>(iidReleaseRequestEvent, rEvent).Description() << std::endl;
   }

   if(NULL != pServiceBase){
      IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
      ASSERT(pIAALService);
      pIAALService->Release(TransactionID());
   }
  }

 void AllocatesReconfService::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void AllocatesReconfService::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
   if ( rEvent.Has(iidExEvent) ) {
      std::cerr << "\n Description  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).Description() << std::endl;
      std::cerr << "\n ExceptionNumber:  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).ExceptionNumber() << std::endl;
      std::cerr << "\n Reason:  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).Reason() << std::endl;

   }

}
// <end IServiceClient interface>


void AllocatesReconfService::deactivateSucceeded( TransactionID const &rTranID )
{
   MSG("deactivateSucceeded");
   m_Sem.Post(1);
}
void AllocatesReconfService::deactivateFailed( IEvent const &rEvent )
{
   ERR("Failed deactivate");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}

void AllocatesReconfService::configureSucceeded( TransactionID const &rTranID )
{
   MSG("configureSucceeded");
   m_Sem.Post(1);
}
void AllocatesReconfService::configureFailed( IEvent const &rEvent )
{
   ERR("configureFailed");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}
void AllocatesReconfService::activateSucceeded( TransactionID const &rTranID )
{
   MSG("activateSucceeded");
   m_Sem.Post(1);
}
void AllocatesReconfService::activateFailed( IEvent const &rEvent )
{
   ERR("activateFailed");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}

void AllocatesReconfService::PrintReconfExceptionDescription(IEvent const &rEvent)
{
   if ( rEvent.Has(iidExTranEvent) ) {
      std::cerr << "ExceptionNumber: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).ExceptionNumber() << std::endl;
      m_errNum  = dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).ExceptionNumber();
   }
}


void AllocatesReconfService::setreconfnvs( char* pbitstream,
                                           btUnsigned64bitInt reconftimeout ,
                                           btUnsigned64bitInt reconfAction ,
                                           btUnsigned64bitInt reactivateDisabled)
{
   m_reconfnvs.Empty();
   m_reconfnvs.Add(AALCONF_MILLI_TIMEOUT,(static_cast<btUnsigned64bitInt>(reconftimeout)) *1000);

   // Reconfigure action
   if(AALCONF_RECONF_ACTION_HONOR_OWNER_ID == reconfAction) {
      m_reconfnvs.Add(AALCONF_RECONF_ACTION,AALCONF_RECONF_ACTION_HONOR_OWNER_ID);
   }
   else {
      m_reconfnvs.Add(AALCONF_RECONF_ACTION,AALCONF_RECONF_ACTION_HONOR_REQUEST_ID);
   }

   // ReActivated state
   if(reactivateDisabled) {
      m_reconfnvs.Add(AALCONF_REACTIVATE_DISABLED,true);
   } else {
      m_reconfnvs.Add(AALCONF_REACTIVATE_DISABLED,false);
   }

   m_reconfnvs.Add(AALCONF_FILENAMEKEY,pbitstream);

}

btBool AllocatesReconfService::AllocatePRService(Runtime *pRuntime)
{
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");
   ConfigRecord.Add(keyRegAFU_ID,ALI_AFUID_UAFU_CONFIG);
   ConfigRecord.Add(keyRegSubDeviceNumber,0);

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "ALI Conf AFU");

   m_pRuntime = pRuntime;
   m_pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      return false;
   }
   return true;
}

btBool AllocatesReconfService::FreePRService()
{
   MSG("Release Service");
   (dynamic_ptr<IAALService>(iidService, m_pFMEService))->Release(TransactionID());
   m_Sem.Wait();
   return true;
}

