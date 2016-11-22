#include "afu_client.h"
#include <functional>

using namespace AAL;

afu_client::afu_client() : service_client()
{
}

afu_client::~afu_client()
{
}

void afu_client::serviceAllocated(IBase *pServiceBase, TransactionID const &rTransID)
{
    mmio_ = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
    umsg_ = dynamic_ptr<IALIUMsg>(iidALI_UMSG_Service, pServiceBase);
    buffer_ = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
    perf_ = dynamic_ptr<IALIPerf>(iidALI_PERF_Service, pServiceBase);
    reset_ = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
    stap_ = dynamic_ptr<IALISignalTap>(iidALI_STAP_Service, pServiceBase);
    service_client::serviceAllocated(pServiceBase, rTransID);
}

void afu_client::release()
{
    for (auto buffer_pair : buffer_map_)
    {
        buffer_pair.second->release();
    }

    service_client::release();
}

int afu_client::reset()
{
    return reset_->afuReset();
}

unsigned char* afu_client::mmio_address()
{
    return mmio_->mmioGetAddress();
}

std::size_t afu_client::mmio_length()
{
    return mmio_->mmioGetLength();
}

void afu_client::mmio_write32(unsigned int offset, int value)
{
    mmio_->mmioWrite32(offset, value);
}

void afu_client::mmio_write64(unsigned int offset, long value)
{
    mmio_->mmioWrite64(offset, value);
}

unsigned int afu_client::mmio_read32(unsigned int offset)
{
    unsigned int value = 0;
    mmio_->mmioRead32(offset, &value);
    return value;
}

long long unsigned int afu_client::mmio_read64(unsigned int offset)
{
    long long unsigned int value = 0;
    mmio_->mmioRead64(offset, &value);
    return value;
}

bool afu_client::feature_id_offset(uint32_t feature_id, uint32_t &offset)
{
    NamedValueSet request;
    request.Add(ALI_GETFEATURE_ID_KEY, feature_id);
    return mmio_->mmioGetFeatureOffset(&offset, request);
}

bool afu_client::feature_type_offset(uint32_t feature_type, uint32_t &offset)
{
    NamedValueSet request;
    request.Add(ALI_GETFEATURE_TYPE_KEY, feature_type);
    return mmio_->mmioGetFeatureOffset(&offset, request);
}

mmio_buffer::ptr_t afu_client::allocate_buffer(std::size_t size, bool track_buffer)
{
    btVirtAddr virt_address = 0;
    btPhysAddr phys_address = 0;
    auto status = buffer_->bufferAllocate(size, &virt_address);
    if (ali_errnumOK != status)
    {
        Log() << "Error allocating buffer" << std::endl;
        throw buffer_allocate_error();
    }
    phys_address = buffer_->bufferGetIOVA(virt_address);
    auto buffer = mmio_buffer::ptr_t(new mmio_buffer(virt_address, phys_address, size, buffer_));
    if (track_buffer)
    {
        buffer_map_[virt_address] = buffer;
    }

    return buffer;
}

void afu_client::release_buffer(btVirtAddr address)
{
    auto iter = buffer_map_.find(address);
    if (iter == buffer_map_.end())
    {
        Log() << "WARNING: Buffer at 0x" << std::hex << address << " not found";
        return;
    }
    iter->second->release();
    buffer_map_.erase(iter);
}

bool afu_client::register_offset(const std::string &regid, unsigned int &offset)
{
    auto regit = registers_.find(regid);
    if (regit == registers_.end())
    {
        return false;
    }
    offset = regit->second.offset();
    return true;
}

bool afu_client::register_write32(const std::string &regid, int value)
{
    unsigned int offset;
    if (register_offset(regid, offset))
    {
        mmio_write32(offset, value);
        return true;
    }
    return false;
}

bool afu_client::register_write64(const std::string &regid, long value)
{
    unsigned int offset;
    if (register_offset(regid, offset))
    {
        mmio_write64(offset, value);
        return true;
    }
    return false;
}

unsigned int afu_client::register_read32(const std::string &regid)
{
    unsigned int offset;
    if (register_offset(regid, offset))
    {
        return mmio_read32(offset);
    }
    return 0;
}

long long unsigned int afu_client::register_read64(const std::string &regid)
{
    unsigned int offset;
    if (register_offset(regid, offset))
    {
        return mmio_read64(offset);
    }
    return 0;
}
