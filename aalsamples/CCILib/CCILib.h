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
/// @file CCILib.h
/// @brief Defines the top-level entry point to the CCI Library.
/// @ingroup CCILib
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHOR: Tim Whisonant, Intel Corporation
///
/// WHEN          WHO    WHAT
/// ==========    ===    ====
/// 02/16/2012    TSW    Original version.@endverbatim
//****************************************************************************
#ifndef __AALSDK_CCILIB_CCILIB_H__
#define __AALSDK_CCILIB_CCILIB_H__
#include <aalsdk/osal/OSServiceModule.h>
#include <aalsdk/AALLogger.h>
USING_NAMESPACE(AAL)

#ifndef CCILIB_VERSION_CURRENT
# define CCILIB_VERSION_CURRENT  2
#endif // CCILIB_VERSION_CURRENT
#ifndef CCILIB_VERSION_REVISION
# define CCILIB_VERSION_REVISION 0
#endif // CCILIB_VERSION_REVISION
#ifndef CCILIB_VERSION_AGE
# define CCILIB_VERSION_AGE      0
#endif // CCILIB_VERSION_AGE
#ifndef CCILIB_VERSION
# define CCILIB_VERSION          "2.0.0"
#endif // CCILIB_VERSION

#define CCILIB_SVC_MOD         "libCCI" AAL_SVC_MOD_EXT
#define CCILIB_SVC_ENTRY_POINT "libCCI" AAL_SVC_MOD_ENTRY_SUFFIX

#if defined ( __AAL_WINDOWS__ )
# ifdef CCILIB_EXPORTS
#    define CCILIB_API __declspec(dllexport)
# else
#    define CCILIB_API __declspec(dllimport)
# endif // CCILIB_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define CCILIB_API    __declspec(0)
#endif // __AAL_WINDOWS__

AAL_DECLARE_MOD(libCCI, CCILIB_API)

#define CCILIB_BEGIN_MOD() AAL_BEGIN_MOD(libCCI, CCILIB_API, CCILIB_VERSION, CCILIB_VERSION_CURRENT, CCILIB_VERSION_REVISION, CCILIB_VERSION_AGE)
#define CCILIB_END_MOD() AAL_END_MOD()

BEGIN_NAMESPACE(CCILib)

/// @brief Provides synchronization for various output streams in a multi-threaded environment.
///
/// @ingroup CCILib
class IOutputSynchronizer
{
public:
   /// @brief Thread-safe print to stdout.
   ///
   /// @param[in]  fmt  printf-like format specifier
   /// @param[in]  ...  Arguments for format specifier
   virtual void Print(const char *fmt, ...) throw() = 0;

   /// @brief Thread-safe log to stdout.
   ///
   /// @param[in]  fmt  printf-like format specifier
   /// @param[in]  ...  Arguments for format specifier
   virtual void Log(const char *fmt, ...) throw() = 0;

   /// @brief Thread-safe log to stdout.
   ///
   /// @param[in]      fmt   printf-like format specifier
   /// @param[in,out]  va_l  Arguments for format specifier
   virtual void Log(const char *fmt, va_list &va_l) throw() = 0;

   /// @brief Thread-safe trace to stderr.
   ///
   /// @param[in]  fmt  printf-like format specifier
   /// @param[in]  ...  Arguments for format specifier
   virtual void Trace(const char *fmt, ...) throw() = 0;

   /// @brief Thread-safe trace to stderr.
   ///
   /// @param[in]      fmt   printf-like format specifier
   /// @param[in,out]  va_l  Arguments for format specifier
   virtual void Trace(const char *fmt, va_list &va_l) throw() = 0;

   /// @brief Retrieve the current log level of the synchronizer.
   ///
   /// Log levels are set from 0 to 7, in the fashion of syslog.h as
   /// follows:
   ///
   ///  0 LOG_EMERG
   ///  1 LOG_ALERT
   ///  2 LOG_CRIT
   ///  3 LOG_ERR
   ///  4 LOG_WARNING
   ///  5 LOG_NOTICE
   ///  6 LOG_INFO
   ///  7 LOG_DEBUG
   ///
   /// @return The current log level for this synchronizer.
   virtual int GetLogLevel() const throw() = 0;

   /// @brief Sets the log level.
   ///
   /// Note that the log level does not affect @c IOutputSynchronizer::Log.
   /// The output synchronizer only provides a place to store the log level
   /// as convenience.
   ///
   /// @param[in]  lvl  New log level.
   virtual void SetLogLevel(int lvl) throw() = 0;

   /// @brief Retrieve the current trace level of the synchronizer.
   ///
   /// Trace levels are set from 0 to 7, in the fashion of syslog.h as
   /// follows:
   ///
   ///  0 TR_EMERG
   ///  1 TR_ALERT
   ///  2 TR_CRIT
   ///  3 TR_ERR
   ///  4 TR_WARNING
   ///  5 TR_NOTICE
   ///  6 TR_INFO
   ///  7 TR_DEBUG
   ///
   /// @return The current trace level for this synchronizer.
   virtual int GetTraceLevel() const throw() = 0;

   /// @brief Sets the trace level.
   ///
   /// Note that the trace level does not affect @c IOutputSynchronizer::Trace.
   /// The output synchronizer only provides a place to store the trace level
   /// as convenience.
   ///
   /// @param[in]  lvl  New trace level.
   virtual void SetTraceLevel(int lvl) throw() = 0;

protected:
   /// IOutputSynchronzier Destructor
   virtual ~IOutputSynchronizer() throw();

   friend class COutputSynchronizer;
};

/// @brief Basis for all CCI Workspaces(buffers).
///
/// @ingroup CCILib
class ICCIWorkspace
{
public:
   /// ICCIWorkspace Destructor
   virtual ~ICCIWorkspace() throw();

   /// Retrieve the user virtual address of the start of this workspace.
   virtual btVirtAddr GetUserVirtualAddress() const throw() = 0;

   /// Retrieve the physical address of the start of this workspace.
   virtual btPhysAddr GetPhysicalAddress() const throw() = 0;

   /// @brief Retrieve the size of the workspace in bytes.
   ///
   /// This size will match the @c SizeInBytes parameter to @c ICCIDevice::AllocateWorkspace
   virtual btWSSize GetSizeInBytes() const throw() = 0;

   /// @brief Determine the internal state of this workspace.
   ///
   /// Queries the status of constructing this workspace object. If the
   /// construction experienced an error, eg memory not available, then
   /// @c ICCIWorkspace::IsOK will return false.
   ///
   /// @retval  true   Workspace is valid and may be used.
   /// @retval  false  Workspace is invalid and may not be used.
   virtual btBool IsOK() const throw() = 0;
};

/// @brief Basis for all CCI Devices.
///
/// @ingroup CCILib
class ICCIDevice
{
public:
   /// @brief Write Configuration/Status Register.
   ///
   /// @param[in]  CSR    Offset of CSR to write.
   /// @param[in]  Value  Value to be written.
   ///
   /// @retval  true   @c CSR written successfully.
   /// @retval  false  Error attempting to write @c CSR.
   virtual btBool SetCSR(btCSROffset CSR, bt32bitCSR Value) throw() = 0;

   /// @brief Write CCI Address into CSR.
   ///
   /// @param[in]  CSR    Offset of CSR to write.
   /// @param[in]  Addr   Address to be written.
   ///
   /// @retval  true   @c Addr written successfully.
   /// @retval  false  Error attempting to write @c Addr.
   virtual btBool SetAddressCSR(btCSROffset CSR, bt64bitCSR Addr) throw() = 0;

   /// @brief Read Configuration/Status Register.
   ///
   /// @param[in]   CSR    Offset of CSR to read.
   /// @param[out]  Value  Value read.
   ///
   /// @retval  true  @c CSR read successfully.
   /// @retval  false  Error attempting to read @c CSR.
   virtual btBool GetCSR(btCSROffset CSR, bt32bitCSR *Value) throw() = 0;

   /// @brief Allocate a contiguous memory region suitable for this device.
   ///
   /// @param[in]  SizeInBytes  Number of bytes to be allocated.
   ///
   /// @retval  NULL  Error allocating workspace.
   virtual ICCIWorkspace *AllocateWorkspace(btWSSize SizeInBytes) throw() = 0;

   /// @brief Free a workspace obtained from @c AllocateWorkspace.
   ///
   /// @param[in]  pWs  Pointer to the workspace to be freed.
   ///
   /// @retval  true   Workspace freed successfully.
   /// @retval  false  Error freeing workspace.
   virtual btBool FreeWorkspace(ICCIWorkspace *pWs) throw() = 0;

   /// @brief Determine the internal state of this device.
   ///
   /// Queries the status of constructing this device object. If the device
   /// construction experienced an error, eg device driver not available, then
   /// @c ICCIDevice::IsOK will return false.
   ///
   /// @retval  true   Device is valid and may be used.
   /// @retval  false  Device is invalid and may not be used.
   virtual btBool IsOK() const throw() = 0;

   /// @brief Set the output stream synchronizer for this device.
   ///
   /// @param[in]  pOS  Pointer to the output stream synchronizer.
   virtual void SetSynchronizer(IOutputSynchronizer *pOS) throw() = 0;

   /// @brief Retrieve the output stream synchronizer for this device.
   ///
   /// Refer to @c IOutputSynchronizer.
   virtual IOutputSynchronizer * GetSynchronizer() const throw() = 0;

protected:
   /// ICCIDevice Destructor
   virtual ~ICCIDevice() throw();
   friend class CNullCCIDeviceFactory;
   friend class CCCIDeviceFactory;
};

/// @brief Interface for Creating/Destroying CCI Device instances and Output Synchronizers.
///
/// @ingroup CCILib
class ICCIDeviceFactory
{
public:
   /// ICCIDeviceFactory Destructor
   virtual ~ICCIDeviceFactory() throw();

   /// @brief Create a CCI Device instance.
   ///
   /// When the Device instance is no longer needed, it should be destroyed
   /// with @c DestroyCCIDevice.
   ///
   /// @retval  NULL  On error.
   virtual ICCIDevice * CreateCCIDevice() throw() = 0;

   /// @brief Destroy a CCI Device instance.
   ///
   /// @param[in]  pCCIDev  The CCI Device instance to be destroyed.
   virtual void DestroyCCIDevice(ICCIDevice *pCCIDev) throw() = 0;

   /// @brief Create/retrieve the output synchronizer singleton.
   ///
   /// Example of transitioning output synchronizer implementations:
   /// @code
   /// // Allocate a NULL Device Factory, simply to get at the IOutputSynchronizer.
   /// ICCIDeviceFactory   *pDevFactory = GetCCIDeviceFactory(CCI_NULL);
   /// IOutputSynchronizer *pSync       = pDevFactory->GetSynchronizer();
   ///
   /// delete pDevFactory;
   ///
   /// MyObject *pMyObj = new MyObject(pSync);
   ///
   /// // (Parse command line, etc.)
   /// void MyObject::ParseCmdLine()
   /// {
   ///    // The application decides it needs to use AAL.
   ///    AAL::ILogger *pLogger = pAALLogger();
   ///
   ///    // (initialize pLogger here)
   ///
   ///    // Save the synchronizer details.
   ///    int LogLevel   = m_pSync->GetLogLevel();
   ///    int TraceLevel = m_pSync->GetTraceLevel();
   ///
   ///    ICCIDeviceFactory *pDevFactory = GetCCIDeviceFactory(CCI_AAL);
   ///    ICCIDevice        *pCCIDevice  = pDevFactory->CreateCCIDevice();
   ///
   ///    pDevFactory->ReleaseSynchronizer();
   ///
   ///    // Don't use m_pSync until we allocate a new one.
   ///
   ///    m_pSync = pDevFactory->GetSynchronizer(pLogger);
   ///    m_pSync->SetLogLevel(LogLevel);
   ///    m_pSync->SetTraceLevel(TraceLevel);
   ///
   ///    pCCIDevice->SetSynchronizer(m_pSync);
   /// @endcode
   ///
   /// @param[in]  pLogger  Pointer to an AAL::ILogger interface, if required.
   ///
   /// @retval  NULL  On error.
   virtual IOutputSynchronizer * GetSynchronizer() throw() = 0;

   /// Destroy the current output synchronizer singleton.
   virtual void ReleaseSynchronizer() throw() = 0;
};

END_NAMESPACE(CCILib)

#define CCILIB_PRINT_BUF_SIZE 512

/// @brief Use the output synchronizer to print a message (variadic form).
/// @ingroup CCILib
///
/// @param[in,out]  pSync  @c IOutputSynchronizer pointer.
/// @param[in]      msg    printf-like format specifier.
/// @param[in]      ...    Format specifier arguments.
#define PRINT(pSync, msg, ...)           \
do                                       \
{                                        \
   (pSync)->Print(msg, ## __VA_ARGS__ ); \
}while(false)

/// @brief Use the output synchronizer to log a message (variadic form).
/// @ingroup CCILib
///
/// Compares the input log level @c lvl to that stored in @c pSync.
/// If @c lvl is less than or equal to the synchronizer's, logs a
/// message which is prefixed by the current source file, line in
/// the file, and function name, using @c IOutputSynchronizer::Log.
///
/// @param[in,out]  pSync  @c IOutputSynchronizer pointer.
/// @param[in]      lvl    The input log level.
/// @param[in]      msg    printf-like format specifier.
/// @param[in]      ...    Format specifier arguments.
#define LOG(pSync, lvl, msg, ...)                   \
do                                                  \
{                                                   \
   if ( (lvl) <= (pSync)->GetLogLevel() ) {         \
      char buf[CCILIB_PRINT_BUF_SIZE];              \
      ::snprintf(buf, sizeof(buf) / sizeof(buf[0]), \
                 msg, ## __VA_ARGS__ );             \
      (pSync)->Log("(%s:%u) %s() : %s",             \
                     __AAL_SHORT_FILE__,            \
                     __LINE__,                      \
                     __AAL_FUNC__,                  \
                     buf);                          \
   }                                                \
}while(false)

// use syslog.h constants to define trace levels.
#define TR_EMERG   LOG_EMERG
#define TR_ALERT   LOG_ALERT
#define TR_CRIT    LOG_CRIT
#define TR_ERR     LOG_ERR
#define TR_WARNING LOG_WARNING
#define TR_NOTICE  LOG_NOTICE
#define TR_INFO    LOG_INFO
#define TR_DEBUG   LOG_DEBUG


#if (1 == ENABLE_DEBUG)

/// @brief Use the output synchronizer to print a trace message (variadic form).
/// @ingroup CCILib
///
/// Compares the input trace level @c lvl to that stored in @c pSync.
/// If @c lvl is less than or equal to the synchronizer's, prints a
/// message which is prefixed by the current source file, line in
/// the file, and function name, using @c IOutputSynchronizer::Trace.
///
/// @param[in,out]  pSync  @c IOutputSynchronizer pointer.
/// @param[in]      lvl    The input trace level.
/// @param[in]      msg    printf-like format specifier.
/// @param[in]      ...    Format specifier arguments.
#define TRACE(pSync, lvl, msg, ...)                 \
do                                                  \
{                                                   \
   if ( (lvl) <= (pSync)->GetTraceLevel() ) {       \
      char buf[CCILIB_PRINT_BUF_SIZE];              \
      ::snprintf(buf, sizeof(buf) / sizeof(buf[0]), \
                 msg, ## __VA_ARGS__ );             \
      (pSync)->Trace("(%s:%u) %s() : %s",           \
                     __AAL_SHORT_FILE__,            \
                     __LINE__,                      \
                     __AAL_FUNC__,                  \
                     buf);                          \
   }                                                \
}while(false)

#else
#define TRACE(pSync, lvl, msg, ...)
#endif // ENABLE_DEBUG


BEGIN_NAMESPACE(CCILib)

/// @addtogroup CCILib
/// @{

/// @brief Selects among the underlying device implementation choices.
///
/// Selects the underlying implementation of the CCI Device Factory returned
/// from @c GetCCIDeviceFactory, and, therefore, that of the Device and
/// Workspaces created by the CCI Device Factory.
enum CCIDeviceImplementation
{
   CCI_NULL    = 0,       ///< Select NULL implementation. See @c ICCIDeviceFactory::GetSynchronizer and @c CNullCCIDevice.
   CCI_AAL     = 2,       ///< Select AAL AFU implementation.
   CCI_DIRECT  = CCI_AAL, ///< Select Direct CCI driver implementation.
   CCI_ASE     = 3,       ///< Select AFU Simulation Environment implementation.
   CCI_SWSIM   = 4
};

/// @brief Create an instance of @c ICCIDeviceFactory.
///
/// Allocates and returns a concrete instance of the @c ICCIDeviceFactory interface.
/// The CCI Device Factory is capable of creating the CCI Device and Workspace concrete
/// matching the chosen @c CCIDeviceImplementation.
///
/// When done using the CCI Device Factory instance, the requesting code should simply
/// delete the @c ICCIDeviceFactory pointer returned by @c GetCCIDeviceFactory.
///
/// @param[in]  Impl  The desired CCI Device Factory implementation.
///
/// @retval  NULL  When the desired CCI Device Factory can not be allocated.
ICCIDeviceFactory * GetCCIDeviceFactory(CCIDeviceImplementation Impl);

/// @} group CCILib

END_NAMESPACE(CCILib)

#endif // __AALSDK_CCILIB_CCILIB_H__

