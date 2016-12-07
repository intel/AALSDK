
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
   ///
   /// @test         When a test that performs the functions of
   ///               fpgadiag/NLB0 is run and passes, it demonstrates
   ///               that DMA buffer allocation and mapping is working
   ///               correctly.
   ///
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

TEST_P( dma_buffer_f, sw_dma_02 )
{
   ///
   /// @test         Validate that the requested amount of memory is
   ///               returned (by writing to it without faulting), or a
   ///               failure to allocate is returned.  The expectation
   ///               is that in Linux, the allocation failure range will
   ///               be for requests over the biult-in limit as
   ///               expressed in /proc/buddyinfo.
   ///
   /// @details      Use the ALIBufferService->bufferAllocate(reqSize,
   ///               $virtAddress) routine to allocate a buffer and
   ///               m_pALIBufferService->bufferFree(m_DSMVirt) to
   ///               release the buffer.
   ///
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

TEST( DmaBuffer, sw_dma_03 )
{
   ///
   /// @test         Validate that the buffer is freed upon bufferFree.
   ///               Requires a back-door query of the kernel memory
   ///               buffer/free operation.
   ///
   /// @brief        Call SUCCEED() for now, which is a gtest no-op
   ///               reminder.
   ///
   SUCCEED();
}

TEST( DmaBuffer, sw_dma_04 )
{
   ///
   /// @test         Test bufferGetIOVA basic operation by running any
   ///               test that performs the functions of fpgadiag/NLBx
   ///               program, as they all allocate buffers and pass the
   ///               IOVA to the hardware, which uses the IOVA to access
   ///               the buffer.
   ///
   /// @internal     This appears to be a duplicate of test 01 because
   /// they both ultimately call fpgadiag with the same commandline
   /// options. The difference is in the descriptions and pass creiteria
   /// only, not the code as far as I can tell.
   ///
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

TEST_F( dma_buffer_f, sw_dma_05 )
{
///
/// @test         Verify that the IOVA operation returns the correct
///               value for known buffers.  E.g. allocate a buffer B of
///               length N mapped at user virtual address of V and given
///               IOVA(V) is P.  Get various IOVA values and ensure that
///               they are what is expected. Specifically: P=IOVA(V),
///               then P+1=IOVA(V+1); P+L-1=IOVA(V+L-1); 0=IOVA(V+L)
///               [indicating failure]
///
/// @todo This test might be a good candidate for refactoring with the
/// mmio_buffer class.  I'm not familiar enough with the class or ALI,
/// so I continued to use raw pointers as per the original sample for
/// buffer allocations for now.

// comandline from the original sample
// ./DMA_Buffer_5 --bus=$0x5e
// ::testing::FLAGS_gtest_death_test_style = "threadsafe";
#define LPBK1_DSM_SIZE MB( 4 )
#define LPBK1_BUFFER_SIZE MB( 2 )
   typedef long unsigned int btPhysAddr_t;

   btWSSize offset, new_offset;

   auto afu = get_service<afu_client>( "NLB0" );
   auto result = afu->reset();
   ASSERT_EQ( result, IALIReset::e_OK ) << "Could not reset AFU";

   auto ali = afu->get_interface<IALIBuffer>( iidALI_BUFF_Service );

   btVirtAddr dsmVirt;

   // Device Status Memory (DSM) is a structure defined by the NLB implementation.
   // User Virtual address of the pointer is returned directly in the function
   ASSERT_TRUE( ali_errnumOK == ali->bufferAllocate( LPBK1_DSM_SIZE, &dsmVirt ) );

   // Save the size and get the IOVA from the User Virtual address. The HW only uses IOVA.
   auto dsmSize = LPBK1_DSM_SIZE;
   btPhysAddr dsmPhys;
   dsmPhys = ali->bufferGetIOVA( dsmVirt );
   ASSERT_TRUE( 0 != dsmPhys );

   // Repeat for the Input and Output Buffers
   btVirtAddr inputVirt;
   ASSERT_TRUE( ali_errnumOK == ali->bufferAllocate( LPBK1_BUFFER_SIZE, &inputVirt ) );

   auto inputSize = LPBK1_BUFFER_SIZE;
   btPhysAddr inputPhys;
   inputPhys = ali->bufferGetIOVA( inputVirt );
   ASSERT_TRUE( 0 != inputPhys );

   btVirtAddr outputVirt;
   ASSERT_TRUE( ali_errnumOK == ali->bufferAllocate( LPBK1_BUFFER_SIZE, &outputVirt ) );

   auto outputSize = LPBK1_BUFFER_SIZE;
   btPhysAddr outputPhys;
   outputPhys = ali->bufferGetIOVA( outputVirt );
   ASSERT_TRUE( 0 != outputPhys );

   // Clear the DSM
   ::memset( dsmVirt, 0, dsmSize );

   // Initialize the source and destination buffers
   ::memset( inputVirt, 0xAF, inputSize );     // Input initialized to AFter
   ::memset( outputVirt, 0xBE, outputSize );   // Output initialized to BEfore

   struct CacheLine
   {   // Operate on cache lines
      btUnsigned32bitInt uint[16];
   };

   struct CacheLine* pCL = reinterpret_cast<struct CacheLine*>( inputVirt );

   for ( auto i = 0; i < inputSize / CL( 1 ); ++i ) {
      pCL[i].uint[15] = 0xA;
   }   // Cache-Line[n] is zero except last uint = n

   // // Check IOVA functionality
   offset
      = ( ali->bufferGetIOVA( inputVirt ) - reinterpret_cast<btPhysAddr_t>( &pCL[0].uint[15] ) );

   for ( auto i = 0; i < inputSize / CL( 1 ); ++i ) {
      auto new_offset = ali->bufferGetIOVA( inputVirt + i * CL( 1 ) ) - reinterpret_cast
                        <btPhysAddr_t>( &pCL[i].uint[15] );
      ASSERT_TRUE( offset == new_offset ) << "*** Continuous/linear virtual address mapping does "
                                             "not match at cache line #: " << i << endl;
   }

   ASSERT_TRUE( ali_errnumOK == ali->bufferFree( dsmVirt ) );
   ASSERT_TRUE( ali_errnumOK == ali->bufferFree( inputVirt ) );
   ASSERT_TRUE( ali_errnumOK == ali->bufferFree( outputVirt ) );
}

INSTANTIATE_TEST_CASE_P( DmaBuffer,
                         dma_buffer_f,
                         ::testing::Combine( ::testing::Range( 1, 6 ),   // size (in MB)
                                             ::testing::Range( 1, 6 )    // duration (in seconds)
                                             ) );
