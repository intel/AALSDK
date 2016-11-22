#pragma once
#include <memory>
#include <aalsdk/AALTypes.h>

class mmio_buffer
{
    public:
        typedef std::shared_ptr<mmio_buffer> ptr_t;

        mmio_buffer(AAL::btVirtAddr virt, AAL::btPhysAddr phys, std::size_t size, AAL::IALIBuffer *buffer_if):
            virtual_address_(virt),
            physical_address_(phys),
            size_(size),
            buffer_if_(buffer_if){}

        ~mmio_buffer()
        {
            release();
        }

        void release()
        {
            if (virtual_address_)
            {
                buffer_if_->bufferFree(virtual_address_);
                virtual_address_ = 0x0;
                physical_address_ = 0x0;
                size_ = 0x0;
            }
        }

        AAL::btVirtAddr address() { return virtual_address_; }
        AAL::btPhysAddr physical() { return physical_address_; }
        std::size_t size() { return size_; }

        template<typename T>
        void write(const T& value, std::size_t offset = 0)
        {
            ::memcpy(virtual_address_ + offset, &value, sizeof(T));
        }

        template<typename T>
        void read(std::size_t offset = 0)
        {
            return static_cast<T>(virtual_address_ + offset);
        }

    private:
        AAL::btVirtAddr virtual_address_;
        AAL::btPhysAddr physical_address_;
        std::size_t size_;
        AAL::IALIBuffer *buffer_if_;
};
