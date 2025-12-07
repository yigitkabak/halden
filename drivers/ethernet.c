typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
    uint8_t payload[1500];
} __attribute__((packed)) ethernet_frame;

typedef struct {
    uint8_t mac_address[6];
    int link_up;
    int speed_mbps;
    int duplex_full;
    uint32_t rx_packets;
    uint32_t tx_packets;
    uint32_t rx_errors;
    uint32_t tx_errors;
} nic_status;

static nic_status nic_info = {0};
static int ethernet_initialized = 0;

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    
    outl(0xCF8, address);
    
    return inl(0xCFC);
}

void pci_write_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    
    outl(0xCF8, address);
    outl(0xCFC, value);
}

int ethernet_detect_controller(void) {
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint32_t vendor_device = pci_read_config(bus, device, 0, 0);
            
            if (vendor_device == 0xFFFFFFFF || vendor_device == 0) {
                continue;
            }
            
            uint32_t class_code = pci_read_config(bus, device, 0, 0x08);
            uint8_t class = (class_code >> 24) & 0xFF;
            uint8_t subclass = (class_code >> 16) & 0xFF;
            
            if (class == 0x02 && subclass == 0x00) {
                uint16_t vendor = vendor_device & 0xFFFF;
                uint16_t device_id = (vendor_device >> 16) & 0xFFFF;
                
                return 1;
            }
        }
    }
    return 0;
}

int ethernet_init(void) {
    if (ethernet_initialized) {
        return 0;
    }
    
    if (!ethernet_detect_controller()) {
        return -1;
    }
    
    nic_info.mac_address[0] = 0x52;
    nic_info.mac_address[1] = 0x54;
    nic_info.mac_address[2] = 0x00;
    nic_info.mac_address[3] = 0x12;
    nic_info.mac_address[4] = 0x34;
    nic_info.mac_address[5] = 0x56;
    
    nic_info.link_up = 0;
    nic_info.speed_mbps = 1000;
    nic_info.duplex_full = 1;
    nic_info.rx_packets = 0;
    nic_info.tx_packets = 0;
    nic_info.rx_errors = 0;
    nic_info.tx_errors = 0;
    
    ethernet_initialized = 1;
    return 0;
}

void ethernet_get_mac(uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = nic_info.mac_address[i];
    }
}

void ethernet_set_mac(const uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        nic_info.mac_address[i] = mac[i];
    }
}

int ethernet_get_link_status(void) {
    return nic_info.link_up;
}

void ethernet_set_link_status(int up) {
    nic_info.link_up = up;
}

void ethernet_get_stats(uint32_t* rx_packets, uint32_t* tx_packets, 
                        uint32_t* rx_errors, uint32_t* tx_errors) {
    if (rx_packets) *rx_packets = nic_info.rx_packets;
    if (tx_packets) *tx_packets = nic_info.tx_packets;
    if (rx_errors) *rx_errors = nic_info.rx_errors;
    if (tx_errors) *tx_errors = nic_info.tx_errors;
}

int ethernet_send_frame(const uint8_t* dest_mac, const uint8_t* data, uint16_t length) {
    if (!ethernet_initialized || !nic_info.link_up) {
        return -1;
    }
    
    if (length > 1500) {
        return -2;
    }
    
    nic_info.tx_packets++;
    return 0;
}

int ethernet_recv_frame(uint8_t* src_mac, uint8_t* data, uint16_t* length) {
    if (!ethernet_initialized || !nic_info.link_up) {
        return -1;
    }
    
    return -1;
}

void ethernet_set_promiscuous(int enable) {
}

int ethernet_is_initialized(void) {
    return ethernet_initialized;
}

void ethernet_reset(void) {
    if (!ethernet_initialized) return;
    
    nic_info.rx_packets = 0;
    nic_info.tx_packets = 0;
    nic_info.rx_errors = 0;
    nic_info.tx_errors = 0;
}

int ethernet_get_speed(void) {
    return nic_info.speed_mbps;
}

int ethernet_is_full_duplex(void) {
    return nic_info.duplex_full;
}