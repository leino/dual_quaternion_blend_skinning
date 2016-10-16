#if !defined(PLATFORM_DEBUG)
#error "PLATFORM_DEBUG is not defined"
#endif

#if PLATFORM_DEBUG==1
#define PLATFORM_ENSURE_CONTEXT_INITIALIZED(ctx) ENSURE(ctx->initialized);
#else
#define PLATFORM_ENSURE_CONTEXT_INITIALIZED(ctx)
#endif

static LRESULT CALLBACK
handle_window_message(
    HWND window,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    
    switch(message)
    {
    case WM_SIZE:
    {
        return 0;
    } break;
        
    case WM_CLOSE:
    {
        DestroyWindow(window);
        return 0;    
    } break;
        
    case WM_ACTIVATEAPP:
    {
        return 0;
    } break;
        
    case WM_DESTROY:
    {
        int exit_code = 0;
        PostQuitMessage(exit_code);
        return 0;
    } break;


    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        ENSURE(false);
        return 0;
    } break;    
    
    default:
    {
        LRESULT const result =
            DefWindowProc(window, message, wparam, lparam);
        return result;
    } break;
    }

}


namespace Platform
{

    struct
    Context
    {
        uint64 ticks_per_second;
        HWND window;
#if defined(PLATFORM_DEBUG)
        bool initialized;
#endif
    };

    struct
    ApplicationContext
    {
        HINSTANCE instance;
        int command_show;
    };

    bool
    try_initialize(
        uint const client_rectangle_x_dimension_screen,
        uint const client_rectangle_y_dimension_screen,
        ApplicationContext const*const application_context,
        Context *const context
        )
    {

        // Tell the system that we want the finest sleep granularity possible
        {
            UINT const period = 1;
            MMRESULT const result = timeBeginPeriod(period);
            if(result != MMSYSERR_NOERROR)
            {
                return false;
            }
        }                
        
        // Get the performance counter resolution
        {
            LARGE_INTEGER frequency;
            BOOL const success = QueryPerformanceFrequency(
                &frequency
                );
            if(!success)
            {
                return false;
            }
            context->ticks_per_second = frequency.QuadPart;
        }

        char const*const window_class_name = "platform_window";
        {
            WNDCLASSEX window_class = {};
            window_class.cbSize = sizeof(WNDCLASSEX);
            window_class.style = CS_HREDRAW | CS_VREDRAW;
            window_class.lpfnWndProc = handle_window_message;
            window_class.hInstance = application_context->instance;
            window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
            window_class.lpszClassName = window_class_name;
            RegisterClassEx(&window_class);
        }

        context->window = 0;
        {
        
            uint window_width_screen;
            uint window_height_screen;
            {
                RECT window_rectangle =
                    {
                        0,
                        0,
                        (LONG)client_rectangle_x_dimension_screen,
                        (LONG)client_rectangle_y_dimension_screen
                    };
                BOOL const success = AdjustWindowRect(&window_rectangle, WS_OVERLAPPEDWINDOW, FALSE);
                if(!success)
                {
                    return false;
                }
                window_width_screen = window_rectangle.right - window_rectangle.left;
                window_height_screen = window_rectangle.bottom - window_rectangle.top;
            }
            
            context->window =
                CreateWindowEx(
                    NULL,
                    window_class_name,
                    window_title,
                    WS_OVERLAPPEDWINDOW,
                    300,
                    300,
                    window_width_screen,
                    window_height_screen,
                    NULL,		// We have no parent window, NULL.
                    NULL,		// We aren't using menus, NULL.
                    application_context->instance,
                    NULL		// We aren't using multiple windows, NULL.
                    );

            if(context->window == 0)
            {
                return false;
            }
            
        }

        ShowWindow(context->window, application_context->command_show);
        
        
#if PLATFORM_DEBUG==1
        context->initialized = true;
#endif

        return true;
    }
    
    void
    read_window_messages(
        bool *const quit_requested,
        int *const exit_code
        )
    {
		*quit_requested = false;

        MSG message = {};
        // NOTE: this means "don't filter messages"
        UINT message_filter_min = 0;
        UINT message_filter_max = 0;
        // NOTE: retrieve all messages!
        HWND handle = 0;
        // NOTE: remove after peeking
        UINT remove_message = PM_REMOVE;
    
        while(true)
        {

            BOOL const message_available =
                PeekMessage(
                    &message,
                    handle,
                    message_filter_min,
                    message_filter_max,
                    remove_message
                    );

            if(!message_available)
                break;
        
            switch(message.message)
            {

            case WM_QUIT:
            {
                *exit_code = (int)message.wParam;
                *quit_requested = true;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
                break;
                
            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
            
            }
            
        }
        
    }
    
    uint64
    read_ticks()
    {

        LARGE_INTEGER performance_count;
        BOOL const success =
            QueryPerformanceCounter(
                &performance_count
                );

        // MSDN:
        // "On systems that run Windows XP or later, the function will always
        //  succeed and will thus never return zero."
        ENSURE(success);

        return uint64(performance_count.QuadPart);
    }

    void
    frame_end_sleep(
        Context const*const context,
        uint64 const frame_start_ticks,
        uint const target_frame_rate
        )
    {
        PLATFORM_ENSURE_CONTEXT_INITIALIZED(context);

        uint64 const target_frame_duration_ticks = context->ticks_per_second/target_frame_rate;
        
        // Sleep-spin 1ms at a time until we can't risk it anymore
        while(true)
        {

            uint64 const frame_duration_ticks = read_ticks() - frame_start_ticks;
            
            if(target_frame_duration_ticks <= frame_duration_ticks)
                break;
            
            uint const remaining_whole_milliseconds =
                (uint)((target_frame_duration_ticks - frame_duration_ticks)/context->ticks_per_second)/1000;

            if(remaining_whole_milliseconds <= 1)
            {
                // Don't risk oversleeping, break out and spin-lock instead.
                break;
            }
            else
            {
                Sleep(1);
            }

        }

        while(true)
        {
            uint64 const frame_duration_ticks = read_ticks() - frame_start_ticks;
            if(frame_duration_ticks >= target_frame_duration_ticks)
            {
                break;
            }
        }

    }

    void
    free_file_memory(void *const address)
    {
        free(address);
    }
    
    FileReadResult
    try_alloc_and_read_entire_file(
        char const*const file_name,
        void **const data,
        size_t *const data_size
        )
    {
        *data = 0;
        
        HANDLE file_handle = 0;    
        {
            DWORD const desired_access = GENERIC_READ;
            // NOTE:
            // Others may read the file, but not write to it.
            // This is important because we want to be able to keep the size fixed until we have finished reading
            // the file into memory and closed the handle.
            DWORD const share_mode = FILE_SHARE_READ;
            LPSECURITY_ATTRIBUTES const security_attributes = 0;
            DWORD const creation_disposition = OPEN_EXISTING;
            DWORD const flags_and_attributes = 0;
            HANDLE const template_file_handle = 0;
        
            file_handle =
                CreateFile(
                    file_name,
                    desired_access,
                    share_mode,
                    security_attributes,
                    creation_disposition,
                    flags_and_attributes,
                    template_file_handle
                    );

            bool const failure = file_handle == INVALID_HANDLE_VALUE;
            if(failure)
            {
                
                DWORD const error = GetLastError();
                if(error == ERROR_FILE_NOT_FOUND)
                {
                    return FileReadResult::NotFound;
                }
                else
                {
                    return FileReadResult::OtherError;
                }
                
            }

        }
        ENSURE( file_handle != 0 );
    
        // Get the file size.
        size_t file_size = 0;
        {
            LARGE_INTEGER file_size_64;
            BOOL success = GetFileSizeEx(file_handle, &file_size_64);
            if( success == FALSE )
            {
                return FileReadResult::OtherError;
            }
            else
            {
                static_assert(sizeof(file_size_64.QuadPart) == sizeof(file_size), "unexpected type sizes");
                file_size = size_t(file_size_64.QuadPart);
            }
        }

        if(file_size > UINT32_MAX)
        {
            return FileReadResult::FileTooLarge;
        }
        
        // NOTE: allocate the memory
        *data = malloc(file_size);        
        if(*data == 0)
        {
            return FileReadResult::NotEnoughMemory;
        }
        ENSURE(*data != 0);

        { // NOTE: read file into buffer
            
            DWORD num_bytes_read;
            static_assert(sizeof(DWORD) == sizeof(UINT32), "unexpected size of DWORD");
            DWORD const num_bytes_to_read = DWORD(file_size); // NOTE: we have already checked that this is ok
            LPOVERLAPPED const overlapped = 0;

            BOOL const success =
                ReadFile(
                    file_handle,
                    *data,
                    num_bytes_to_read,
                    &num_bytes_read,
                    overlapped
                    );
    
            if(!success)
            {
                free_file_memory(*data);
                return FileReadResult::OtherError;
            }

            // NOTE/STUDY: as far as I know, this is guaranteed?
            ENSURE(file_size == num_bytes_read);
            *data_size = num_bytes_read;
        }

        { // NOTE: close the file
            BOOL success = CloseHandle(file_handle);
            if(!success)
            {
                return FileReadResult::LeakedFileHandle;
            }
        }

        return FileReadResult::Ok;
    }
    
}
