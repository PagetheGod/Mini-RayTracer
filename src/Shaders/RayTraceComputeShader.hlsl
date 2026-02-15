//Technically we can just move the uint data upwards but I counted and it seems it doesn't save any space
cbuffer GlobalBuffer
{
    float3 CameraPos;
    uint Padding1;
    float3 ViewportUpperLeft;
    uint Padding2;
    float3 FirstPixelPos;
    uint Padding3;
    float3 DeltaU;
    uint Padding4;
    float3 DeltaV;
    uint ObjectCount;
    uint2 ScreenSize; //Screen width, height
    uint Depth;
    uint SampleCount;
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

struct RandomState
{
    uint Seed;
};

StructuredBuffer<SphereTransformType> SphereTransformBuffer : register(t0);
StructuredBuffer<SphereMaterialType> SphereMaterialBuffer : register(t1);
RWTexture2D<float4> OutputBuffer : register(u0);

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

//Random number helpers
uint PCGHash(uint Seed)
{
    //A nice and simple hash function that acts as a random number generator. 
    //Good balance between performance and quality. I don't understand math magic here
    //Note that PCG is not a single function. It's a family of hashing functions(or recipe to make hashing functions)
    uint State = Seed * 747796405u + 2891336453u;
    uint Word = ((State >> ((State >> 28u) + 4u)) ^ State) * 277803737u;
    return (Word >> 22u) ^ Word;
}


//According to claude, FNV1a is not good in this context as it can produce lots of noises(stripes and patterns due to low quality)
//But I tried it out using like <= 5 samples and it looks pretty much the same as PCG
uint FNV1AHash(uint Seed)
{
    const uint FNVPrime = 16777619u;
    const uint OffsetBasis = 2166136261u;
    uint Hash = OffsetBasis;
    Hash ^= Seed;
    Hash *= FNVPrime;
    return Hash;
}

void InitRandomState(inout RandomState State, uint2 PixelXY, uint SampleIndex, uint Width, uint Height)
{
    State.Seed = PixelXY.x + PixelXY.y * Width + SampleIndex * Width * Height;
}

float RandomFloat(inout RandomState State)
{
    State.Seed = PCGHash(State.Seed);
    return (State.Seed / 4294967295.0) * 2.f - 1.f; //Map to [-1, 1]
}

//Helper to generate a float within an interval
//The interval should always be symmetric about 0, otherwise we need to do some extra work to map the result to the desired range.
float RandomFloatInterval(inout RandomState State, float Interval)
{
    State.Seed = PCGHash(State.Seed);
    return (State.Seed / 4294967295.0) * 2.f * Interval - Interval; //Map to [-Interval, Interval]
}

float3 RandomUnitVector(inout RandomState State)
{
    float3 Vec;
    while (true)
    {
        Vec = float3(RandomFloat(State), RandomFloat(State), RandomFloat(State));
        float LengthSquared = dot(Vec, Vec);
        if (LengthSquared > 1e-38 && LengthSquared <= 1.f)
        {
            break;
        }
    }
    return normalize(Vec);
}

//Helpers
bool IsNearZero(float3 Vec)
{
    const float Epsilon = 1e-8f;
    return (abs(Vec.x) <= Epsilon) && (abs(Vec.y) <= Epsilon) && (abs(Vec.z) <= Epsilon);
}

bool SphereHit(const Ray R, float Min, float Max, const float3 Center, const float Radius, inout HitRecord OutHitRecord)
{
    float3 RayOrigin = R.Origin;
    float3 RayDirection = R.Direction;
    float3 RayOriToCenter = Center - RayOrigin;
    
    float a = dot(RayDirection, RayDirection);
    float h = dot(RayDirection, RayOriToCenter);
    float c = dot(RayOriToCenter, RayOriToCenter) - Radius * Radius;
    
    float Discriminant = h * h - a * c;
    if (Discriminant < 0.f)
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
    OutHitRecord.MaterialType = 0; //Default to lambertian because we need to fully initialize. The actual material type will be set in the HitWorld function.
    OutHitRecord.Normal = (OutHitRecord.HitPoint - Center) / Radius; //Outward normal
    OutHitRecord.IsFrontFace = dot(RayDirection, OutHitRecord.Normal) < 0.f;
    OutHitRecord.Normal = OutHitRecord.IsFrontFace ? OutHitRecord.Normal : -OutHitRecord.Normal;
    return true;
}


bool HitWorld(const Ray R, float Min, float Max, inout HitRecord OutHitRecord, inout MaterialScatterData OutScatterData)
{
    bool HasHit = false;
    float ClosestSoFar = Max;
    for (uint i = 0; i < ObjectCount; i++)
    {
        if (SphereHit(R, Min, ClosestSoFar, SphereTransformBuffer[i].SphereCenter, SphereTransformBuffer[i].Radius, OutHitRecord))
        {
            HasHit = true;
            ClosestSoFar = OutHitRecord.t;
            OutScatterData.Attenuation = SphereMaterialBuffer[i].Albedo;
            OutScatterData.FuzzOrRI = SphereMaterialBuffer[i].FuzzOrRI;
            OutHitRecord.MaterialType = SphereMaterialBuffer[i].Type;
        }

    }
    return HasHit;
}


bool LambertianScatter(const Ray R, inout float4 OutAttenuation, const HitRecord InHitRecord, const MaterialScatterData InScatterData, inout Ray ScatteredRay, inout RandomState RandState)
{
    float3 ScatterDirection = InHitRecord.Normal + RandomUnitVector(RandState);
    if (IsNearZero(ScatterDirection))
    {
        ScatterDirection = InHitRecord.Normal;
    }
    
    ScatteredRay.Origin = InHitRecord.HitPoint;
    ScatteredRay.Direction = ScatterDirection;
    
    OutAttenuation = float4(InScatterData.Attenuation, 1.f);
    return true;
}

bool MetalicScatter(const Ray R, inout float4 OutAttenuation, const HitRecord InHitRecord, const MaterialScatterData InScatterData, inout Ray ScatteredRay, inout RandomState RandState)
{
    float3 Reflected = reflect(normalize(R.Direction), InHitRecord.Normal);
    float3 ScatteredDirection = Reflected + InScatterData.FuzzOrRI * RandomUnitVector(RandState);
    ScatteredRay.Origin = InHitRecord.HitPoint;
    ScatteredRay.Direction = ScatteredDirection;
    OutAttenuation = float4(InScatterData.Attenuation, 1.f);
    return true;
}

bool DielectricScatter(const Ray R, inout float4 OutAttenuation, const HitRecord InHitRecord, const MaterialScatterData InScatterData, inout Ray ScatteredRay, inout RandomState RandState)
{
    OutAttenuation = float4(1.f, 1.f, 1.f, 1.f);
    
    const float3 UnitDirection = normalize(R.Direction);
    
    float RelativeRI = InHitRecord.IsFrontFace ? (1.f / InScatterData.FuzzOrRI) : InScatterData.FuzzOrRI;
    
    float CosTheta = min(dot(-UnitDirection, InHitRecord.Normal), 1.f);
    float SinTheta = sqrt(1.f - CosTheta * CosTheta);
    
    bool CanRefract = RelativeRI * SinTheta <= 1.f;
    float3 Direction;
    
    if (CanRefract)
    {
        Direction = refract(UnitDirection, InHitRecord.Normal, RelativeRI);
    }
    else
    {
        Direction = reflect(UnitDirection, InHitRecord.Normal);
    }
    
    ScatteredRay.Origin = InHitRecord.HitPoint;
    ScatteredRay.Direction = Direction;
    return true;
}


bool DispatchScatter(const Ray R, inout float4 OutAttenuation, const HitRecord InHitRecord, const MaterialScatterData InScatterData, inout Ray ScatteredRay, inout RandomState RandState)
{
    switch (InHitRecord.MaterialType)
    {
        case 0:
            return LambertianScatter(R, OutAttenuation, InHitRecord, InScatterData, ScatteredRay, RandState);
        case 1:
            return MetalicScatter(R, OutAttenuation, InHitRecord, InScatterData, ScatteredRay, RandState);
        case 2:
            return DielectricScatter(R, OutAttenuation, InHitRecord, InScatterData, ScatteredRay, RandState);
        default:
            return false;
    }
}


float4 PerformPathTrace(const Ray R, inout RandomState RandState)
{
    float4 Color = float4(0.f, 0.f, 0.f, 1.f);
    HitRecord TempHitRecord;
    MaterialScatterData TempScatterData;
    Ray CurrentRay = R;
    float4 TotalAttenuation = float4(1.f, 1.f, 1.f, 1.f);
    
    for (uint i = 0; i < Depth; i++)
    {
        if (HitWorld(CurrentRay, 0.001f, 1.#INF, TempHitRecord, TempScatterData))
        {
            float4 Attenuation;
            Ray ScatteredRay;
            if (DispatchScatter(CurrentRay, Attenuation, TempHitRecord, TempScatterData, ScatteredRay, RandState))
            {
                TotalAttenuation *= Attenuation;
                CurrentRay = ScatteredRay;
            }
            else
            {
                return float4(0.f, 0.f, 0.f, 1.f);
            }
        }
        else
        {
            //Background color, simple gradient based on ray direction
            float3 UnitDirection = normalize(CurrentRay.Direction);
            float t = 0.5f * (UnitDirection.y + 1.f);
            Color += ((1.f - t) * float4(0.9f, 0.9f, 0.9f, 1.f) + t * float4(0.5f, 0.7f, 1.f, 1.f));
            return TotalAttenuation * Color;
        
        }
    }
    
    return float4(0.f, 0.f, 0.f, 1.f);
}





//Probably far from an efficient shader due to lots of branchings
[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    //Set up the pixel position and color
    float2 PixelXY = DTid.xy;
    float3 PixelPos = FirstPixelPos + PixelXY.x * DeltaU + PixelXY.y * DeltaV;
    float4 PixelColor = float4(0.f, 0.f, 0.f, 1.f);
    const float SampleScaleFactor = 1.f / SampleCount;
    
    for (uint i = 0; i < SampleCount; i++)
    {   
        RandomState State;
        InitRandomState(State, PixelXY, i, ScreenSize.x, ScreenSize.y);
        
        float2 Offset = float2(RandomFloatInterval(State, 0.5f), RandomFloatInterval(State, 0.5f));
        float3 CurrentSample = PixelPos + Offset.x * DeltaU + Offset.y * DeltaV;
        float3 RayDir = normalize(CurrentSample - CameraPos);
        Ray CurrentRay;
        CurrentRay.Direction = RayDir;
        CurrentRay.Origin = CameraPos;
        
        PixelColor += PerformPathTrace(CurrentRay, State);
    }
    
    PixelColor *= SampleScaleFactor;
    PixelColor = saturate(PixelColor);
    OutputBuffer[DTid.xy] = PixelColor;
}


