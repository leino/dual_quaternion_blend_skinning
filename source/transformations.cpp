namespace Transformations
{

    using namespace Quaternions;
    using namespace DualQuaternions;
    using namespace Vector3;
    
    void
    rotation_axis_angle(Vec3 *const axis, float const angle, Quaternion *const q)
    {
        float const half_angle = angle/2.0f;
        q->part.vector = *axis;
        scale(Numerics::sin(half_angle), &q->part.vector);
        q->part.scalar = Numerics::cos(half_angle);
    }
    
    void
    rotation_x_axis(float const angle, Quaternion *const q)
    {
        float const half_angle = angle/2.0f;
        q->part.vector.coordinate.x = Numerics::sin(half_angle);
        q->part.vector.coordinate.y = 0.0f;
        q->part.vector.coordinate.z = 0.0f;
        q->part.scalar = Numerics::cos(half_angle);
    }
    
    void
    rotation_y_axis(float const angle, Quaternion *const q)
    {
        float const half_angle = angle/2.0f;
        q->part.vector.coordinate.x = 0.0f;
        q->part.vector.coordinate.y = Numerics::sin(half_angle);
        q->part.vector.coordinate.z = 0.0f;
        q->part.scalar = Numerics::cos(half_angle);
    }    

    void
    rotation_z_axis(float const angle, Quaternion *const q)
    {
        float const half_angle = angle/2.0f;
        q->part.vector.coordinate.x = 0.0f;
        q->part.vector.coordinate.y = 0.0f;
        q->part.vector.coordinate.z = Numerics::sin(half_angle);
        q->part.scalar = Numerics::cos(half_angle);
    }        

    void
    rotation_x_axis(float const angle, DualQuaternion *const dq)
    {
        float const half_angle = angle/2.0f;
        Quaternion *const q = &dq->part.real;
        q->part.vector.coordinate.x = Numerics::sin(half_angle);
        q->part.vector.coordinate.y = 0.0f;
        q->part.vector.coordinate.z = 0.0f;
        q->part.scalar = Numerics::cos(half_angle);
        dq->part.non_real = {};
    }    

    void
    rotation_y_axis(float const angle, DualQuaternion *const dq)
    {
        float const half_angle = angle/2.0f;
        Quaternion *const q = &dq->part.real;
        q->part.vector.coordinate.x = 0.0f;
        q->part.vector.coordinate.y = Numerics::sin(half_angle);
        q->part.vector.coordinate.z = 0.0f;
        q->part.scalar = Numerics::cos(half_angle);
        dq->part.non_real = {};
    }    

    void
    rotation_z_axis(float const angle, DualQuaternion *const dq)
    {
        float const half_angle = angle/2.0f;
        Quaternion *const q = &dq->part.real;
        q->part.vector.coordinate.x = 0.0f;
        q->part.vector.coordinate.y = 0.0f;
        q->part.vector.coordinate.z = Numerics::sin(half_angle);
        q->part.scalar = Numerics::cos(half_angle);
        dq->part.non_real = {};
    }    

    void
    translation_x_axis(float const distance, DualQuaternion *const dq)
    {
        *dq = {};
        dq->part.real.part.scalar = 1.0f;
        dq->part.non_real.part.vector.coordinate.x = distance/2.0f;
    }

    void
    translation_y_axis(float const distance, DualQuaternion *const dq)
    {
        *dq = {};
        dq->part.real.part.scalar = 1.0f;
        dq->part.non_real.part.vector.coordinate.y = distance/2.0f;
    }

    void
    translation_z_axis(float const distance, DualQuaternion *const dq)
    {
        *dq = {};
        dq->part.real.part.scalar = 1.0f;
        dq->part.non_real.part.vector.coordinate.z = distance/2.0f;
    }

}
