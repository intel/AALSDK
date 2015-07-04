// Copyright (c) 2012-2015, Intel Corporation
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
/// @file CCILib.cpp
/// @brief Concrete CCI Device Factory implementation.
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
///    This application is for example purposes only. It is not intended
///    to represent a model for developing commercially-deployable
///    applications. It is designed to show working examples of
///    the AAL programming model and APIs.
///
/// AUTHOR: Tim Whisonant, Intel Corporation
///
/// WHEN          WHO    WHAT
/// ==========    ===    ====
/// 11/10/2014    TSW    CCILib / XL adapter.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <new>
#include <cstdio>
#include <cstdarg>

#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/utils/SingleAFUApp.h>
#include <aalsdk/service/ICCIAFU.h>
#include <aalsdk/service/ICCIClient.h>
#include <aalsdk/service/CCIAFUService.h>

#include "CCILib.h"

BEGIN_NAMESPACE(CCILib)

IOutputSynchronizer::~IOutputSynchronizer() throw() {}

class COutputSynchronizer : public IOutputSynchronizer
{
public:
   virtual void Print(const char *fmt, ...) throw()
   {
      va_list va_l;
      va_start(va_l, fmt);

      {
         AutoLock(&m_MtxCout);
         ::vsnprintf(m_pCoutBuf, m_CoutBufSize, fmt, va_l);
         std::cout << m_pCoutBuf;
      }

      va_end(va_l);
   }

   virtual void Log(const char *fmt, ...) throw()
   {
      va_list va_l;
      va_start(va_l, fmt);

      Log(fmt, va_l);

      va_end(va_l);
   }

   virtual void Log(const char *fmt, va_list &va_l) throw()
   {
      AutoLock(&m_MtxCout);
      ::vsnprintf(m_pCoutBuf, m_CoutBufSize, fmt, va_l);
      m_pLogger->Log(m_LogLvl, m_pLogger->GetOss(m_LogLvl) << m_pCoutBuf);
   }

   virtual void Trace(const char *fmt, ...) throw()
   {
      va_list va_l;
      va_start(va_l, fmt);

      Trace(fmt, va_l);

      va_end(va_l);
   }

   virtual void Trace(const char *fmt, va_list &va_l) throw()
   {
      AutoLock(&m_MtxCerr);
      ::vsnprintf(m_pCerrBuf, m_CerrBufSize, fmt, va_l);
      m_pLogger->Log(m_TraceLvl, m_pLogger->GetOss(m_TraceLvl) << m_pCerrBuf);
   }

   virtual int GetLogLevel() const throw() { return m_LogLvl; }
   virtual void SetLogLevel(int l) throw() { m_LogLvl = l;    }

   virtual int GetTraceLevel() const throw() { return m_TraceLvl; }
   virtual void SetTraceLevel(int l) throw() { m_TraceLvl = l;    }

protected:
   COutputSynchronizer(size_t OutBufSize,
                       size_t ErrBufSize,
                       int    LogLvl=LOG_DEBUG,
                       int    TraceLvl=TR_DEBUG) throw() :
      m_pCoutBuf(NULL),
      m_CoutBufSize(OutBufSize),
      m_pCerrBuf(NULL),
      m_CerrBufSize(ErrBufSize),
      m_LogLvl(0),
      m_TraceLvl(0),
      m_pLogger(NULL)
   {
      m_pCoutBuf = new(std::nothrow) char[m_CoutBufSize];
      m_pCerrBuf = new(std::nothrow) char[m_CerrBufSize];

      m_pLogger = pAALLogger();
      m_pLogger->AddToMask(LM_All, 8);
      m_pLogger->SetDestination(ILogger::CERR);

      SetLogLevel(LogLvl);
      SetTraceLevel(TraceLvl);
   }

   ~COutputSynchronizer() throw()
   {
      std::cout << std::flush;

      if ( m_pCoutBuf != NULL ) {
         delete[] m_pCoutBuf;
      }
      if ( m_pCerrBuf != NULL ) {
         delete[] m_pCerrBuf;
      }
   }

   // (no copies)
   COutputSynchronizer(const COutputSynchronizer & ) throw();
   COutputSynchronizer & operator = (const COutputSynchronizer & ) throw();

   char            *m_pCoutBuf;
   size_t           m_CoutBufSize;
   char            *m_pCerrBuf;
   size_t           m_CerrBufSize;
   int              m_LogLvl;
   int              m_TraceLvl;
   ILogger         *m_pLogger;
   CriticalSection  m_MtxCout;
   CriticalSection  m_MtxCerr;

   static IOutputSynchronizer *GetInstance() throw();
   static void                 DestroyInstance() throw();
   static IOutputSynchronizer *sm_pInstance;
   static CriticalSection      sm_SingletonMtx;

   friend class CNullCCIDeviceFactory;
   friend class CCCIDeviceFactory;
};

IOutputSynchronizer * COutputSynchronizer::sm_pInstance = NULL;
CriticalSection       COutputSynchronizer::sm_SingletonMtx;

IOutputSynchronizer * COutputSynchronizer::GetInstance() throw()
{
   AutoLock(&COutputSynchronizer::sm_SingletonMtx);

   if ( NULL == COutputSynchronizer::sm_pInstance ) {
      COutputSynchronizer::sm_pInstance =
               new(std::nothrow) COutputSynchronizer(CCILIB_PRINT_BUF_SIZE, CCILIB_PRINT_BUF_SIZE);
   }

   IOutputSynchronizer *pSync = COutputSynchronizer::sm_pInstance;

   return pSync;
}

void COutputSynchronizer::DestroyInstance() throw()
{
   AutoLock(&COutputSynchronizer::sm_SingletonMtx);

   if ( COutputSynchronizer::sm_pInstance != NULL ) {
      delete COutputSynchronizer::sm_pInstance;
      COutputSynchronizer::sm_pInstance = NULL;
   }
}

////////////////////////////////////////////////////////////////////////////////

ICCIWorkspace::~ICCIWorkspace() throw() {}

class CCCIWorkspace : public ICCIWorkspace
{
public:
   virtual btVirtAddr GetUserVirtualAddress() const throw() { return m_VirtAddr;         }
   virtual btPhysAddr    GetPhysicalAddress() const throw() { return m_PhysAddr;         }
   virtual btWSSize          GetSizeInBytes() const throw() { return m_SizeInBytes;      }
   virtual btBool IsOK()                      const throw() { return NULL != m_VirtAddr; }

protected:
   CCCIWorkspace(btVirtAddr VirtAddr, btPhysAddr PhysAddr, btWSSize SizeInBytes) throw() :
      m_VirtAddr(VirtAddr),
      m_PhysAddr(PhysAddr),
      m_SizeInBytes(SizeInBytes)
   {}
   ~CCCIWorkspace() throw() {}
   friend class CCCIDevice;

   btVirtAddr m_VirtAddr;
   btPhysAddr m_PhysAddr;
   btWSSize   m_SizeInBytes;
};

////////////////////////////////////////////////////////////////////////////////

ICCIDevice::~ICCIDevice() throw() {}

class CNullCCIDevice : public ICCIDevice
{
public:

   virtual btBool SetCSR(btCSROffset , bt32bitCSR   ) throw() { return false; }
   virtual btBool SetAddressCSR(btCSROffset CSR, bt64bitCSR Addr) throw() { return false; }
   virtual btBool GetCSR(btCSROffset , bt32bitCSR * ) throw() { return false; }

   virtual ICCIWorkspace *AllocateWorkspace(btWSSize ) throw() { return NULL; }
   virtual btBool FreeWorkspace(ICCIWorkspace * ) throw() { return false; }

   virtual btBool IsOK() const throw() { return true; }

   virtual void SetSynchronizer(IOutputSynchronizer *pSync) throw()
   {
      m_pSync = pSync;
      ASSERT(m_pSync != NULL);
   }
   virtual IOutputSynchronizer * GetSynchronizer() const throw() { return m_pSync; }

protected:
   /// CNullCCIDevice Constructor
   CNullCCIDevice(IOutputSynchronizer *pSync) throw() :
      m_pSync(NULL)
   { SetSynchronizer(pSync); }

private:
   IOutputSynchronizer *m_pSync;
   friend class CNullCCIDeviceFactory;
};

class CCCIDevice : public ICCIDevice,
                   public ICCIClient,
                   public CAASBase
{
public:
   virtual btBool SetCSR(btCSROffset CSR, bt32bitCSR Value) throw()
   {
      return m_pCCIAFU->CSRWrite(CSR, Value);
   }

   virtual btBool SetAddressCSR(btCSROffset CSR, bt64bitCSR Addr) throw()
   {
      return m_pCCIAFU->CSRWrite64(CSR, Addr);
   }

   virtual btBool GetCSR(btCSROffset CSR, bt32bitCSR *Value) throw()
   {
      btCSRValue v = 0;
      btBool res = m_pCCIAFU->CSRRead(CSR, &v);
      *Value = (bt32bitCSR)v;
      return res;
   }

   virtual ICCIWorkspace *AllocateWorkspace(btWSSize SizeInBytes) throw()
   {
      m_WkspcVirt = NULL;
      m_WkspcPhys = 0;
      m_WkspcSize = 0;

      TransactionID tid;
      m_pCCIAFU->WorkspaceAllocate(SizeInBytes, tid);

      Wait(); // for the alloc notification.

      if ( NULL == m_WkspcVirt ) {
         return NULL;
      }

      return new(std::nothrow) CCCIWorkspace(m_WkspcVirt, m_WkspcPhys, m_WkspcSize);
   }

   virtual btBool FreeWorkspace(ICCIWorkspace *pWs) throw()
   {
      if ( NULL == pWs ) {
         return false;
      }

      TransactionID tid;
      m_pCCIAFU->WorkspaceFree(pWs->GetUserVirtualAddress(), tid);

      Wait(); // for the free notification.

      delete pWs;

      return m_bIsOK;
   }

   virtual btBool IsOK() const throw() { return m_bIsOK; }

   virtual void SetSynchronizer(IOutputSynchronizer *pSync) throw()
   {
      m_pSync = pSync;
      ASSERT(m_pSync != NULL);
   }

   virtual IOutputSynchronizer * GetSynchronizer() const throw()
   {
      return m_pSync;
   }

   void CCIAFU(ICCIAFU *pCCIAFU) { m_pCCIAFU = pCCIAFU; }

protected:
   CCCIDevice(IOutputSynchronizer *pSync, ICCIAFU *pCCIAFU) throw() :
      m_bIsOK(true),
      m_pSync(NULL),
      m_pCCIAFU(pCCIAFU),
      m_WkspcVirt(NULL),
      m_WkspcPhys(0),
      m_WkspcSize(0)
   {
      SetSynchronizer(pSync);
      m_Sem.Create(0, INT_MAX);
      SetSubClassInterface(iidCCIClient, dynamic_cast<ICCIClient *>(this));
   }
   ~CCCIDevice() throw() { m_Sem.Destroy(); }

   // (no copies)
   CCCIDevice(const CCCIDevice & ) throw();
   CCCIDevice & operator = (const CCCIDevice & ) throw();

   void Wait() { m_Sem.Wait();  }
   void Post() { m_Sem.Post(1); }

   virtual void      OnWorkspaceAllocated(TransactionID const &TranID,
                                          btVirtAddr           WkspcVirt,
                                          btPhysAddr           WkspcPhys,
                                          btWSSize             WkspcSize)
   {
      m_WkspcVirt = WkspcVirt;
      m_WkspcPhys = WkspcPhys;
      m_WkspcSize = WkspcSize;
      Post();
   }

   virtual void OnWorkspaceAllocateFailed(const IEvent & )
   {
      m_bIsOK = false;
      Post();
   }

   virtual void          OnWorkspaceFreed(TransactionID const & )
   {
      Post();
   }

   virtual void     OnWorkspaceFreeFailed(const IEvent & )
   {
      m_bIsOK = false;
      Post();
   }

   btBool               m_bIsOK;
   IOutputSynchronizer *m_pSync;
   ICCIAFU             *m_pCCIAFU;
   btVirtAddr           m_WkspcVirt;
   btPhysAddr           m_WkspcPhys;
   btWSSize             m_WkspcSize;
   CSemaphore           m_Sem;
   friend class CCCIDeviceFactory;
};

////////////////////////////////////////////////////////////////////////////////

ICCIDeviceFactory::~ICCIDeviceFactory() throw() {}

class CNullCCIDeviceFactory : public ICCIDeviceFactory
{
public:
   virtual ICCIDevice * CreateCCIDevice() throw()
   {
      return new(std::nothrow) CNullCCIDevice(GetSynchronizer());
   }

   virtual void DestroyCCIDevice(ICCIDevice *pCCIDev) throw()
   {
      ASSERT(NULL != pCCIDev);
      if ( NULL != pCCIDev ) {
         delete pCCIDev;
      }
   }

   virtual IOutputSynchronizer * GetSynchronizer() throw()
   {
      return COutputSynchronizer::GetInstance();
   }

   virtual void ReleaseSynchronizer() throw()
   {
      COutputSynchronizer::DestroyInstance();
   }

protected:
   CNullCCIDeviceFactory() throw() {}
   ~CNullCCIDeviceFactory() throw() {}
   friend ICCIDeviceFactory * GetCCIDeviceFactory(CCIDeviceImplementation );
};

class CCCIDeviceFactory : public ICCIDeviceFactory,
                          public ISingleAFUApp<ICCIAFU>,
                          public ICCIClient
{
public:
   virtual ICCIDevice * CreateCCIDevice() throw()
   {
      btcString AFUName = "CCIAFU";

      TRACE(GetSynchronizer(), TR_INFO, "Allocating %s Service\n", AFUName);

      // NOTE: This example is bypassing the Resource Manager's configuration record lookup
      //  mechanism.  This code is workaround code and is subject to change.

      NamedValueSet Manifest(CCIAFU_MANIFEST);

      Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, AFUName);
      Manifest.Add(CCIAFU_NVS_KEY_TARGET, m_AFUTarget.c_str());

      m_pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);

      Wait(); // for Service Allocated notification.

      if ( IsOK() ) {
         return NewDevice(GetSynchronizer(), m_pProprietary);
      }

      return NULL;
   }

   virtual void DestroyCCIDevice(ICCIDevice *pCCIDev) throw()
   {
      ASSERT(NULL != pCCIDev);
      if ( NULL != pCCIDev ) {
         DelDevice(pCCIDev);
      }

      if ( NULL != m_pAALService ) {
         m_pAALService->Release(TransactionID(), 0);
         Wait(); // For service freed notification.
         m_pAALService  = NULL;
         m_pProprietary = NULL;
      }
   }

   virtual IOutputSynchronizer * GetSynchronizer() throw()
   {
      return COutputSynchronizer::GetInstance();
   }

   virtual void ReleaseSynchronizer() throw()
   {
      COutputSynchronizer::DestroyInstance();
   }

protected:
   CCCIDeviceFactory(std::string AFUTarget) throw() :
      m_AALRuntime(this),
      m_AFUTarget(AFUTarget)
   {
      SetSubClassInterface(iidCCIClient, dynamic_cast<ICCIClient *>(this));

      NamedValueSet args;

      if ( (0 == m_AFUTarget.compare(CCIAFU_NVS_VAL_TARGET_ASE)) ||
           (0 == m_AFUTarget.compare(CCIAFU_NVS_VAL_TARGET_SWSIM)) ) {
         args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);
      } else {
         NamedValueSet ConfigRecord;
         ConfigRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
         args.Add(AALRUNTIME_CONFIG_RECORD, &ConfigRecord);
      }

      TRACE(GetSynchronizer(), TR_INFO, "Starting the AAL Runtime\n");
      if ( m_AALRuntime.start(args) ) {
         Wait(); // For Runtime Started notification.

         if ( !IsOK() ) {
            LOG(GetSynchronizer(), LOG_ERR, "AAL Runtime IsOK check failed\n");
         }

      } else {
         LOG(GetSynchronizer(), LOG_ERR, "AAL Runtime start failed\n");
         m_bIsOK = false;
      }
   }
   ~CCCIDeviceFactory() throw()
   {
      Stop(); // Do runtime stop.
   }

   typedef std::list<CCCIDevice *> dev_list;

   ICCIDevice * NewDevice(IOutputSynchronizer *pSync, ICCIAFU *pCCIAFU)
   {
      CCCIDevice *pDev = new(std::nothrow) CCCIDevice(pSync, pCCIAFU);

      ASSERT(NULL != pDev);
      if ( NULL == pDev ) {
         return NULL;
      }

      {
         AutoLock(this);
         m_DevList.push_back(pDev);
      }

      return pDev;
   }

   void DelDevice(ICCIDevice *pDev)
   {
      AutoLock(this);

      dev_list::iterator iter = find(m_DevList.begin(), m_DevList.end(), dynamic_cast<CCCIDevice *>(pDev));

      if ( m_DevList.end() != iter ) {
         m_DevList.erase(iter);
      }

      delete pDev;
   }

   friend ICCIDeviceFactory * GetCCIDeviceFactory(CCIDeviceImplementation );

   std::string m_AFUTarget;
   Runtime     m_AALRuntime;
   dev_list    m_DevList;

   void OnRuntimeStarted(IRuntime    *pRT,
                         const NamedValueSet &Args);
   void OnRuntimeStopped(IRuntime *pRT);
   void OnRuntimeStartFailed(const IEvent &e);
   void OnRuntimeAllocateServiceFailed(IEvent const &e);
   void OnRuntimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                          TransactionID const &tid);
   void OnRuntimeEvent(const IEvent &e);

   void OnServiceAllocated(IBase               *pServiceBase,
                           TransactionID const &tid);
   void OnServiceAllocateFailed(const IEvent &e);
   void OnServiceReleaseFailed(const IEvent &e);
   void OnServiceReleased(TransactionID const &tid);
   void OnServiceEvent(const IEvent &e);

   void      OnWorkspaceAllocated(TransactionID const &TranID,
                                  btVirtAddr           WkspcVirt,
                                  btPhysAddr           WkspcPhys,
                                  btWSSize             WkspcSize);
   void          OnWorkspaceFreed(TransactionID const &TranID);
   void OnWorkspaceAllocateFailed(const IEvent &Event);
   void     OnWorkspaceFreeFailed(const IEvent &Event);
};

void CCCIDeviceFactory::OnRuntimeStarted(IRuntime            *pRT,
                                         const NamedValueSet &Args)
{
   TRACE(GetSynchronizer(), TR_INFO, "Runtime Started\n");
   Post();
}

void CCCIDeviceFactory::OnRuntimeStopped(IRuntime *pRT)
{
   TRACE(GetSynchronizer(), TR_INFO, "Runtime Stopped\n");
}

void CCCIDeviceFactory::OnRuntimeStartFailed(const IEvent &e)
{
   m_bIsOK = false;
   LOG(GetSynchronizer(), LOG_ERR, "Runtime Start Failed\n");
}

void CCCIDeviceFactory::OnRuntimeAllocateServiceFailed(IEvent const &e)
{
   m_bIsOK = false;
   LOG(GetSynchronizer(), LOG_ERR, "Service Allocate Failed (rt)\n");
}

void CCCIDeviceFactory::OnRuntimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                                          TransactionID const &tid)
{
   TRACE(GetSynchronizer(), TR_INFO, "Service Allocated (rt)\n");
}

void CCCIDeviceFactory::OnRuntimeEvent(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
      m_bIsOK = false;
      LOG(GetSynchronizer(), LOG_ERR, "Unknown exception (rt)\n");
      Post();
      return;
   }

   TRACE(GetSynchronizer(), TR_INFO, "Unknown event (rt)\n");
   Post();
}

void CCCIDeviceFactory::OnServiceAllocated(IBase               *pServiceBase,
                                           TransactionID const &tid)
{
   TRACE(GetSynchronizer(), TR_INFO, "Service Allocated\n");
}

void CCCIDeviceFactory::OnServiceAllocateFailed(const IEvent &e)
{
   m_bIsOK = false;
   LOG(GetSynchronizer(), LOG_ERR, "Service Allocate Failed\n");
}

void CCCIDeviceFactory::OnServiceReleased(TransactionID const &tid)
{
   TRACE(GetSynchronizer(), TR_INFO, "Service Freed\n");
}

void CCCIDeviceFactory::OnServiceReleaseFailed(const IEvent &e)
{
   m_bIsOK = false;
   LOG(GetSynchronizer(), LOG_ERR, "Runtime Start Failed\n");
}

void CCCIDeviceFactory::OnServiceEvent(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
      m_bIsOK = false;
      LOG(GetSynchronizer(), LOG_ERR, "Unknown exception\n");
      Post();
      return;
   }

   TRACE(GetSynchronizer(), TR_INFO, "Unknown event\n");
   Post();
}

void CCCIDeviceFactory::OnWorkspaceAllocated(TransactionID const &TranID,
                                             btVirtAddr           WkspcVirt,
                                             btPhysAddr           WkspcPhys,
                                             btWSSize             WkspcSize)
{
   AutoLock(this);

   dev_list::const_iterator iter = m_DevList.begin();

   while ( m_DevList.end() != iter ) {
      CCCIDevice *pDev = *iter;
      pDev->OnWorkspaceAllocated(TranID, WkspcVirt, WkspcPhys, WkspcSize);
      ++iter;
   }
}

void CCCIDeviceFactory::OnWorkspaceFreed(TransactionID const &TranID)
{
   AutoLock(this);

   dev_list::const_iterator iter = m_DevList.begin();

   while ( m_DevList.end() != iter ) {
      CCCIDevice *pDev = *iter;
      pDev->OnWorkspaceFreed(TranID);
      ++iter;
   }
}

void CCCIDeviceFactory::OnWorkspaceAllocateFailed(const IEvent &Event)
{
   AutoLock(this);

   dev_list::const_iterator iter = m_DevList.begin();

   while ( m_DevList.end() != iter ) {
      CCCIDevice *pDev = *iter;
      pDev->OnWorkspaceAllocateFailed(Event);
      ++iter;
   }
}

void CCCIDeviceFactory::OnWorkspaceFreeFailed(const IEvent &Event)
{
   AutoLock(this);

   dev_list::const_iterator iter = m_DevList.begin();

   while ( m_DevList.end() != iter ) {
      CCCIDevice *pDev = *iter;
      pDev->OnWorkspaceFreeFailed(Event);
      ++iter;
   }
}

////////////////////////////////////////////////////////////////////////////////

ICCIDeviceFactory * GetCCIDeviceFactory(CCIDeviceImplementation Impl)
{
   switch ( Impl ) {
      case CCI_NULL  : return new(std::nothrow) CNullCCIDeviceFactory();
      case CCI_AAL   : return new(std::nothrow) CCCIDeviceFactory(CCIAFU_NVS_VAL_TARGET_FPGA);
      case CCI_ASE   : return new(std::nothrow) CCCIDeviceFactory(CCIAFU_NVS_VAL_TARGET_ASE);
      case CCI_SWSIM : return new(std::nothrow) CCCIDeviceFactory(CCIAFU_NVS_VAL_TARGET_SWSIM);
      default        : break;
   }

   return NULL;
}

END_NAMESPACE(CCILib)

CCILIB_BEGIN_MOD()
/* default commands only */
CCILIB_END_MOD()

