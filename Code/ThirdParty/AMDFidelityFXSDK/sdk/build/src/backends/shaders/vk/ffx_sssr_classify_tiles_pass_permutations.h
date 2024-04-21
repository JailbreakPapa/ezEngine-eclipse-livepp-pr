#include "ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb.h"
#include "ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2.h"

typedef union ffx_sssr_classify_tiles_pass_PermutationKey {
    struct {
        uint32_t FFX_SSSR_OPTION_INVERTED_DEPTH : 1;
    };
    uint32_t index;
} ffx_sssr_classify_tiles_pass_PermutationKey;

typedef struct ffx_sssr_classify_tiles_pass_PermutationInfo {
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
} ffx_sssr_classify_tiles_pass_PermutationInfo;

static const uint32_t g_ffx_sssr_classify_tiles_pass_IndirectionTable[] = {
    0,
    1,
};

static const ffx_sssr_classify_tiles_pass_PermutationInfo g_ffx_sssr_classify_tiles_pass_PermutationInfo[] = {
    { g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_size, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_data, 1, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_CBVResourceNames, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_CBVResourceBindings, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_CBVResourceCounts, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_CBVResourceSets, 5, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureSRVResourceNames, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureSRVResourceBindings, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureSRVResourceCounts, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureSRVResourceSets, 2, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureUAVResourceNames, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureUAVResourceBindings, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureUAVResourceCounts, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_TextureUAVResourceSets, 0, 0, 0, 0, 0, 3, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_BufferUAVResourceNames, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_BufferUAVResourceBindings, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_BufferUAVResourceCounts, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_BufferUAVResourceSets, 1, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_SamplerResourceNames, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_SamplerResourceBindings, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_SamplerResourceCounts, g_ffx_sssr_classify_tiles_pass_1ab28fae12592455e67d2ceca730eccb_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_size, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_data, 1, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_CBVResourceNames, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_CBVResourceBindings, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_CBVResourceCounts, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_CBVResourceSets, 5, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureSRVResourceNames, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureSRVResourceBindings, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureSRVResourceCounts, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureSRVResourceSets, 2, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureUAVResourceNames, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureUAVResourceBindings, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureUAVResourceCounts, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_TextureUAVResourceSets, 0, 0, 0, 0, 0, 3, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_BufferUAVResourceNames, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_BufferUAVResourceBindings, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_BufferUAVResourceCounts, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_BufferUAVResourceSets, 1, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_SamplerResourceNames, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_SamplerResourceBindings, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_SamplerResourceCounts, g_ffx_sssr_classify_tiles_pass_d8508c2b23b0390177a11a3d8947eed2_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

