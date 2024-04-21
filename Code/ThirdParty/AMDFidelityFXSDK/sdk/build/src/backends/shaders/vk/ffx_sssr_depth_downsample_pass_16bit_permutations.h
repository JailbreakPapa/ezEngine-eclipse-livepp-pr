#include "ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028.h"
#include "ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd.h"

typedef union ffx_sssr_depth_downsample_pass_16bit_PermutationKey {
    struct {
        uint32_t FFX_SSSR_OPTION_INVERTED_DEPTH : 1;
    };
    uint32_t index;
} ffx_sssr_depth_downsample_pass_16bit_PermutationKey;

typedef struct ffx_sssr_depth_downsample_pass_16bit_PermutationInfo {
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
} ffx_sssr_depth_downsample_pass_16bit_PermutationInfo;

static const uint32_t g_ffx_sssr_depth_downsample_pass_16bit_IndirectionTable[] = {
    1,
    0,
};

static const ffx_sssr_depth_downsample_pass_16bit_PermutationInfo g_ffx_sssr_depth_downsample_pass_16bit_PermutationInfo[] = {
    { g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_size, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_data, 0, 0, 0, 0, 0, 1, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureSRVResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureSRVResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureSRVResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureSRVResourceSets, 1, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureUAVResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureUAVResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureUAVResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_BufferUAVResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_BufferUAVResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_BufferUAVResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_BufferUAVResourceSets, 1, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_SamplerResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_SamplerResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_SamplerResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_c78621141c7c0cf767e73a324344c028_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_size, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_data, 0, 0, 0, 0, 0, 1, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureSRVResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureSRVResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureSRVResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureSRVResourceSets, 1, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureUAVResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureUAVResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureUAVResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_BufferUAVResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_BufferUAVResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_BufferUAVResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_BufferUAVResourceSets, 1, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_SamplerResourceNames, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_SamplerResourceBindings, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_SamplerResourceCounts, g_ffx_sssr_depth_downsample_pass_16bit_32da14b25731dfddab47c622d1f57ddd_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

