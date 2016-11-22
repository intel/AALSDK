from aal import service_manager
from ctypes import c_char, c_uint32, c_uint64
import time
kb = 1024
mb = 1024*kb

def test_mmio_01(self=None):
    sm = service_manager()
    sm.create_service('NLB0')
    afu = sm.get_service('NLB0')
    dsm = afu.buffer_allocate(mb*4, 0)
    inp = afu.buffer_allocate(mb*2, 0xAF)
    out = afu.buffer_allocate(mb*2, 0xBE)
    print inp[15]
    inp[15] = c_uint64(0xA)
    print inp[15]
    afu.reset()
    afu.register_write("CSR_AFU_DSM_BASEL", dsm.physical())
    afu.register_write("CSR_CTL", 0x0)
    afu.register_write("CSR_CTL", 0x1)
    afu.register_write("CSR_SRC_ADDR", inp.physical(cache_aligned=True))
    afu.register_write("CSR_DST_ADDR", out.physical(cache_aligned=True))
    afu.register_write("CSR_NUM_LINES", inp.size/64)
    afu.register_write("CSR_CFG", 0x200)

    afu.register_write("CSR_CTL", 0x3)
    while 0 == dsm.from_type(c_uint32, 0x40).value & 0x1:
        time.sleep(0.00001)
    afu.register_write("CSR_CTL", 0x7)

    afu.buffer_release(dsm)
    afu.buffer_release(inp)
    afu.buffer_release(out)

def test_mmio_03(self):
    self.exe('./valapp libafu_tests.so mmio_mapping SW-MMIO-03 --service-name NLB0')






    

if __name__ == '__main__':
    test_mmio_01()


