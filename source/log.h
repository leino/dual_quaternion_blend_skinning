#define LOG_OUTPUT_VISUAL_STUDIO_CONSOLE 0
#define LOG_OUTPUT_STDOUT 1

#define LOG_OUTPUT (LOG_OUTPUT_VISUAL_STUDIO_CONSOLE)
#define LOG_VARIABLE_FLOAT(var) { using namespace Log; string(#var); string(" = "); float32(var); newline(); }

namespace Log
{
    
    inline void
    string(char const*const message)
    {
        
#if LOG_OUTPUT == LOG_OUTPUT_VISUAL_STUDIO_CONSOLE
        OutputDebugStringA(message);
#elif LOG_OUTPUT == LOG_OUTPUT_STDOUT
        printf(message);
#endif
        
    }

    inline void
    integer_32(int32 const n)
    {
        // We need a buffer size of ceil(log10(2^32))=10,
        // plus one character for the sign bit, and one for zero termination
        size_t const size = 10+1+1;
        char buffer[size];
        int needed_buffer_size = _snprintf(buffer, size, "%d", n);
        if(needed_buffer_size > size)
        {
            string("warning: log_uint32 buffer size too small");
            ENSURE(false);
            return;
        }
        else if(needed_buffer_size == size)
        {
            string("warning: log_uint32 buffer size too small to append a null terminator");
            ENSURE(false);            
            return;
        }
        ENSURE( buffer[needed_buffer_size] == '\0' );
        string(buffer);
    }

    inline void
    uint32(uint32 const n)
    {
        // We need a buffer size of ceil(log10(2^32))=10,
        // plus one character for the sign bit, and one for zero termination
        size_t const size = 10+1+1;
        char buffer[size];
        int needed_buffer_size = _snprintf(buffer, size, "%d", n);
        if(needed_buffer_size > size)
        {
            string("warning: log_uint32 buffer size too small");
            ENSURE(false);
            return;
        }
        else if(needed_buffer_size == size)
        {
            string("warning: log_uint32 buffer size too small to append a null terminator");
            ENSURE(false);            
            return;
        }
        ENSURE( buffer[needed_buffer_size] == '\0' );
        string(buffer);
    }    
    
    inline void newline()
    {
        string("\n");
    }
    
    inline void float32(float const x)
    {

        // TODO: find out how long a floating point number can be when printed
        // out as a string
        size_t const size = 100;
    
        char buffer[size];
        int needed_buffer_size = _snprintf(buffer, size, "%f", x);
        if(needed_buffer_size > size){
            string("warning: log_float32 buffer size too small");
            return;
        }else if(needed_buffer_size == size){
            string("warning: log_float32 buffer size too small to append a null terminator");
            return;
        }
        ENSURE( buffer[needed_buffer_size] == '\0' );
        string(buffer);
        
    }
    
    void tuple_float32_2(float x, float y)
    {
        string("(");
        float32(x);
        string(", ");
        float32(y);
        string(")");
    
    }    
    
}
