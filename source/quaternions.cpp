namespace Quaternions
{

    void
    identity(Quaternion *const q)
    {
        q->component.x = 0.0f;
        q->component.y = 0.0f;
        q->component.z = 0.0f;
        q->component.w = 1.0f;
    }

    void
    zero(Quaternion *const q)
    {
        q->component.x = 0.0f;
        q->component.y = 0.0f;
        q->component.z = 0.0f;
        q->component.w = 0.0f;
    }    

    void
    vector(Vec3 const*const v, Quaternion *const q)
    {
        q->part.vector = *v;
        q->component.w = 0.0f;
    }

    void
    scalar(float const s, Quaternion *const q)
    {
        q->part.vector = {};
        q->component.w = s;
    }    
    
    void
    product(Quaternion const*const p, Quaternion const*const q, Quaternion *const r)
    {
        Vector3::cross_product(&p->part.vector, &q->part.vector, &r->part.vector);
        Vector3::scale_add(p->part.scalar, &q->part.vector, &r->part.vector);
        Vector3::scale_add(q->part.scalar, &p->part.vector, &r->part.vector);
        r->part.scalar = p->part.scalar*q->part.scalar - Vector3::inner_product(&p->part.vector, &q->part.vector);
    }    

    void
    sum(Quaternion const*const p, Quaternion const*const q, Quaternion *const r)
    {
        r->component.x = p->component.x + q->component.x;
        r->component.y = p->component.y + q->component.y;
        r->component.z = p->component.z + q->component.z;
        r->component.w = p->component.w + q->component.w;
    }
    
}
