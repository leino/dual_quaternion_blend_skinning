namespace Vector4
{

    float
    lerp(float const from, float const to, float const t)
    {
        return from + (to - from)*t;
    }
    
    void
    lerp(Vec4 const*const from, Vec4 const*const to, float const t, Vec4 *const result)
    {
        result->coordinates[0] = lerp(from->coordinates[0], to->coordinates[0], t);
        result->coordinates[1] = lerp(from->coordinates[1], to->coordinates[1], t);
        result->coordinates[2] = lerp(from->coordinates[2], to->coordinates[2], t);
        result->coordinates[3] = lerp(from->coordinates[3], to->coordinates[3], t);
    }
    
}
