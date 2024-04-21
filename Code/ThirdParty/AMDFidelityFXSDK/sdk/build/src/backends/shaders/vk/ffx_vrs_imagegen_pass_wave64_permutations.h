#include "ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19.h"
#include "ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a.h"
#include "ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735.h"
#include "ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790.h"
#include "ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e.h"
#include "ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33.h"

typedef union ffx_vrs_imagegen_pass_wave64_PermutationKey {
    struct {
        uint32_t FFX_VRS_OPTION_ADDITIONALSHADINGRATES : 1;
        uint32_t FFX_VARIABLESHADING_TILESIZE : 2;
    };
    uint32_t index;
} ffx_vrs_imagegen_pass_wave64_PermutationKey;

typedef struct ffx_vrs_imagegen_pass_wave64_PermutationInfo {
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
} ffx_vrs_imagegen_pass_wave64_PermutationInfo;

static const uint32_t g_ffx_vrs_imagegen_pass_wave64_IndirectionTable[] = {
    3,
    4,
    1,
    5,
    2,
    0,
    0,
    0,
};

static const ffx_vrs_imagegen_pass_wave64_PermutationInfo g_ffx_vrs_imagegen_pass_wave64_PermutationInfo[] = {
    { g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_size, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_data, 1, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_CBVResourceNames, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_CBVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_CBVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_CBVResourceSets, 2, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureSRVResourceNames, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureSRVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureSRVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureSRVResourceSets, 1, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureUAVResourceNames, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureUAVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureUAVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_46bb3a81c8ae6f56f025584fbe172b19_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_size, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_data, 1, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_CBVResourceNames, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_CBVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_CBVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_CBVResourceSets, 2, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureSRVResourceNames, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureSRVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureSRVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureSRVResourceSets, 1, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureUAVResourceNames, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureUAVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureUAVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_52e7259a8b6940d894c8be9c8356792a_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_size, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_data, 1, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_CBVResourceNames, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_CBVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_CBVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_CBVResourceSets, 2, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureSRVResourceNames, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureSRVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureSRVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureSRVResourceSets, 1, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureUAVResourceNames, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureUAVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureUAVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_61c20620b0ea6d33d9d402b74ee1d735_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_size, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_data, 1, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_CBVResourceNames, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_CBVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_CBVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_CBVResourceSets, 2, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureSRVResourceNames, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureSRVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureSRVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureSRVResourceSets, 1, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureUAVResourceNames, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureUAVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureUAVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_818dee6f7794e4f34130f5f3eb750790_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_size, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_data, 1, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_CBVResourceNames, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_CBVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_CBVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_CBVResourceSets, 2, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureSRVResourceNames, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureSRVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureSRVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureSRVResourceSets, 1, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureUAVResourceNames, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureUAVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureUAVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_4188fe257a727ba01da45864854e4d4e_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_size, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_data, 1, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_CBVResourceNames, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_CBVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_CBVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_CBVResourceSets, 2, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureSRVResourceNames, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureSRVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureSRVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureSRVResourceSets, 1, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureUAVResourceNames, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureUAVResourceBindings, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureUAVResourceCounts, g_ffx_vrs_imagegen_pass_wave64_4093015e164ebf67aa1855548dfeae33_TextureUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};

