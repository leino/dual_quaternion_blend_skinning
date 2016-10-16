namespace Platform
{

    enum FileReadResult
    {
        Ok,
        NotFound,
        NotEnoughMemory,
        FailedToDetermineSize,
        FileTooLarge, // 4G (UINT32_MAX) size limit
        LeakedFileHandle,
        OtherError,
    };
    
    struct Context;
    struct ApplicationContext;
    
    bool
    try_initialize(
        uint const client_rectangle_x_dimension_screen,
        uint const client_rectangle_y_dimension_screen,
        ApplicationContext const*const application_context,
        Context *const context
        );    

    void read_window_messages(
        // Has the user requested that the window be closed?
        bool *const quit_requested,
        // This value should be returned by the application main function
        int *const exit_code
        );
    
    uint64 read_ticks();

    // This sleep is entended for use at the end of a frame in order to lock the frame rate at the desired
    // target number of milliseconds. The frame start and end timestamps should encompass the entire frame, and the
    // end timestamp should be recorded just prior to the sleep.
    void
        frame_end_sleep(
            Context const*const context,
            uint64 const frame_start_ticks,
            uint const target_frame_rate
            );

};

extern char const*const window_title;
extern int
run(
    uint const viewport_x_dimension_screen,
    uint const viewport_y_dimension_screen,
    uint const target_frame_rate,
    Platform::Context const*const platform_context,
    IDXGISwapChain *const swap_chain,
    ID3D11Device *const d3d_device,
    ID3D11DeviceContext *const d3d_device_context
    );
