typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#define INTEL_FEATURE_SSE       (1 << 0)
#define INTEL_FEATURE_SSE2      (1 << 1)
#define INTEL_FEATURE_SSE3      (1 << 2)
#define INTEL_FEATURE_SSSE3     (1 << 3)
#define INTEL_FEATURE_SSE4_1    (1 << 4)
#define INTEL_FEATURE_SSE4_2    (1 << 5)
#define INTEL_FEATURE_AVX       (1 << 6)
#define INTEL_FEATURE_AVX2      (1 << 7)
#define INTEL_FEATURE_HT        (1 << 8)
#define INTEL_FEATURE_TURBO     (1 << 9)

static uint32_t intel_features = 0;
static int intel_initialized = 0;

int intel_is_intel_cpu(void) {
    uint32_t ebx, ecx, edx;
    asm volatile("cpuid" : "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    
    if (ebx == 0x756e6547 && edx == 0x49656e69 && ecx == 0x6c65746e) {
        return 1;
    }
    return 0;
}

void intel_detect_features(void) {
    if (!intel_is_intel_cpu()) {
        return;
    }
    
    uint32_t eax, ebx, ecx, edx;
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    if (edx & (1 << 25)) intel_features |= INTEL_FEATURE_SSE;
    if (edx & (1 << 26)) intel_features |= INTEL_FEATURE_SSE2;
    if (ecx & (1 << 0))  intel_features |= INTEL_FEATURE_SSE3;
    if (ecx & (1 << 9))  intel_features |= INTEL_FEATURE_SSSE3;
    if (ecx & (1 << 19)) intel_features |= INTEL_FEATURE_SSE4_1;
    if (ecx & (1 << 20)) intel_features |= INTEL_FEATURE_SSE4_2;
    if (ecx & (1 << 28)) intel_features |= INTEL_FEATURE_AVX;
    
    if (edx & (1 << 28)) intel_features |= INTEL_FEATURE_HT;
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(7), "c"(0));
    if (ebx & (1 << 5)) intel_features |= INTEL_FEATURE_AVX2;
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(6));
    if (eax & (1 << 1)) intel_features |= INTEL_FEATURE_TURBO;
}

int intel_init(void) {
    if (!intel_is_intel_cpu()) {
        return -1;
    }
    
    intel_detect_features();
    intel_initialized = 1;
    
    return 0;
}

uint32_t intel_get_features(void) {
    return intel_features;
}

int intel_has_feature(uint32_t feature) {
    return (intel_features & feature) != 0;
}

void intel_enable_optimizations(void) {
    if (!intel_initialized) return;
    
    if (intel_features & INTEL_FEATURE_SSE) {
        uint32_t cr0, cr4;
        asm volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 &= ~(1 << 2);
        cr0 |= (1 << 1);
        asm volatile("mov %0, %%cr0" :: "r"(cr0));
        
        asm volatile("mov %%cr4, %0" : "=r"(cr4));
        cr4 |= (1 << 9);
        cr4 |= (1 << 10);
        asm volatile("mov %0, %%cr4" :: "r"(cr4));
    }
}

int intel_is_initialized(void) {
    return intel_initialized;
}

void intel_get_family_model(uint32_t* family, uint32_t* model) {
    uint32_t eax, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    *family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    *model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
}