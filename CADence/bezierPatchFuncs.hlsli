

float3 Bezier2(float3 b0, float3 b1, float t)
{
    float3 ptRes = lerp(b0, b1, t);
    return ptRes;
}

float3 Bezier3(float3 b0, float3 b1, float3 b2, float t)
{
    float3 pt1 = Bezier2(b0, b1, t);
    float3 pt2 = Bezier2(b1, b2, t);
    float3 ptRes = lerp(pt1, pt2, t);
    return ptRes;
}

// Calculates value of a polynomial in Bernstein Basis using the De Casteljau alg.
float3 DeCastel(float3 b0, float3 b1, float3 b2, float3 b3, float t)
{
    float3 pt1 = Bezier3(b0, b1, b2, t);
    float3 pt2 = Bezier3(b1, b2, b3, t);
    float3 ptRes = lerp(pt1, pt2, t);
    return ptRes;
}

float3 DeCastelDeBoor(float3 b0, float3 b1, float3 b2, float3 b3, float t)
{
    float3 first, second, third, last;
    
    float3 aux1 = lerp(b0, b1, 2.f / 3.f);
    float3 aux2 = lerp(b2, b3, 1.f / 3.f);
    second = lerp(b1, b2, 1.f / 3.f);
    third = lerp(b1, b2, 2.f / 3.f);
    first = lerp(aux1, second, 0.5f);
    last = lerp(third, aux2, 0.5f);
        
    return DeCastel(first, second, third, last, t);
}

float3 BezierC2PatchPoint(float3 controlPts[16], float2 uv)
{
    float3 u0 = DeCastelDeBoor(controlPts[0], controlPts[1], controlPts[2], controlPts[3], uv.x);
    float3 u1 = DeCastelDeBoor(controlPts[4], controlPts[5], controlPts[6], controlPts[7], uv.x);
    float3 u2 = DeCastelDeBoor(controlPts[8], controlPts[9], controlPts[10], controlPts[11], uv.x);
    float3 u3 = DeCastelDeBoor(controlPts[12], controlPts[13], controlPts[14], controlPts[15], uv.x);
    
    float3 res = DeCastelDeBoor(u0, u1, u2, u3, uv.y);
    return res;
}

// Calculates a point on a Bezier patch described by Bernstein polynomials
float3 BezierPatchPoint(float3 controlPts[16], float2 uv)
{
    float3 u0 = DeCastel(controlPts[0], controlPts[1], controlPts[2], controlPts[3], uv.x);
    float3 u1 = DeCastel(controlPts[4], controlPts[5], controlPts[6], controlPts[7], uv.x);
    float3 u2 = DeCastel(controlPts[8], controlPts[9], controlPts[10], controlPts[11], uv.x);
    float3 u3 = DeCastel(controlPts[12], controlPts[13], controlPts[14], controlPts[15], uv.x);
    
    float3 res = DeCastel(u0, u1, u2, u3, uv.y);
    return res;
}

float3 BezierPatchTangent(float3 controlPts[16], float2 uv)
{
    float3 u0 = DeCastel(controlPts[0], controlPts[1], controlPts[2], controlPts[3], uv.x);
    float3 u1 = DeCastel(controlPts[4], controlPts[5], controlPts[6], controlPts[7], uv.x);
    float3 u2 = DeCastel(controlPts[8], controlPts[9], controlPts[10], controlPts[11], uv.x);
    float3 u3 = DeCastel(controlPts[12], controlPts[13], controlPts[14], controlPts[15], uv.x);
    return Bezier3(u1, u2, u3, uv.y) - Bezier3(u0, u1, u2, uv.y);
}

float3 BezierPatchBitangent(float3 controlPts[16], float2 uv)
{
    float3 v0 = DeCastel(controlPts[0], controlPts[4], controlPts[8], controlPts[12], uv.y);
    float3 v1 = DeCastel(controlPts[1], controlPts[5], controlPts[9], controlPts[13], uv.y);
    float3 v2 = DeCastel(controlPts[2], controlPts[6], controlPts[10], controlPts[14], uv.y);
    float3 v3 = DeCastel(controlPts[3], controlPts[7], controlPts[11], controlPts[15], uv.y);
    return Bezier3(v1, v2, v3, uv.x) - Bezier3(v0, v1, v2, uv.x);
}

float3 BezierPatchC2Tangent(float3 controlPts[16], float2 uv)
{        
    float u = uv.x;
    float v = uv.y;
    
    float3 aux0 = DeCastelDeBoor(controlPts[0], controlPts[4], controlPts[8], controlPts[12], v);
    float3 aux1 = DeCastelDeBoor(controlPts[1], controlPts[5], controlPts[9], controlPts[13], v);
    float3 aux2 = DeCastelDeBoor(controlPts[2], controlPts[6], controlPts[10], controlPts[14], v);
    float3 aux3 = DeCastelDeBoor(controlPts[3], controlPts[7], controlPts[11], controlPts[15], v);
    
    // Convert Aux De Boors to beziers    
    float3 bez0, bez1, bez2, bez3;
    float3 auxb1 = lerp(aux0, aux1, 2.f / 3.f);
    float3 auxb2 = lerp(aux2, aux3, 1.f / 3.f);
    
    bez1 = lerp(aux1, aux2, 1.f / 3.f);
    bez2 = lerp(aux1, aux2, 2.f / 3.f);
    bez0 = lerp(auxb1, bez1, 0.5f);
    bez3 = lerp(bez2, auxb2, 0.5f);
    
    float3 der1 = 3 * (bez1 - bez0);
    float3 der2 = 3 * (bez2 - bez1);
    float3 der3 = 3 * (bez3 - bez2);
    
    return Bezier3(der1, der2, der3, u);
}

float3 BezierPatchC2Bitangent(float3 controlPts[16], float2 uv)
{
    float u = uv.x;
    float v = uv.y;
    
    float3 aux0 = DeCastelDeBoor(controlPts[0], controlPts[1], controlPts[2], controlPts[3], u);
    float3 aux1 = DeCastelDeBoor(controlPts[4], controlPts[5], controlPts[6], controlPts[7], u);
    float3 aux2 = DeCastelDeBoor(controlPts[8], controlPts[9], controlPts[10], controlPts[11], u);
    float3 aux3 = DeCastelDeBoor(controlPts[12], controlPts[13], controlPts[14], controlPts[15], u);
    
    // Convert Aux De Boors to beziers    
    float3 bez0, bez1, bez2, bez3;    
    float3 auxb1 = lerp(aux0, aux1, 2.f / 3.f);
    float3 auxb2 = lerp(aux2, aux3, 1.f / 3.f);    
    bez1 = lerp(aux1, aux2, 1.f / 3.f);
    bez2 = lerp(aux1, aux2, 2.f / 3.f);
    
    bez0 = lerp(auxb1, bez1, 0.5f);
    bez3 = lerp(bez2, auxb2, 0.5f);
    
    float3 der1 = 3 * (bez1 - bez0);
    float3 der2 = 3 * (bez2 - bez1);
    float3 der3 = 3 * (bez3 - bez2);
    
    return Bezier3(der1, der2, der3, v);
}


// Temp solution - TODO incorporate in regular de Casteljau 
float3 BezierPatchNormal(float3 controlPts[16], float2 uv)
{
    // Calculate tangent in u direction    
    float3 tanU = BezierPatchTangent(controlPts, uv);
    // Calculate tangent in v direction
    float3 tanV = BezierPatchBitangent(controlPts, uv);
        
    // Calculate normal
    float3 normal = cross(tanU, tanV);
    return normal;
}

float3 BezierPatchC2Normal(float3 controlPts[16], float2 uv)
{
    float3 tanU = BezierPatchC2Tangent(controlPts, uv);
    float3 tanV = BezierPatchC2Bitangent(controlPts, uv);

    float3 normal = cross(tanV, tanU);
    return normal;
}