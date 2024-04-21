#include "ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4.h"
#include "ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d.h"

typedef union ffx_dof_composite_pass_PermutationKey {
    struct {
        uint32_t FFX_DOF_OPTION_MAX_RING_MERGE_LOG : 1;
        uint32_t FFX_DOF_OPTION_COMBINE_IN_PLACE : 1;
        uint32_t FFX_DOF_OPTION_REVERSE_DEPTH : 1;
    };
    uint32_t index;
} ffx_dof_composite_pass_PermutationKey;

typedef struct ffx_dof_composite_pass_PermutationInfo {
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
} ffx_dof_composite_pass_PermutationInfo;

static const uint32_t g_ffx_dof_composite_pass_IndirectionTable[] = {
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
};

static const ffx_dof_composite_pass_PermutationInfo g_ffx_dof_composite_pass_PermutationInfo[] = {
    { g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_size, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_data, 1, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_CBVResourceNames, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_CBVResourceBindings, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_CBVResourceCounts, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_CBVResourceSets, 3, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureSRVResourceNames, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureSRVResourceBindings, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureSRVResourceCounts, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureSRVResourceSets, 3, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureUAVResourceNames, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureUAVResourceBindings, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureUAVResourceCounts, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_SamplerResourceNames, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_SamplerResourceBindings, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_SamplerResourceCounts, g_ffx_dof_composite_pass_e18835ff10fc1d3c5d7446daf6a244c4_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_size, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_data, 1, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_CBVResourceNames, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_CBVResourceBindings, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_CBVResourceCounts, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_CBVResourceSets, 2, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureSRVResourceNames, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureSRVResourceBindings, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureSRVResourceCounts, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureSRVResourceSets, 3, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureUAVResourceNames, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureUAVResourceBindings, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureUAVResourceCounts, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_SamplerResourceNames, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_SamplerResourceBindings, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_SamplerResourceCounts, g_ffx_dof_composite_pass_156613ac265cbd784eb611d5c45aa62d_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

