#include "ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56.h"
#include "ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e.h"

typedef union ffx_parallelsort_scatter_pass_PermutationKey {
    struct {
        uint32_t FFX_PARALLELSORT_OPTION_HAS_PAYLOAD : 1;
    };
    uint32_t index;
} ffx_parallelsort_scatter_pass_PermutationKey;

typedef struct ffx_parallelsort_scatter_pass_PermutationInfo {
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
} ffx_parallelsort_scatter_pass_PermutationInfo;

static const uint32_t g_ffx_parallelsort_scatter_pass_IndirectionTable[] = {
    1,
    0,
};

static const ffx_parallelsort_scatter_pass_PermutationInfo g_ffx_parallelsort_scatter_pass_PermutationInfo[] = {
    { g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_size, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_data, 1, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_CBVResourceNames, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_CBVResourceBindings, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_CBVResourceCounts, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_CBVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_BufferUAVResourceNames, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_BufferUAVResourceBindings, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_BufferUAVResourceCounts, g_ffx_parallelsort_scatter_pass_e1e1c7f9d92d5468afe6abebddea6e56_BufferUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_size, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_data, 1, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_CBVResourceNames, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_CBVResourceBindings, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_CBVResourceCounts, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_CBVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_BufferUAVResourceNames, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_BufferUAVResourceBindings, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_BufferUAVResourceCounts, g_ffx_parallelsort_scatter_pass_d8f90befc5fda52be60b7a0992e19a8e_BufferUAVResourceSets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};

