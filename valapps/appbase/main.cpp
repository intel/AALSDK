//****************************************************************************
#include <iostream>
#include <string>
#include "appbase.h"
#include "appconstants.h"

class HelloALIResetApp: public appbase
{
public:

    HelloALIResetApp(const string& appType, const string& AFUId, const string& serviceName);
    ~HelloALIResetApp();

    // App-specific methods
    btInt run();    ///< Return 0 if success

};

/**
 * Class constructor
 *
 * @param appType Determines application type, valid options are "HW", "ASE"
 * @param AFUId Specifies the AFU identifier
 * @param serviceName
 * @param requestBuffers
 */
HelloALIResetApp::HelloALIResetApp(const string& appType,
                                   const string& AFUId,
                                   const string& serviceName) : appbase(appType, AFUId, serviceName)
{

    // Place additional logic here

}

HelloALIResetApp::~HelloALIResetApp()
{

    // Place additional logic here

}

/// - Allocates the necessary buffers to be used by the Reset AFU algorithm
/// - Executes the Reset algorithm
/// - Cleans up.

btInt HelloALIResetApp::run()
{

    btUnsigned32bitInt x;

    // Allocate the service
    requestService();

    // Allocate the buffer
    requestBuffers();

    // We have the service and the DSM buffer
    appMSG("Starting Test...");
    if((m_Status == APP_IS_OK) && (true == m_bIsOK)) {

        // Original code puts DSM Reset prior to AFU Reset, but ccipTest
        // reverses that. We are following ccipTest here.

        // Initiate AFU Reset
        m_pALIResetService->afuReset();

        x = 0x00001000;
        printf("Write x = 0x%08x\n", x);
        m_pALIMMIOService->mmioWrite32(0x104, (btUnsigned32bitInt)x);
        SleepMicro(1000);

        printf("Count up.\n");
        m_pALIMMIOService->mmioWrite32(0x100, (btUnsigned32bitInt)1);
        SleepMicro(3000);

        x = 0;
        m_pALIMMIOService->mmioRead32(0x104, &x);
        printf("Read x = 0x%08x\n", x);

        printf("Count down.\n");
        m_pALIMMIOService->mmioWrite32(0x100, (btUnsigned32bitInt)2);
        SleepMicro(3000);

        m_pALIMMIOService->mmioRead32(0x104, &x);
        printf("Read x = 0x%08x\n", x);
        SleepMicro(3000);

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
    // pAALLogger()->AddToMask(LM_All, LOG_DEBUG);
    HelloALIResetApp theApp("HW",
                            "A944F6E7-15D3-4D95-9452-15DBD47C76BD",
                            "Service");

    if(!theApp.isOK()){
        appERR("Runtime Failed to Start");
        exit(1);
    }
    btInt Result = theApp.run();

    appMSG("Done");
    return Result;

}
