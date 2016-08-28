//****************************************************************************
#include <iostream>
#include <string>
#include <appbase.h>
#include <appconstants.h>

class HelloALIBufferApp: public appbase
{
public:

    HelloALIBufferApp(const string& appType, const string& AFUId, const string& serviceName);
    ~HelloALIBufferApp();

    // App-specific methods
    btInt run();    ///< Return 0 if success

    void setBufferSize(btUnsigned32bitInt bufferSize);

    void setBufferDelay(btUnsigned16bitInt bufferDelay);

private:
    btUnsigned32bitInt m_BufferSize;
    btUnsigned16bitInt m_BufferDelay;
};

/**
 * Class constructor
 *
 * @param appType Determines application type, valid options are "HW", "ASE"
 * @param AFUId Specifies the AFU identifier
 * @param serviceName
 * @param requestBuffers
 */
HelloALIBufferApp::HelloALIBufferApp(const string& appType,
                                     const string& AFUId,
                                     const string& serviceName) : appbase(appType, AFUId, serviceName)
{

    // Place additional logic here
    m_BufferSize = 1;
    m_BufferDelay = 2;

}

HelloALIBufferApp::~HelloALIBufferApp()
{

    // Place additional logic here

}

void HelloALIBufferApp::setBufferSize(btUnsigned32bitInt bufferSize)
{

    m_BufferSize = bufferSize;

}

void HelloALIBufferApp::setBufferDelay(btUnsigned16bitInt bufferDelay)
{

    m_BufferDelay = bufferDelay;

}

/// - Allocates the necessary buffers to be used by the Reset AFU algorithm
/// - Executes the Reset algorithm
/// - Cleans up.

btInt HelloALIBufferApp::run()
{

    // DMA Buffer support   Over-all    Enno    When fpgadiag/NLB0 is run and passes, it demonstrates that DMA buffer allocation and mapping is working correctly.  26
    // IALIBuffer   Enno    Validate that the requested amount of memory is returned (by writing to it without faulting), or a failure to allocate is returned. The expectation is that in Linux the allocation failure range will be for requests over the built-in limit as expressed in /proc/buddyinfo. 26
    // IALIBuffer   Enno    Validate that the buffer is freed upon bufferFree. Requires a backdoor query of the kernel memory buffer/free operation.    26
    // IALIBuffer   Enno    Test bufferGetIOVA basic operation by running any fpgadiag/NLBx program, as they all allocate buffers and pass the IOVA to the hardware, which uses the IOVA to access the buffer.  26
    // IALIBuffer   Enno    Verify that the IOVA operation returns the correct value for known buffers. E.g. allocate a buffer B of length N mapped at user virtual address of V and given IOVA(V) is P. Get various IOVA values and ensure that they are what is expected. Specifically: P=IOVA(V), then P+1=IOVA(V+1); P+L-1=IOVA(V+L-1); 0=IOVA(V+L) [indicating failure]    26

    // Request the service
    requestService();

    // We have the service and the DSM buffer
    appMSG("Starting Test...");
    if((m_Status == APP_IS_OK) && (true == m_bIsOK)) {

        // Original code puts DSM Reset prior to AFU Reset, but ccipTest
        // reverses that. We are following ccipTest here.

        // Initiate AFU Reset
        m_pALIResetService->afuReset();

        // Request the buffers
        appMSG("Requesting buffers...");
        requestBuffers(appMB(m_BufferSize),
                       0,
                       0);
        cout<<"*** Current buffer size is "<<m_BufferSize<<" MB"<<endl;

        // Operate on cache lines
        struct CacheLine {
            btUnsigned32bitInt uint[16];
        };

        // Cache-Line[n] is zero except last uint = n
        cout<<"*** Writing to buffer..."<<endl;
        struct CacheLine *pCL = reinterpret_cast<struct CacheLine *>(m_InputVirt);
        for (btUnsigned32bitInt i = 0; i < m_InputSize / CL(1) ; ++i ) {
            pCL[i].uint[15] = i;
        };

        // Hold buffer for extra timing
        cout<<"*** Holding buffer for "<<m_BufferDelay<<" seconds."<<endl;
        SleepMilli(1000 * m_BufferDelay);

        // Release the buffers
        appMSG("Releasing buffers...");
        releaseBuffers();

    }
    appMSG("Done Running Test");

    // Exit
    requestExit();

    // Clean-up and return
    return m_Result;
}

/**
 * Main function implementing the application
 *
 * @param argc
 * @param argv
 *
 * @return
 */
int main(int argc, char *argv[])
{
    btInt opt;
    btUnsigned32bitInt bufferSize = 1;
    btUnsigned16bitInt bufferDelay = 2;

    while ((opt = getopt(argc,argv,"s:d:")) != EOF)
        switch(opt)
        {
        case 's': bufferSize = (btUnsigned32bitInt) atoi(optarg); cout <<" Passing buffer size."<<endl; break;
        case 'd': bufferDelay = (btUnsigned16bitInt) atoi(optarg); cout <<" Passing buffer delay."<<endl; break;
        case '?': cerr<<"Usage is \n -s [value]: defines buffer size on MB. \n -d [value]: defines buffer delay in seconds. \n"<<endl;
        default: cout<<endl; abort(); }

    HelloALIBufferApp theApp("HW",
                             "D8424DC4-A4A3-C413-F89E-433683F9040B",
                             "Buffer Test");
    theApp.setRequestBuffers(true);
    theApp.setBufferSize(bufferSize);
    theApp.setBufferDelay(bufferDelay);

    if(!theApp.isOK()){
        appERR("Runtime Failed to Start");
        exit(1);
    }
    btInt Result = theApp.run();

    appMSG("Done");
    return Result;
}
