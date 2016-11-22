from valapps import disabled
from os import path
import os

NumSocket = os.getenv('NumSocket')

_config = {}

_config['NumSocket']=NumSocket

if path.exists("rtl.json"):
    import json
    with open("rtl.json", "r") as fd:
        rtl = json.load(fd)
    _config["bs1"] = rtl["mode0"][rtl.get("mode0idx",0)]
    _config["bs2"] = rtl["mode3"][rtl.get("mode3idx",0)]


def test_SW_PR_01a(self):
    '''
    sw-pr-01a
    In the same application process, reconfigure a valid green NLB bitstream and run NLB/fpgadiag to demonstrate it works.  Reconfigure a different valid green NLB bitstream and run NLB/fpgadiag to demonstrate that one works, as well
    '''
    _config['TestCaseNum']=1

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_02(self):
    '''
    sw-pr-02
    Deactivate a PR slot that is already free and verify that deactivate return success
    '''
    _config['TestCaseNum']=2

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_03(self):
    '''
    sw-pr-03
    Deactivate a PR slot that has an AFU but is not owned, and verify that deactivate 
    returns success
    '''
    _config['TestCaseNum']=3

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_04(self):
    '''
    sw-pr-04
    Within one application process, deactivate a PR slot that has an AFU that is owned, 
    providing HONOR_OWNER flag, and verify that the owning process is notified of the 
    request, and upon releasing the AFU, the Deactivate succeeds. 
    '''
    _config['TestCaseNum']=4

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_05(self):
    '''
    sw-pr-05
    Within one application process, deactivate a PR slot that has an AFU that is owned, 
    providing HONOR_OWNER flag, and verify that the owning process is notified of the 
    request, and upon NOT releasing the AFU, the Deactivate fails.
    '''
    _config['TestCaseNum']=5

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_06(self):
    '''
    sw-pr-06
    Within one application process, deactivate a PR slot that has an AFU that is owned, 
    providing HONOR_REQUESTER flag, and verify that the owning process is notified of the 
    request, and upon releasing the AFU, the Deactivate succeeds.
    '''
    _config['TestCaseNum']=6

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_07(self):
    '''
    sw-pr-07
    Within one application process, deactivate a PR slot that has an AFU that is owned, 
    providing HONOR_REQUESTER flag, and verify that the owning process is notified of the 
    request, and upon NOT releasing the AFU, the Deactivate succeeds anyway.
    '''
    _config['TestCaseNum']=7

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_08(self):
    '''
    sw-pr-08
    Attemp to Reconfigure and provide an invalid filenmae for the green bitstream, and
    verify that the download fails.
    '''
    _config['TestCaseNum']=8

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_09(self):
    '''
    sw-pr-09
    Active a PR slot that is free, and verify that the Activate is unsuccessful, but 
    does not prevent subsequent Deactivate and Reconfigure. 
    '''
    _config['TestCaseNum']=9

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_10(self):
    '''
    sw-pr-10
    Activate a PR slot that has been successfully Reconfigured,a nd verify that the Activate
    is successful. 
    '''
    _config['TestCaseNum']=10

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_11(self):
    '''
    sw-pr-11
    For an AFU that is owned, send 10 reconfiguration() calls with the HONOR_OWNER flag and 
    AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests upon receiving 
    serviceReleaseRequest(). Verify that ali_errnumDeviceBusy is returned by each 
    reconfDeactivate(). Verify that 10 deactivateFailed() notificiations are received by the
    client.  On the 11th call, change the behavior such that serviceReleaseRequest() releases
    the AFU.  Verify that ali_errnumOK is returned by reconfDeactivate(). Verify that 
    deactivateSucceeded() is received by the client. 
    '''
    _config['TestCaseNum']=11

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_12(self):
    '''
    sw-pr-12
    For an AFU that is owned, send 10 reconfiguration() calls with the HONOR_OWNER flag and 
    AALCONF_MILLI_TIMEOUT set to 100. The AFU denies all 10 requests upon receiving 
    serviceReleaseRequest(). Verify that ali_errnumDeviceBusy is returned by each 
    reconfConfigure(). Verify that 10 configureFailed() notificiations are received by the
    client.  On the 11th call, change the behavior such that serviceReleaseRequest() releases
    the AFU.  Verify that ali_errnumOK is returned by reconfDeactivate(). Verify that 
    deactivateSucceeded() is received by the client. 
    '''
    _config['TestCaseNum']=12

    command = "./SW_PR_SINGLE_APP {NumSocket} {bs1} {bs2} {TestCaseNum}".format(**_config)
      
    print command

    self.exe(command)


def test_SW_PR_01b_4b_6b_7b_5b(self):
    '''
    SW_PR_01b_4b_6b_7b_5b
    PR with 2 Application tests.
    '''

    command = "./SW_PR_TWO_APP {NumSocket} {bs1}".format(**_config)
      
    print command

    self.exe(command)



