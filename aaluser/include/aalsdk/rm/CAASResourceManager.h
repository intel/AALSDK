// Copyright (c) 2008-2015, Intel Corporation
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
/// @file CAASResourceMangager.h
/// @brief Defines internal datastructures for Resource Manager Subsystem
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/27/2008     HM       Initial version started
/// 09/12/2008     HM       Major checkin for new remote database
/// 09/12/2008     HM       Added configuration update message support
/// 10/05/2008     HM       Converted to use AALLogger, removed extra errBuffer
/// 11/13/2008     HM       Added NVSFromConfigUpdate
/// 11/16/2008     HM       Backdoor versions of Goal Record and Policy Mgr,
///                            and various utilities
/// 11/30/2008     HM       Added terminating signal handler
/// 01/04/2009     HM       Updated Copyright
/// 02/17/2009     HM       Added InstRecMap member to hold the instance
///                            records directly instead of putting them in the
///                            database
/// 03/08/2009     HM       Moved DestroyRMIoctlReq() in from KernelStructs.h
///                            and decomposed it for more detailed use.
///                            Specifically, added DestroyRMIoctlReqPayload()@endverbatim
//****************************************************************************
#ifndef __AALSDK_RM_CAASRESOURCEMANAGER_H__
#define __AALSDK_RM_CAASRESOURCEMANAGER_H__

//#define NEW_INST_RECS                   // Turn on the new Instance Record processing
                                          //    while preserving the old code

#include <aalsdk/AALTypes.h>
#include <aalsdk/ResMgr.h>                // Definitions for user mode RM users
#include <aalsdk/IServiceClient.h>        // IServiceClient
#include <aalsdk/rm/RegDBSkeleton.h>      // Brings in the skeleton, which brings in the database
#include <aalsdk/rm/InstanceRecord.h>     // InstRecMap

#include <aalsdk/AAL.h>                   // for service declaration
#include <aalsdk/aas/AALServiceModule.h>  //
#include <aalsdk/aas/AALService.h>        //
#include <aalsdk/IServiceClient.h>        //
#include <aalsdk/osal/IDispatchable.h>    //

/// @todo Document CResMgr and related.

BEGIN_NAMESPACE(AAL)

/*
* Data structures for maps
*/
class nvsContainer                  // just a structure for a list/map so can add things later
{                                   // default ctor/dtor should be correct
public:
   NamedValueSet m_nvs;             // container value semantics, copyable
}; // nvsContainer

typedef std::map<DatabaseKey_t, nvsContainer>   nvsMap_t; // Map [primary key, NVS Container]
typedef nvsMap_t::iterator                      nvsMap_itr_t;
typedef nvsMap_t::const_iterator                nvsMap_citr_t;

typedef std::list<nvsContainer>                 nvsList_t; // List [NVS Container]
typedef nvsList_t::iterator                     nvsList_itr_t;
typedef nvsList_t::const_iterator               nvsList_citr_t;

/*
* nvsList
*/
class nvsList {
public:
   nvsList_t   m_nvsList;           // list containing NVS's
   //nvsList();                     // default ctor should create list
   //~nvsList();                    // default dtor should clean up, as all items in list are destructed and then the list
};


/*
 * IResMgr service interface - FIXME: name collision with interface for
 *                             CResourceManager (remote resource manager)
 */
#define iidResMgrService __INTC_IID(AAL_sysResMgr,0x0001)

class IResMgrService
{
public:
    virtual int start(const TransactionID &rtid, btBool spawnThread = true) = 0;
    virtual int fdServer() = 0;
    virtual ~IResMgrService() {};
};


/*
* CResMgr object itself
*/
enum CResMgrState { eCRMS_Starting, eCRMS_Running, eCRMS_Stopping, eCRMS_Stopped };
//=============================================================================
// Name: CResMgr
// Description: Hold kernel-related interface guts to run the server pump.
//              Also a regular allocateable AAL service for modularity.
//=============================================================================
class CResMgr : public ServiceBase, public IResMgrService
{
private:
   std::string             m_sResMgrDevName;    // name of the generic pipe this server is servicing
   RegDBSkeleton          *m_pRegDBSkeleton;    // The skeleton is the interface to the database
   struct aalrm_ioctlreq  *m_pIoctlReq;         // Kernel message data structure
   int                     m_fdServer;          // file handle for m_szResMgrDeviceName
   btBool                  m_bIsOK;             // True if this object is functioning correctly
   NamedValueSet          *m_pOptArgs;          // Points to optional arguments for constructor
   std::string             m_sDatabasePath;     // Default database path
   CResMgrState            m_state;             // For startup and shutdown processing
   InstRecMap              m_InstRecMap;        // Map of Instance Records

                                                // To hold a device handle
   void                   *m_mydevice;          // TODO: replace with proper list of devices
   IServiceClient         *m_pAALServiceClient; // AAL Service client

   OSLThread              *m_pResMgrThread;     // Thread executing resource manager
   int                    m_resMgrRetVal;       // Return value of resource manager thread

   // internal thread entry point
   static void             _resMgrThread(OSLThread *pThread, void *pContext);
   // main event loop
   int                     _run();

   // Accessors & Mutators not yet used externally
   const std::string       sResMgrDevName()     { return m_sResMgrDevName; }
   RegDBSkeleton *         pRegDBSkeleton()     { return m_pRegDBSkeleton; }
   // No copying allowed
   CResMgr(const CResMgr & );
   CResMgr & operator = (const CResMgr & );
   // Worker functions
   void                    NVSFromConfigUpdate  (const aalrms_configUpDateEvent& pcfgUpDate, NamedValueSet& nvs);
   void                    AddNullHandleRecToList (const NamedValueSet& nvs, nvsList& listGoal);
   btBool                  IsBackdoorRecordGood (const NamedValueSet* pInstRec);
   btBool                  ComputeBackdoorGoalRecords (const NamedValueSet& nvsManifest, nvsList& listGoal);
   btBool                  ComputeBackdoorPolicy (const nvsList& listGoal, NamedValueSet& nvsGoal);
//#ifndef NEW_INST_RECS
//   btBool                  IncrementNumAllocated ( NamedValueSet& nvsGoal);
//#endif

//#define PUBLIC_FOR_TESTING
#ifdef PUBLIC_FOR_TESTING
public:
#endif
   // Should be private
   btBool                  ComputeGoalRecords   (const NamedValueSet& nvsManifest, nvsList& listGoal);
   btBool                  GetPolicyResults     (const nvsList& listGoal, NamedValueSet& nvsGoal);
   // End of Should be private

public:
// TODO: Figure out how to pass parameters to service
//   CResMgr(NamedValueSet     *pOptArgs = NULL,
//           const std::string &sDevName = "/dev/aalrms");
    DECLARE_AAL_SERVICE_CONSTRUCTOR(CResMgr, ServiceBase),
        m_sResMgrDevName ("/dev/aalrms"),
        m_pRegDBSkeleton (NULL),
        m_pIoctlReq (NULL),
        m_fdServer (-1),
        m_bIsOK (false),
        m_pOptArgs (NULL),		// FIXME: how to pass into service?
        m_sDatabasePath (),// Will be initialized below
        m_state (eCRMS_Starting),
        m_InstRecMap (),
        m_mydevice (0),
        m_pAALServiceClient(NULL)
    {
        SetInterface(iidResMgrService, dynamic_cast<IResMgrService *>(this));
    }



   virtual ~CResMgr();
   // Accessors & Mutators
   btBool                  bIsOK()              { return m_bIsOK;     }
   struct aalrm_ioctlreq*  pIoctlReq()          { return m_pIoctlReq; }
   int                     fdServer()           { return m_fdServer;  }
   CResMgrState            state()              { AutoLock(this); return m_state;     }
   // Core Functionality
   int                     Get_AALRMS_Msg       (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   int                     Parse_AALRMS_Msg     (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   int                     Send_AALRMS_Msg      (int fdServer, struct aalrm_ioctlreq *pIoctlReq, int *pRealRetVal=NULL);
   // Special functions
   int                     EnableConfigUpdates  (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   // General Parser functions spawned from Parse_AALRMS_Msg
   int                     DoRequestDevice      (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   int                     DoReleaseDevice      (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   int                     DoRegistrar          (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   int                     DoShutdown           (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   int                     DoRestart            (int fdServer, struct aalrm_ioctlreq *pIoctlReq);
   int                     DoConfigUpdate       (int fdServer, struct aalrm_ioctlreq *pIoctlReq);


   // ServiceBase
   btBool                  init                 (IBase *pclientBase,
                                                 NamedValueSet const &optArgs,
                                                 TransactionID const &rtid);
   // IResMgr
   int                     start                (const TransactionID &rtid, btBool spawnThread = true);
   btBool                  Release              (TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

}; // class CResMgr


/*
* IoctlReq utility functions
*/


inline void * DestroyRMIoctlReqPayload(aalrm_ioctlreq *pIoctlReq)
{
   if ( pIoctlReq->payload ) {
      delete[] reinterpret_cast<unsigned char*>(pIoctlReq->payload);
   }
   pIoctlReq->payload = NULL;
   pIoctlReq->size    = 0;
   return NULL;
}

inline aalrm_ioctlreq * DestroyRMIoctlReq(aalrm_ioctlreq *pIoctlReq)
{
   DestroyRMIoctlReqPayload(pIoctlReq);
   delete pIoctlReq;
   return NULL;
}


END_NAMESPACE(AAL)


#endif // __AALSDK_RM_CAASRESOURCEMANAGER_H__

