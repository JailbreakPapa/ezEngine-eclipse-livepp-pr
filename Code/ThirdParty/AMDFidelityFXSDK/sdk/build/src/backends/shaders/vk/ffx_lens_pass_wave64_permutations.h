#include "ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419.h"

typedef union ffx_lens_pass_wave64_PermutationKey {
    struct {
        uint32_t FFX_LENS_OPTION_LINEAR_SAMPLE : 1;
        uint32_t FFX_LENS_OPTION_WAVE_INTEROP_LDS : 1;
    };
    uint32_t index;
} ffx_lens_pass_wave64_PermutationKey;

typedef struct ffx_lens_pass_wave64_PermutationInfo {
    const uint32_t       blobSize;
    const unsigned char* blobData;


    const uint32_t  numConstantBuffers;
    const char**    constantBufferNames;
    const uint32_t* constantBufferBindings;
    const uint32_t* constantBufferCounts;
    const uint32_t* constantBufferSpaces;

    const uint32_t  numSRVTextures;
    const char**    srvTextureNames;
    const uint32_t* srvTextureBindings;
    const uint32_t* srvTextureCounts;
    const uint32_t* srvTextureSpaces;

    const uint32_t  numUAVTextures;
    const char**    uavTextureNames;
    const uint32_t* uavTextureBindings;
    const uint32_t* uavTextureCounts;
    const uint32_t* uavTextureSpaces;

    const uint32_t  numSRVBuffers;
    const char**    srvBufferNames;
    const uint32_t* srvBufferBindings;
    const uint32_t* srvBufferCounts;
    const uint32_t* srvBufferSpaces;

    const uint32_t  numUAVBuffers;
    const char**    uavBufferNames;
    const uint32_t* uavBufferBindings;
    const uint32_t* uavBufferCounts;
    const uint32_t* uavBufferSpaces;

    const uint32_t  numSamplers;
    const char**    samplerNames;
    const uint32_t* samplerBindings;
    const uint32_t* samplerCounts;
    const uint32_t* samplerSpaces;

    const uint32_t  numRTAccelerationStructures;
    const char**    rtAccelerationStructureNames;
    const uint32_t* rtAccelerationStructureBindings;
    const uint32_t* rtAccelerationStructureCounts;
    const uint32_t* rtAccelerationStructureSpaces;
} ffx_lens_pass_wave64_PermutationInfo;

static const uint32_t g_ffx_lens_pass_wave64_IndirectionTable[] = {
    0,
    0,
    0,
    0,
};

static const ffx_lens_pass_wave64_PermutationInfo g_ffx_lens_pass_wave64_PermutationInfo[] = {
    { g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_size, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_data, 1, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_CBVResourceNames, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_CBVResourceBindings, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_CBVResourceCounts, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_CBVResourceSets, 1, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureSRVResourceNames, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureSRVResourceBindings, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureSRVResourceCounts, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureSRVResourceSets, 1, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureUAVResourceNames, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureUAVResourceBindings, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureUAVResourceCounts, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_SamplerResourceNames, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_SamplerResourceBindings, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_SamplerResourceCounts, g_ffx_lens_pass_wave64_957a38bc813229e5ae14871dd82ae419_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

