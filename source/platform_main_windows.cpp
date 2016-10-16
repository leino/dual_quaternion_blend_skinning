bool
try_initialize_direct3d11(
    HWND const window,
    bool const windowed,
    uint const viewport_x_dimension_screen,
    uint const viewport_y_dimension_screen,
    uint const target_frame_rate,
    IDXGISwapChain **const swap_chain,
    ID3D11Device **const d3d_device,
    ID3D11DeviceContext **const d3d_device_context
    )
{

    *swap_chain = 0;
    *d3d_device = 0;
    *d3d_device_context = 0;
    {
        IDXGIAdapter* adapter = 0;
        D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_HARDWARE;
        HMODULE software_rasterization_module = 0;
        
#if HELLO_D3D11_WINDOW_BUILDTYPE == HELLO_D3D11_WINDOW_BUILDTYPE_RELEASE
        UINT flags = D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY;
#elif HELLO_D3D11_WINDOW_BUILDTYPE == HELLO_D3D11_WINDOW_BUILDTYPE_INTERNAL
        UINT flags = 0;
        flags |= D3D11_CREATE_DEVICE_DEBUG;
        // TODO: try to turn this on
        // NOTE: needed for shader debugging
        // NOTE: if device does not support shader debugging, this will cause device creation to fail!
        // TODO: need a way of querying if driver supports shader debugging
        // flags |= D3D11_CREATE_DEVICE_DEBUGGABLE
        // NOTE: this may simplify debugging
        // flags |= D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS
#else
#error Unrecognized build type
#endif
        
        UINT sdk_version = D3D11_SDK_VERSION;
        
        DXGI_SWAP_CHAIN_DESC swap_chain_description = {};
        swap_chain_description.BufferDesc.Width = viewport_x_dimension_screen;
        swap_chain_description.BufferDesc.Height = viewport_y_dimension_screen;
        // NOTE: refresh rate has no effect in windowed mode
        swap_chain_description.BufferDesc.RefreshRate.Numerator = target_frame_rate;
        swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_description.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
        // TODO: play around with these, they control anti-aliasing, etc...
        swap_chain_description.SampleDesc.Count = 1;
        swap_chain_description.SampleDesc.Quality = 0;
        swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        // NOTE: if in fullscreen mode, the front buffer is counted, otherwise not
        swap_chain_description.BufferCount = windowed ? 1 : 2;
        swap_chain_description.OutputWindow = window;
        swap_chain_description.Windowed = windowed ? TRUE : FALSE;
        swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        // STUDY: DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
        swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0};
        UINT num_feature_levels = ARRAY_LENGTH(feature_levels);
        ENSURE(num_feature_levels == 1);

        D3D_FEATURE_LEVEL first_supported_feature_level;
        
        HRESULT result =
            D3D11CreateDeviceAndSwapChain(
                adapter,
                driver_type,
                software_rasterization_module,
                flags,
                feature_levels,
                num_feature_levels,
                sdk_version,
                &swap_chain_description,
                swap_chain,
                d3d_device,
                &first_supported_feature_level,
                d3d_device_context
                );
        
        if( FAILED(result) )
        {
            using namespace Log;
            string("failed to create d3d device and swap chain");
            newline();
            return false;
        }        

        // TODO: figure out how to use the d3d debug layer
    }
    ENSURE( swap_chain != 0 );
    ENSURE( d3d_device != 0 );
    ENSURE( d3d_device_context != 0 );
    
    return true;
}

int CALLBACK
WinMain(
    HINSTANCE application_instance,
    HINSTANCE /*previous_application_instance*/,
    LPSTR /*command_line*/,
    int command_show
    )
{

    Platform::ApplicationContext application_context = {};
    application_context.instance = application_instance;
    application_context.command_show = command_show;
    
    bool const windowed = true;
    uint const target_frame_rate = 60;
    uint const viewport_x_dimension_screen = 1024;
    uint const viewport_y_dimension_screen = 768;    
    
    Platform::Context platform_context = {};
    if(
        !Platform::try_initialize(
            viewport_x_dimension_screen,
            viewport_y_dimension_screen,
            &application_context,
            &platform_context
            )
        )
    {
        return 0;
    }

    IDXGISwapChain* swap_chain;
    ID3D11Device* d3d_device;
    ID3D11DeviceContext* d3d_device_context;    
    if(
        !try_initialize_direct3d11(
            platform_context.window,
            windowed,
            viewport_x_dimension_screen,
            viewport_y_dimension_screen,
            target_frame_rate,
            &swap_chain,
            &d3d_device,
            &d3d_device_context
            )
        )
    {
        return 0;
    }

    int const exit_code =
        run(
            viewport_x_dimension_screen,
            viewport_y_dimension_screen,
            target_frame_rate,
            &platform_context,
            swap_chain,
            d3d_device,
            d3d_device_context
            );

    swap_chain->Release();
    d3d_device->Release();
    d3d_device_context->Release();
    
    return exit_code;
}
