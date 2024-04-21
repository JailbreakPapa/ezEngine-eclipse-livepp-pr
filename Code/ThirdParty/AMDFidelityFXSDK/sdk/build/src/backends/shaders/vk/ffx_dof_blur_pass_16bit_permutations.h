#include "ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84.h"
#include "ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613.h"

typedef union ffx_dof_blur_pass_16bit_PermutationKey {
    struct {
        uint32_t FFX_DOF_OPTION_MAX_RING_MERGE_LOG : 1;
        uint32_t FFX_DOF_OPTION_COMBINE_IN_PLACE : 1;
        uint32_t FFX_DOF_OPTION_REVERSE_DEPTH : 1;
    };
    uint32_t index;
} ffx_dof_blur_pass_16bit_PermutationKey;

typedef struct ffx_dof_blur_pass_16bit_PermutationInfo {
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
} ffx_dof_blur_pass_16bit_PermutationInfo;

static const uint32_t g_ffx_dof_blur_pass_16bit_IndirectionTable[] = {
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
};

static const ffx_dof_blur_pass_16bit_PermutationInfo g_ffx_dof_blur_pass_16bit_PermutationInfo[] = {
    { g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_size, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_data, 1, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_CBVResourceNames, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_CBVResourceBindings, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_CBVResourceCounts, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_CBVResourceSets, 2, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureSRVResourceNames, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureSRVResourceBindings, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureSRVResourceCounts, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureSRVResourceSets, 2, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureUAVResourceNames, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureUAVResourceBindings, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureUAVResourceCounts, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_BufferUAVResourceNames, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_BufferUAVResourceBindings, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_BufferUAVResourceCounts, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_BufferUAVResourceSets, 2, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_SamplerResourceNames, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_SamplerResourceBindings, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_SamplerResourceCounts, g_ffx_dof_blur_pass_16bit_30b9407769819caf2367604541e89b84_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_size, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_data, 1, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_CBVResourceNames, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_CBVResourceBindings, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_CBVResourceCounts, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_CBVResourceSets, 2, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureSRVResourceNames, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureSRVResourceBindings, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureSRVResourceCounts, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureSRVResourceSets, 2, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureUAVResourceNames, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureUAVResourceBindings, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureUAVResourceCounts, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_TextureUAVResourceSets, 0, 0, 0, 0, 0, 1, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_BufferUAVResourceNames, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_BufferUAVResourceBindings, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_BufferUAVResourceCounts, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_BufferUAVResourceSets, 2, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_SamplerResourceNames, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_SamplerResourceBindings, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_SamplerResourceCounts, g_ffx_dof_blur_pass_16bit_c8ab9ddcac0d2674a510f781e7dbb613_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

