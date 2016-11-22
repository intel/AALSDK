import atexit
import ctypes
import json

class mmio_buffer_struct(ctypes.Structure):
    _fields_ = [('virtual_addr', ctypes.c_void_p),
                ('physical_addr', ctypes.c_uint64),
                ('size', ctypes.c_uint64)]

class mmio_buffer(object):
    def __init__(self, buffer_struct):
        self.buffer_struct = buffer_struct
        self.size = buffer_struct.size
        self.c_buffer_t = ctypes.c_char*buffer_struct.size
        self.c_buffer = self.c_buffer_t.from_address(buffer_struct.virtual_addr)

    def __getitem__(self, offset):
        return ctypes.c_char(self.c_buffer[offset])

    def __setitem__(self, offset, value):
        v = type(value).from_address(self.buffer_struct.virtual_addr+offset)
        v = value

    def from_type(self, ctype_type, offset):
        return ctype_type.from_address(self.buffer_struct.virtual_addr+offset)

    def virtual(self):
        return  self.buffer_struct.virtual_addr

    def physical(self, cache_aligned = False):
        if cache_aligned:
            return self.buffer_struct.physical_addr >> 6
        else:
            return self.buffer_struct.physical_addr




class service_manager(object):
    def __init__(self, config='services.json'):
        with open(config, 'r') as fd:
            self.services = json.load(fd)
        self.caal = ctypes.CDLL('libaalcif.so')
        self.caal.service_manager_define_services(config)
        self.caal.service_manager_start()
        atexit.register(self.caal.service_manager_shutdown)
        self.caal.mmio_read64.restype = ctypes.c_uint64
        self.caal.mmio_read32.restype = ctypes.c_uint32
        self.caal.buffer_allocate.restype = mmio_buffer_struct

    def get_service(self, name):
        spec = next((svc for svc in self.services['services'] if svc['alias'] == name), None)
        return afu_client(self.caal, name, spec)

    def shutdown(self):
        self.caal.service_manager_shutdown()

    def create_service(self, name):
        self.caal.service_manager_create_service(name)

    def create_services(self, name):
        self.caal.service_manager_create_services(name)

    def reconfigure(self, rtlfile):
        return self.caal.reconfigure(rtlfile)

class afu_client(object):
    def __init__(self, caal, name, spec=None):
        self.caal = caal
        self.name = name
        self.spec = spec
        self.register_map = {}
        self.parse_registers(self.spec)


    def parse_registers(self, data):
        if self.spec is not data:
            self.spec = data
        for reginfo in data.get("registers", []):
            reginfo["offset"] = long(reginfo["offset"], 16)
            self.register_map[reginfo["id"]] = reginfo

    def release(self):
        self.caal.service_release(self.name)

    def mmio_read32(self, offset):
        return self.caal.mmio_read32(self.name, offset)

    def mmio_read64(self, offset):
        return self.caal.mmio_read64(self.name, offset)

    def mmio_write32(self, offset, value):
        self.caal.mmio_write32(self.name, offset, value)

    def mmio_write64(self, offset, value):
        self.caal.mmio_write64(self.name, offset, value)

    def reset(self):
        self.caal.afu_reset(self.name)

    def reconfigure(self, rtlfile):
        self.caal.reconfigure(rtlfile)

    def buffer_allocate(self, size, fillwith=None):
        buf = self.caal.buffer_allocate(self.name, size)
        if fillwith is not None:
            ctypes.memset(buf.virtual_addr, fillwith, size)
        buff = mmio_buffer(buf)
        atexit.register(self.buffer_release, buff)
        return buff

    def buffer_release(self, buf):
        self.caal.buffer_release(self.name, buf.buffer_struct.virtual_addr)

    def register_write(self, regid, value):
        if regid in self.register_map:
            reginfo = self.register_map[regid]
            width = reginfo["width"]
            offset = reginfo["offset"]
            value_type = ctypes.c_int64 if width == 64 else ctypes.c_int32
            func = getattr(self, 'mmio_write{}'.format(width))
            func(offset, value_type(value))


    def register_read(self, regid):
        if regid in self.register_map:
            reginfo = self.register_map[regid]
            width = reginfo["width"]
            offset = reginfo["offset"]
            value_type = ctypes.c_int64 if width == 64 else ctypes.c_int32
            func = getattr(self, 'mmio_read{}'.format(width))
            return func(offset)


def testit():
    sm = service_manager()

    afu = sm.get_service('NLB')
    addr = afu.buffer_allocate(1024)

    print "virtual:", hex(addr.virtual_addr)
    print "physical:", hex(addr.physical_addr)
    afu.reset()
    afu.register_write("CSR_SCRATCHPAD0", 1)

    print(afu.register_read("CSR_SCRATCHPAD0"))
    afu.reset()
    print(afu.mmio_read64(0x0100))
    afuid_l = str(hex(afu.register_read("AFU_ID_L")))
    afuid_h = str(hex(afu.register_read("AFU_ID_H")))
    print afuid_l, afuid_h


if __name__ == '__main__':
    testit()

