#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES
#include <windows.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include "numbers.h"
#include "array.h"
#include "integer.h"
#include "ensure.h"
#include "ifdef_sanity_checks.h"
#include "log.h"
#include "platform.h"
#include "platform_windows.cpp"
#include "platform_main_windows.cpp"
#include "numerics.cpp"
#include "vec4.h"
#include "vec4.cpp"
#include "vec2.h"
#include "vec2.cpp"
#include "vec3.h"
#include "vec3.cpp"
#include "matrix4.h"
#include "matrix4.cpp"
#include "transform3p.cpp"
#include "quaternions.h"
#include "quaternions.cpp"
#include "dual_quaternions.h"
#include "dual_quaternions.cpp"
#include "transformations.cpp"

char const*const window_title = "Dual quaternion blend skinning demo";

// NOTE: This must match the shader input layout, be careful about padding
struct Vertex
{
    Vec4 position_model;
    Vec4 normal_model;
    float bone_weights[4];
    Vec2 position_texture;
    float __padding[2];
};

// NOTE: This must match the shader input layout, be careful about padding
struct FlatVertex
{
    Vec4 position_model;
    float bone_weights[4];
};

// NOTE: This must match the constant buffer in the shader, be careful about padding!
struct TransformConstants
{
    DualQuaternions::DualQuaternion model_to_world_transform[2];
};

enum VertexShaders
{
    TubeVertexShader,
    FlatVertexShader,

    NumVertexShaders
};

enum PixelShaders
{
    TubePixelShader,
    FlatPixelShader,

    NumPixelShaders
};    

int const num_axial_segments = 50;
int const num_radial_segments = 30;
int const num_axial_slices = num_axial_segments + 1;
int const num_radial_slices = num_radial_segments;

struct VertexInputElementDescription
{
    static uint const MAX_NUM_ELEMENTS = 4;
    D3D11_INPUT_ELEMENT_DESC element_descriptions[MAX_NUM_ELEMENTS];
    int num_elements;
};

int
run(
    uint const viewport_x_dimension_screen,
    uint const viewport_y_dimension_screen,
    uint const target_frame_rate,
    Platform::Context const*const platform_context,
    IDXGISwapChain *const swap_chain,
    ID3D11Device *const d3d_device,
    ID3D11DeviceContext *const d3d_device_context
    )
{    

    TransformConstants transform_constants;

    int const num_bones = 2;
    float const tube_height = 4.0f;
    int const num_bone_vertices = num_bones*2;
    
    // Get a render target view on the back buffer
    ID3D11RenderTargetView* render_target_view = 0;
    {
        
        ID3D11Texture2D* back_buffer_texture = 0;
        {
            // NOTE: the back buffer has index 0
            UINT buffer = 0;
            REFIID riid = __uuidof(ID3D11Texture2D);
            HRESULT result =
                swap_chain->GetBuffer(
                    buffer,
                    riid,
                    (void**)&back_buffer_texture
                    );
            if( FAILED(result) )
            {
                Log::string("failed to get back buffer texture");
                return 0;
            }
        }
        ENSURE( back_buffer_texture != 0 );

        {
            ID3D11Resource *resource = back_buffer_texture;
            // NOTE: 0 indicates that we want a view with the same format as the buffer
            D3D11_RENDER_TARGET_VIEW_DESC* render_target_view_description = 0;
            HRESULT result =
                d3d_device->CreateRenderTargetView(
                    resource,
                    render_target_view_description,
                    &render_target_view
                    );
            if( FAILED(result) )
            {
                Log::string("failed to get render target view on back buffer");
                return 0;
            }
        }

        back_buffer_texture->Release();

    }
    ENSURE(render_target_view);

    // NOTE: create depth stencil buffer texture
    ID3D11Texture2D * depth_stencil_texture = 0;
    {

        D3D11_TEXTURE2D_DESC description = {};
        description.Width = viewport_x_dimension_screen;
        description.Height = viewport_y_dimension_screen;
        description.MipLevels = 1;
        description.ArraySize = 1;
        description.Format = DXGI_FORMAT_D32_FLOAT;
        description.SampleDesc.Count = 1;
        description.SampleDesc.Quality = 0;
        description.Usage = D3D11_USAGE_DEFAULT;
        description.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        description.CPUAccessFlags = 0;
        description.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA *const initial_data = 0;
        
        HRESULT const result =
            d3d_device->CreateTexture2D(
                &description,
                initial_data,
                &depth_stencil_texture
                );

        if(FAILED(result))
        {
            Log::string("failed to create depth stencil texture");
            Log::newline();
            return 0;
        }
        
    }
    ENSURE(depth_stencil_texture != 0);

    // NOTE: create the depth stencil state
    ID3D11DepthStencilState * depth_stencil_state = 0;
    {

        D3D11_DEPTH_STENCIL_DESC description = {};
        description.DepthEnable = TRUE;
        description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        description.DepthFunc = D3D11_COMPARISON_GREATER;
        description.StencilEnable = FALSE;
        description.StencilReadMask = 0xff;
        description.StencilWriteMask = 0xff;        
        description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        description.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        description.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        description.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        description.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        
        HRESULT const result =
            d3d_device->CreateDepthStencilState(
                &description,
                &depth_stencil_state
                );
        
        if(FAILED(result))
        {
            Log::string("failed to create the depth stencil state");
            Log::newline();
            return 0;
        }
        
    }
    ENSURE(depth_stencil_state != 0);

    // NOTE: create a view on the depth stencil texture
    ID3D11DepthStencilView * depth_stencil_view = 0;
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC description = {};
        description.Format = DXGI_FORMAT_D32_FLOAT;
        description.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        description.Flags = 0;
        description.Texture2D.MipSlice = 0;
        
        HRESULT const result =
            d3d_device->CreateDepthStencilView(
                depth_stencil_texture,
                &description,
                &depth_stencil_view
                );

        if(FAILED(result))
        {
            Log::string("failed to create depth stencil buffer view");
            Log::newline();
            return 0;
        }
        
    }
    ENSURE(depth_stencil_view != 0);
    
    {
        uint const num_views = 1;
        ID3D11RenderTargetView* render_target_views[num_views] = {render_target_view};
        d3d_device_context->OMSetRenderTargets(
            num_views,
            render_target_views,
            depth_stencil_view
            );
    }

    {
        uint const num_viewports = 1;
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = (FLOAT)viewport_x_dimension_screen;
        viewport.Height = (FLOAT)viewport_y_dimension_screen;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        D3D11_VIEWPORT viewports[num_viewports] = {viewport};
        d3d_device_context->RSSetViewports(num_viewports, viewports);
    }

    ID3D11Buffer * tube_vertex_buffer = 0;
    {
        float const radius = 0.5f;
        
        uint const num_vertices = num_axial_slices * num_radial_slices;
        Vertex vertices[num_vertices] = {};
        int vertex_idx = 0;

        int const num_blended_axial_slices = 10;
        
        for(int axial_slice_idx=0; axial_slice_idx < num_axial_slices; axial_slice_idx++)
        {
            
            float axial_slice_position = float(axial_slice_idx)/float(num_axial_segments);

#define LINEAR_BLEND_CURVES 0
#define SMOOTHSTEP_BLEND_CURVES 1

#define CURVE_TYPE SMOOTHSTEP_BLEND_CURVES

            float const d = axial_slice_position - 0.5f;
            
#if CURVE_TYPE == LINEAR_BLEND_CURVES
            float w[2] = {};
            float const a = 0.2f;
            if(d < -a)
            {
                w[0] = 1.0f;
                w[1] = 0.0f;
            }
            else if(d > +a)
            {
                w[0] = 0.0f;
                w[1] = 1.0f;
            }
            else
            {
                w[0] = 1.0f - (+d - (-a))/(2.0f*a);
                w[1] = 1.0f - (-d - (-a))/(2.0f*a);
            }
#else if CURVE_TYPE == SMOOTHSTEP_BLEND_CURVES
            float w[2] = {};
            float const a = 0.25f;
            if(d < -a)
            {
                w[0] = 1.0f;
                w[1] = 0.0f;
            }
            else if(d > +a)
            {
                w[0] = 0.0f;
                w[1] = 1.0f;
            }
            else
            {
                float const ss1 = Numerics::smoothstep(+a, -a, d);
                float const ss2 = Numerics::smoothstep(-a, +a, d);
                float const s = ss1 + ss2;
                w[0] = ss1 / s;
                w[1] = ss2 / s;
            }            
#endif
            
            for(int radial_slice_idx=0; radial_slice_idx < num_radial_slices; radial_slice_idx++)
            {
                
                float const radial_position = float(radial_slice_idx)/float(num_radial_segments);
                float const radial_angle = -2.0f*PI_FLOAT*radial_position;
                float const c = Numerics::cos(radial_angle);
                float const s = Numerics::sin(radial_angle);
                float const x = c*radius;
                float const y = s*radius;
                
                Vertex *const vertex = &vertices[vertex_idx];
                vertex->bone_weights[0] = w[0];
                vertex->bone_weights[1] = w[1];
                vertex->position_model.coordinate.x = x;
                vertex->position_model.coordinate.y = y;
                vertex->position_model.coordinate.z =
                    tube_height*axial_slice_position;
                vertex->position_model.coordinate.w = 1.0f;
                vertex->normal_model.coordinate.x = c;
                vertex->normal_model.coordinate.y = s;
                vertex->normal_model.coordinate.z = 0.0f;
                vertex->normal_model.coordinate.w = 1.0f;
                vertex->position_texture.coordinate.x =
                    radial_position < 0.5f ? (2.0f*radial_position) : (1 - 2.0f*(radial_position-0.5f));
                vertex->position_texture.coordinate.y = axial_slice_position;
                vertex_idx++;
            }
        }
        D3D11_BUFFER_DESC description = {};
        description.ByteWidth = sizeof(Vertex)*num_vertices;
        description.Usage = D3D11_USAGE_IMMUTABLE;
        description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        description.CPUAccessFlags = 0;
        description.MiscFlags = 0;
        // STUDY: why not set this to the size of the buffer element? sizeof(WidgetVertex)
        description.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initial_data = {};        
        initial_data.pSysMem = vertices;
            
        HRESULT result = d3d_device->CreateBuffer(&description, &initial_data, &tube_vertex_buffer);

        if( FAILED(result) )
        {
            return 0;
        }
        
    }
    ENSURE(tube_vertex_buffer != 0);

    ID3D11Buffer * bone_vertex_buffer = 0;
    {
        FlatVertex vertices[num_bone_vertices] = {};

        for(int bone_idx=0; bone_idx < num_bones; bone_idx++)
        {

            int const lo_slice_idx = bone_idx+0;
            int const hi_slice_idx = bone_idx+1;
            
            int const lo_vertex_idx = 2*bone_idx+0;
            int const hi_vertex_idx = 2*bone_idx+1;
            
            float const lo_z = tube_height*lo_slice_idx/float(num_bones);
            float const hi_z = tube_height*hi_slice_idx/float(num_bones);
            FlatVertex *const lo_vertex = &vertices[lo_vertex_idx];
            FlatVertex *const hi_vertex = &vertices[hi_vertex_idx];
            
            lo_vertex->position_model.coordinate.x = 0.0f;
            lo_vertex->position_model.coordinate.y = 0.0f;
            lo_vertex->position_model.coordinate.z = lo_z;
            lo_vertex->position_model.coordinate.w = 1.0f;
            lo_vertex->bone_weights[bone_idx] = 1.0f;
            
            hi_vertex->position_model.coordinate.x = 0.0f;
            hi_vertex->position_model.coordinate.y = 0.0f;
            hi_vertex->position_model.coordinate.z = hi_z;
            hi_vertex->position_model.coordinate.w = 1.0f;
            hi_vertex->bone_weights[bone_idx] = 1.0f;
            
        }
        D3D11_BUFFER_DESC description = {};
        description.ByteWidth = sizeof(FlatVertex)*num_bone_vertices;
        description.Usage = D3D11_USAGE_IMMUTABLE;
        description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        description.CPUAccessFlags = 0;
        description.MiscFlags = 0;
        // STUDY: why not set this to the size of the buffer element? sizeof(WidgetVertex)
        description.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initial_data = {};
        initial_data.pSysMem = vertices;
            
        HRESULT result = d3d_device->CreateBuffer(&description, &initial_data, &bone_vertex_buffer);

        if( FAILED(result) )
        {
            return 0;
        }
        
    }
    ENSURE(bone_vertex_buffer != 0);    
    
    ID3D11Buffer* tube_index_buffer = 0;
    uint const num_indices = num_axial_segments*num_radial_segments*3*2;
    {
        int indices[num_indices];
        int quad_idx = 0;

        for(int axial_segment_idx=0; axial_segment_idx < num_axial_segments; axial_segment_idx++)
        {
            int const axial_slice_lo = axial_segment_idx + 0;
            int const axial_slice_hi = axial_segment_idx + 1;
            
            for(int radial_segment_idx=0; radial_segment_idx < num_radial_segments; radial_segment_idx++)
            {
                int const radial_slice_lo = Numerics::remainder(num_radial_slices, radial_segment_idx + 0);
                int const radial_slice_hi = Numerics::remainder(num_radial_slices, radial_segment_idx + 1);
                
                int *const quad_indices = &indices[2*3*quad_idx];

                int const axial_lo_radial_lo_idx = axial_slice_lo*num_radial_slices + radial_slice_lo;
                int const axial_lo_radial_hi_idx = axial_slice_lo*num_radial_slices + radial_slice_hi;
                int const axial_hi_radial_lo_idx = axial_slice_hi*num_radial_slices + radial_slice_lo;
                int const axial_hi_radial_hi_idx = axial_slice_hi*num_radial_slices + radial_slice_hi;
                
                quad_indices[0] = axial_lo_radial_lo_idx;
                quad_indices[1] = axial_hi_radial_lo_idx;
                quad_indices[2] = axial_lo_radial_hi_idx;

                quad_indices[3] = axial_lo_radial_hi_idx;
                quad_indices[4] = axial_hi_radial_lo_idx;
                quad_indices[5] = axial_hi_radial_hi_idx;
                
                quad_idx++;                
            }
            
        }
        
        D3D11_BUFFER_DESC description = {};
        description.ByteWidth = sizeof(int)*num_indices;
        description.Usage = D3D11_USAGE_IMMUTABLE;
        description.BindFlags = D3D11_BIND_INDEX_BUFFER;
        description.CPUAccessFlags = 0;
        description.MiscFlags = 0;
        description.StructureByteStride = 0;
            
        D3D11_SUBRESOURCE_DATA initial_data = {};
        initial_data.pSysMem = indices;
            
        HRESULT result = d3d_device->CreateBuffer(&description, &initial_data, &tube_index_buffer);

        if( FAILED(result) )
        {
            return false;
        }
        
    }
    ENSURE(tube_index_buffer != 0);

    // NOTE: create the texture
    ID3D11Texture2D* texture = 0;
    {

        uint const texture_width = 128;
        uint const texture_height = 128;
        Vec4 const color1 = {0.2f, 0.1f, 0.7f, 1.0f};
        Vec4 const color2 = {0.1f, 0.5f, 0.3f, 1.0f};
        Vec4 texture_data[texture_width*texture_height] = {};
        for(int row_idx=0; row_idx < texture_height; row_idx++)
        {
            float const u = float(row_idx)/float(texture_height-1);
            for(int column_idx=0; column_idx < texture_width; column_idx++)
            {
                float const v = float(column_idx)/float(texture_width-1);
                Vec4 *const color = &texture_data[texture_width*row_idx + column_idx];
                if(column_idx % (texture_width >> 2) == 0)
                    *color = {};
                int const i = row_idx / (texture_height >> 4);
                int const j = (column_idx + (texture_width >> 3)) / (texture_width >> 2);
                if((i + j) % 2 == 0)
                {
                    *color = color1;
                }
                else
                {
                    *color = color2;
                }
            }
        }
        
        
        D3D11_TEXTURE2D_DESC description = {};
        description.Width = texture_width;
        description.Height = texture_height;
        description.MipLevels = 1;
        description.ArraySize = 1;
        description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        description.SampleDesc.Count = 1;
        description.SampleDesc.Quality = 0;
        description.Usage = D3D11_USAGE_IMMUTABLE;
        description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        description.CPUAccessFlags = 0;
        description.MiscFlags = 0;
  
        D3D11_SUBRESOURCE_DATA initial_data = {};
        initial_data.pSysMem = texture_data;
        initial_data.SysMemPitch = sizeof(Vec4)*texture_width;
        // NOTE: not using depth levels so this is irrelevant
        initial_data.SysMemSlicePitch = 0;
  
        HRESULT result =
            d3d_device->CreateTexture2D(
                &description,
                &initial_data,
                &texture
                );
        
        if(FAILED(result))
        {
            using namespace Log;
            string("failed to create texture");
            newline();
            return 0;
        }
        
    }
    ENSURE(texture != 0);

    // NOTE: shader view of texture resource
    ID3D11ShaderResourceView * texture_shader_resource_view = 0;
    {

        ID3D11Resource *const resource = texture;
        D3D11_SHADER_RESOURCE_VIEW_DESC description = {};
        description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        description.Texture2D.MostDetailedMip = 0;
        description.Texture2D.MipLevels = uint(-1);
        
        HRESULT const result =
            d3d_device->CreateShaderResourceView(
                resource,
                &description,
                &texture_shader_resource_view
                );
        
        if(FAILED(result))
        {
            using namespace Log;
            string("failed to create shader resource view");
            newline();
            return 0;
        }
        
    }
    ENSURE(texture_shader_resource_view != 0);

    // NOTE: create a sampler state for the texture
    ID3D11SamplerState * tube_sampler_state = 0;
    {

        D3D11_SAMPLER_DESC description = {};
        description.Filter = D3D11_FILTER_ANISOTROPIC;// D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        description.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        description.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        description.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        description.MipLODBias = float(0);
        description.MaxAnisotropy = 1;
        description.ComparisonFunc = D3D11_COMPARISON_NEVER;
        description.BorderColor[0] = 1.0f;
        description.BorderColor[1] = 1.0f;
        description.BorderColor[2] = 1.0f;
        description.BorderColor[3] = 1.0f;
        description.MinLOD = FLOAT_MIN;
        description.MaxLOD = FLOAT_MAX;
        
        HRESULT const result = d3d_device->CreateSamplerState(
            &description,
            &tube_sampler_state
            );

        if(FAILED(result))
        {
            using namespace Log;
            string("failed to create sampler state");
            newline();
        }
        
    }
    ENSURE(tube_sampler_state != 0);
    
    union
    {
        struct
        {
            ID3D11PixelShader* tube;
            ID3D11PixelShader* flat;
        } shader;

        ID3D11PixelShader* shaders[PixelShaders::NumPixelShaders];
        
    } pixel_shaders;
    
    char const*const pixel_shader_filenames[PixelShaders::NumPixelShaders] =
        {
            "pixel_shader.cso",
            "flat_pixel_shader.cso",
        };
    
    for(int shader_idx=0; shader_idx < PixelShaders::NumPixelShaders; shader_idx++)
    {
        using namespace Platform;
        
        char const*const filename = pixel_shader_filenames[shader_idx];
        void* byte_code = 0;
        size_t byte_code_size = 0;
        {
            FileReadResult const result =
                try_alloc_and_read_entire_file(filename, &byte_code, &byte_code_size);
            
            if(result != FileReadResult::Ok)
            {
                Log::string("failed to read pixel shader file ");
                Log::string(filename);
                Log::string("\n");
				return 0;
            }
        }
        ENSURE(byte_code != 0);
        ENSURE(byte_code_size != 0);

        // STUDY: what is this?
        ID3D11ClassLinkage* class_linkage = 0;
        
        HRESULT result = d3d_device->CreatePixelShader(
            byte_code,
            byte_code_size,
            class_linkage,
            &pixel_shaders.shaders[shader_idx]
            );

        Platform::free_file_memory(byte_code);        
        
        if( FAILED(result) )
        {
            Log::string("failed to compile pixel shader");
            Log::string("(");
            Log::string(filename);
            Log::string(")");
            Log::string("\n");
            return 0;
        }        
        
    }
    ENSURE(solid_pixel_shader != 0);
    
    union
    {
        struct
        {
            
            ID3D11InputLayout* tube;
            ID3D11InputLayout* flat;
        } layout;

        ID3D11InputLayout* layouts[VertexShaders::NumVertexShaders];
        
    } vertex_input_layouts;
    ENSURE_STATIC(sizeof(vertex_input_layouts) == VertexShaders::NumVertexShaders*sizeof(ID3D11InputLayout*));
    
    union
    {
        struct
        {
            ID3D11VertexShader* tube;
            ID3D11VertexShader* flat;
        } shader;

        ID3D11VertexShader* shaders[VertexShaders::NumVertexShaders];
    } vertex_shaders;
    ENSURE_STATIC(sizeof(vertex_shaders) == VertexShaders::NumVertexShaders*sizeof(ID3D11VertexShader*));

    char const*const vertex_shader_filenames[VertexShaders::NumVertexShaders] =
        {
            "vertex_shader.cso",
            "flat_vertex_shader.cso"
        };

    VertexInputElementDescription vertex_input_element_descriptions[VertexShaders::NumVertexShaders];
    
    // NOTE: tube vertex layout description
    {
        int const shader_idx = int(VertexShaders::TubeVertexShader);
        
        vertex_input_element_descriptions[shader_idx].num_elements = 4;        
        D3D11_INPUT_ELEMENT_DESC *const element_descriptions =
            vertex_input_element_descriptions[shader_idx].element_descriptions;
        
        element_descriptions[0].SemanticName = "POSITION";
        element_descriptions[0].SemanticIndex = 0; // NOTE: not relevant
        element_descriptions[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        element_descriptions[0].InputSlot = 0;
        element_descriptions[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        element_descriptions[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element_descriptions[0].InstanceDataStepRate = 0;            

        element_descriptions[1].SemanticName = "NORMAL";
        element_descriptions[1].SemanticIndex = 0; // NOTE: not relevant
        element_descriptions[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        element_descriptions[1].InputSlot = 0;
        element_descriptions[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        element_descriptions[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element_descriptions[1].InstanceDataStepRate = 0;            

        element_descriptions[2].SemanticName = "BLENDWEIGHT";
        element_descriptions[2].SemanticIndex = 0; // NOTE: not relevant
        element_descriptions[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        element_descriptions[2].InputSlot = 0;
        element_descriptions[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        element_descriptions[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element_descriptions[2].InstanceDataStepRate = 0;                    
        
        element_descriptions[3].SemanticName = "TEXCOORD";
        element_descriptions[3].SemanticIndex = 0; // NOTE: not relevant
        element_descriptions[3].Format = DXGI_FORMAT_R32G32_FLOAT;
        element_descriptions[3].InputSlot = 0;
        element_descriptions[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        element_descriptions[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element_descriptions[3].InstanceDataStepRate = 0;
        
    }

    // NOTE: flat vertex layout description
    {
        int const shader_idx = int(VertexShaders::FlatVertexShader);
        
        vertex_input_element_descriptions[shader_idx].num_elements = 2;
        D3D11_INPUT_ELEMENT_DESC *const element_descriptions =
            vertex_input_element_descriptions[shader_idx].element_descriptions;
        
        element_descriptions[0].SemanticName = "POSITION";
        element_descriptions[0].SemanticIndex = 0; // NOTE: not relevant
        element_descriptions[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        element_descriptions[0].InputSlot = 0;
        element_descriptions[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        element_descriptions[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element_descriptions[0].InstanceDataStepRate = 0;            

        element_descriptions[1].SemanticName = "BLENDWEIGHT";
        element_descriptions[1].SemanticIndex = 0; // NOTE: not relevant
        element_descriptions[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        element_descriptions[1].InputSlot = 0;
        element_descriptions[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        element_descriptions[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element_descriptions[1].InstanceDataStepRate = 0;            
        
    }    
    
    for(int shader_idx=0; shader_idx < VertexShaders::NumVertexShaders; shader_idx++)
    {
        using namespace Platform;
        char const*const filename = vertex_shader_filenames[shader_idx];
        void* byte_code = 0;
        size_t byte_code_size = 0;
        {
            FileReadResult const result = try_alloc_and_read_entire_file(filename, &byte_code, &byte_code_size);
            if(result != FileReadResult::Ok)
            {
                Log::string("failed to load vertex shader file ");
                Log::string(filename);
                Log::string("\n");
                return 0;
            }
        }
        ENSURE(byte_code != 0);
        ENSURE(byte_code_size != 0);

        {
            // STUDY: what is this?
            ID3D11ClassLinkage* class_linkage = 0;
        
            HRESULT const result =
                d3d_device->CreateVertexShader(
                    byte_code,
                    byte_code_size,
                    class_linkage,
                    &vertex_shaders.shaders[shader_idx]
                    );

            if( FAILED(result) )
            {
                Log::string("failed to compile vertex shader");
                Log::string(" (");
                Log::string(filename);
                Log::string(")");
                Log::string("\n");
                Platform::free_file_memory(byte_code);
                return 0;
            }            
        }

        {
            D3D11_INPUT_ELEMENT_DESC *const element_descriptions =
                vertex_input_element_descriptions[shader_idx].element_descriptions;
            
            uint const num_elements = vertex_input_element_descriptions[shader_idx].num_elements;
        
            HRESULT result = d3d_device->CreateInputLayout(
                element_descriptions,
                num_elements,
                byte_code,
                byte_code_size,
                &vertex_input_layouts.layouts[shader_idx]
                );

            if( FAILED(result) )
            {
                Log::string("failed to create vertex input layout");
                Log::string(" (");
                Log::string(filename);
                Log::string(")");
                Log::string("\n");
                Platform::free_file_memory(byte_code);
                return 0;
            }
        }
            
        Platform::free_file_memory(byte_code);            
            
    }    

    ID3D11Buffer* transform_constant_buffer = 0;
    {
        D3D11_BUFFER_DESC description = {};
        description.ByteWidth = sizeof(TransformConstants);
        description.Usage = D3D11_USAGE_DYNAMIC;
        description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        description.MiscFlags = 0;
        description.StructureByteStride = 0;
        
        D3D11_SUBRESOURCE_DATA* initial_data = 0;
        
        HRESULT result = d3d_device->CreateBuffer(
            &description,
            initial_data,
            &transform_constant_buffer
            );
        
        if(FAILED(result))
        {
            Log::string("failed to create dynamic constant buffer");
            Log::newline();
            return 0;
        }
    }
    ENSURE(transform_constant_buffer != 0);
    
    uint64 const first_frame_ticks = Platform::read_ticks();

    int exit_code = 0;
    uint64 elapsed_ticks = 0;
    while(true)
    {
        float const time = float(elapsed_ticks) / float(platform_context->ticks_per_second);

        {
            using namespace Matrix4;
            using namespace Transform3p;
            using namespace DualQuaternions;
            using namespace Transformations;
            
            {
                DualQuaternion d1;
                rotation_x_axis(
                    -0.5f*PI_FLOAT,
                    &d1
                    );
                DualQuaternion d2;
                rotation_y_axis(
                    -PI_FLOAT*0.5f,
                    &d2
                    );
                DualQuaternion d3;
                rotation_z_axis(
                    time,
                    &d3
                    );
                DualQuaternion d4;
                translation_y_axis(
                    -tube_height/2.0f,
                    &d4
                    );
                
                DualQuaternion wiggle;
                {
                    DualQuaternion rotation;
                    rotation_x_axis(
                        0.5f*PI_FLOAT*sin(1.0f*time),
                        &rotation
                        );
                    DualQuaternion twist;
                    rotation_z_axis(
                        0.25f*PI_FLOAT*cos(5.0f*time),
                        &twist
                        );
                    product(&rotation, &twist, &wiggle);
                }
                
                DualQuaternion unrest;
                translation_z_axis(
                    -tube_height/2.0f,
                    &unrest
                    );
                DualQuaternion rest;
                translation_z_axis(
                    +tube_height/2.0f,
                    &rest
                    );
                
                DualQuaternion parent;
                product(&d4, &d2, &d1, &d3, &parent);
                
                transform_constants.model_to_world_transform[0] = parent;
                product(&parent, &rest, &wiggle, &unrest, &transform_constants.model_to_world_transform[1]);
            }
            
        }
        
        bool exit_requested;
        Platform::read_window_messages(&exit_requested, &exit_code);

        if(exit_requested)
        {
            break;
        }

        uint64 const frame_start_ticks = Platform::read_ticks();

        // NOTE: set depth stencil state
        {
            UINT const reference_value = 1;
            d3d_device_context->OMSetDepthStencilState(
                depth_stencil_state,
                reference_value
                );
        }        

        {
            uint const flags = D3D11_CLEAR_DEPTH;
            float const depth = 0.0f;
            uint8 const stencil = 0;
            d3d_device_context->ClearDepthStencilView(
                depth_stencil_view,
                flags,
                depth,
                stencil
                );
            
        }
        
        {
            FLOAT const clear_color[4] = {0.1f, 0.11f, 0.12f, 0.0};
            d3d_device_context->ClearRenderTargetView(render_target_view, clear_color);
        }
        
        {
            uint transform_constant_buffer_slot = 0;
            ID3D11Buffer *const buffers[] = {transform_constant_buffer};
            uint const num_buffers = ARRAY_LENGTH(buffers);
            d3d_device_context->VSSetConstantBuffers(
                transform_constant_buffer_slot,
                num_buffers,
                buffers
                );
        }        

        // NOTE: update the transform constant buffer
        {
            ID3D11Resource *const resource = transform_constant_buffer;
            uint const subresource = 0;
            D3D11_MAP const map_type = D3D11_MAP_WRITE_DISCARD;
            uint const map_flags = 0;
            D3D11_MAPPED_SUBRESOURCE mapped_subresource = {};
            
            HRESULT const result =
                d3d_device_context->Map(
                    resource,
                    subresource,
                    map_type,
                    map_flags,
                    &mapped_subresource
                    );

            if(FAILED(result))
            {
                Log::string("failed to update the dynamic constant buffer");
                ENSURE(false);
            }
            
            TransformConstants *const constants = (TransformConstants*)mapped_subresource.pData;
            memcpy(constants, &transform_constants, sizeof(TransformConstants));

            d3d_device_context->Unmap(resource, subresource);
        }

        // NOTE: draw the tube
        {
            
            d3d_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            d3d_device_context->IASetInputLayout(vertex_input_layouts.layout.tube);

            {
                uint num_class_instances = 0;
                ID3D11ClassInstance** class_instances = 0;
                d3d_device_context->VSSetShader(
                    vertex_shaders.shader.tube,
                    class_instances,
                    num_class_instances
                    );
            }

            {
                ID3D11ClassInstance** class_instances = 0;
                uint const num_class_instances = 0;
                d3d_device_context->PSSetShader(
                    pixel_shaders.shader.tube,
                    class_instances,
                    num_class_instances
                    );            
            }

            {
                uint const start_slot = 0;
                ID3D11ShaderResourceView *const views[] = {texture_shader_resource_view};
                uint const num_views = ARRAY_LENGTH(views);
                d3d_device_context->PSSetShaderResources(
                    start_slot,
                    num_views,
                    views
                    );        
            }        
        
            {
                uint input_slot = 0;
                uint const num_buffers = 1;
                ID3D11Buffer* buffers[num_buffers] = {tube_vertex_buffer};
                uint strides[num_buffers] = {sizeof(Vertex)};
                uint offsets[num_buffers] = {0};
                d3d_device_context->IASetVertexBuffers(
                    input_slot,
                    num_buffers,
                    buffers,
                    strides,
                    offsets
                    );
            }

            {

                DXGI_FORMAT const format = DXGI_FORMAT_R32_UINT;
                uint const offset = 0;
                
                d3d_device_context->IASetIndexBuffer(
                    tube_index_buffer,
                    format,
                    offset
                    );
                
            }

            {
                uint const start_slot = 0;
                ID3D11SamplerState *const samplers[] = {tube_sampler_state};
                uint const num_samplers = ARRAY_LENGTH(samplers);
                d3d_device_context->PSSetSamplers(
                    start_slot,
                    num_samplers,
                    samplers
                    );
            }        

            {
                uint const index_count = num_indices;
                uint const start_index = 0;
                int const base_vertex_index = 0;
            
                d3d_device_context->DrawIndexed(
                    index_count,
                    start_index,
                    base_vertex_index
                    );

            }
        }

        // NOTE: draw the bones
        {
            d3d_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
            d3d_device_context->IASetInputLayout(vertex_input_layouts.layout.flat);

            {
                uint const start_slot = 0;
                ID3D11SamplerState *const samplers[1] = {0};
                uint const num_samplers = ARRAY_LENGTH(samplers);
                d3d_device_context->PSSetSamplers(
                    start_slot,
                    num_samplers,
                    samplers
                    );
            }        
            
            
            {
                uint num_class_instances = 0;
                ID3D11ClassInstance** class_instances = 0;
                d3d_device_context->VSSetShader(
                    vertex_shaders.shader.flat,
                    class_instances,
                    num_class_instances
                    );
            }

            {
                ID3D11ClassInstance** class_instances = 0;
                uint const num_class_instances = 0;
                d3d_device_context->PSSetShader(
                    pixel_shaders.shader.flat,
                    class_instances,
                    num_class_instances
                    );            
            }
            
            {
                uint input_slot = 0;
                uint const num_buffers = 1;
                ID3D11Buffer* buffers[num_buffers] = {bone_vertex_buffer};
                uint strides[num_buffers] = {sizeof(FlatVertex)};
                uint offsets[num_buffers] = {0};
                d3d_device_context->IASetVertexBuffers(
                    input_slot,
                    num_buffers,
                    buffers,
                    strides,
                    offsets
                    );
            }

#if 0
            {
                uint const vertex_count = num_bone_vertices;
                uint const start_vertex_location = 0;
                
                d3d_device_context->Draw(
                    vertex_count,
                    start_vertex_location
                    );
            }
#endif

        }
            
        {
            UINT sync_interval = 0;
            UINT flags = 0;
            swap_chain->Present(sync_interval, flags);
        }

        Platform::frame_end_sleep(platform_context, frame_start_ticks, target_frame_rate);
        elapsed_ticks = Platform::read_ticks() - first_frame_ticks;
    }    

    render_target_view->Release();
    tube_vertex_buffer->Release();
    tube_index_buffer->Release();
    for(int shader_idx = 0; shader_idx < VertexShaders::NumVertexShaders; shader_idx++)
    {
        vertex_shaders.shaders[shader_idx]->Release();
        vertex_input_layouts.layouts[shader_idx]->Release();
    }
    for(int shader_idx = 0; shader_idx < PixelShaders::NumPixelShaders; shader_idx++)
    {
        pixel_shaders.shaders[shader_idx]->Release();
    }
    transform_constant_buffer->Release();
    texture->Release();
    tube_sampler_state->Release();
    texture_shader_resource_view->Release();
    depth_stencil_texture->Release();
    
    return exit_code;
}
