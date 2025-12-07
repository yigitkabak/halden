typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#define AMD_FEATURE_3DNOW       (1 << 0)
#define AMD_FEATURE_3DNOWEXT    (1 << 1)
#define AMD_FEATURE_SSE4A       (1 << 2)
#define AMD_FEATURE_SSE5        (1 << 3)
#define AMD_FEATURE_XOP         (1 << 4)
#define AMD_FEATURE_FMA4        (1 << 5)
#define AMD_FEATURE_TBM         (1 << 6)
#define AMD_FEATURE_TURBO_CORE  (1 << 7)
#define AMD_FEATURE_COOL_QUIET  (1 << 8)
#define AMD_FEATURE_SVM         (1 << 9)

static uint32_t amd_features = 0;
static int amd_initialized = 0;

int amd_is_amd_cpu(void) {
    uint32_t ebx, ecx, edx;
    asm volatile("cpuid" : "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    
    if (ebx == 0x68747541 && edx == 0x69746e65 && ecx == 0x444d4163) {
        return 1;
    }
    return 0;
}

void amd_detect_features(void) {
    if (!amd_is_amd_cpu()) {
        return;
    }
    
    uint32_t eax, ebx, ecx, edx;
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000001));
    
    if (edx & (1 << 31)) amd_features |= AMD_FEATURE_3DNOW;
    if (edx & (1 << 30)) amd_features |= AMD_FEATURE_3DNOWEXT;
    
    if (ecx & (1 << 6)) amd_features |= AMD_FEATURE_SSE4A;
    
    if (ecx & (1 << 11)) amd_features |= AMD_FEATURE_XOP;
    
    if (ecx & (1 << 16)) amd_features |= AMD_FEATURE_FMA4;
    
    if (ecx & (1 << 21)) amd_features |= AMD_FEATURE_TBM;
    
    if (ecx & (1 << 2)) amd_features |= AMD_FEATURE_SVM;
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000007));
    
    if (edx & (1 << 9)) amd_features |= AMD_FEATURE_TURBO_CORE;
    
    if (edx & (1 << 1)) amd_features |= AMD_FEATURE_COOL_QUIET;
}

int amd_init(void) {
    if (!amd_is_amd_cpu()) {
        return -1;
    }
    
    amd_detect_features();
    amd_initialized = 1;
    
    return 0;
}

uint32_t amd_get_features(void) {
    return amd_features;
}

int amd_has_feature(uint32_t feature) {
    return (amd_features & feature) != 0;
}

void amd_enable_optimizations(void) {
    if (!amd_initialized) return;
    
    uint32_t cr0, cr4;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2);
    cr0 |= (1 << 1);
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
    
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 9);
    cr4 |= (1 << 10);
    asm volatile("mov %0, %%cr4" :: "r"(cr4));
    
    if (amd_features & AMD_FEATURE_3DNOW) {
    }
}

int amd_is_initialized(void) {
    return amd_initialized;
}

void amd_get_family_model(uint32_t* family, uint32_t* model) {
    uint32_t eax, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    uint32_t base_family = (eax >> 8) & 0xF;
    uint32_t ext_family = (eax >> 20) & 0xFF;
    
    if (base_family == 0xF) {
        *family = base_family + ext_family;
    } else {
        *family = base_family;
    }
    
    uint32_t base_model = (eax >> 4) & 0xF;
    uint32_t ext_model = (eax >> 16) & 0xF;
    
    if (base_family == 0xF || base_family == 0x6) {
        *model = (ext_model << 4) | base_model;
    } else {
        *model = base_model;
    }
}

void amd_get_cache_info(uint32_t* l1_cache, uint32_t* l2_cache, uint32_t* l3_cache) {
    uint32_t eax, ebx, ecx, edx;
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000005));
    *l1_cache = ((ecx >> 24) & 0xFF);
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000006));
    *l2_cache = ((ecx >> 16) & 0xFFFF);
    
    *l3_cache = ((edx >> 18) & 0x3FFF) * 512;
}