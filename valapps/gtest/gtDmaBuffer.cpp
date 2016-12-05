
#include "afu_test.h"
#include <iostream>
#include "gtest/gtest.h"

using namespace std;
using namespace AAL;

class dma_buffer_f : public test_context, public ::testing::TestWithParam<tuple<int, int>>
{
protected:
   dma_buffer_f()
   {
   }

   virtual ~dma_buffer_f()
   {
   }

   virtual void SetUp()
   {
   }

   virtual void TearDown()
   {
   }
};

TEST( DmaBuffer, sw_dma_01 )
{
   pid_t pid;
   int status;

   pid = fork();
   if ( pid == 0 ) {
      // comandline from the original sample
      //./fpgadiag --mode=lpbk1 --target=fpga --begin=65535 --read-vc=vh0 --write-vc=vh0
      // ::testing::FLAGS_gtest_death_test_style = "threadsafe";
      int retval = execl( "./.packages/bin/fpgadiag",
                          "fpgadiag",
                          "--mode=lpbk1",
                          "--target=fpga",
                          "--begin=65535",
                          "--read-vc=vh0",
                          "--write-vc=vh0" );
      exit( retval );
   } else {
      wait( &status );
      EXPECT_EQ( EXIT_SUCCESS, status );
   }
}

TEST_P( dma_buffer_f, allocate_hold )
{
   tuple<int, int> values = GetParam();
   auto afu = get_service<afu_client>( "NLB0" );

   auto result = afu->reset();

   ASSERT_EQ( result, IALIReset::e_OK ) << "Could not reset AFU";

   auto size = get<0>( values ) * afu_test::constants::MB;
   auto duration = get<1>( values );

   if ( size < 5 * afu_test::constants::MB ) {
      mmio_buffer::ptr_t buffer;
      ASSERT_NO_THROW( buffer = afu->allocate_buffer( size ) ) << "buffer allocation failed"
                                                               << endl;
      ASSERT_NE( buffer.get(), nullptr );

      uint8_t* cl = buffer->address();
      for ( btUnsigned32bitInt i = 0; i < size / CL( 1 ); ++i ) {
         cl[i + 15] = i;
      }
      this_thread::sleep_for( chrono::seconds( duration ) );
      buffer->release();
   } else {
      mmio_buffer::ptr_t buffer;
      ASSERT_THROW( buffer = afu->allocate_buffer( size ), buffer_allocate_error )
         << "buffer allocate successful when expecting to throw and error" << endl;
   }
}

INSTANTIATE_TEST_CASE_P( sw_dma_02,
                         dma_buffer_f,
                         ::testing::Combine( ::testing::Range( 1, 6 ),   // size (in MB)
                                             ::testing::Range( 1, 6 )    // duration (in seconds)
                                             ) );
