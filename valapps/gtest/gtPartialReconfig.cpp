#include "afu_test.h"
#include <iostream>
#include "gtest/gtest.h"
#include "reconfigure_client.h"
#include "process.h"
#include "nlb_client.h"

using namespace std;
using action = reconfigure_client::action_t;

class reconfigure_f : public test_context, public ::testing::TestWithParam<int>
{
    protected:
        reconfigure_f()
        {
            cerr << "**** reconfigure_f" << endl;
        }

        virtual ~reconfigure_f()
        {
        }

        virtual void SetUp()
        {
            args()("bitstream1", 'a')
                  ("bitstream2", 'b').parse();
        }

        virtual void TearDown()
        {
        }

        reconfigure_client::ptr_t pr_;
};

TEST_F(reconfigure_f, sw_pr_01a)
{
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");
    auto bs2 = args().get_string("bitstream2");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "reconfigure not successful";

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = this->get_service<nlb_client>("NLB0");
    auto lb_success = nlb->loopback1(4, 2);
    ASSERT_TRUE(lb_success);
    nlb->release();
    auto afu_status = nlb->status();
    ASSERT_EQ(afu_status, service_client::status_t::released) << "AFU not released";

    status = pr->reconfigure(bs2, 1000, action::honor_owner);
    cout << "reconfigure result is :" << status << std::endl;
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success);

    // release afu to allow fpgadiag to use it
    nlb->release();
    auto p = utils::process::start("fpgadiag", {"--mode=read"});
    p.wait();

    p = utils::process::start("fpgadiag", {"--mode=write"});
    p.wait();

    status = pr->reconfigure(bs1, 1000, action::honor_owner);
    cout << "reconfigure result is :" << status << std::endl;
    ASSERT_EQ(status ,reconfigure_client::status_t::reconfigure_success);
}


TEST_F(reconfigure_f, sw_pr_02)
{
    // SW-PR-02 Partial Reconfiguration IALIReconfigure
    // Deactivate a PR slot that is already free and verify that
    // deactivate returns success.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    status = pr->deactivate(1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::deactivate_success) << "error deactivating afu";
}

TEST_F(reconfigure_f, sw_pr_03)
{
    // SW-PR-03 Partial Reconfiguration IALIReconfigure
    // Deactivate a PR slot that has an AFU but is not owned, and verify that
    // deactivate returns success.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    status = pr->deactivate(1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::deactivate_success) << "error deactivating afu";
}

TEST_F(reconfigure_f, sw_pr_04a)
{
    //SW-PR-04a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_OWNER flag, and verify that the owning process is notified of the request,
    // and upon releasing the AFU, the Deactivate succeeds.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = get_service<nlb_client>("NLB0");
    nlb->set_releasable(true);
    auto fv = nlb->loopback1_async(4, 2);

    status = pr->deactivate(1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::deactivate_success) << "error deactivating afu";

    // wait for the loopback to finish, then get result
    fv.wait();
    ASSERT_NE(fv.get(), true) << "Loopback was successful";
}

TEST_F(reconfigure_f, sw_pr_05a)
{
    // SW-PR-05a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_OWNER flag, and verify that the owning process is notified of the request,
    // and upon NOT releasing the AFU, the Deactivate fails.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(4, 2);

    status = pr->deactivate(10, action::honor_owner);
    ASSERT_NE(status, reconfigure_client::status_t::deactivate_success) << "no error deactivating afu";

    fv.wait();
    ASSERT_EQ(fv.get(), true) << "Loopback was not successful";

    nlb->release();
    auto afu_status = nlb->status();
    ASSERT_EQ(afu_status, service_client::status_t::released);
}

TEST_F(reconfigure_f, sw_pr_06a)
{
    // SW-PR-06a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_REQUESTER flag, and verify that the owning process is notified of the request,
    // and upon releasing the AFU, the Deactivate succeeds.   Medium   open  Alpha
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = get_service<nlb_client>("NLB0");
    nlb->set_releasable(true);
    auto fv = nlb->loopback1_async(4, 2);

    status = pr->deactivate(1000, action::honor_request); // do not honor honor - honor requester
    ASSERT_EQ(status, reconfigure_client::status_t::deactivate_success) << "error deactivating afu";

    fv.wait();
    ASSERT_NE(fv.get(), true) << "Loopback was successful";
}

TEST_F(reconfigure_f, sw_pr_07a)
{
    // SW-PR-07a   Partial Reconfiguration IALIReconfigure
    // Within one application process, deactivate a PR slot that has an AFU that is owned,
    // providing HONOR_REQUESTER flag, and verify that the owning process is notified of the request,
    // and upon NOT releasing the AFU, the Deactivate succeeds anyway.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(4, 2);

    status = pr->deactivate(10, action::honor_request); // do not honor owner - honor requester
    ASSERT_EQ(status, reconfigure_client::status_t::deactivate_success) << "no error deactivating afu";

    fv.wait();
    //ASSERT_NE(fv.get(), true) << "Loopback was successful";
}

TEST_F(reconfigure_f, sw_pr_08)
{
    // SW-PR-08  Partial Reconfiguration IALIReconfigure
    // Attempt to Reconfigure and provide an invalid filename for the green bitstream,
    // and verify that the download fails.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto status = pr->reconfigure("/not/a/good/path/f.rbf", 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_error) << "no error reconfiguring bitstream with invalid filename";
}

TEST_F(reconfigure_f, sw_pr_09)
{
    // SW-PR-09 Partial Reconfiguration IALIReconfigure
    // Activate a PR slot that is free, and verify that the Activate is unsuccessful,
    // but does not prevent subsequent Deactivate and Reconfigure.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    status = pr->deactivate(5, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::deactivate_success) << "error deactivating afu";

    status = pr->deactivate(5, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::activate_error) << "no error activating afu";

    status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";
}

TEST_F(reconfigure_f, sw_pr_10)
{
    // SW-PR-10 Partial Reconfiguration IALIReconfigure
    // Activate a PR slot that has been successfully Reconfigured,
    // and verify that the Activate is successful.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    status = pr->deactivate(1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::activate_success) << "error activating afu";
}

TEST_P(reconfigure_f, sw_pr_11)
{
    // SW-PR-11 Partial Reconfiguration IALIReconfigure stress1
    // For an AFU that is owned, send 10 reconfDeactivate() calls with the HONOR_OWNER flag
    // and AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests upon receiving
    // serviceReleaseRequest().  Verify that ali_errnumDeviceBusy is returned by each reconfDeactivate().
    // Verify that 10 deactivateFailed() notifications are received by the client.
    // On the 11th call, change the behavior such that serviceReleaseRequest() releases the AFU.
    // Verify that ali_errnumOK is returned by reconfDeactivate().
    // Verify that deactivateSucceeded() is received by the client.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");
    int iterations = GetParam();

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    auto nlb = get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(4, 2);

    status = pr->deactivate(5, action::honor_owner); // honor owner
    ASSERT_NE(status, reconfigure_client::status_t::deactivate_success) << "no error deactivating afu";

    for ( int i = 0; i < iterations; ++i)
    {
        status = pr->deactivate(100, action::honor_owner); // honor owner
        ASSERT_NE(status, reconfigure_client::status_t::deactivate_success) << "Deactivate successful when it shouldn't be";
    }

    nlb->set_releasable(true);
    status = pr->deactivate(5, action::honor_owner); // honor owner
    ASSERT_EQ(status, reconfigure_client::status_t::deactivate_success) << "Deactivate NOT successful when it should be";

    fv.wait();
    //ASSERT_NE(fv.get(), true) << "Loopback was successful";
}

TEST_P(reconfigure_f, sw_pr_12)
{
    // SW-PR-12 Partial Reconfiguration IALIReconfigure stress2
    // For an AFU that is owned, send 10 reconfConfigure() calls with the HONOR_OWNER flag
    // and AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests
    // upon receiving serviceReleaseRequest().  Verify that ali_errnumDeviceBusy is
    // returned by each reconfConfigure(). Verify that 10 configureFailed() notifications
    // are received by the client.  On the 11th call, change the behavior such that
    // serviceReleaseRequest() releases the AFU.  Verify that ali_errnumOK is
    // returned by reconfConfigure().  Verify that configureSucceeded() is received by the client.
    auto pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    auto bs1 = args().get_string("bitstream1");
    int iterations = GetParam();

    auto status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "error reconfiguring bitstream";

    // excercise the AFU via the nlb_client::loopback1 function
    auto nlb = get_service<nlb_client>("NLB0");
    nlb->set_releasable(false);
    auto fv = nlb->loopback1_async(4, 2);

    status = pr->deactivate(5, action::honor_owner); // honor owner
    ASSERT_NE(status, reconfigure_client::status_t::deactivate_success) << "no error deactivating afu";

    for ( int i = 0; i < iterations; ++i)
    {
        status = pr->reconfigure(bs1, 100, action::honor_owner);
        ASSERT_NE(status, reconfigure_client::status_t::reconfigure_success) << "Reconfigure successful when it shouldn't be";
    }

    nlb->set_releasable(true);
    status = pr->reconfigure(bs1, 100, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "Reconfigure NOT successful when it should be";

    fv.wait();
    //ASSERT_NE(fv.get(), true) << "Loopback was successful";
}

INSTANTIATE_TEST_CASE_P(basic_stress,
        reconfigure_f,
        ::testing::Values(10));
