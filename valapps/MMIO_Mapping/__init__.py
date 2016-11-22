from valapps import disabled
import os

NumSocket = os.getenv('NumSocket')
SystemConfig = os.getenv('SystemConfig')


def test_SW_MMIO_01(self):
    '''
    SW_MMIO_01
    
    RTL: NLB mode0 (need to modify AFU ID to match the loaded AFU image)
    Option1:  ./fpgadiag __mode=lpbk1 __begin=1
    Option2:  ./MMIO_Mapping --test=1 --bus=<pci device bus number> 
    PASS: test pass without error & app return 0
    FAIL: error & app return value is not 0
    '''
    if SystemConfig == 'native':
    	command = "./SW_LOAD_AFU " + NumSocket + " 0 " 
    	print command
    	self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 0 "    
    print command
    self.exe(command)


def test_SW_MMIO_02a(self):
    '''
    SW_MMIO_02a
    
    Allocate a test AFU that expose the entire AFU MMIO region, use mmio32 read/write 
    api to test mmio access through the entire region.

    PASS: test pass without error & app return 0
    FAIL: error & app return value is not 0
    '''
    if SystemConfig == 'native':
    	command = "./SW_LOAD_AFU " + NumSocket + " 1 "  
    	print command
    	self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 1 "    
    print command
    self.exe(command)


def test_SW_MMIO_02b(self):
    '''
    SW_MMIO_02b
    Allocate a test AFU that expose the entire AFU MMIO region, use mmio64 read/write 
    api to test mmio access through the entire region.
  
    PASS: test pass without error & app return 0
    FAIL: error & app return value is not 0
    '''
    if SystemConfig == 'native':
    	command = "./SW_LOAD_AFU " + NumSocket + " 2 "
    	print command
    	self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 2 "    
    print command
    self.exe(command)


def test_SW_MMIO_03(self):
    '''
    SW_MMIO_03
    
    Allocate a test AFU that expose the entire AFU MMIO region, 
    get the MMIO address and length, and then attempt to write beyound
    the length.  Should not able to write to the memory area when address is 
    outside of the AFU MMIO region.
    Option1:  ./MMIO_Mapping 3

    PASS: test pass without error & app return 0
    FAIL: error & app return value is not 0
    '''
    if SystemConfig == 'native':
    	command = "./SW_LOAD_AFU " + NumSocket + " 3 "  
    	print command
    	self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 3 "    
    print command
    self.exe(command)


def test_SW_MMIO_04(self):
    '''
    SW_MMIO_04
    
    Verify the returned length of the MMIO space matched the size supported by HW

    PASS: test pass without error & app return 0
    FAIL: error & app return value is not 0
    '''
    command = "./SW_MMIO_MAPPING " + NumSocket + " 4 "    
    print command
    self.exe(command)


def test_SW_MMIO_05a(self):
    '''
    SW_MMIO_05a
    
    Allocate a test AFU that expose specific DFH header information to verify the correct
    discovery of DFH features using mmioGetFeatureOffset() and friends.

    PASS: test pass without error & app return 0
    FAIL: error & app return value is not 0
    '''
    if SystemConfig == 'native':
       command = "./SW_LOAD_AFU " + NumSocket + " 5 "  
       print command
       self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 98 "    
    print command
    self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 5 "    
    print command
    self.exe(command)


def test_SW_MMIO_05b(self):
    '''
    SW_MMIO_05b
    
    Allocate a test AFU that expose specific DFH header information to verify not able to
    discover DFH features using mmioGetFeatureOffset() and friends.

    PASS: test pass without error & app return 0
    FAIL: error & app return value is not 0
    '''
    if SystemConfig == 'native':
    	command = "./SW_LOAD_AFU " + NumSocket + " 6 "   
    	print command
    	self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 99 "    
    print command
    self.exe(command)

    command = "./SW_MMIO_MAPPING " + NumSocket + " 6 "    
    print command
    self.exe(command)

def test_restore_NLB_mode0_AFU(self):
    '''
    reload NLB mode0 AFU
    '''
    if SystemConfig == 'native':
    	command = "./SW_LOAD_AFU " + NumSocket + " 0 "   
    	print command
    	self.exe(command)
    


