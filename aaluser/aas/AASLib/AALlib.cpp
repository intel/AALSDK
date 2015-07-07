// Copyright (c) 2006-2015, Intel Corporation
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
/// @file AALlib.cpp
/// @brief Implements global system initialization and shutdown functions.
/// @ingroup AALCore
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/04/2006     JG       Initial version started
/// 04/23/2007     JG       Extended this module to implement the
///                         other global elements of AAS such as
///                         System initialization functions
/// 08/25/2007     HM       Fixed FactoryOnly instance of AASServiceContainer
///                            sent in SystemInit message to be SC_ALL
/// 08/25/2007     HM       Cleaned up the use of btUnsignedInt64 as proxy for
///                            btID, and the initialization of exception and
///                            exception transaction objects, and the use
///                            of Factory vs. Registrar versions of SC
///                            TODO: still need to figure out how EDS fits
/// 10/05/2007     JG       Renamed to AALlib.cpp
/// 10/31/2007     HM       Cleaned up whitespace
/// 11/27/2007     JG/HM    Created the Assassin Object for asynchronous
///                            shutdown and cleanup.
/// 11/27/2007     JG/HM    Cleaned up Leaks
/// 12/01/2007     JG       Fixed Windows build bugs
///                JG       Fixed Windows dll names for built in services
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 03/14/2008     JG       Modified Completion codes for Non-PreEmptive mode
///                         shutdown and to enable a more orderly shutdown
///                         of system services
/// 03/28/2008     HM       Removed now extraneous call to
///                            CAASServiceContainer::Disable()
/// 05/08/2008     HM       Comments & License
/// 06/24/2008     HM       Added <stdlib.h> for getenv()
/// 11/18/2008     HM       Removed #if 0/#endif around new Factory code
///                            to ensure proper merging behavior
/// 01/04/2009     HM       Updated Copyright
/// 06/30/2009     JG       Redesigned shutdown protocol to provide for an
///                            more orderly shutdown of plug-in services loaded
///                            via Service factory
///                         Added support for Shutdown Max time system
///                            configuration parameter and system config parms
///                            in general
/// 07/15/2009     HM/JG    Tweaked Shutdown processing a bit. Added explicit
///                            non-Logger tracing support via DEBUG_BEYOND_LOGGER
/// 11/13/2010     AG       fix double delete in factory service failure
///                            handling
/// 09/20/2011     JG       Added a quick feature to enable kernel-less oper.
///                            Disables Registrar. This is not a long term
///                            implementation.
/// 10/21/2011     JG       Added code for Windows compatibility@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/aas/AASService.h"

#ifdef __AAL_WINDOWS__

//=============================================================================
// Name: DllMain
// Description: Windows dll entry points.
// Interface: public
// Inputs: hModule - Module (or instance) handle. IDs the instance of the DLL
//         ul_reason_for_call - Type of attach or detach
// Outputs: Success or failure.
// Comments:
//=============================================================================
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH :
         break;
      case DLL_THREAD_ATTACH  :
         break;
      case DLL_THREAD_DETACH  :
         break;
      case DLL_PROCESS_DETACH :
         break;
   }
   return TRUE;
}

#endif // __AAL_WINDOWS__


#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAS_BEGIN_MOD()
   /* Only default cmds at the moment. */
AAS_END_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////                            ////////////////////////////
///////////////////                 Doxygen                 ////////////////////
////////////////////////                            ////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Group Hierarchy /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @defgroup AALCore AALSDK Core User-Mode Framework
/// @brief Core AAL functionality.

   /// @defgroup BasicTypes Basic Data Types
   /// @ingroup AALCore

   /// @defgroup OSAL OS Abstraction Layer
   /// @ingroup AALCore

   /// @defgroup CommonBase Common Base Class
   /// @ingroup AALCore

   /// @defgroup AALRUNTIME AAL Run Time Framework
   /// @ingroup AALCore

   /// @defgroup Events Event Infrastructure
   /// @ingroup AALCore

      /// @defgroup SysEvents System Events
      /// @ingroup Events

   /**
    @defgroup Services Service Infrastructure
    @ingroup AALCore
   
    The AAL Service is comprised of a Service Module (IServiceModule) which
     implements the load-able Service executable (e.g., shared or dynamic link library).
   
    The Service becomes load-able by defining a Service Factory (ISvcsFact) using one of
     the provided Service Factory templates. The Service Factory is responsible for
     construction details such as in-proc, daemon, remote. For example
     @code InProcSvcsFact< MyService >@endcode specifies that the
     Service is implemented in-process (i.e., in the same process address space) where
     @code IPCSvcsFact< MyService >@endcode defines a Service Factory that constructs a
     Service that uses IPC to communicate.
   
    The DEFINE_SERVICE_PROVIDER_2_0_ACCESSOR() macro instantiates the Service Factory and
     the well-known entry point (GetServiceProviderfcn_t) to the Service Module.
   
    AALServiceModule implements the IServiceModule interface, instantiates the "real" service
     through an ISvcsFact and optional Marshaler and Transport interfaces. As Services are
     created, AALServiceModule tracks pointers to the Service objects.
   
    The Service object must be a ServiceBase derived class. This allows
     the Service to provide implementations of most of the required IServiceModule
     interactions.
   
    The Marshaler is a template class that is responsible for marshaling the
     interface appropriately for the object. The default in-proc marshaler is
     a stub.
   
    Initialization Complete Function Object (Event)
     The initialization process for a Service is an asynchronous operation
     that may involve the initialization of derived classes in the
     inheritance tree.  The SDK guarantees that all super classes of the SDK
     have completed initialization before the most derived subclass' init()
     method is called.  This ensures that the framework and all it facilities
     are available to the subclass.  This requires that the inheritance tree
     be initialized from the top (base class) down, in order. This is analogous
     to a traditional constructor order.
   
    To do this there needs to be a synchronization mechanism in place so that
     each derived can perform its initialization AFTER its immediate base.
     This is accomplished using AAL Event based Function Objects.
   
    When base class' _init() function is called an optional InitComplete
     function object event is passed.  The event contains a pointer to the
     callers completion member function.  When a _init() is provided a
     InitComplete event, it must post it when it has completed processing.
   */

      // (Service Writer's Guide appears below)

      /// @defgroup ServiceModule API for Plug-able Services
      /// @ingroup Services

      /// @defgroup AFU Accelerator Function Unit
      /// @ingroup Services

         /// @defgroup ICCIAFU ICCIAFU - Service interface for interacting with CCI-attached AFU's.
         /// @ingroup AFU 

         /// @defgroup ISPLAFU ISPLAFU - Service interface for interacting with SPL-attached AFU's.
         /// @ingroup AFU

/// @defgroup UserModeServiceSDK User Mode Service SDK
/// @brief The User Mode Service SDK provides tools to facilitate designing and building compliant AAL User Mode Services.
///
/// The AAL User Mode Service Software Developers Kit consists of a collection of classes and templates designed
/// to facilitate developing compliant AAL User Mode Services. The ServiceBase class provides implementation for
///  the canonical interfaces required of a Service. In addition the ServiceBase provides convenient member
///  functions that enable the developer access to useful data and Platform Services.

         /// @defgroup ServiceBase
         /// @ingroup UserModeServiceSDK

         /// @defgroup DeviceServiceBase
         /// @ingroup UserModeServiceSDK

         /// @defgroup InProcFactory
         /// @brief Responsible for instantiating an in-process Service instance
         ///
/// The AAL Service is comprised of a Service Library which implements the
///  the load-able module  (e.g., shared or dynamic link library).
///
/// The module  becomes load-able by defining a factory class using one of the
///  provided templates InProcSvcsFact<> which defines how the service is
///  implemented. For example InProcSvcsFact<> specifies that the service is
///  implemented in-proc (i.e., in the same process space) where IPCSvcsFact<>
///  defines a factory that constructs a service that uses IPC to communicate.
/// An AALServiceModule class implements a wrapper for the service
///  IServiceModule interface, instantiates the "real" service through a
///  factory and optional marshaler and transport.
///
/// The Service container is implemented as a class AALServiceModule.
///  The container creates and holds a pointer to the Service object.
///
///  The Service object must be an IServiceModule derived class. This allows
///  the service to provide implementation for most of the IServiceModule
///  interfaces.  The container forwards all such calls through to the
///  service, performing canonical behavior for the service.
         /// @ingroup UserModeServiceSDK

   /// @defgroup Dispatchable
   /// @brief Used to schedule work. E.g.,to schedule a callback.
   ///
   /// The Message Delivery Service is more than the name implies.  Its purpose is to queue and schedule the
   /// dispatch of work items known as Dispatchables. Dispatchables in AAL are a type of object similar to a
   /// functor.  Dispatchables are defined as part of the OS abstraction library OSAL.
   /// Dispatchable objects implement the IDispatchable interface
   /// Like classic functor objects, the IDispatchable has the concrete object implement the operator().
   /// The functionality that takes place in the Dispatchable is entirely implementation-dependent.
   /// The Message Delivery Service enqueues Dispatchables and then dispatches them by invoking their
   /// operator() at some later time, on a thread dedicated to dispatching.
   /// @ingroup UserModeServiceSDK

/// @defgroup SysServices System Services
/// @ingroup Services

         /// @defgroup ServiceBroker
         /// @ingroup SysServices
         ///AAL implements a Factory design pattern to abstract the instantiation of Services from the application.
         ///The AAL Service Broker provides the interface used to request a Service. The Service Broker interacts with
         ///the Registrar to determine how a Service is to be instantiated.

         /// @defgroup MDS
         /// @ingroup SysServices
         /// The Message Delivery Service (MDS) is a mandatory core service. MDS is used for scheduling callbacks,
         /// AAL Events to objects and can even be used to schedule work items in the form of functors.

         /// @defgroup Registrar
         /// @ingroup SysServices

         /// @defgroup ResMgr Resource Manager
         /// @ingroup SysServices

         /// @defgroup uAIA
         /// @ingroup SysServices

   /// @defgroup Utils Utilities
   /// @ingroup AALCore

      /// @defgroup AASUtils AAS Utilities
      /// @ingroup Utils

      /// @defgroup EventUtils Event Utilities
      /// @ingroup Utils

      /// @defgroup RMUtils Resource Manager Utilities
      /// @ingroup Utils

      /// @defgroup SingleAFUApp ISingleAFUApp - Template helper class for AAL applications requiring a single AFU.
      /// @ingroup Utils

      /// @defgroup CSyncClient CSyncClient - Helper class for synchronous AAL applications.
      /// @ingroup Utils

/// @defgroup Debugging Internal Debugging Facilities
/// @brief Debug Logging & Assertions

/// @defgroup aalclp AAL Command Line Parser
/// @brief Generic Linux Command Line Parser library.

/// @defgroup UtilityAFUs AALSDK Utility AFUs
/// @brief Utility AALSDK Accelerator Function Units.

   /// @defgroup CCIAFU CCIAFU - Generalized Implementation of ICCIAFU
   /// @ingroup UtilityAFUs
   /// @brief Facilitates @ref HWCCIAFU "Hardware", @ref ASECCIAFU "ASE", and @ref SWSimCCIAFU "Software-simulated CCI" access via ICCIAFU.

      /// @defgroup HWCCIAFU HWCCIAFU - FPGA Implementation of ICCIAFU
      /// @ingroup CCIAFU 
      /// @brief Hardware CCI access via ICCIAFU.

      /// @defgroup ASECCIAFU ASECCIAFU - ASE Implementation of ICCIAFU
      /// @ingroup CCIAFU 
      /// @brief ASE-based CCI access via ICCIAFU.

      /// @defgroup SWSimCCIAFU SWSimCCIAFU - Software Implementation of ICCIAFU
      /// @ingroup CCIAFU 
      /// @brief Simulated CCI access via ICCIAFU.

   /// @defgroup SPLAFU SPLAFU - Generalized Implementation of ISPLAFU
   /// @ingroup UtilityAFUs
   /// @brief Facilitates @ref HWSPLAFU "Hardware", @ref ASESPLAFU "ASE", and @ref SWSimSPLAFU "Software-simulated SPL" access via ISPLAFU.

      /// @defgroup HWSPLAFU HWSPLAFU - FPGA Implementation of ISPLAFU
      /// @ingroup SPLAFU 
      /// @brief Hardware SPL access via ISPLAFU.

      /// @defgroup ASESPLAFU ASESPLAFU - ASE Implementation of ISPLAFU
      /// @ingroup SPLAFU 
      /// @brief ASE-based SPL access via ISPLAFU.

      /// @defgroup SWSimSPLAFU SWSimSPLAFU - Software Implementation of ISPLAFU
      /// @ingroup SPLAFU 
      /// @brief Simulated SPL access via ISPLAFU.

/// @defgroup UtilApps AALSDK Utility Applications
/// @brief Applications that provide information about the current AALSDK installation.

   /// @defgroup aalscan Scan for AALSDK Service Modules
   /// @ingroup UtilApps
   /// @brief AALSDK Utility.

   /// @defgroup data_model Show the C/C++ data model for the current compiler
   /// @ingroup UtilApps
   /// @brief AALSDK Utility.


////////////////////////////////////////////////////////////////////////////////
// Main (outline) page /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
@mainpage

<ul>
  <li>@ref AALCore "AALSDK Core User-Mode Framework"</li>
    <ul>
      <li>@ref BasicTypes "Basic Data Types"</li>
      <li>@ref OSAL       "OS Abstraction Layer"</li>
      <li>@ref CommonBase "Common Base Class"</li>
      <li>@ref AALRUNTIME  "AAL Run Time Framework"</li>
      <li>@ref Events     "Event Infrastructure"</li>
        <ul>
          <li>@ref SysEvents "System Events"</li>
        </ul>
      <li>@ref UserModeServiceSDK   "User Mode Service SDK"</li>
        <ul>
          <li>@ref ServiceBase           "Service Base"</li>
          <li>@ref DeviceServiceBase     "Device Service Base"</li>
          <li>@ref InProcFactory         "In-process Service Factories"</li>
          <li>@ref Dispatchable          "Dispatchable - Scheduleable Objects"</li>
          <li>@ref AFU                   "Accelerator Function Unit (AFU)"</li>
            <ul>
              <li>@ref ICCIAFU  "CCI Service Interface"</li>
              <li>@ref ISPLAFU  "SPL Service Interface"</li>
            </ul>
       </ul>
     <li>@ref SysServices          "System Services"</li>
         <ul>
           <li>@ref ServiceBroker  "Service Broker"</li>
           <li>@ref MDS            "Message Delivery"</li>
           <li>@ref Registrar      "Registrar"</li>
           <li>@ref ResMgr         "Resource Manager"</li>
         </ul>
      <li>@ref Utils "Utilities"</li>
        <ul>
          <li>@ref AASUtils     "AAS Utilities"</li>
          <li>@ref RMUtils      "Resource Manager Utilities"</li>
          <li>@ref SingleAFUApp "ISingleAFUApp - AAL application template"</li>
          <li>@ref CSyncClient  "CSyncClient - synchronous AAL application base class"</li>
        </ul>
  <li>@ref Debugging "Internal Debugging Facilities"</li>
  <li>@ref UtilityAFUs "AALSDK Utility AFUs"</li>
    <ul>
      <li>@ref CCIAFU      "CCIAFU"</li>
      <ul>
        <li>@ref HWCCIAFU     "HWCCIAFU"</li>
        <li>@ref ASECCIAFU    "ASECCIAFU"</li>
        <li>@ref SWSimCCIAFU  "SWSimCCIAFU"</li>
      </ul>
      <li>@ref SPLAFU      "SPLAFU"</li>
      <ul>
        <li>@ref HWSPLAFU     "HWSPLAFU"</li>
        <li>@ref ASESPLAFU    "ASESPLAFU"</li>
        <li>@ref SWSimSPLAFU  "SWSimSPLAFU"</li>
      </ul>
    </ul>
  <li>@ref UtilApps "AALSDK Utility Applications"</li>
    <ul>
      <li>@ref aalscan    "aalscan - Scan for AALSDK Service Modules"</li>
      <li>@ref data_model "data_model - Show the C/C++ data model for the current compiler"</li>
    </ul>
</ul>


@verbatim
   INFORMATION IN THIS DOCUMENT IS PROVIDED IN  CONNECTION  WITH  INTEL(R)
   PRODUCTS. NO LICENSE, EXPRESS OR IMPLIED, BY ESTOPPEL OR  OTHERWISE, TO
   ANY INTELLECTUAL PROPERTY RIGHTS IS GRANTED BY THIS DOCUMENT. EXCEPT AS
   PROVIDED IN INTEL'S TERMS AND CONDITIONS OF  SALE  FOR  SUCH  PRODUCTS,
   INTEL ASSUMES NO LIABILITY WHATSOEVER, AND  INTEL DISCLAIMS ANY EXPRESS
   OR IMPLIED WARRANTY, RELATING TO  SALE AND/OR  USE  OF  INTEL  PRODUCTS
   INCLUDING LIABILITY OR WARRANTIES  RELATING TO FITNESS FOR A PARTICULAR
   PURPOSE, MERCHANTABILITY, OR INFRINGEMENT  OF  ANY PATENT, COPYRIGHT OR
   OTHER INTELLECTUAL PROPERTY RIGHT.
                            
   UNLESS OTHERWISE AGREED IN WRITING BY INTEL, THE INTEL PRODUCTS ARE NOT
   DESIGNED NOR INTENDED FOR ANY APPLICATION IN WHICH THE  FAILURE  OF THE
   INTEL PRODUCT COULD CREATE A SITUATION WHERE PERSONAL INJURY  OR  DEATH
   MAY OCCUR.
                                        
   Intel may make changes to specifications and  product  descriptions  at
   any time, without notice.  Designers  must  not  rely on the absence or
   characteristics of  any  features  or instructions marked "reserved" or
   "undefined." Intel reserves these for  future definition and shall have
   no responsibility whatsoever for conflicts or incompatibilities arising
   from future changes to them. The information  here is subject to change
   without notice. Do not finalize a design with this information.
                                                             
   The  products described  in this document may contain design defects or
   errors known as errata which  may  cause  the  product  to deviate from
   published specifications. Current characterized errata are available on
   request.
                                                                         
   Contact your local Intel sales office or your distributor to obtain the
   latest specifications and before placing your product order.
                                                                               
   Copies  of  documents  which have an order number and are referenced in
   this document, or other Intel literature, may be obtained by calling 1-
   800-548-4725, or by visiting Intel's Web Site, www.intel.com.@endverbatim


@verbatim
   The information in this manual is subject to change without notice and
   Intel Corporation assumes no responsibility or liability for any
   errors or inaccuracies that may appear in this document or any
   software that may be provided in association with this document. This
   document and the software described in it are furnished under license
   and may only be used or copied in accordance with the terms of the
   license. No license, express or implied, by estoppel or otherwise, to
   any intellectual property rights is granted by this document. The
   information in this document is provided in connection with Intel
   products and should not be construed as a commitment by Intel
   Corporation.
   
   EXCEPT AS PROVIDED IN INTEL'S TERMS AND CONDITIONS OF SALE FOR SUCH
   PRODUCTS, INTEL ASSUMES NO LIABILITY WHATSOEVER, AND INTEL DISCLAIMS
   ANY EXPRESS OR IMPLIED WARRANTY, RELATING TO SALE AND/OR USE OF INTEL
   PRODUCTS INCLUDING LIABILITY OR WARRANTIES RELATING TO FITNESS FOR A
   PARTICULAR PURPOSE, MERCHANTABILITY, OR INFRINGEMENT OF ANY PATENT,
   COPYRIGHT OR OTHER INTELLECTUAL PROPERTY RIGHT. Intel products are not
   intended for use in medical, life saving, life sustaining, critical
   control or safety systems, or in nuclear facility applications.
    
   Designers must not rely on the absence or characteristics of any
   features or instructions marked "reserved" or "undefined." Intel
   reserves these for future definition and shall have no responsibility
   whatsoever for conflicts or incompat- ibilities arising from future
   changes to them.
     
   The software described in this document may contain software defects
   which may cause the product to deviate from published
   specifications. Current characterized software defects are available
   on request.
      
   Intel, the Intel logo, Intel SpeedStep, Intel NetBurst, Intel
   NetStructure, MMX, Intel386, Intel486, Celeron, Intel Centrino, Intel
   Xeon, Intel XScale, Itanium, Pentium, Pentium II Xeon, Pentium III
   Xeon, Pentium M, and VTune are trademarks or registered trademarks of
   Intel Corporation or its subsidiaries in the United States and other
   countries.

   * Other names and brands may be claimed as the property of others. 
        
   Copyright 2003-2014, Intel Corporation.@endverbatim 


Unless otherwise stated, any software source code reprinted in this document
is furnished under a software license and may only be used or copied in
accordance with the terms of that license. Specifically:

@verbatim
Copyright (c) 2003-2014, Intel Corporation

Redistribution  and  use  in source  and  binary  forms,  with  or  without
modification, are permitted provided that the following conditions are met:

* Redistributions of  source code  must retain the  above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither the name  of Intel Corporation  nor the names of its contributors
  may be used to  endorse or promote  products derived  from this  software
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.@endverbatim

This document contains information on products in the design phase of
development.


For flex, The Fast Lexical Analyzer:

@verbatim
   Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 The Flex
Project.

   Copyright (C) 1990, 1997 The Regents of the University of California.
All rights reserved.

   This code is derived from software contributed to Berkeley by Vern
Paxson.

   The United States Government has rights in this work pursuant to
contract no. DE-AC03-76SF00098 between the United States Department of
Energy and the University of California.

   Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the
     distribution.

   Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.@endverbatim

*/





