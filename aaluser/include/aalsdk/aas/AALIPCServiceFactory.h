// Copyright(c) 2011-2016, Intel Corporation
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
/// @file AALIPCServiceFactory.h
/// @brief AAL remote services. 
/// @ingroup Services
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/30/2011     JG       Split out device services
/// 09/01/2011     JG       Changed AALServiceContainer into AASServiceModule
/// 09/05/2011     JG       Implemented lists of service is a module.
///                            clean-up and release of multiple services
/// 01/12/2012     JG       Modified for cleaned up Service init() protocol
/// 04/23/2012     HM       Added check for EOF in getmsg() to handle remote
///                            hang-up
/// 07/14/2012     HM       Matched returned value to return type@endverbatim
//****************************************************************************
#ifndef __AALSDK_AAS_AALIPCSERVICEFACTORY_H__
#define __AALSDK_AAS_AALIPCSERVICEFACTORY_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALNVSMarshaller.h>
#include <aalsdk/osal/Sleep.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////                                                   //////////////
///////                          AAL SERVICE                            ///////
//////////////                                                   //////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// @addtogroup InProcFactory
/// @{
///
/// Factory template for creating a multiplicity of Services implemented
///  within the client address space.
//
/// The AAL Service is comprised of a Service package which implements the
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
//
///  The Service object must be an IServiceModule derived class. This allows
///  the service to provide implementation for most of the IServiceModule
///  interfaces.  The container forwards all such calls through to the
///  service, performing canonical behavior for the service.
//
/// The service factory is a template that instantiates the service. The factory
///  is responsible for construction details such as in-proc, daemon, remote.
//
// The marshaler is a template class that is responsible for marshaling the
//  interface appropriately for the object. The default in-proc marshaler is
//  a stub.
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BEGIN_NAMESPACE(AAL)


#define AAL_SERVICE_NAME                        "AAL_SERVICE_NAME"
#define AAL_SERVICE_CONNECTION_PARMS            "AAL_SERVICE_CONNECTION_PARMS"
#define AAL_SERVICE_ARGS                        "AAL_SERVICE_ARGS"
#define AAL_SERVICE_COMM_POR                    "AAL_SERVICE_COMM_POR"
#define AAL_SERVICE_COMM_PORT                   "AAL_SERVICE_COMM_PORT"
#define AAL_SERVICE_CONNECTION_TYPE             "AAL_SERVICE_CONNECTION_TYPE"
#define AAL_SERVICE_COMM_HOST                   "AAL_SERVICE_COMM_HOST"
#define AAL_SERVICE_CONNECTION_MAX_SERVER_WAIT  "AAL_SERVICE_CONNECTION_MAX_SERVER_WAIT"
#define MSG_BUFFER_SIZE                         65534
#define MAX_MSG_SIZE                            MSG_BUFFER_SIZE+1

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (~0)
#endif


//=============================================================================
// Name: TCPIPxport
// Description: Class provides the implementation of the AAL Service
//              TCP/IP IPC transport.
//=============================================================================
/// @brief Class provides the implementation of the AAL Service
///              TCP/IP IPC transport.
class TCPIPxport : public IAALTransport
{
public:
   
   TCPIPxport():
         m_socket(0),
         m_clientsock(0),
         m_port(0),
         m_clientaddrlen(0)
   {
   }
   
   TCPIPxport(TCPIPxport const& rother )
   {
      m_socket = rother.m_socket;
      m_clientsock = rother.m_clientsock;
      m_port = rother.m_port;
      m_clientaddrlen = rother.m_clientaddrlen;
      m_clientaddr = rother.m_clientaddr;
   }


   ~TCPIPxport()
   {
      disconnect();
   }

   //=============================================================================
   // Name: connectremote
   // Description: Connect to the remote server
   // Inputs optArgs - named ValueSet contains connection arguments
   //=============================================================================
   /// @brief Connects to the remote server
   ///
   /// @param[in] optArgs - named ValueSet contains connection arguments
   btBool connectremote(NamedValueSet const &optArgs)
   {
      btcString host;

     NamedValueSet const *connParmskvs;
     NamedValueSet const *configRecordkvs;

     // Get the configuration record
     if(!optArgs.Has(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED)){
      return false;
     }else{
      optArgs.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configRecordkvs);
     }

     // Check for connection parameters
     if(!configRecordkvs->Has(AAL_SERVICE_CONNECTION_PARMS)){
       return false;
     }else{
       configRecordkvs->Get(AAL_SERVICE_CONNECTION_PARMS, &connParmskvs);
     }

     if(!connParmskvs->Has(AAL_SERVICE_COMM_HOST)){
        return false;
     }else{
        connParmskvs->Get(AAL_SERVICE_COMM_HOST, &host);
     }

     if(!connParmskvs->Has(AAL_SERVICE_COMM_PORT)){
        return false;
     }else{
        connParmskvs->Get(AAL_SERVICE_COMM_PORT, &m_port);
     }

     eservice_connection_types conntype;

     if(!connParmskvs->Has(AAL_SERVICE_CONNECTION_TYPE)){
        return false;
     }else{
        connParmskvs->Get(AAL_SERVICE_CONNECTION_TYPE, reinterpret_cast<int*>(&conntype));
     }

#if defined( __AAL_WINDOWS__ )
   WORD    wVersionRequested;
   WSADATA wsaData;
   int     wsaerr;
   // Using MAKEWORD macro, Winsock version request 2.2
   wVersionRequested = MAKEWORD(2, 2);
    
   wsaerr = WSAStartup(wVersionRequested, &wsaData);
   if (wsaerr != 0)
   {
       /* Tell the user that we could not find a usable */
       /* WinSock DLL.*/
       printf("The Winsock dll not found!\n");
       return 0;
   }
   else
   {
       printf("The Winsock dll found!\n");
       printf("The status: %s.\n", wsaData.szSystemStatus);
   }
#endif // __AAL_WINDOWS__

     if(conntype != conn_type_tcp){
        cerr<<"Wrong connectiontype\n";
        return false;
     }

     m_hostent = gethostbyname(host);
     if (0 == m_hostent){
        perror("Unknown host");
        return false;
     }

     memcpy((char *)&m_clientaddr.sin_addr,
            (char *)m_hostent->h_addr,
             m_hostent->h_length);

     m_clientaddr.sin_family = AF_INET;
     m_clientaddr.sin_port = htons(m_port);

     m_clientsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     if (m_socket < 0){
        perror("socket");
        return false;
     }

     int maxwait=10;
     if(connParmskvs->Has(AAL_SERVICE_CONNECTION_MAX_SERVER_WAIT)){
        connParmskvs->Get(AAL_SERVICE_CONNECTION_MAX_SERVER_WAIT, reinterpret_cast<int*>(&maxwait));
     }

     m_clientaddrlen = sizeof(struct sockaddr_in);

     do{
        if( 0 == connect(m_clientsock,(struct sockaddr *)&m_clientaddr,m_clientaddrlen) ){
           return true;
        }
        SleepSec(1);
     }while(maxwait--);

     perror("connecting socket");
     cout <<"host " <<host << "port "<< m_port <<endl;
     close(m_socket);
     return false;


#if 0
     int bufsize = sizeof(m_buffer);
     if(0 != setsockopt(m_clientsock,SOL_SOCKET,SO_SNDBUF,&bufsize,sizeof(int))){
        perror("setsocket");
        close(m_socket);
        return false;
     }
#endif
     return true;
   }

   //=============================================================================
   // Name: waitforconnect
   // Description: Wait for a connection from a client
   // Inputs optArgs - named ValueSet contains connection arguments
   //=============================================================================
   /// @brief Wait for a connection from a client
   ///
   /// @param[in] optArgs named ValueSet contains connection arguments
   btBool waitforconnect(NamedValueSet const &optArgs)
   {

      //------------------------------
      // get the connection parameters
      //------------------------------
      NamedValueSet const *connParmskvs;

      if(!optArgs.Has(AAL_SERVICE_CONNECTION_PARMS)){
        return false;
      }else{
        optArgs.Get(AAL_SERVICE_CONNECTION_PARMS, &connParmskvs);
      }

      // Port number to use
      if(!connParmskvs->Has(AAL_SERVICE_COMM_PORT)){
        return false;
      }else{
        connParmskvs->Get(AAL_SERVICE_COMM_PORT, &m_port);
      }

      // Make sure the client expects to be using
      //  this connection type
      eservice_connection_types conntype;

      if(!connParmskvs->Has(AAL_SERVICE_CONNECTION_TYPE)){
        return false;
      }else{
        connParmskvs->Get(AAL_SERVICE_CONNECTION_TYPE, reinterpret_cast<int*>(&conntype));
      }

      if(conntype != conn_type_tcp){
        cerr<<"Wrong connectiontype\n";
        return false;
      }

#if defined( __AAL_WINDOWS__ )
   WORD    wVersionRequested;
   WSADATA wsaData;
   int     wsaerr;
   // Using MAKEWORD macro, Winsock version request 2.2
   wVersionRequested = MAKEWORD(2, 2);
    
   wsaerr = WSAStartup(wVersionRequested, &wsaData);
   if (wsaerr != 0)
   {
       /* Tell the user that we could not find a usable */
       /* WinSock DLL.*/
       printf("The Winsock dll not found!\n");
       return 0;
   }
   else
   {
       printf("The Winsock dll found!\n");
       printf("The status: %s.\n", wsaData.szSystemStatus);
   }
#endif // __AAL_WINDOWS__
      // Create the socket
      m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if(m_socket < 0) {
        perror("Opening socket");
        return false;
      }
#if 0
      int bufsize = sizeof(m_buffer);
      m_clientaddrlen = sizeof(struct sockaddr_in);

      if(0 != setsockopt(m_socket,SOL_SOCKET,SO_RCVBUF,&bufsize,sizeof(int))){
        perror("setsocket");
        goto err;
      }
#endif


      {
         // Define an address on the Internet domain
         //  for this server
         struct sockaddr_in      sockaddr;
         int length = sizeof(sockaddr);
         memset(&sockaddr,0,length);

         sockaddr.sin_family=AF_INET;
         sockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
         sockaddr.sin_port=htons(m_port);

         // Bind the socket to the address
         if (bind(m_socket,(struct sockaddr *)&sockaddr,length)<0) {
            perror("binding");
            goto err;
         }

         // Enable the process to listen on the socket
         if(listen(m_socket, 5) <0){
           perror("Listening");
           goto err;
         }

         // Wait for a connection
         //m_clientsock=accept(m_socket,(struct sockaddr *)&m_clientaddr,&m_clientaddrlen);
         m_clientsock=accept(m_socket,NULL,NULL);
         if(m_clientsock == INVALID_SOCKET){
           perror("accept");
         }
      }

      return true;

err:
   close(m_socket);
   return false;

   }


   //=============================================================================
   // Name: disconnect
   // Description: Disconnect remote party and destroy channel
   // Inputs optArgs - named ValueSet contains connection arguments
   //=============================================================================
   /// @brief Disconnect remote party and destroy channel
   ///
   /// @retval True.
   btBool disconnect(void)
   {

      if(0 !=m_socket){
            close(m_socket);
            m_socket=0;
      }

         if(0 !=m_clientsock){
            close(m_clientsock);
            m_clientsock=0;
      }
      return true;

   }

 
   //=============================================================================
   // Name: getmsg
   // Description: Get a message from remote end
   // Inputs: *len - pointer of where to return message length
   // Outputs: length of message (-1) in case of error, 0 means EOF (remote close)
   // Returns: pointer to message (NULL in case of error or EOF)
   //=============================================================================
   /// @brief Gets a message from remote end
   ///
   ///Returns a pointer to message (NULL in case of error or EOF)
   ///@param[in] *len pointer of where to return message length
   ///           length of message (-1) in case of error, 0 means EOF (remote close)
   ///@return The message string.
   btcString getmsg(btWSSize *len)
   {
      // Get the length of the message as a string for portability
      char lenstr[7] = {0};
      int  bytes_recv;

      bytes_recv = recv(m_clientsock, lenstr, sizeof(lenstr), 0);
      ASSERT(bytes_recv >= 0);
      if ( bytes_recv < 0 ) {
         *len = (btWSSize)-1;
         return NULL;
      }
      *len = bytes_recv;

      if ( 0 == bytes_recv ) {
         return NULL;      // EOF seen, not an error
      }

      // Convert the string buffer size to int.
      sscanf(lenstr, "%d", bytes_recv);

      // Read the message TODO deal with real long messages
      ASSERT(bytes_recv <= MSG_BUFFER_SIZE);
      if ( bytes_recv > MSG_BUFFER_SIZE ) {
         *len = bytes_recv;
         return NULL;
      }

      // Read the message
      bytes_recv = recv(m_clientsock, m_buffer, bytes_recv, 0);        

      ASSERT(bytes_recv >= 0);
      if ( bytes_recv < 0 ) {
         *len = (btWSSize)-1;
         return NULL;
      }
      *len = bytes_recv;

      if ( 0 == bytes_recv ) {
         return NULL;      // EOF seen, not an error
      }

      return m_buffer;
   }

   //=============================================================================
   // Name: putmsg
   // Description: Sned a message to remote end
   // Inputs:  pmsg - pointer to message
   //          len  - length of message in octets
   // Returns: number of bytes sent (-1 if error)
   //=============================================================================
   int putmsg(btcString pmsg, btWSSize len)
   {
      char lenstr[7]={0};
      sprintf(lenstr,"%llu",len);

      send(m_clientsock,
           lenstr,
           sizeof(lenstr),
           0);

      int n = send(m_clientsock,
                   pmsg,
                   len,
                   0);

      if(n<0){
         perror("sendto");
      }

      return n;
   }

protected:
   TCPIPxport & operator = (const TCPIPxport & );

   int                 m_socket;
   int                 m_clientsock;
   int                 m_port;
   char                m_buffer[MSG_BUFFER_SIZE];
   socklen_t           m_clientaddrlen;
   struct sockaddr_in  m_clientaddr;
   struct hostent     *m_hostent;
};


//=============================================================================
/// IPCSvcsFact
/// Template provides the implementation of the AAL Service
///              factory for inter-proces (same node) service creation
///@param[in] I - Concrete class name of proxy
///@param[in] T - Transport - Remote Service only
///@param[in] M - Marshaller - Remote Service only
///@param[in] U - Unmarshaller - Remote Service only
//=============================================================================
template <typename I,
          class T=TCPIPxport,
          class M=NVSMarshaller,
          class U=NVSUnMarshaller> class IPCSvcsFact: public ISvcsFact
{
public:
   IBase* CreateServiceObject(AALServiceModule    *container,
                              btEventHandler       eventHandler,
                              btApplicationContext context,
                              TransactionID const &rtid,
                              NamedValueSet const &optArgs)
   {
      // Get the connection parameters
      btStringArray     ppArgs;
      
      NamedValueSet const *connParmskvs, *configRecordkvs;
      // Get the configuration record
      if(!optArgs.Has(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED)){
         return NULL;
      }else{
         optArgs.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configRecordkvs);
      }

      // Check for connection parameters
      if(!configRecordkvs->Has(AAL_SERVICE_CONNECTION_PARMS)){
         return NULL;
      }else{
         configRecordkvs->Get(AAL_SERVICE_CONNECTION_PARMS, &connParmskvs);
      }

      btcString         pServiceName=NULL;
      if(connParmskvs->Has(AAL_SERVICE_NAME)){
         connParmskvs->Get(AAL_SERVICE_NAME, &pServiceName);
      }
      if(NULL !=pServiceName) {
         if(connParmskvs->Has(AAL_SERVICE_ARGS)){
            connParmskvs->Get(AAL_SERVICE_ARGS, &ppArgs);
         }

   #ifdef __AAL_LINUX__
         // Fork the child
         pid_t  m_pid = fork();

         if(m_pid==0){
             cerr<<"I am child\n";
             execvp(pServiceName,ppArgs);
         }else{
             cerr<<"I am parent of pid \n"<< m_pid;
         }
   #else
         std::string name(pServiceName);
         PROCESS_INFORMATION pi;
         STARTUPINFO si;
         memset(&si,0,sizeof(si));

         char cmdline[2048];
         strcpy(cmdline,name.c_str());
   #if 0
         cerr<<"Lauching " <<cmdline<< endl;
         if( CreateProcess(NULL,
                           cmdline,
                           NULL,
                           NULL,
                           FALSE, 
                           CREATE_NEW_CONSOLE,
                           NULL,
                           NULL,
                           &si,
                           &pi) != true){
            cerr<<"failed to start process\n" << GetLastError();
            return NULL;
         }
   #endif       
   #endif
      }// Service name not provided

      I* pService = new I(container,new T, new M, new U);

      // Initialize the service
      IBase * ptr = pService->_init(eventHandler,context,rtid,optArgs);
      if( NULL == ptr ){
         delete pService;
         // Kill process
         return NULL;
      }
      return ptr;
   }
};

/// @}

END_NAMESPACE(AAL)


#endif // __AALSDK_AAS_AALIPCSERVICEFACTORY_H__

