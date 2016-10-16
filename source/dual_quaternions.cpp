namespace DualQuaternions
{

    void
    identity(DualQuaternion *const r)
    {
        *r = {};
        r->part.real.part.scalar = 1.0f;
    }

    void
    zero(DualQuaternion *const r)
    {
        *r = {};
    }
    
    
    void
    product(
        DualQuaternion const*const p,
        DualQuaternion const*const q,
        DualQuaternion *const r
        )
    {

        // NOTE: real part
        Quaternions::product(&p->part.real, &q->part.real, &r->part.real);
        // NOTE: non-real part
        Quaternions::Quaternion t0;
        Quaternions::product(&p->part.real, &q->part.non_real, &t0);
        Quaternions::Quaternion t1;
        Quaternions::product(&p->part.non_real, &q->part.real, &t1);
        Quaternions::sum(&t0, &t1, &r->part.non_real);
    }

    void
    product(
        DualQuaternion const*const a,
        DualQuaternion const*const b,
        DualQuaternion const*const c,
        DualQuaternion *const r
        )
    {
        DualQuaternion t;
        product(a, b, &t);
        product(&t, c, r);
    }    

    void
    product(
        DualQuaternion const*const a,
        DualQuaternion const*const b,
        DualQuaternion const*const c,
        DualQuaternion const*const d,
        DualQuaternion *const r
        )
    {
        
        DualQuaternion t1;
        product(a, b, &t1);
        
        DualQuaternion t2;
        product(c, d, &t2);
        
        product(&t1, &t2, r);
    }    
    
    
}
