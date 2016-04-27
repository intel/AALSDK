if exist "%3\aalsdk" goto aalsdk_osal
mkdir "%3\aalsdk"
:aalsdk_osal
if exist "%3\aalsdk\osal" goto aalsdk_aas
mkdir "%3\aalsdk\osal"
:aalsdk_aas
if exist "%3\aalsdk\aas" goto aalsdk_rm
mkdir "%3\aalsdk\aas"
:aalsdk_rm
if exist "%3\aalsdk\rm" goto aalsdk_uaia
mkdir "%3\aalsdk\rm"
:aalsdk_uaia
if exist "%3\aalsdk\uaia" goto aalsdk_service
mkdir "%3\aalsdk\uaia"
:aalsdk_service
if exist "%3\aalsdk\service" goto aalsdk_aalclp
mkdir "%3\aalsdk\service"
:aalsdk_aalclp
if exist "%3\aalsdk\aalclp" goto do_subs
mkdir "%3\aalsdk\aalclp"

:do_subs

if exist "%3\aalsdk\osal\OSALService.h" goto AASService_h
%1 %2\osal\OSALService.h.in     %3\aalsdk\osal\OSALService.h     @OSAL_SVC_MOD@       libOSAL

:AASService_h
if exist "%3\aalsdk\aas\AASService.h" goto AALRuntimeModule_h
%1 %2\aas\AASService.h.in       %3\aalsdk\aas\AASService.h       @AAS_SVC_MOD@        libAAS

:AALRuntimeModule_h
if exist "%3\aalsdk\aas\AALRuntimeModule.h" goto ResMgrService_h
%1 %2\aas\AALRuntimeModule.h.in %3\aalsdk\aas\AALRuntimeModule.h @AALRUNTIME_SVC_MOD@ libaalrt

:ResMgrService_h
if exist "%3\aalsdk\rm\ResMgrService.h" goto RRMService_h
%1 %2\rm\ResMgrService.h.in     %3\aalsdk\rm\ResMgrService.h     @RESMGR_SVC_MOD@     libAASResMgr

:RRMService_h
if exist "%3\aalsdk\rm\RRMService.h" goto AIAService_h
%1 %2\rm\RRMService.h.in        %3\aalsdk\rm\RRMService.h        @RRM_SVC_MOD@        librrm

:AIAService_h
if exist "%3\aalsdk\uaia\AIAService.h" goto ASEALIAFUService_h
%1 %2\uaia\AIAService.h.in      %3\aalsdk\uaia\AIAService.h      @AIASERVICE_SVC_MOD@ libaia

:ASEALIAFUService_h
if exist "%3\aalsdk\service\ASEALIAFUService.h" goto HWALIAFUService_h
%1 %2\service\ASEALIAFUService.h.in               %3\aalsdk\service\ASEALIAFUService.h.SVC_MOD   @ASEALIAFU_SVC_MOD@    libASEALIAFU
%1 %3\aalsdk\service\ASEALIAFUService.h.SVC_MOD   %3\aalsdk\service\ASEALIAFUService.h           @ASEALIAFU_SVC_MLEN@   12

:HWALIAFUService_h
if exist "%3\aalsdk\service\HWALIAFUService.h" goto SWSimALIAFUService_h
%1 %2\service\HWALIAFUService.h.in                %3\aalsdk\service\HWALIAFUService.h.SVC_MOD    @HWALIAFU_SVC_MOD@     libHWALIAFU
%1 %3\aalsdk\service\HWALIAFUService.h.SVC_MOD    %3\aalsdk\service\HWALIAFUService.h            @HWALIAFU_SVC_MLEN@    11

:SWSimALIAFUService_h
if exist "%3\aalsdk\service\SWSimALIAFUService.h" goto aalclp_h
%1 %2\service\SWSimALIAFUService.h.in             %3\aalsdk\service\SWSimALIAFUService.h.SVC_MOD @SWSIMALIAFU_SVC_MOD@  libSWSimALIAFU
%1 %3\aalsdk\service\SWSimALIAFUService.h.SVC_MOD %3\aalsdk\service\SWSimALIAFUService.h         @SWSIMALIAFU_SVC_MLEN@ 14

:aalclp_h
if exist "%3\aalsdk\aalclp\aalclp.h" goto done
%1 %2\aalclp\aalclp.h.in                       %3\aalsdk\aalclp\aalclp.h.PACKAGE           @PACKAGE@           "aalsdk"
%1 %3\aalsdk\aalclp\aalclp.h.PACKAGE           %3\aalsdk\aalclp\aalclp.h.PACKAGE_VERSION   @PACKAGE_VERSION@   "5.0.2"
%1 %3\aalsdk\aalclp\aalclp.h.PACKAGE_VERSION   %3\aalsdk\aalclp\aalclp.h.AALSDK_COPYRIGHT  @AALSDK_COPYRIGHT@  "Copyright(c) 2003-2016, Intel Corporation"
%1 %3\aalsdk\aalclp\aalclp.h.AALSDK_COPYRIGHT  %3\aalsdk\aalclp\aalclp.h.PACKAGE_BUGREPORT @PACKAGE_BUGREPORT@ "joe.grecco@intel.com"
%1 %3\aalsdk\aalclp\aalclp.h.PACKAGE_BUGREPORT %3\aalsdk\aalclp\aalclp.h.PACKAGE_URL       @PACKAGE_URL@       ""
%1 %3\aalsdk\aalclp\aalclp.h.PACKAGE_URL       %3\aalsdk\aalclp\aalclp.h                   @GIT_COMMIT_ID@     "unknown"

:done
exit /b 0
