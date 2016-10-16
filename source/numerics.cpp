namespace Numerics
{

    inline float
    sin(float x)
    {
        // TODO: intrinsic
        return sinf(x);
    }

    inline float
    lerp_float(float const from, float const to, float const t)
    {
        return from + (to - from)*t;
    }
    
    inline float
    floor(float const x)
    {
        // TODO: intrinsic
        return floorf(x);
    }

    
    
    inline float
    exp_float(float const x)
    {
        // TODO: intrinsic
        return expf(x);
    }    
    
    inline float
    cos(float x)
    {
        // TODO: intrinsic
        return cosf(x);
    }

    inline float
    tan_float(float x)
    {
        // TODO: intrinsic
        return tanf(x);
    }    
    
    inline float
    power(float base, float exponent)
    {
        // TODO: intrinsic
        return powf(base, exponent);
    }


    inline float
    square_root(float x)
    {
        // TODO: intrinsic
        return sqrtf(x);
    }

    inline int
    sign_int(int const x)
    {
        if(x < 0)
        {
            return -1;
        }
        else if(x > 0)
        {
            return +1;
        }
        else
        {
            return 0;
        }
    }
    
// NOTE: inclusive bounds
    inline int
    clamped_int(int const min, int const max, int const x)
    {
        ENSURE( min <= max );
        if( x <= min )
        {
            return min;
        }
        else if( x >= max )
        {
            return max;
        }
        else
        {
            return x;
        }
    }

    inline float
    absolute_value(float x)
    {
        // TODO: intrinsic?
        if( x < 0.0f )
        {
            return -x;
        }
        else
        {
            return +x;
        }
    }
    
    inline float
    clamped(float min, float max, float x)
    {
        // TODO: intrinsic?
        if(x <= min)
        {
            return min;
        }
        else if(x >= max)
        {
            return max;
        }
        else
        {
            return x;
        }
    }

    float
    smoothstep(float const edge0, float const edge1, float const x)
    {
        float const t = clamped(0.0f, 1.0f, (x - edge0)/(edge1 - edge0)); 
        return t*t*(3.0f - 2.0f*t);
    }
    
    inline uint
    max_uint(uint a, uint b)
    {
        if(a > b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }    
    
    inline int
    max_int(int a, int b)
    {
        if(a > b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }

    inline int
    min_int(int a, int b)
    {
        if(a < b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }    
    
    inline float
    max_float(float a, float b)
    {
        // TODO: intrinsic?
        if(a > b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }

    inline float
    min_float(float a, float b)
    {
        // TODO: intrinsic?
        if(a < b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }    

    // NOTE: using the "round down" convention
    inline float
    remainder_float(float const modulus, float const x)
    {
        ENSURE(modulus > 0.0f);

        float const quotient = floor(x/modulus);
        float const result = x - quotient*modulus;
        
        ENSURE(result >= 0.0f);
        ENSURE(result <= modulus);
        
        return result;
    }    

    // NOTE: using the "round down" convention    
    inline int
    remainder(int const modulus, int const x)
    {
        ENSURE(modulus > 0);

        int result = -1;
        // TODO: intrinsic?
        if( x >= 0 )
        {
            result = x % modulus;
        }
        else
        {
            result = modulus - ((-x) % modulus);
        }

        ENSURE(result >= 0);
        ENSURE(result < modulus);
    
        return result;
    }

    inline float
    log_float(float const base, float const x)
    {
        // TODO: intrinsic?
        // NOTE:
        // x = b^log_base(x) = (10^logf(b))^log_base(x) = 10^(logf(b) * log_base(x))
        // logf(x) = logf(b) * log_base(x)
        // log_base(x) = logf(x) / logf(b)
        float const log_base_inv = 1.0f / logf(base);
        float const result = logf(x) * log_base_inv;
        return result;
    }

    inline float
    square(float const x)
    {
        return power(x, 2.0f);
    }

};
