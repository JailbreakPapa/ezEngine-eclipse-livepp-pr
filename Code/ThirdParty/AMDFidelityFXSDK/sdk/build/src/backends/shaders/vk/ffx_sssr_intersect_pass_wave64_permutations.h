#include "ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d.h"
#include "ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8.h"

typedef union ffx_sssr_intersect_pass_wave64_PermutationKey {
    struct {
        uint32_t FFX_SSSR_OPTION_INVERTED_DEPTH : 1;
    };
    uint32_t index;
} ffx_sssr_intersect_pass_wave64_PermutationKey;

typedef struct ffx_sssr_intersect_pass_wave64_PermutationInfo {
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
} ffx_sssr_intersect_pass_wave64_PermutationInfo;

static const uint32_t g_ffx_sssr_intersect_pass_wave64_IndirectionTable[] = {
    1,
    0,
};

static const ffx_sssr_intersect_pass_wave64_PermutationInfo g_ffx_sssr_intersect_pass_wave64_PermutationInfo[] = {
    { g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_size, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_data, 1, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_CBVResourceNames, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_CBVResourceBindings, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_CBVResourceCounts, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_CBVResourceSets, 6, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureSRVResourceNames, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureSRVResourceBindings, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureSRVResourceCounts, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureSRVResourceSets, 1, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureUAVResourceNames, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureUAVResourceBindings, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureUAVResourceCounts, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_TextureUAVResourceSets, 0, 0, 0, 0, 0, 2, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_BufferUAVResourceNames, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_BufferUAVResourceBindings, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_BufferUAVResourceCounts, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_BufferUAVResourceSets, 1, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_SamplerResourceNames, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_SamplerResourceBindings, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_SamplerResourceCounts, g_ffx_sssr_intersect_pass_wave64_e383b7586605e15f9426fc654d0e028d_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_size, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_data, 1, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_CBVResourceNames, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_CBVResourceBindings, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_CBVResourceCounts, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_CBVResourceSets, 6, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureSRVResourceNames, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureSRVResourceBindings, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureSRVResourceCounts, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureSRVResourceSets, 1, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureUAVResourceNames, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureUAVResourceBindings, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureUAVResourceCounts, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_TextureUAVResourceSets, 0, 0, 0, 0, 0, 2, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_BufferUAVResourceNames, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_BufferUAVResourceBindings, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_BufferUAVResourceCounts, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_BufferUAVResourceSets, 1, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_SamplerResourceNames, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_SamplerResourceBindings, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_SamplerResourceCounts, g_ffx_sssr_intersect_pass_wave64_be15940b4952408cdc1e9391604cead8_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

