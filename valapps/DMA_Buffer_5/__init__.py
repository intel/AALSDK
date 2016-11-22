from valapps import params
import itertools
import os

NumSocket = os.getenv('NumSocket')

def test_SW_BUF_05(self):
    '''
    SW-BUF-05
    Verify that the IOVA operation retunrs that correct value for known buffers.    
    '''
    command = "./SW_BUF_05 " + NumSocket    
    print command
    self.exe(command)


