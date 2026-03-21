#include <Shaders/Common/GlobalConstants.h>

CONSTANT_BUFFER(ezRazorConstants, 4)
{
  FLOAT2(RazorViewportSize);
  FLOAT2(RazorAtlasSize);
};
