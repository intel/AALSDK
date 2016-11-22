from os import environ
import os

_env = {"LD_LIBRARY_PATH":":".join(["../vallib", environ.get("LD_LIBRARY_PATH", "")])}
NumSocket = os.getenv('NumSocket')


def test_SW_PERFC_01(self):
    '''
    SW-PERFC-01
    Sanity check of Performance Counter acquisition when idle 
    '''
    command = "./SW_PERC_01 " + NumSocket + " 1 "
      
    print command

    self.exe(command, env=_env)

def test_SW_PERFC_02(self):
    '''
    SW-PERFC-01
    Sanity check of Performance Counter acquisition when idle 
    '''
    command = "./SW_PERC_01 " + NumSocket + " 2 "
      
    print command

    self.exe(command, env=_env)
 

def test_SW_PERFC_03(self):
    '''
    SW-PERFC-01
    Sanity check of Performance Counter acquisition when idle 
    '''
    command = "./SW_PERC_01 " + NumSocket + " 3 "
      
    print command

    self.exe(command, env=_env)


