from valapps import params
import itertools
import os

NumSocket = os.getenv('NumSocket')

def test_SW_BUF_01(self):
    '''
    SW-BUF-01-04
    This test perform the functions of fpgdiag/NLB0 is running and passes, it demonstrates
    that DMA buffer allocation and mapping is working correcly. It also test bufferGetIOVA 
    basic operation by running any test that performs the functions of fpgadiag/NLB0 program,
    as they all allocate buffers and pass the IOVA to the hardware, which uses the IOVA to 
    access the buffer    
    '''
    #NumSocket = os.getenv('NumSocket')
    command = "./SW_BUF_01_04 " + NumSocket    
    print command
    self.exe(command)

@params(range(1,6), 2) 
def test_SW_BUF_02(self, size, duration):
    '''
    SW-BUF-02
    This test allocates a given size (in MB) and holds it for a given duration (in seconds).
    '''
    #NumSocket = os.getenv('NumSocket')

    print("SW_BUF_02 socket:{} size:{}, duration: {}".format(NumSocket, size, duration))
    command = "./SW_BUF_02 {} {} {}".format(NumSocket, size, duration)

    self.assertEqual(0, self.exit_code(command))
    
def test_SW_BUF_04(self):
    '''
    SW-BUF-01-04
    This test bufferGetIOVA basic operation by running any test that performs the functions
    of NLB0 programs, as they all allocated buffers and pass the IOVA to the hardware,
    which uses the IOVA to access the buffer    
    '''
    NumSocket = os.getenv('NumSocket')

    command = "./SW_BUF_01_04 " + NumSocket    
    print command
    self.exe(command)
