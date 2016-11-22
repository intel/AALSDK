// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/Partial_Reconfig/PR_SingleApp/main.cpp
//
// Copyright(c) 2007-2016, Intel Corporation
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
/// @file main.cpp
/// @brief Partial reconfiguration of green bit stream service .
/// @ingroup Partial_Reconfig
/// @verbatim
/// AAL Partial reconfiguration test application
///
///    This application is for testing purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to test PR functionality.
///
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/06/2016     RP       Initial version started based on older sample
//****************************************************************************

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>

#include "../PR_SingleApp/ALINLB.h"
#include "../PR_SingleApp/ALIReconf.h"
#include "arguments.h"
#include "utils.h"

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
//#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#define TEST_CASE(x) std::cout << std::endl << x << std::endl

#define MSG(x)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl


BEGIN_C_DECLS
struct ALIConfigCommandLine configCmdLine = { 0,"","",1,1,0,0 };

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "Partial_Reconfig",
                             "0",
                             "",
                             help_msg_callback,
                             &configCmdLine);
END_C_DECLS


class PRTest: public CAASBase, public IRuntimeClient
{
public:

   PRTest(const arguments &args);
   ~PRTest();

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};    // Not Used
   void runtimeStarted(IRuntime  *pRuntime, const NamedValueSet &rConfigParms);
   void runtimeStopped(IRuntime *pRuntime);
   void runtimeStartFailed(const IEvent &rEvent);
   void runtimeStopFailed(const IEvent &rEvent);
   void runtimeAllocateServiceFailed( IEvent const &rEvent);
   void runtimeAllocateServiceSucceeded(IBase *pClient, TransactionID const &rTranID);
   void runtimeEvent(const IEvent &rEvent);
   // <end IRuntimeClient interface>

   bool findFpgadiag();
   btBool FreeRuntime();
   btBool runTests();
   btBool allocRecongService();
   Runtime* getRuntime() { return &m_Runtime;}
   btInt Errors() const  { return m_Errors; }

   // Test cases
   void sw_pr_03();
   void sw_pr_02();
   void sw_pr_01a();
   void sw_pr_04a();
   void sw_pr_05a();
   void sw_pr_06a();
   void sw_pr_07a();
   void sw_pr_08();
   void sw_pr_09();
   void sw_pr_10();
   void sw_pr_11();
   void sw_pr_12();


protected:
   Runtime                 m_Runtime;           ///< AAL Runtime
   CSemaphore              m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt                   m_Result;            ///< Returned result v; 0 if success
   btInt                   m_Errors;            /// < Partial Reconfiguration errors
   AllocatesReconfService  m_reconfAFU;         /// < Reconfiguration Service
   AllocatesNLBService     m_ALINLB ;           /// < NLB Service
   arguments               m_args;
   std::string             m_fpgadiag;

};

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
PRTest::PRTest(const arguments &args) :
         m_Runtime(this),
         m_Result(0),
         m_Errors(0),
         m_args(args),
         m_reconfAFU(args),
         m_ALINLB(args)
{

   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   m_Sem.Create(0, 1);

   NamedValueSet configArgs;
   NamedValueSet configRecord;

   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);

   if(!m_Runtime.start(configArgs)){
      m_bIsOK = false;
      return;
   }
   m_Sem.Wait();
   m_bIsOK = true;
}

/// @brief   Destructor
///
PRTest::~PRTest()
{
   m_Sem.Destroy();
}


bool PRTest::findFpgadiag()
{
   char *envpath = NULL;
   envpath = getenv("LD_LIBRARY_PATH");

   if (m_args.have("fpgadiag"))
   {
      m_fpgadiag = m_args.get_string("fpgadiag");
      if (utils::path_exists(m_fpgadiag))
      {
          return true;
      }
      else
      {
          return false;
      }
   }
   else if (0 != envpath)
   {
      std::vector<std::string> splits = utils::split<std::string>(envpath, ":");
      std::vector<std::string>::const_iterator iter = splits.begin();
      struct stat buffer;
      std::string path = "";
      for ( ; iter < splits.end(); ++iter)
      {
          path = *iter + "/../bin/fpgadiag";
          if (utils::path_exists(path))
          {
              m_fpgadiag = path;
              return true;
          }
      }
   }
   return false;
}

void PRTest::runtimeStarted( IRuntime            *pRuntime,
                             const NamedValueSet &rConfigParms)
{
   m_bIsOK = true;
   m_Sem.Post(1);
}

void PRTest::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void PRTest::runtimeStartFailed(const IEvent &rEvent)
{
   ERR("Runtime start failed");
   PrintExceptionDescription(rEvent);
}

void PRTest::runtimeStopFailed(const IEvent &rEvent)
{
   MSG("Runtime stop failed");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void PRTest::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   ERR("Runtime AllocateService failed");
   PrintExceptionDescription(rEvent);
}

void PRTest::runtimeAllocateServiceSucceeded(IBase *pClient,
                                        TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void PRTest::runtimeEvent(const IEvent &rEvent)
{
   MSG("Generic message handler (runtime)");
}

btBool PRTest::FreeRuntime()
{
   MSG("PRTest::FreeRuntime");
   m_Runtime.stop();
   m_Sem.Wait();

}

btBool PRTest::allocRecongService()
{
   // Allocate Service
   m_reconfAFU.AllocatePRService(&m_Runtime);
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("--- Failed to Allocate Reconfigure Service --- ");
      return false;
   }
   //strcpy(m_args.get_string("bitstream1"),BitStreamFile);
   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),1000,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);

   m_reconfAFU.reconfConfigure();
   if(false == m_reconfAFU.IsOK() ) {
     ++m_Errors;
     ERR("--- Failed to  Reconfigure Bitstream --- ");
     return false;
   }

}

btBool PRTest::runTests()
{
   if( false == allocRecongService() ) {
      ERR("--- Failed to allocate Reconf Service  --- ");
      return false;
   }
 
   cout << "----INPUT ARGUEMENTS  ---- "  << endl;
   cout << "m_args.get_string('bitstream1'): " << m_args.get_string("bitstream1") << endl;
   cout << "m_args.get_string('bitstream2'): " << m_args.get_string("bitstream2") << endl;
   cout << "testcasenum: " << configCmdLine.testcasenum << endl;

   cout << endl << "-------- START TEST CASES  ---------" << endl << endl;
   int testcase = m_args.get_int("testcase");
   switch(testcase){
      case 1: {
         sw_pr_01a();
         break;
      }
      case 2: {
         sw_pr_02();
         break;
      }
      case 3: {
         sw_pr_03();
         break;
      }
      case 4: {
         sw_pr_04a();
         break;
      }
      case 5: {
         sw_pr_05a();
         break;
      }
      case 6: {
         sw_pr_06a();
         break;
      }
      case 7: {
         sw_pr_07a();;
         break;
      }
      case 8 : {
        sw_pr_08();;
         break;
      }
      case 9 : {
         sw_pr_09();
         break;
      }
      case 10: {
         sw_pr_10();
         break;
      }
      case 11: {
         sw_pr_11();
         break;
      }
      case 12: {
         sw_pr_12();
         break;
      }
      // All test cases
      case 0: {
         sw_pr_01a();
         sw_pr_02();
         sw_pr_03();
         sw_pr_04a();
         sw_pr_05a();
         sw_pr_06a();
         sw_pr_07a();
         sw_pr_08();
         sw_pr_09();
         sw_pr_10();
         sw_pr_11();
         sw_pr_12();
         break;
      }

      default:
         std::cout<<"Please type: ./Partial_Reconfig  to display help menu ! "<<endl;
   }

   cout << endl<< "-------- END TEST CASES  ---------" << endl<<  endl;

   m_reconfAFU.FreePRService();
   return true;
}

void PRTest::sw_pr_01a()
{
   // SW-PR-01a   Partial Reconfiguration Baseline
   // In the same application process, reconfigure a valid green NLB bitstream
   // and run NLB/fpgadiag to demonstrate it works.
   // Reconfigure a different valid green NLB bitstream and
   // run NLB/fpgadiag to demonstrate that one works, as well.

   TEST_CASE("-------- sw_pr_01a START ---------");
   if (!findFpgadiag())
   {
      ERR("Don't know where to find fpgadiag.");
      //exit(1);
   }

   int res       = 0;

   // Reconfigure bitstream
   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),1000,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfConfigure();
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_01a: Reconf FAIL");
      return ;
   }

   TEST_CASE("-------- Reconfigurted Mode0 bitstream---------");
   std::string loopback  = m_fpgadiag + " --mode=lpbk1 -r=vl0";
   std::string moderead  = m_fpgadiag + " --mode=read -r=vl0";
   std::string modewrite = m_fpgadiag + " --mode=write -r=vl0";

   //cout << endl << "Mode0 Bitstream Path" <<path << endl;

   res = system(loopback.c_str());
   if ( -1 == res ) {
      ++m_Errors;
      ERR("sw_pr_01a: Mode0  FAILED to RUN");
   }

   TEST_CASE("-------- Reconfigurted Mode3 bitstream---------");

   // Reconfigure bitstream
   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream2"),1000,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfConfigure();
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_01a: Reconf FAIL");
      return ;
   }

   //cout << endl << "Mode3 Bitstream Path" <<path << endl;

   res = system(moderead.c_str());
   if ( -1 == res ) {
      ++m_Errors;
      ERR("sw_pr_01a: Mode3 Read FAILED to RUN");
   }

   //cout << endl << "Mode7 Bitstream Path" <<path << endl;

   res = system(modewrite.c_str());
   if ( -1 == res ) {
      ++m_Errors;
      ERR("sw_pr_01a: Mode3 Write FAILED to RUN");
   }


   // Reconfigure bitstream
   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"), 1000, AALCONF_RECONF_ACTION_HONOR_OWNER_ID, false);
   m_reconfAFU.reconfConfigure();
   if (false == m_reconfAFU.IsOK()) {
      ++m_Errors;
      ERR("sw_pr_01a: Reconf FAIL");
      return;
   }

   TEST_CASE("-------- sw_pr_01a PASS ---------");

   return ;
}

void PRTest::sw_pr_02()
{
   // SW-PR-02 Partial Reconfiguration IALIReconfigure
   // Deactivate a PR slot that is already free and verify that
   // deactivate returns success.

   TEST_CASE("-------- sw_pr_02 START  ---------");

   // Reconfigure bitstream
   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),1000,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.setIsOk(true);
   m_reconfAFU.reconfDeactivate();
   if(false == m_reconfAFU.IsOK() ) {
     ++m_Errors;
     ERR("sw_pr_02: Deactiavte  FAIL ");
     return;

   }

   // Deactiavte AFU Device
   m_reconfAFU.reconfDeactivate();
   if(( true == m_reconfAFU.IsOK() ) &&
        (m_reconfAFU.getErrnum() != ali_errnumNoAFU) ) {
     ++m_Errors;
     ERR("sw_pr_02:Deactiavte  FAIL");
     return;
   }

   TEST_CASE("-------- sw_pr_02 PASS ---------");
}


void PRTest::sw_pr_03()
{
   // SW-PR-03 Partial Reconfiguration IALIReconfigure
   // Deactivate a PR slot that has an AFU but is not owned, and verify that
   // deactivate returns success.

   TEST_CASE("-------- sw_pr_03 START ---------");

   // Reconfigure bitstream
   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),1000,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfActivate();
   m_reconfAFU.setIsOk(true);

   // Deactivate AFU
   m_reconfAFU.reconfDeactivate();
   if(false == m_reconfAFU.IsOK() ) {
     ++m_Errors;
     ERR("sw_pr_03:Deactiavte  FAIL");
     return;

   }

   TEST_CASE("-------- sw_pr_03 PASS ---------");
}


void PRTest::sw_pr_04a()
{
   //SW-PR-04a   Partial Reconfiguration IALIReconfigure
   // Within one application process, deactivate a PR slot that has an AFU that is owned,
   // providing HONOR_OWNER flag, and verify that the owning process is notified of the request,
   // and upon releasing the AFU, the Deactivate succeeds.

   TEST_CASE("-------- sw_pr_04a START ---------");

   m_reconfAFU.reconfActivate();
   m_reconfAFU.setIsOk(true);

   // Allocate NLB Resource
   m_ALINLB.AllocateNLBService(&m_Runtime);
   if(false == m_ALINLB.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_04a: ALLOC NLB Service FAIL --- ");
      return;
   }

   m_ALINLB.setReleaseService(true);
   OSLThread *t = new OSLThread( AllocatesNLBService::NLBThread,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 &m_ALINLB);


   SleepSec(1);

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfDeactivate();
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_04a:ALLOC NLB Service FAIL --- ");
      goto done_0;
   }

   TEST_CASE("-------- sw_pr_04a PASS ---------");
   return ;

done_0:
   delete t;
   m_ALINLB.FreeNLBService();
}

void PRTest::sw_pr_05a()
{
   // SW-PR-05a   Partial Reconfiguration IALIReconfigure
   // Within one application process, deactivate a PR slot that has an AFU that is owned,
   // providing HONOR_OWNER flag, and verify that the owning process is notified of the request,
   // and upon NOT releasing the AFU, the Deactivate fails.

   TEST_CASE("-------- sw_pr_05a START ---------");

   m_reconfAFU.reconfActivate();
   m_reconfAFU.setIsOk(true);

   SleepSec(1);

   m_ALINLB.AllocateNLBService(&m_Runtime);
   if(false == m_ALINLB.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_05a:ALLOC NLB Service FAIL ");
      return;
   }
   SleepSec(1);

   m_ALINLB.setReleaseService(false);
   OSLThread *t = new OSLThread( AllocatesNLBService::NLBThread,
                               OSLThread::THREADPRIORITY_NORMAL,
                               &m_ALINLB);

   SleepSec(1);

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),10,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfDeactivate();
   if(( true == m_reconfAFU.IsOK() ) &&
      (m_reconfAFU.getErrnum() != ali_errnumDeActiveTimeout) ){
      ++m_Errors;
      ERR("sw_pr_05a:ALLOC NLB Service FAIL");
      goto done_0;
   }



   delete t;
   m_ALINLB.FreeNLBService();

   TEST_CASE("-------- sw_pr_05a PASS ---------");
   return ;
 done_0:

   delete t;
   m_ALINLB.FreeNLBService();
}

void PRTest::sw_pr_06a()
{
   // SW-PR-06a   Partial Reconfiguration IALIReconfigure
   // Within one application process, deactivate a PR slot that has an AFU that is owned,
   // providing HONOR_REQUESTER flag, and verify that the owning process is notified of the request,
   // and upon releasing the AFU, the Deactivate succeeds.   Medium   open  Alpha

   TEST_CASE("-------- sw_pr_06a START ---------");

   m_reconfAFU.reconfActivate();
   m_reconfAFU.setIsOk(true);
   SleepSec(1);

   m_ALINLB.AllocateNLBService(&m_Runtime);
   if(false == m_ALINLB.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_06a:ALLOC NLB Service FAIL ");
      return;
   }

   SleepSec(1);
   m_ALINLB.setReleaseService(true);
   OSLThread *t = new OSLThread( AllocatesNLBService::NLBThread,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 &m_ALINLB);


   SleepSec(1);
   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_REQUEST_ID,false);
   m_reconfAFU.reconfDeactivate();
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_06a:ALLOC NLB Service FAIL ");
      goto done_0;
   }

   TEST_CASE("-------- sw_pr_06a PASS ---------");

   delete t;
   return ;

done_0:
   delete t;
   m_ALINLB.FreeNLBService();
}

void PRTest::sw_pr_07a()
{
   // SW-PR-07a   Partial Reconfiguration IALIReconfigure
   // Within one application process, deactivate a PR slot that has an AFU that is owned,
   // providing HONOR_REQUESTER flag, and verify that the owning process is notified of the request,
   // and upon NOT releasing the AFU, the Deactivate succeeds anyway.

   TEST_CASE("--------  sw_pr_07a START ---------");

   m_reconfAFU.reconfActivate();
   m_reconfAFU.setIsOk(true);
   SleepSec(1);

   m_ALINLB.AllocateNLBService(&m_Runtime);
   if(false == m_ALINLB.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_07a: ALLOC NLB Service FAIL ");
      return;
   }

   SleepSec(1);
   m_ALINLB.setReleaseService(false);
   OSLThread *t = new OSLThread( AllocatesNLBService::NLBThread,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 &m_ALINLB);
   SleepSec(1);

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_REQUEST_ID,false);
   m_reconfAFU.reconfDeactivate();
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_07a: ALLOC NLB Service FAIL ");
      goto done_0;
   }

   TEST_CASE("--------  sw_pr_07a PASS ---------");

   delete t;
   return ;

done_0:
   delete t;
   m_ALINLB.FreeNLBService();
}


void PRTest::sw_pr_08()
{
   // SW-PR-08  Partial Reconfiguration IALIReconfigure
   // Attempt to Reconfigure and provide an invalid filename for the green bitstream,
   // and verify that the download fails.

   TEST_CASE("--------  sw_pr_08 START ---------");

   std::string wrongFile = BitStreamWrongFile;
   m_reconfAFU.setreconfnvs(wrongFile,10,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);

   m_reconfAFU.reconfConfigure();

   if(true == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_08:Deactivate  FAIL");
      return ;
   }
   TEST_CASE("--------  sw_pr_08 PASS ---------");
}


void PRTest::sw_pr_09()
{
   // SW-PR-09 Partial Reconfiguration IALIReconfigure
   // Activate a PR slot that is free, and verify that the Activate is unsuccessful,
   // but does not prevent subsequent Deactivate and Reconfigure.

   TEST_CASE("--------  sw_pr_09 START ---------");

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfDeactivate();
   m_reconfAFU.reconfActivate();

   // Activate fails if user try to activate active AFU.
   // Activate is unsuccessful
   m_reconfAFU.reconfActivate();

   //  Test case fails if  Activate is successful.
   if(true == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_09:Deactivate  FAIL");
      return ;
   }

   m_reconfAFU.setIsOk(true);
   m_reconfAFU.reconfConfigure();

   if(( false == m_reconfAFU.IsOK() ) &&
      (m_reconfAFU.getErrnum() != ali_errnumNoAFU) ){

      ++m_Errors;
      ERR("sw_pr_09:Deactivate  FAIL");
      return ;
   }

   TEST_CASE("--------  sw_pr_09 PASS ---------");

}


void PRTest::sw_pr_10()
{
   // SW-PR-10 Partial Reconfiguration IALIReconfigure
   // Activate a PR slot that has been successfully Reconfigured,
   // and verify that the Activate is successful.

   TEST_CASE("--------  sw_pr_10 START ---------");

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,true);

   m_reconfAFU.reconfConfigure();
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_10: Deactivate  FAIL ");
      return ;
   }

   m_reconfAFU.setIsOk(true);
   m_reconfAFU.reconfActivate();

   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_10:Activate  FAIL ");
      return ;
   }

   TEST_CASE("--------  sw_pr_10 PASS ---------");

}

void PRTest::sw_pr_11()
{
   // SW-PR-11 Partial Reconfiguration IALIReconfigure stress1
   // For an AFU that is owned, send 10 reconfDeactivate() calls with the HONOR_OWNER flag
   // and AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests upon receiving
   // serviceReleaseRequest().  Verify that ali_errnumDeviceBusy is returned by each reconfDeactivate().
   // Verify that 10 deactivateFailed() notifications are received by the client.
   // On the 11th call, change the behavior such that serviceReleaseRequest() releases the AFU.
   // Verify that ali_errnumOK is returned by reconfDeactivate().
   // Verify that deactivateSucceeded() is received by the client.

   TEST_CASE("--------  sw_pr_11 START ---------");

   m_ALINLB.AllocateNLBService(&m_Runtime);
   if(false == m_ALINLB.IsOK() ) {
   ++m_Errors;
      ERR("sw_pr_11:ALLOC NLB Service FAIL");
      return;
   }
   SleepSec(1);
   m_ALINLB.setReleaseService(false);
   OSLThread *t = new OSLThread( AllocatesNLBService::NLBThread,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 &m_ALINLB);


   SleepSec(1);

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfDeactivate();
   m_ALINLB.setReleaseService(false);
   m_reconfAFU.setIsOk(true);

   for(int i=0; i<=10 ;i++) {

      cout << "index i=" << i<< endl;
      m_reconfAFU.reconfDeactivate();
      if(( true == m_reconfAFU.IsOK() ) &&
         (m_reconfAFU.getErrnum() != ali_errnumDeviceBusy) ){
         ++m_Errors;
         ERR("sw_pr_11:ALLOC NLB Service FAIL");
         goto done_0;
      }
   }
   m_ALINLB.setReleaseService(true);
   m_reconfAFU.setIsOk(true);
   m_reconfAFU.reconfDeactivate();

   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_11:ALLOC NLB Service FAIL");
      goto done_0;
   }

   delete t;
   TEST_CASE("--------  sw_pr_11 PASS ---------");
   return ;

done_0:
   delete t;
   m_ALINLB.FreeNLBService();
}


void PRTest::sw_pr_12()
{
   // SW-PR-12 Partial Reconfiguration IALIReconfigure stress2
   // For an AFU that is owned, send 10 reconfConfigure() calls with the HONOR_OWNER flag
   // and AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests
   // upon receiving serviceReleaseRequest().  Verify that ali_errnumDeviceBusy is
   // returned by each reconfConfigure(). Verify that 10 configureFailed() notifications
   // are received by the client.  On the 11th call, change the behavior such that
   // serviceReleaseRequest() releases the AFU.  Verify that ali_errnumOK is
   // returned by reconfConfigure().  Verify that configureSucceeded() is received by the client.

   TEST_CASE("--------  sw_pr_12 START ---------");

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);

   m_reconfAFU.reconfConfigure();
   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_12: Deactivate  FAIL ");
      return ;
   }


   m_ALINLB.AllocateNLBService(&m_Runtime);
   if(false == m_ALINLB.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_12:ALLOC NLB Service FAIL");
      return;
   }

   SleepSec(1);
   m_ALINLB.setReleaseService(false);
   OSLThread *t = new OSLThread( AllocatesNLBService::NLBThread,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 &m_ALINLB);


   SleepSec(1);

   m_reconfAFU.setreconfnvs(m_args.get_string("bitstream1"),5,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   m_reconfAFU.reconfDeactivate();
   m_ALINLB.setReleaseService(false);
   m_reconfAFU.setIsOk(true);

   for(int i=0; i<=10 ;i++) {
      cout << "index i=" << i<< endl;
      m_reconfAFU.reconfConfigure();
      if(( true == m_reconfAFU.IsOK() ) &&
         (m_reconfAFU.getErrnum() != ali_errnumDeviceBusy) ){
         ++m_Errors;
         ERR("sw_pr_12:ALLOC NLB Service FAIL");
         goto done_0;
      }
   }

   m_ALINLB.setReleaseService(true);
   m_reconfAFU.setIsOk(true);
   m_reconfAFU.reconfConfigure();

   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("sw_pr_12:ALLOC NLB Service FAIL");
      goto done_0;
   }

   delete t;
   TEST_CASE("--------  sw_pr_12 PASS ---------");
   return ;

 done_0:
   delete t;
   m_ALINLB.FreeNLBService();
}


//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: none
// Outputs: none
// Comments: Main initializes the system. The rest of the example is implemented
//           in the object theApp.
//=============================================================================

int main(int argc, char *argv[])
{
   arguments argparse;
   argparse("bus",        'b', optional_argument, "pci bus number")
           ("function",   'f', optional_argument, "pci function number")
           ("device",     'd', optional_argument, "pci device number")
           ("fpgadiag",   'F', optional_argument, "location of fpgadiag binary")
           ("bitstream1", 'A', required_argument)
           ("bitstream2", 'B', required_argument)
           ("testcase",   't', required_argument)
           ("timeout",    'T', optional_argument)
           ("action",     'a', optional_argument)
           ("reactivate-disbled", 'd', optional_argument)
           ;
   if (!argparse.parse(argc, argv)) return -1;

   btInt Result =0;

   PRTest theApp(argparse);
   if(!theApp.IsOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }

   // Test cases
   theApp.runTests();

   // Free Runtime
   theApp.FreeRuntime();
   MSG("Done");

   Result = theApp.Errors();

   std::cout << "Error count:"<< Result << std::endl;

   return Result;
}
