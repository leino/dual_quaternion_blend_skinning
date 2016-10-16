namespace Vector2
{

    inline void
    difference(Vec2 const*const a, Vec2 const*const b, Vec2 *const result)
    {
        result->coordinates[0] = a->coordinates[0] - b->coordinates[0];
        result->coordinates[1] = a->coordinates[1] - b->coordinates[1];
    }

    inline float
    length_squared(Vec2 const*const a)
    {
        float result =
            a->coordinates[0]*a->coordinates[0] +
            a->coordinates[1]*a->coordinates[1];
        return result;
    }

    inline float
    distance_squared(Vec2 const*const a, Vec2 const*const b)
    {
        Vec2 d;
        difference(a, b, &d);
        return length_squared(&d);
    }

    inline void
    linear_combination_2(float const a, Vec2 const*const u, float const b, Vec2 const*const v, Vec2 *const result)
    {
        *result =
            {
                a * u->coordinate.x + b * v->coordinate.x,
                a * u->coordinate.y + b * v->coordinate.y
            };
    }
    
}
