from os import environ
import os

NumSocket = os.getenv('NumSocket')

def test_SW_RESET_01(self):
    '''
    SW-RESET-01
    This is a simple reset test. This test runs the AFU_Reset app which will set the appropriate CSR to trigger a reset.
    It will then use dmesg to look for indications that the AFU has indeed restarted
    '''
    command = "./SW_RESET_01 " + NumSocket
      
    print command

    self.exe(command)


