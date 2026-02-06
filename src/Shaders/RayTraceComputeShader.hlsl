#define Depth 50
#define SampleCount 100

cbuffer GlobalBuffer
{
    float3 CameraPos;
    float3 ViewportUpperLeft;
    float3 FirstPixelPos;
    float3 DeltaU;
    float3 DeltaV;
    uint ObjectCount;
};

cbuffer SampleOffsetBuffer
{
    float2 SampleOffsets[SampleCount];
};

struct SphereTransformType
{
    float3 SphereCenter;
    float Radius;
};

struct SphereMaterialType
{
    float3 Albedo;
    float FuzzOrRI;
    uint Type; //0:lambertian, 1:metal, 2:dielectric
};

//Non buffer struct type
struct Ray
{
    float3 Origin;
    float3 Direction;
};

struct HitRecord
{
    float3 HitPoint;
    float3 Normal;
    float t;
    bool IsFrontFace;
    uint MaterialType;
};

struct MaterialScatterData
{
    float FuzzOrRI;
    float3 Attenuation;
};

StructuredBuffer<SphereTransformType> SphereTransformBuffer : register(t0);
StructuredBuffer<SphereMaterialType> SphereMaterialBuffer : register(t1);
//RWStructuredBuffer;

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
    //Set up the pixel position and color
    float2 PixelXY = DTid.xy;
    float3 PixelPos = FirstPixelPos + PixelXY.x * DeltaU + PixelXY.y * DeltaV;
    float4 Color = float4(0.f, 0.f, 0.f, 1.f);
    const float SampleScaleFactor = 1.f / SampleCount;
    
    for (int i = 0; i < SampleCount; i++)
    {
        float3 CurrentSample = PixelPos + SampleOffsets[i].x * DeltaU + SampleOffsets[i].y * DeltaV;
        float3 RayDir = normalize(CurrentSample - CameraPos);
        Ray CurrentRay;
        CurrentRay.Direction = RayDir;
        CurrentRay.Origin = CameraPos;
    }

}


//Helpers
float4 PerformPathTrace(const Ray R)
{
    float4 Color = float4(0.f, 0.f, 0.f, 1.f);
    
    
    return Color;
}

bool SphereHit(const Ray R, float Min, float Max, const float3 Center, const float Radius, inout HitRecord OutHitRecord)
{
    float3 RayOrigin = R.Origin;
    float3 RayDirection = R.Direction;
    float3 RayOriToCenter = RayOrigin - Center;
    
    float a = dot(RayDirection, RayDirection);
    float h = dot(RayDirection, RayOriToCenter);
    float c = dot(RayOriToCenter, RayOriToCenter) - Radius * Radius;
    
    float Discriminant = h * h - a * c;
    if(Discriminant < 0.f)
    {
        return false;
    }
    
    float SqrtD = sqrt(Discriminant);
    float Root = (h - SqrtD) / a;
    
    if (Root < Min || Root > Max)
    {
        Root = (h + SqrtD) / a;
        if (Root < Min || Root > Max)
        {
            return false;
        }
    }
    
    OutHitRecord.HitPoint = RayOrigin + Root * RayDirection;
    OutHitRecord.t = Root;
    OutHitRecord.Normal = (OutHitRecord.HitPoint - Center) / Radius;//Outward normal
    OutHitRecord.IsFrontFace = dot(RayDirection, OutHitRecord.Normal) < 0.f;
    OutHitRecord.Normal = OutHitRecord.IsFrontFace ? OutHitRecord.Normal : -OutHitRecord.Normal;
    
}


bool HitWorld(const Ray R, float Min, float Max, inout HitRecord OutHitRecord, inout OutScatterData)
{
    bool HasHit = false;
    float ClosestSoFar = Max;
    for (int i = 0; i < ObjectCount; i++)
    {
        if (SphereHit(R, Min, ClosestSoFar, SphereTransformBuffer[i].SphereCenter, SphereTransformBuffer[i].Radius, OutHitRecord))
        {
            
        }

    }

}