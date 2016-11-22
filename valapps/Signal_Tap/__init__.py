from valapps import disabled
import os

NumSocket = os.getenv('NumSocket')

def test_SW_STAP_01(self):
  '''
  Obtain the signal tap resource of a port and verify the signature CSR build into the Signal
  Tap hardware.  This proves access to the HW and mapping to user space of the correct location
  '''
  command = "./SW_STAP_01 " + NumSocket
      
  print command

  self.exe(command)

@disabled
def test_SW_STAP_02(self):
    self.exe("../../aalsamples/MMLink/SW/mmlink")
