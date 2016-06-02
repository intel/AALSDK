


#ifndef __RECONF_SERVICE__
#define __RECONF_SERVICE__

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aalclp/aalclp.h>
#include <string.h>

#define BitStreamFile       "/home/aravuri/kernelpr/pr_script/bitstream/10.rbf"
#define BitStreamWrongFile  "/home/aravuri/kernelpr/pr_script/bitstream/100.rbf"

using namespace std;
using namespace AAL;

BEGIN_C_DECLS

int aliconigafu_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int aliconigafu_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );
int aliconigafu_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption);
int aliconigafu_on_dash_only(AALCLP_USER_DEFINED user);
int aliconigafu_on_dash_dash_only(AALCLP_USER_DEFINED user);

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );
int  ParseCmds(struct ALIConfigCommandLine *pconfigcmd, int argc, char *argv[]);
void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs);
void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs);
int  verifycmds(struct ALIConfigCommandLine *cl);


// Command Line Paramaters
struct  ALIConfigCommandLine
{
   btUIntPtr          flags;
   #define ALICONIFG_CMD_FLAG_HELP      0x00000001
   #define ALICONIFG_CMD_FLAG_VERSION   0x00000002
   #define ALICONIFG_CMD_PARSE_ERROR    0x00000003
   char    bitstream_file[200];
   int     reconftimeout;
   int     reconfAction;
   bool    reactivateDisabled;
   char    reconfInterface[20];
};

END_C_DECLS

/// @brief   Since this is a simple application, our App class implements IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
class AllocatesReconfService: public CAASBase, public IServiceClient, IALIReconfigure_Client
{
public:

   AllocatesReconfService();
   ~AllocatesReconfService();

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase, TransactionID const &rTranID);
   void serviceAllocateFailed(const IEvent &rEvent);
   void serviceReleased(const AAL::TransactionID&);
   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &);
   void serviceReleaseFailed(const AAL::IEvent&);
   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   // <IALIReconfigure_Client interface>
   virtual void deactivateSucceeded( TransactionID const &rTranID );
   virtual void deactivateFailed( IEvent const &rEvent );

   virtual void configureSucceeded( TransactionID const &rTranID );
   virtual void configureFailed( IEvent const &rEvent );

   virtual void activateSucceeded( TransactionID const &rTranID );
   virtual void activateFailed( IEvent const &rEvent );
   // <end IALIReconfigure_Client interface>

   void PrintReconfExceptionDescription(IEvent const &theEvent);

   btBool FreePRService();
   btBool AllocatePRService(Runtime *pRuntime);
   void setreconfnvs( char* pbitstream,
                      btUnsigned64bitInt reconftimeout ,
                      btUnsigned64bitInt reconfAction ,
                      btUnsigned64bitInt reactivateDisabled);

   btID getErrnum() {  return m_errNum; }
   void setIsOk(btBool status) { m_bIsOK =status; }
   IALIReconfigure* getReconfService() { return m_pALIReconfService;}

   void reconfConfigure()  {
      m_pALIReconfService->reconfConfigure(TransactionID(), m_reconfnvs);
      m_Sem.Wait();
   }

   void reconfDeactivate() {
      m_pALIReconfService->reconfDeactivate(TransactionID(), m_reconfnvs);
      m_Sem.Wait();
   }

   void reconfActivate() {
      m_pALIReconfService->reconfActivate(TransactionID(), m_reconfnvs);
      m_Sem.Wait();
   }

protected:
   Runtime              *m_pRuntime;           ///< AAL Runtime
   IBase                *m_pFMEService;        ///< The generic AAL Service interface for the AFU.
   IALIReconfigure      *m_pALIReconfService;  ///< Pointer to Buffer Service
   CSemaphore            m_Sem;                ///< For synchronizing with the AAL runtime.
   btInt                 m_Result;             ///< Returned result v; 0 if success
   NamedValueSet         m_reconfnvs;          ///< Reconfigure input parameters
   btID                  m_errNum;             ///< AAL Error code
};
#endif  //__RECONF_SERVICE__
