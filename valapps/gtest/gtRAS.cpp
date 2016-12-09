#include <gtest/gtest.h>
#include <vector>
#include "test_context.h"
#include "afu_client.h"
#include "nlb_client.h"
#include <aalsdk/service/IALIAFU.h>

using namespace std;
using namespace AAL;

// FME RAS error injection csr offset
#define FME_RAS_ERRINJ_CSROFFSET  0x4078

//0b111
// CatastError
// FatalError
// Warning Error
#define FME_RAS_ERRINJ_VLAUE      0x3

class RAS_f : public test_context , public ::testing::Test
{
    protected:
         RAS_f()
        {
        }

        virtual ~RAS_f()
        {
        }

        virtual void SetUp()
        {
           inject_errors();
        }

        virtual void TearDown()
        {
           clear_injected_errors();
        }

        void inject_errors()
        {
           auto fme = get_service<afu_client>("FME");
           auto fme_status = fme->status();
           ASSERT_EQ(service_client::status_t::allocated, fme_status);
           fme->mmio_write64(FME_RAS_ERRINJ_CSROFFSET, FME_RAS_ERRINJ_VLAUE);

        }
        void clear_injected_errors()
        {
           auto fme = get_service<afu_client>("FME");
           auto fme_status = fme->status();
           ASSERT_EQ(service_client::status_t::allocated, fme_status);
           fme->mmio_write64(FME_RAS_ERRINJ_CSROFFSET, 0x0);

        }
        void sw_fme_error_01()
        {
            // sw_fme_error_01   Verify FME kernel error logs

            find_kerneltraces("FME Error0");

            find_kerneltraces("PCIe0 Error");

            find_kerneltraces("PCIe1 Error");

            find_kerneltraces("FME First Error");

            find_kerneltraces("FME Next Error");

        }

        void sw_ras_error_02()
        {
            // sw_ras_error_02   Verify RAS kernel error logs

            find_kerneltraces("RAS GBS Error");

            find_kerneltraces("RAS BBS Error");

            find_kerneltraces("RAS Warning Error");

            find_kerneltraces("Injected Fatal error");

            find_kerneltraces("Injected Catastrophic error");

        }

        void sw_port_error_03()
        {
            // sw_port_error_03   Verify PORT kernel error logs

            find_kerneltraces("PORT Error");

            find_kerneltraces("PORT First Error");

            find_kerneltraces("PORT Malfromed req");

        }

        void sw_ap_error_04()
        {
            // sw_ap_error_04   Verify AP State kernel error logs

            find_kerneltraces("FPGA Trigger AP6");

            find_kerneltraces("FPGA Trigger AP1");

            find_kerneltraces("FPGA Trigger AP2");
        }

        btInt find_kerneltraces(string search_str)
        {
           btInt res   =  -1;
           FILE *pfile  = NULL;
           pfile = popen("dmesg","r");

           if(NULL != pfile) {

              while(true) {
                 char* line ;
                 char buf[2000];
                 line = fgets(buf,sizeof(buf),pfile);

                 if(NULL == line) break;

                 std::string str(buf);
                 std::size_t found = str.find(search_str);

                 if(found != std::string::npos)  {
                    std::cout << "------Traces:: START -----" << std::endl;
                    std::cout << buf << std::endl ;
                    std::cout << "------Traces:: END --------"<< std::endl;

                    res = 0;
                 }
              }
           }

           pclose(pfile);

           return res;
        }


 };

TEST_F(RAS_f, sw_RAS_01)
{
    SCOPED_TRACE("sw_RAS_01");
    sw_fme_error_01();
}

TEST_F(RAS_f, sw_RAS_02)
{
    SCOPED_TRACE("sw_RAS_02");
    sw_ras_error_02();
}

TEST_F(RAS_f, sw_RAS_03)
{
    SCOPED_TRACE("sw_RAS_03");
    sw_port_error_03();
}

TEST_F(RAS_f, sw_RAS_04)
{
    SCOPED_TRACE("sw_RAS_04");
    sw_ap_error_04();
}
