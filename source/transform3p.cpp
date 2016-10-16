namespace Transform3p
{

    // NOTE:
    // All transforms are stored row major, and made to operate on column vectors.
    // A left handed coordinate system is assumed.
    
    using namespace Vector3;
    using namespace Matrix4;
    using namespace Numerics;

    inline void
    scaling(float const x_scale, float const y_scale, float const z_scale, Mat4 *const matrix)
    {
        *matrix = 
            {
                x_scale,    0.0f,    0.0f,    0.0f,
                0.0f,    y_scale,    0.0f,    0.0f,
                0.0f,       0.0f, z_scale,    0.0f,
                0.0f,       0.0f,    0.0f,    1.0f
            };
    }

    inline void
    rotation_x(float const angle, Mat4 *const matrix)
    {
        float const c = Numerics::cos(angle);
        float const s = Numerics::sin(angle);
        
        *matrix = 
            {
                1,  0,  0, 0.0f,
                0,  c, -s, 0.0f,
                0,  s,  c, 0.0f,
                0,  0,  0, 1.0f
            };
        
    }    

    inline void
    rotation_y(float const angle, Mat4 *const matrix)
    {
        float const c = Numerics::cos(angle);
        float const s = Numerics::sin(angle);
        
        *matrix = 
            {
                c,  0,  s, 0.0f,
                0,  1,  0, 0.0f,
               -s,  0,  c, 0.0f,
                0,  0,  0, 1.0f
            };
        
    }
    
    inline void
    rotation_z(float const angle, Mat4 *const matrix)
    {
        float const c = Numerics::cos(angle);
        float const s = Numerics::sin(angle);
        
        *matrix = 
            {
                c, -s, 0, 0.0f,
                s,  c, 0, 0.0f,
                0,  0, 1, 0.0f,
                0,  0, 0, 1.0f
            };
        
    }    
    
    inline void
    lookat(Vec3 const*const eye, Vec3 const*const target, Vec3 const*const up, Mat4 *const matrix)
    {

        Vec3 look_direction;
        difference(target, eye, &look_direction);
                
        Vec3 z_axis;
        normalized(&look_direction, &z_axis);

        Vec3 x_axis;
        cross_product(up, &z_axis, &x_axis);
        normalize(&x_axis);
                
        Vec3 y_axis;
        cross_product(&z_axis, &x_axis, &y_axis);
                
        float const eye_x = inner_product(&x_axis, eye);
        float const eye_y = inner_product(&y_axis, eye);
        float const eye_z = inner_product(&z_axis, eye);
                
        *matrix = 
            {
                +x_axis.coordinate.x, +x_axis.coordinate.y, +x_axis.coordinate.z, -eye_x,
                +y_axis.coordinate.x, +y_axis.coordinate.y, +y_axis.coordinate.z, -eye_y,
                +z_axis.coordinate.x, +z_axis.coordinate.y, +z_axis.coordinate.z, -eye_z,
                                0.0f,                 0.0f,                 0.0f,   1.0f
            };
        
    }

    // NOTE:
    // This transform will cause the frustum described by
    // horizontal_fov, near_z and far_z to be squeezed into
    // [-1,+1]^2 x [0,1]
    // after the perspective division.
    // NOTE:
    // Aspect ratio means width divided by height
    inline void
    perspective_projection(
        float const horizontal_fov, float const aspect_ratio,
        float const near_z, float const far_z,
        Mat4 *const matrix
        )
    {

        // TODO: cot?
        float const w = 1.0f / tan_float(horizontal_fov * 0.5f);
        float const h = w * aspect_ratio;
        float const Q = far_z/(far_z - near_z);
        float const N =  -Q*near_z;
                
        *matrix =
            {
                w, 0, 0, 0,
                0, h, 0, 0,
                0, 0, Q, N,
                0, 0, 1, 0
            };
        
    }
    
}
