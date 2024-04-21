#include "ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de.h"
#include "ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744.h"

typedef union ffx_dof_downsample_depth_pass_wave64_16bit_PermutationKey {
    struct {
        uint32_t FFX_DOF_OPTION_MAX_RING_MERGE_LOG : 1;
        uint32_t FFX_DOF_OPTION_COMBINE_IN_PLACE : 1;
        uint32_t FFX_DOF_OPTION_REVERSE_DEPTH : 1;
    };
    uint32_t index;
} ffx_dof_downsample_depth_pass_wave64_16bit_PermutationKey;

typedef struct ffx_dof_downsample_depth_pass_wave64_16bit_PermutationInfo {
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
} ffx_dof_downsample_depth_pass_wave64_16bit_PermutationInfo;

static const uint32_t g_ffx_dof_downsample_depth_pass_wave64_16bit_IndirectionTable[] = {
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
};

static const ffx_dof_downsample_depth_pass_wave64_16bit_PermutationInfo g_ffx_dof_downsample_depth_pass_wave64_16bit_PermutationInfo[] = {
    { g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_size, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_data, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_CBVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_CBVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_CBVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_CBVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureSRVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureSRVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureSRVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureSRVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_BufferUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_BufferUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_BufferUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_BufferUAVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_SamplerResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_SamplerResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_SamplerResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_ed96c44a85c604710bbe0c9bb1a692de_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_size, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_data, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_CBVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_CBVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_CBVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_CBVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureSRVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureSRVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureSRVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureSRVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_BufferUAVResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_BufferUAVResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_BufferUAVResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_BufferUAVResourceSets, 1, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_SamplerResourceNames, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_SamplerResourceBindings, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_SamplerResourceCounts, g_ffx_dof_downsample_depth_pass_wave64_16bit_d035c4f577924c174074deb83f5d5744_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

