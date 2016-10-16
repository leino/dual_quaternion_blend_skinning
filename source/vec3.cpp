namespace Vector3
{
    
    inline void
    difference(Vec3 const*const a, Vec3 const*const b, Vec3 *const difference)
    {
        
        *difference =
            {
                a->coordinates[0] - b->coordinates[0],
                a->coordinates[1] - b->coordinates[1],
                a->coordinates[2] - b->coordinates[2]
            };
        
    }

    inline void
    cross_product(Vec3 const*const a, Vec3 const*const b, Vec3 *const cross_product)
    {
        
        *cross_product =
            {
                a->coordinates[1]*b->coordinates[2] - a->coordinates[2]*b->coordinates[1],
                a->coordinates[2]*b->coordinates[0] - a->coordinates[0]*b->coordinates[2],
                a->coordinates[0]*b->coordinates[1] - a->coordinates[1]*b->coordinates[0]
            };
        
    }    

    float
    inner_product(Vec3 const*const a, Vec3 const*const b)
    {        
        return
            a->coordinates[0]*b->coordinates[0] +
            a->coordinates[1]*b->coordinates[1] +
            a->coordinates[2]*b->coordinates[2]
            ;
    }
    
    inline void
    scale(float const a, Vec3 *const v)
    {

        *v =
            {
                a * v->coordinates[0],
                a * v->coordinates[1],
                a * v->coordinates[2]
            };
        
    }

    inline void
    scale_add(float const a, Vec3 const*const v, Vec3 *const w)
    {
        w->coordinates[0] += a*v->coordinates[0];
        w->coordinates[1] += a*v->coordinates[1];
        w->coordinates[2] += a*v->coordinates[2];
        w->coordinates[3] += a*v->coordinates[3];
    }
    
    inline float
    length_squared(Vec3 const*const v)
    {
        
        using namespace Numerics;
        
        return
            square(v->coordinates[0]) + 
            square(v->coordinates[1]) +
            square(v->coordinates[2]);
        
    }

    inline float
    length(Vec3 const*const v)
    {

        return Numerics::square_root( length_squared(v) );
        
    }    
    
    inline void
    normalize(Vec3 *const a)
    {

        float const a_length = length(a);
        scale(1.0f / a_length, a);
        
    }    

    inline void
    normalized(Vec3 const*const a, Vec3 *const r)
    {
        *r = *a;
        normalize(r);
    }
    
}
