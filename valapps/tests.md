Test specification in JSON format {#tests}
=================================================

The tests input file must be formatted such that the top level structure is a dictionary (denoted by the {} 
characters) with one key called "tests" and an optional key called "collateral". 
The value of the "tests" key is a list structure (denoted by the [] characters) and must have at least one test specification. If included, the value for the "collateral" key is a list structure and must have at least one collateral specification. The following table shows the data that makes up the test specification.

Field Name | Description | Required
-----------|-------------|----------
name | The name of the test library. | Yes
args | A dictionary structure of arguments. Keys are the argument names and values are the argument values. | No
filter | A google test filter to reduce the tests executed | No
disabled | A boolean flag used to indicate to runit.py to skip the test | No

The following table shows the collateral specification.

Field Name | Description | Required
-----------|-------------|----------
name       | The name of the collateral package | Yes
version    | The version of the collateral package | No (but may be in the future)
type       | The type of collateral (binary, library, source) | No
url        | The URL of the collateral | Yes
fetcher    | The fetcher used to get the collateral in a directory structure | Yes
unpacker   | The unpacker used to unpack the collateral | No
installer  | The installer used to install the collateral | No

The dictionary structure of the fetcher, unpacker, and installer will have at a minimum 
the name. The rest of the keys/values vary by the type of fetcher/unpacker/installer.
