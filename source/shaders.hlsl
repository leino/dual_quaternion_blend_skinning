static const float PI = 3.14159265f;

struct DualQuaternion
{
    float4 real;
    float4 non_real;
};

cbuffer Transform : register(b0)
{
    DualQuaternion model_to_world_transform[2];
};

struct Vertex
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 weights : BLENDWEIGHT;
    float2 position_texture : TEXCOORD;
};

struct FlatVertex
{
    float4 position : POSITION;
    float4 bone_weights : BLENDWEIGHT;
};

struct ScreenVertex
{
    float4 position_viewport : SV_POSITION;
    float2 position_texture : TEXCOORD;
    float intensity : COLOR;
};

struct FlatScreenVertex
{
    float4 position_viewport : SV_POSITION;
};

static float4 camera_position_world = float4(0,0,-6,0);

Texture2D color_texture;
SamplerState state;

float3
quaternion_vector_conjugate(float4 q, float3 v)
{
  float3 t = 2 * cross(q.xyz, v);
  return v + q.w*t + cross(q.xyz, t);
}

float4
quaternion_product(float4 p, float4 q)
{
  return
    float4(
      cross(p.xyz, q.xyz) + q.w*p.xyz + p.w*q.xyz,
      p.w*q.w - dot(p.xyz, q.xyz)
    );
}

float
quaternion_norm_squared(float4 q)
{
    return dot(q, q);
}

float
quaternion_norm(float4 q)
{
    return length(q);
}

float3
dual_quaternion_vector_conjugate(DualQuaternion dq, float3 v)
{
    return
        quaternion_vector_conjugate(dq.real, v) +
        2.0f*(cross(dq.real.xyz, dq.non_real.xyz) + dq.real.w*dq.non_real.xyz - dq.non_real.w*dq.real.xyz);
}

DualQuaternion
dual_quaternion_sum(DualQuaternion dq1, DualQuaternion dq2)
{
    DualQuaternion r;
    r.real = dq1.real + dq2.real;
    r.non_real = dq1.non_real + dq2.non_real;
    return r;
}

DualQuaternion
dual_quaternion_scale(float s, DualQuaternion dq)
{
    DualQuaternion r;
    r.real = s*dq.real;
    r.non_real = s*dq.non_real;
    return r;
}


// NOTE: product of a dual quaternion with a dual number
DualQuaternion
dual_quaternion_dual_product(DualQuaternion dq, float2 d)
{
    DualQuaternion r;
    r.real = d[0]*dq.real;
    r.non_real = d[1]*dq.real + d[0]*dq.non_real;
    return r;
}

// NOTE: divide a dual quaternion by a dual number
DualQuaternion
dual_quaternion_dual_quotient(DualQuaternion dq, float2 d)
{
    float2 dinv = float2(1.0f/d[0], -d[1]/(d[0]*d[0]));
    return dual_quaternion_dual_product(dq, dinv);
}

float2
dual_quaternion_norm(DualQuaternion dq)
{
    float real_part_norm = quaternion_norm(dq.real);
    return
        float2(
            real_part_norm,
            dot(dq.real, dq.non_real)/real_part_norm
            );
}

DualQuaternion
dual_quaternion_blend(float t1, float t2, DualQuaternion dq1, DualQuaternion dq2)
{
    DualQuaternion dqs = 
        dual_quaternion_sum(
            dual_quaternion_scale(t1, dq1),
            dual_quaternion_scale(t2, dq2)
            );
    float2 dqsn = dual_quaternion_norm(dqs);
    DualQuaternion dqb = dual_quaternion_dual_quotient(dqs, dqsn);
    return dqb;
}

float4x4 camera_to_viewport_transform()
{
    float field_of_view_y = PI/2.0f;
    float aspect_ratio = 1024.0f / 768.0f;
    float far_z = 0.1f;
    float near_z = 10.0f;

    float w = 1.0f / tan(field_of_view_y * 0.5f);
    float h = w * aspect_ratio;
    float Q = far_z/(far_z - near_z);
    float N = -Q*near_z;
    
    float4x4 camera_to_viewport =
        {
            w, 0, 0, 0,
            0, h, 0, 0,
            0, 0, Q, N,
            0, 0, 1, 0,
        };

    return camera_to_viewport;
}


ScreenVertex
vertex_shader(Vertex vertex)
{

    DualQuaternion model_to_pose_transform =
        dual_quaternion_blend(
            vertex.weights[0],
            vertex.weights[1],
            model_to_world_transform[0],
            model_to_world_transform[1]
            );

    // NOTE: pose-to-world transform is identity for now
    float4 model_to_world_orientation = model_to_pose_transform.real;
    
    float4 position_oriented_model =
        float4(
            dual_quaternion_vector_conjugate(
                model_to_pose_transform,
                vertex.position.xyz
                ),
            1.0f
            );
    float4 position_world =
        position_oriented_model;
    float4 position_camera =
        position_world - camera_position_world;
    float4 position_viewport =
        mul(camera_to_viewport_transform(), position_camera);
    float3 normal_camera =
        quaternion_vector_conjugate(model_to_world_orientation, vertex.normal.xyz);
    float intensity = 0.5f - 0.5f*normal_camera.z;

    ScreenVertex screen_vertex;
    screen_vertex.position_viewport = position_viewport;
    screen_vertex.position_texture = vertex.position_texture;
    screen_vertex.intensity = pow(intensity, 3.0f);
    return screen_vertex;
    
}

FlatScreenVertex
flat_vertex_shader(FlatVertex vertex)
{

    float4 position_oriented_model =
        float4(
            dual_quaternion_vector_conjugate(
                dual_quaternion_blend(
                    vertex.bone_weights[0],
                    vertex.bone_weights[1],
                    model_to_world_transform[0],
                    model_to_world_transform[1]
                    ),
                vertex.position.xyz
                ),
            1.0f
            );

    float4 position_world =
        position_oriented_model;
    float4 position_camera =
        position_world - camera_position_world;
    float4 position_viewport =
        mul(camera_to_viewport_transform(), position_camera);

    FlatScreenVertex screen_vertex;
    screen_vertex.position_viewport = position_viewport;
    return screen_vertex;
    
}

float4
flat_pixel_shader(FlatScreenVertex screen_vertex) : SV_TARGET
{
    float4 color = float4(1,1,1,1);
    return color;
}

float4
pixel_shader(ScreenVertex screen_vertex) : SV_TARGET
{
    float4 color = screen_vertex.intensity*color_texture.Sample(state, screen_vertex.position_texture);
    color.a = 1.0f;
    return color;
}
