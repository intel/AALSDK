Services specification in JSON format {#services}
=================================================

The services input file must be formatted such that the top level structure is a dictionary (denoted by the {} 
characters) with one key called "services". The value of this key is a list structure (denoted by the [] characters) and must have at least one service specification. The following table shows the data that makes up the service 
secification. 

Field Name | Description | Required
-----------|-------------|----------
service_lib| The library containing the AAL service (example: libALI) | Yes
alias      | The friendly name of the service. Will be used by service_manager::create_service(const std::string&) to associate with these parameters. | Yes
client_type| The service_client derived class to instantiate when creating the service. If not specifed here, afu_client will be used. | No
afu_id | The AFU identifier (GUID) generated and used when developing the AAL service | Yes
include_aia | Used to tell the service_manager to include AIA parameter. | Yes
bus | The PCIe bus number (in hex) where the AFU is loaded (to be loaded) | No
device | The PCIe device number (in hex) where the AFU is loaded. | No
function | The PCIe function number (in hex) where the AFU is loaded. | No
registers | A list of register mapping information defined by the AFU. This is experimental and is used by the Python code in aal.py | No


The following table shows the register mapping specification that is used in the registers list (described above).

Field Name | Description | Required
-----------|-------------|----------
id         | The id used in the green bitstream. | Yes
offset     | The register offset used in the green bitstream. | Yes
type       | Refers to read-only (RO), read-write (RW) or reserved (RsvdZ) registers | Yes
width      | The size (in bits) of the register. Currently, either 32 or 64. | Yes
comment    | A description of the register. | No
