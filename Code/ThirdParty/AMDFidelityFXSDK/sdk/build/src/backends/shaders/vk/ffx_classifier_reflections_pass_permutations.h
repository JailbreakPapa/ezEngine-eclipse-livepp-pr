#include "ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb.h"
#include "ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3.h"

typedef union ffx_classifier_reflections_pass_PermutationKey {
    struct {
        uint32_t FFX_CLASSIFIER_OPTION_INVERTED_DEPTH : 1;
        uint32_t FFX_CLASSIFIER_OPTION_CLASSIFIER_MODE : 1;
    };
    uint32_t index;
} ffx_classifier_reflections_pass_PermutationKey;

typedef struct ffx_classifier_reflections_pass_PermutationInfo {
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
} ffx_classifier_reflections_pass_PermutationInfo;

static const uint32_t g_ffx_classifier_reflections_pass_IndirectionTable[] = {
    0,
    1,
    0,
    1,
};

static const ffx_classifier_reflections_pass_PermutationInfo g_ffx_classifier_reflections_pass_PermutationInfo[] = {
    { g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_size, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_data, 1, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_CBVResourceNames, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_CBVResourceBindings, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_CBVResourceCounts, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_CBVResourceSets, 7, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureSRVResourceNames, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureSRVResourceBindings, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureSRVResourceCounts, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureSRVResourceSets, 3, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureUAVResourceNames, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureUAVResourceBindings, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureUAVResourceCounts, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_TextureUAVResourceSets, 0, 0, 0, 0, 0, 4, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_BufferUAVResourceNames, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_BufferUAVResourceBindings, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_BufferUAVResourceCounts, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_BufferUAVResourceSets, 2, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_SamplerResourceNames, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_SamplerResourceBindings, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_SamplerResourceCounts, g_ffx_classifier_reflections_pass_ced2d22561c0ed40d8785f75588993eb_SamplerResourceSets, 0, 0, 0, 0, 0, },
    { g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_size, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_data, 1, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_CBVResourceNames, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_CBVResourceBindings, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_CBVResourceCounts, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_CBVResourceSets, 7, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureSRVResourceNames, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureSRVResourceBindings, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureSRVResourceCounts, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureSRVResourceSets, 3, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureUAVResourceNames, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureUAVResourceBindings, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureUAVResourceCounts, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_TextureUAVResourceSets, 0, 0, 0, 0, 0, 4, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_BufferUAVResourceNames, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_BufferUAVResourceBindings, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_BufferUAVResourceCounts, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_BufferUAVResourceSets, 2, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_SamplerResourceNames, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_SamplerResourceBindings, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_SamplerResourceCounts, g_ffx_classifier_reflections_pass_eb1dda164d77cf7132fcfeaefeacd4d3_SamplerResourceSets, 0, 0, 0, 0, 0, },
};

