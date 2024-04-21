#include "ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db.h"
#include "ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec.h"

typedef union ffx_cacao_upscale_bilateral_5x5_pass_PermutationKey {
    struct {
        uint32_t FFX_CACAO_OPTION_APPLY_SMART : 1;
    };
    uint32_t index;
} ffx_cacao_upscale_bilateral_5x5_pass_PermutationKey;

typedef struct ffx_cacao_upscale_bilateral_5x5_pass_PermutationInfo {
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
} ffx_cacao_upscale_bilateral_5x5_pass_PermutationInfo;

static const uint32_t g_ffx_cacao_upscale_bilateral_5x5_pass_IndirectionTable[] = {
    1,
    0,
};

static const ffx_cacao_upscale_bilateral_5x5_pass_PermutationInfo g_ffx_cacao_upscale_bilateral_5x5_pass_PermutationInfo[] = {
    { g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_size, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_data, 1, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_CBVResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_CBVResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_CBVResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_CBVResourceSets, 3, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureSRVResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureSRVResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureSRVResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureSRVResourceSets, 1, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureUAVResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureUAVResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureUAVResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_SamplerResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_SamplerResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_SamplerResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_7dfd33eb22cf82cc66be26c3734933db_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_size, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_data, 1, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_CBVResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_CBVResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_CBVResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_CBVResourceSets, 3, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureSRVResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureSRVResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureSRVResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureSRVResourceSets, 1, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureUAVResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureUAVResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureUAVResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_SamplerResourceNames, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_SamplerResourceBindings, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_SamplerResourceCounts, g_ffx_cacao_upscale_bilateral_5x5_pass_ebd5a536cafe9b7b6ff62a07700d20ec_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

