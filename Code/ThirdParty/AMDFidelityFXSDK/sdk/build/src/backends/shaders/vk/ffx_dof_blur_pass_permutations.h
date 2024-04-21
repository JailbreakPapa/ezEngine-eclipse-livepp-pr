#include "ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d.h"
#include "ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94.h"

typedef union ffx_dof_blur_pass_PermutationKey {
    struct {
        uint32_t FFX_DOF_OPTION_MAX_RING_MERGE_LOG : 1;
        uint32_t FFX_DOF_OPTION_COMBINE_IN_PLACE : 1;
        uint32_t FFX_DOF_OPTION_REVERSE_DEPTH : 1;
    };
    uint32_t index;
} ffx_dof_blur_pass_PermutationKey;

typedef struct ffx_dof_blur_pass_PermutationInfo {
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
} ffx_dof_blur_pass_PermutationInfo;

static const uint32_t g_ffx_dof_blur_pass_IndirectionTable[] = {
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
};

static const ffx_dof_blur_pass_PermutationInfo g_ffx_dof_blur_pass_PermutationInfo[] = {
    { g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_size, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_data, 1, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_CBVResourceNames, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_CBVResourceBindings, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_CBVResourceCounts, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_CBVResourceSets, 2, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureSRVResourceNames, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureSRVResourceBindings, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureSRVResourceCounts, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureSRVResourceSets, 2, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureUAVResourceNames, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureUAVResourceBindings, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureUAVResourceCounts, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_BufferUAVResourceNames, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_BufferUAVResourceBindings, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_BufferUAVResourceCounts, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_BufferUAVResourceSets, 2, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_SamplerResourceNames, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_SamplerResourceBindings, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_SamplerResourceCounts, g_ffx_dof_blur_pass_6f73d8aee162e0458686794b13ad898d_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_size, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_data, 1, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_CBVResourceNames, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_CBVResourceBindings, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_CBVResourceCounts, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_CBVResourceSets, 2, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureSRVResourceNames, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureSRVResourceBindings, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureSRVResourceCounts, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureSRVResourceSets, 2, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureUAVResourceNames, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureUAVResourceBindings, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureUAVResourceCounts, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_BufferUAVResourceNames, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_BufferUAVResourceBindings, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_BufferUAVResourceCounts, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_BufferUAVResourceSets, 2, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_SamplerResourceNames, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_SamplerResourceBindings, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_SamplerResourceCounts, g_ffx_dof_blur_pass_5ccb6dd68e4364b811092737bd16cb94_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

