#include "ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61.h"
#include "ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3.h"

typedef union ffx_dof_composite_pass_16bit_PermutationKey {
    struct {
        uint32_t FFX_DOF_OPTION_MAX_RING_MERGE_LOG : 1;
        uint32_t FFX_DOF_OPTION_COMBINE_IN_PLACE : 1;
        uint32_t FFX_DOF_OPTION_REVERSE_DEPTH : 1;
    };
    uint32_t index;
} ffx_dof_composite_pass_16bit_PermutationKey;

typedef struct ffx_dof_composite_pass_16bit_PermutationInfo {
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
} ffx_dof_composite_pass_16bit_PermutationInfo;

static const uint32_t g_ffx_dof_composite_pass_16bit_IndirectionTable[] = {
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
};

static const ffx_dof_composite_pass_16bit_PermutationInfo g_ffx_dof_composite_pass_16bit_PermutationInfo[] = {
    { g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_size, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_data, 1, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_CBVResourceNames, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_CBVResourceBindings, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_CBVResourceCounts, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_CBVResourceSets, 2, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureSRVResourceNames, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureSRVResourceBindings, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureSRVResourceCounts, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureSRVResourceSets, 3, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureUAVResourceNames, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureUAVResourceBindings, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureUAVResourceCounts, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_SamplerResourceNames, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_SamplerResourceBindings, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_SamplerResourceCounts, g_ffx_dof_composite_pass_16bit_0202556e988e839f348a89f1f95e6a61_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_size, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_data, 1, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_CBVResourceNames, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_CBVResourceBindings, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_CBVResourceCounts, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_CBVResourceSets, 3, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureSRVResourceNames, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureSRVResourceBindings, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureSRVResourceCounts, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureSRVResourceSets, 3, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureUAVResourceNames, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureUAVResourceBindings, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureUAVResourceCounts, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_SamplerResourceNames, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_SamplerResourceBindings, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_SamplerResourceCounts, g_ffx_dof_composite_pass_16bit_a37c4175bf2e78e6622bf7e7975b6fb3_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

