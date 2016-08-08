//****************************************************************************
#include <iostream>
#include <string>
#include <appbase.h>
#include <appconstants.h>

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

///             - Allocates the necessary buffers to be used by the Reset AFU algorithm
///             - Executes the Reset algorithm
///             - Cleans up.

btInt HelloALIResetApp::run()
{
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
    btInt use_fpga = 0;
    btInt opt;
    string afu_id = "C000C966-0D82-4272-9AEF-FE5F84570612";

    while ((opt = getopt(argc,argv,"c:d")) != EOF)
        switch(opt)
        {
        case 'c': use_fpga = 1; cout <<" cci_flag is enabled."<<endl; break;
        case '?': cerr<<"Usage is \n -c [value]: enables default AFU ID "<<endl;
        default: cout<<endl; abort(); }

    // Use proper AFU ID if FPGA board is available
    if(use_fpga) afu_id = "A944F6E7-15D3-4D95-9452-15DBD47C76BD";

    HelloALIResetApp theApp("HW",
                            afu_id.c_str(),
                            "Reset Test");
    theApp.setRequestBuffers(false);

    if(!theApp.isOK()){
        appERR("Runtime Failed to Start");
        exit(1);
    }
    btInt Result = theApp.run();

    appMSG("Done");
    return Result;
}
