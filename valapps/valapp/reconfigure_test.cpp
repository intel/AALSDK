#include "reconfigure_test.h"
#include "test_manager.h"
#include <aalsdk/service/IALIAFU.h>
#include "reconfigure_client.h"
#include "nlb_client.h"
#include "process.h"

namespace
{
    bool r = test_manager::register_test<reconfigure_test>();
}

using namespace std;
using action = reconfigure_client::action_t;

void reconfigure_test::register_tests()
{
    register_test("SW-PR-01a", &reconfigure_test::sw_pr_01a)
                 ("bitstream1", 'b')
                 ("bitstream2", 's');
    register_test("SW-PR-02", &reconfigure_test::sw_pr_02)
                 ("bitstream1", 'b');
    register_test("SW-PR-03", &reconfigure_test::sw_pr_03)
                 ("bitstream1", 'b');
    register_test("SW-PR-04a", &reconfigure_test::sw_pr_04a)
                 ("bitstream1", 'b');
    register_test("SW-PR-05a", &reconfigure_test::sw_pr_05a)
                 ("bitstream1", 'b');
    register_test("SW-PR-06a", &reconfigure_test::sw_pr_06a)
                 ("bitstream1", 'b');
    register_test("SW-PR-00", &reconfigure_test::sw_pr_07a)
                 ("bitstream1", 'b');
    register_test("SW-PR-00", &reconfigure_test::sw_pr_08)
                 ("bitstream1", 'b');
    register_test("SW-PR-00", &reconfigure_test::sw_pr_09)
                 ("bitstream1", 'b');
    register_test("SW-PR-00", &reconfigure_test::sw_pr_10)
                 ("bitstream1", 'b');
    register_test("SW-PR-00", &reconfigure_test::sw_pr_11)
                 ("bitstream1", 'b');
    register_test("SW-PR-00", &reconfigure_test::sw_pr_12)
                 ("bitstream1", 'b');

}

void reconfigure_test::setup()
{

}

void reconfigure_test::teardown()
{
}

void reconfigure_test::sw_pr_01a(const arguments &args)
{
    auto bs1 = args.get_string("bitstream1");
    auto bs2 = args.get_string("bitstream2");
    Log() << "reconfigure test" << std::endl;

    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = context()->get_service<nlb_client>("NLB0");
    auto lb_success = nlb->loopback1(2, 4);
    TEST_FAIL(!lb_success, "Loopback was not successful");

    status = pr->reconfigure(bs2, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");
    
    // release afu to allow fpgadiag to use it
    nlb->release();
    auto p = utils::process::start("fpgadiag", {"--mode=read"});
    p.wait();

    p = utils::process::start("fpgadiag", {"--mode=write"});
    p.wait();

    status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");
}

void reconfigure_test::sw_pr_02(const arguments &args)
{
    // SW-PR-02 Partial Reconfiguration IALIReconfigure
    // Deactivate a PR slot that is already free and verify that
    // deactivate returns success.

    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    status = pr->deactivate(1000, action::honor_owner);
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "error deactivating afu");
}

void reconfigure_test::sw_pr_03(const arguments &args)
{
    // SW-PR-04a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_OWNER flag, and verify that the owning process is notified of the request,
    // and upon releasing the AFU, the Deactivate succeeds.

    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");


    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    status = pr->activate(1000, action::honor_owner);
    Log() << "activate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "error activating afu");

    status = pr->deactivate(1000, action::honor_owner);
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "error deactivating afu");
}

void reconfigure_test::sw_pr_04a(const arguments &args)
{
    //SW-PR-04a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_OWNER flag, and verify that the owning process is notified of the request,
    // and upon releasing the AFU, the Deactivate succeeds.
    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");


    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = context()->get_service<nlb_client>("NLB0");
    nlb->set_releasable(true);
    auto fv = nlb->loopback1_async(2, 4);

    status = pr->deactivate(1000, action::honor_owner);
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "error deactivating afu");

    // wait for the loopback to finish, then get result
    fv.wait();
    TEST_FAIL(fv.get() == true, "Loopback was successful");
}

void reconfigure_test::sw_pr_05a(const arguments &args)
{
   // SW-PR-05a   Partial Reconfiguration IALIReconfigure
   // Within one application process, deactivate a PR slot that has an AFU that is owned,
   // providing HONOR_OWNER flag, and verify that the owning process is notified of the request,
   // and upon NOT releasing the AFU, the Deactivate fails.

    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");


    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = context()->get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(2, 4);

    status = pr->deactivate(10, action::honor_owner);
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status == reconfigure_client::status_t::deactivate_success, "no error deactivating afu");

    fv.wait();
    TEST_FAIL(fv.get() == true, "Loopback was successful");
}

void reconfigure_test::sw_pr_06a(const arguments &args)
{

    // SW-PR-06a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_REQUESTER flag, and verify that the owning process is notified of the request,
    // and upon releasing the AFU, the Deactivate succeeds.   Medium   open  Alpha
    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = context()->get_service<nlb_client>("NLB0");
    nlb->set_releasable(true);
    auto fv = nlb->loopback1_async(2, 4);

    status = pr->deactivate(10, action::honor_request); // do not honor honor - honor requester
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "error deactivating afu");

    fv.wait();
    TEST_FAIL(fv.get() == true, "Loopback was successful");
}

void reconfigure_test::sw_pr_07a(const arguments& args)
{
    // SW-PR-07a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_REQUESTER flag, and verify that the owning process is notified of the request,
    // and upon NOT releasing the AFU, the Deactivate succeeds anyway.
    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = context()->get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(2, 4);

    status = pr->deactivate(10, action::honor_request); // do not honor owner - honor requester
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "no error deactivating afu");

    fv.wait();
    //TEST_FAIL(fv.get() == true, "Loopback was successful");
}

void reconfigure_test::sw_pr_08(const arguments& args)
{
    // SW-PR-08  Partial Reconfiguration IALIReconfigure
    // Attempt to Reconfigure and provide an invalid filename for the green bitstream,
    // and verify that the download fails.
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");

    auto status = pr->reconfigure("/not/a/good/path/f.rbf", 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_error, "no error reconfiguring bitstream with invalid filename");
}

void reconfigure_test::sw_pr_09(const arguments& args)
{
    // SW-PR-09 Partial Reconfiguration IALIReconfigure
    // Activate a PR slot that is free, and verify that the Activate is unsuccessful,
    // but does not prevent subsequent Deactivate and Reconfigure.

    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");


    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    status = pr->deactivate(5, action::honor_owner);
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "error deactivating afu");

    status = pr->deactivate(5, action::honor_owner);
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::activate_error, "no error activating afu");

    status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");
}

void reconfigure_test::sw_pr_10(const arguments& args)
{
    // SW-PR-10 Partial Reconfiguration IALIReconfigure
    // Activate a PR slot that has been successfully Reconfigured,
    // and verify that the Activate is successful.
    auto bs1 = args.get_string("bitstream1");
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");


    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    status = pr->deactivate(1000, action::honor_owner);
    Log() << "activate result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::activate_success, "error activating afu");
}

void reconfigure_test::sw_pr_11(const arguments& args)
{
    // SW-PR-11 Partial Reconfiguration IALIReconfigure stress1
    // For an AFU that is owned, send 10 reconfDeactivate() calls with the HONOR_OWNER flag
    // and AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests upon receiving
    // serviceReleaseRequest().  Verify that ali_errnumDeviceBusy is returned by each reconfDeactivate().
    // Verify that 10 deactivateFailed() notifications are received by the client.
    // On the 11th call, change the behavior such that serviceReleaseRequest() releases the AFU.
    // Verify that ali_errnumOK is returned by reconfDeactivate().
    // Verify that deactivateSucceeded() is received by the client.

    auto bs1 = args.get_string("bitstream1");
    auto iterations = args.get_int("iterations", 10);
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");


    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    auto nlb = context()->get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(2, 4);

    status = pr->deactivate(5, action::honor_owner); // honor owner
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status == reconfigure_client::status_t::deactivate_success, "no error deactivating afu");

    for ( int i = 0; i < iterations; ++i)
    {
        Log() << "Iteration: " << i << std::endl;
        status = pr->deactivate(100, action::honor_owner); // honor owner
        TEST_FAIL(status == reconfigure_client::status_t::deactivate_success, "Deactivate successful when it shouldn't be");
    }

    nlb->set_releasable(true);
    status = pr->deactivate(5, action::honor_owner); // honor owner
    TEST_FAIL(status != reconfigure_client::status_t::deactivate_success, "Deactivate NOT successful when it should be");

    fv.wait();
    //TEST_FAIL(fv.get() == true, "Loopback was successful");
}

void reconfigure_test::sw_pr_12(const arguments& args)
{
    // SW-PR-12 Partial Reconfiguration IALIReconfigure stress2
    // For an AFU that is owned, send 10 reconfConfigure() calls with the HONOR_OWNER flag
    // and AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests
    // upon receiving serviceReleaseRequest().  Verify that ali_errnumDeviceBusy is
    // returned by each reconfConfigure(). Verify that 10 configureFailed() notifications
    // are received by the client.  On the 11th call, change the behavior such that
    // serviceReleaseRequest() releases the AFU.  Verify that ali_errnumOK is
    // returned by reconfConfigure().  Verify that configureSucceeded() is received by the client.

    auto bs1 = args.get_string("bitstream1");
    auto iterations = args.get_int("iterations", 10);
    auto pr = context()->get_service<reconfigure_client>("PR");
    TEST_ERROR(!pr, "Could not get PR service");


    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    Log() << "reconfigure result is :" << status << std::endl;
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "error reconfiguring bitstream");

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = context()->get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(2, 4);

    status = pr->deactivate(5, action::honor_owner); // honor owner
    Log() << "deactivate result is :" << status << std::endl;
    TEST_FAIL(status == reconfigure_client::status_t::deactivate_success, "no error deactivating afu");

    for ( int i = 0; i < iterations; ++i)
    {
        Log() << "Iteration: " << i << std::endl;
        status = pr->reconfigure(bs1, 100, action::honor_owner);
        TEST_FAIL(status == reconfigure_client::status_t::reconfigure_success, "Reconfigure successful when it shouldn't be");
    }

    nlb->set_releasable(true);
    status = pr->reconfigure(bs1, 100, action::honor_owner);
    TEST_FAIL(status != reconfigure_client::status_t::reconfigure_success, "Reconfigure NOT successful when it should be");

    fv.wait();
    //TEST_FAIL(fv.get() == true, "Loopback was successful");
}


