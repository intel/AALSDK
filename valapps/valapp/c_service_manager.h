
extern "C" {
    typedef struct 
    {
        unsigned char* virtual_addr;
        unsigned long physical_addr;
        unsigned long size;
    } mmio_buffer_t;

    void service_manager_shutdown();
    
    void service_manager_start();
    
    void service_manager_define_services(char* config_file);

    void service_manager_create_services(char* config_file);
    
    void service_manager_create_service(char* service_name);

    void* service_manager_get_client(char* name);

    void service_release(char* service_name);

    unsigned int reconfigure(char* filename);

    mmio_buffer_t buffer_allocate(char* service, long size);

    void buffer_release(char* service, unsigned char* buffer);

    unsigned int mmio_read32(char* service, unsigned int address);
    
    long long mmio_read64(char* service, unsigned int address);

    void mmio_write32(char* service, unsigned int address, int value);
    
    void mmio_write64(char* service, unsigned int address, long value);

    void afu_reset(char* service);



}
