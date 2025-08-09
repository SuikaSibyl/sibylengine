RWTexture2D<float4> rw_output;

[shader("compute")]
[numthreads(32, 4, 1)]
void ComputeMain(
    int3 dtid: SV_DispatchThreadID,
    int3 gtid: SV_GroupThreadID,
    int3 gid: SV_GroupID,
    int gi: SV_GroupIndex
) {
    // The resolution of the image, which is the same as the launch size:
    const int2 resolution = int2(512, 512);
    const int2 pixel = int2(dtid.xy);
    // If the pixel is outside of the image, don't do anything:
    if (any(pixel >= resolution)) return;
    
    rw_output[pixel] = float4(float2(pixel + 0.5) / resolution, 1, 1);
}