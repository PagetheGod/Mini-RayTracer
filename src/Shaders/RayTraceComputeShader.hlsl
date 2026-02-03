cbuffer GlobalBuffer
{
    float3 CameraPos;
    float3 ViewportUpperLeft;
    float3 FirstPixelPos;
    float3 DeltaU;
    float3 DeltaV;
    float Padding;
};


struct SphereTransformType
{
    float3 SphereCenter;
    float Radius;
};

struct SphereMaterialType
{
    float3 Albedo;
    float Fuzz;
    float RefractionIndex;
    uint Type;
};

StructuredBuffer<SphereTransformType> SphereTransformBuffer : register(t0);
StructuredBuffer<SphereMaterialType> SphereMaterialBuffer : register(t1);

//Let's try the frequently suggested 8x8(64-pixel tile) render.
//The numthreads can be expressed in 3D, 2D, or 1D manner, which is why it has three components
/*8x8 matches nicely to a 64 pixel tile. The details are:
* We have three level indexing in compute shader threads. First a group ID, that indicates which group of the threads we belong to
* In our case, it indicates which 8x8 tile we are currently in
* Then we have an GroupThread ID, it tells us where we are inside our group.
* The SV_DispatchThreadID is then calculated using GroupID * num threads(per component) + GroupThreadID
* Then the result can be retrieved from DTid.xy. 
* Example: we are at the (100, 50) pixel. We can get that by 12(12th tile in x) * 8 + 4(group thread ID) = 100
* and (6th tile in y) * 8 + 2(group thread ID) = 50
*/

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 PixelXY = DTid.xy;
}