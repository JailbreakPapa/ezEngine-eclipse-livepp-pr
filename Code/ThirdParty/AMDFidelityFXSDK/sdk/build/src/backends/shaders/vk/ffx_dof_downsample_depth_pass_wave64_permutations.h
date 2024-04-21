#include "ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae.h"
#include "ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26.h"

typedef union ffx_dof_downsample_depth_pass_wave64_PermutationKey {
    struct {
        uint32_t FFX_DOF_OPTION_MAX_RING_MERGE_LOG : 1;
        uint32_t FFX_DOF_OPTION_COMBINE_IN_PLACE : 1;
        uint32_t FFX_DOF_OPTION_REVERSE_DEPTH : 1;
    };
    uint32_t index;
} ffx_dof_downsample_depth_pass_wave64_PermutationKey;

typedef struct ffx_dof_downsample_depth_pass_wave64_PermutationInfo {
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
} ffx_dof_downsample_depth_pass_wave64_PermutationInfo;

static const uint32_t g_ffx_dof_downsample_depth_pass_wave64_IndirectionTable[] = {
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
};

static const ffx_dof_downsample_depth_pass_wave64_PermutationInfo g_ffx_dof_downsample_depth_pass_wave64_PermutationInfo[] = {
    { g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_size, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_data, 1, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_CBVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_CBVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_CBVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_CBVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureSRVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureSRVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureSRVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureSRVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_BufferUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_BufferUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_BufferUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_BufferUAVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_SamplerResourceNames, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_SamplerResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_SamplerResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_33a8121bf9d5b4526d4bce34ff1404ae_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_size, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_data, 1, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_CBVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_CBVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_CBVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_CBVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureSRVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureSRVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureSRVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureSRVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_BufferUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_BufferUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_BufferUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_BufferUAVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_SamplerResourceNames, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_SamplerResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_SamplerResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_d9bd733ad9457424c4c4913b9fad6f26_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

