RWStructuredBuffer<float> rw_output;

[shader("compute")]
[numthreads(256, 1, 1)]
void ComputeMain(
    int3 dtid: SV_DispatchThreadID,
    int3 gtid: SV_GroupThreadID,
    int3 gid: SV_GroupID,
    int gi: SV_GroupIndex
) {
    const uint threadID = dtid.x;
    rw_output[threadID] = rw_output[threadID] * rw_output[threadID];
}