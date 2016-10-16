namespace Matrix4
{

    inline void
    transpose(Mat4 *const m)
    {
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<i; j++)
            {
                float const old_ij = m->element[i][j];
                m->element[i][j] = m->element[j][i];
                m->element[j][i] = old_ij;
            }
        }
    }

    inline void
    product(Mat4 const*const a, Mat4 const*const b, Mat4 *const result)
    {
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<4; j++)
            {
                result->element[i][j] = 0.0f;                
                for(int k=0; k<4; k++)
                {
                    result->element[i][j] += a->element[i][k]*b->element[k][j];
                }
            }
        }
    }        

    inline void
    transformed(Mat4 const*const m, Vec4 const*const a, Vec4 *const result)
    {
        for(int i=0; i<4; i++)
        {
            result->coordinates[i] = 0.0f;
            for(int j=0; j<4; j++)
            {
                result->coordinates[i] += m->element[i][j]*a->coordinates[j];
            }
        }
    }        
    
    
    // NOTE:
    // Mostly for debugging and testing purposes!
    // In more specific circumstances, calculating the inverse is way cheaper than the general case!
    // Example: if the matrix is orthogonal, the inverse is the transpose! Zero multiplications!
    inline float
    inverse(Mat4 const*const a, Mat4 *const r)
    {

        float const*const m = a->elements;
        float *const e = r->elements;

        e[ 0] =
            +
            m[ 5]*m[10]*m[15]
            - 
            m[ 5]*m[11]*m[14]
            - 
            m[ 9]*m[ 6]*m[15]
            + 
            m[ 9]*m[ 7]*m[14]
            +
            m[13]*m[ 6]*m[11]
            - 
            m[13]*m[ 7]*m[10]
            ;

        e[ 4] =
            -
            m[ 4]*m[10]*m[15]
            + 
            m[ 4]*m[11]*m[14]
            + 
            m[ 8]*m[ 6]*m[15]
            - 
            m[ 8]*m[ 7]*m[14]
            - 
            m[12]*m[ 6]*m[11]
            + 
            m[12]*m[ 7]*m[10]
            ;

        e[ 8] =
            +
            m[ 4]*m[ 9]*m[15]
            - 
            m[ 4]*m[11]*m[13]
            - 
            m[ 8]*m[ 5]*m[15]
            + 
            m[ 8]*m[ 7]*m[13]
            + 
            m[12]*m[ 5]*m[11]
            - 
            m[12]*m[ 7]*m[ 9]
            ;

        e[12] =
            -
            m[ 4]*m[ 9]*m[14]
            + 
            m[ 4]*m[10]*m[13]
            +
            m[ 8]*m[ 5]*m[14]
            - 
            m[ 8]*m[ 6]*m[13]
            - 
            m[12]*m[ 5]*m[10]
            + 
            m[12]*m[ 6]*m[ 9]
            ;

        e[ 1] =
            -
            m[ 1]*m[10]*m[15]
            + 
            m[ 1]*m[11]*m[14]
            + 
            m[ 9]*m[ 2]*m[15]
            - 
            m[ 9]*m[ 3]*m[14]
            - 
            m[13]*m[ 2]*m[11]
            + 
            m[13]*m[ 3]*m[10]
            ;

        e[ 5] =
            +
            m[ 0]*m[10]*m[15]
            - 
            m[ 0]*m[11]*m[14]
            - 
            m[ 8]*m[ 2]*m[15]
            + 
            m[ 8]*m[ 3]*m[14]
            + 
            m[12]*m[ 2]*m[11]
            - 
            m[12]*m[ 3]*m[10]
            ;

        e[ 9] =
            -
            m[ 0]*m[ 9]*m[15]
            + 
            m[ 0]*m[11]*m[13]
            + 
            m[ 8]*m[ 1]*m[15]
            - 
            m[ 8]*m[ 3]*m[13]
            - 
            m[12]*m[ 1]*m[11]
            + 
            m[12]*m[ 3]*m[ 9]
            ;

        e[13] =
            +
            m[ 0]*m[ 9]*m[14]
            - 
            m[ 0]*m[10]*m[13]
            - 
            m[ 8]*m[ 1]*m[14]
            + 
            m[ 8]*m[ 2]*m[13]
            + 
            m[12]*m[ 1]*m[10]
            - 
            m[12]*m[ 2]*m[ 9]
            ;

        e[ 2] =
            +
            m[ 1]*m[ 6]*m[15]
            - 
            m[ 1]*m[ 7]*m[14]
            - 
            m[ 5]*m[ 2]*m[15]
            + 
            m[ 5]*m[ 3]*m[14]
            + 
            m[13]*m[ 2]*m[ 7]
            - 
            m[13]*m[ 3]*m[ 6]
            ;

        e[ 6] =
            -
            m[ 0]*m[ 6]*m[15]
            + 
            m[ 0]*m[ 7]*m[14]
            + 
            m[ 4]*m[ 2]*m[15]
            - 
            m[ 4]*m[ 3]*m[14]
            - 
            m[12]*m[ 2]*m[ 7]
            + 
            m[12]*m[ 3]*m[ 6]
            ;

        e[10] =
            +
            m[ 0]*m[ 5]*m[15]
            - 
            m[ 0]*m[ 7]*m[13]
            - 
            m[ 4]*m[ 1]*m[15]
            + 
            m[ 4]*m[ 3]*m[13]
            + 
            m[12]*m[ 1]*m[ 7]
            - 
            m[12]*m[ 3]*m[ 5]
            ;

        e[14] =
            -
            m[ 0]*m[ 5]*m[14]
            + 
            m[ 0]*m[ 6]*m[13]
            + 
            m[ 4]*m[ 1]*m[14]
            - 
            m[ 4]*m[ 2]*m[13]
            - 
            m[12]*m[ 1]*m[ 6]
            + 
            m[12]*m[ 2]*m[ 5]
            ;

        e[ 3] =
            -
            m[ 1]*m[ 6]*m[11]
            + 
            m[ 1]*m[ 7]*m[10]
            + 
            m[ 5]*m[ 2]*m[11]
            - 
            m[ 5]*m[ 3]*m[10]
            - 
            m[ 9]*m[ 2]*m[ 7]
            + 
            m[ 9]*m[ 3]*m[ 6]
            ;

        e[ 7] =
            +
            m[ 0]*m[ 6]*m[11]
            - 
            m[ 0]*m[ 7]*m[10]
            - 
            m[ 4]*m[ 2]*m[11]
            + 
            m[ 4]*m[ 3]*m[10]
            + 
            m[ 8]*m[ 2]*m[ 7]
            - 
            m[ 8]*m[ 3]*m[ 6]
            ;

        e[11] =
            -
            m[ 0]*m[ 5]*m[11]
            + 
            m[ 0]*m[ 7]*m[ 9]
            + 
            m[ 4]*m[ 1]*m[11]
            - 
            m[ 4]*m[ 3]*m[ 9]
            - 
            m[ 8]*m[ 1]*m[ 7]
            + 
            m[ 8]*m[ 3]*m[ 5]
            ;

        e[15] =
            +
            m[ 0]*m[ 5]*m[10]
            - 
            m[ 0]*m[ 6]*m[ 9]
            - 
            m[ 4]*m[ 1]*m[10]
            + 
            m[ 4]*m[ 2]*m[ 9]
            + 
            m[ 8]*m[ 1]*m[ 6]
            - 
            m[ 8]*m[ 2]*m[ 5]
            ;

        float const det = m[0]*e[0] + m[1]*e[4] + m[2]*e[8] + m[3]*e[12];
        float const det_inv = 1.0f / det;

        for(int i=0; i<16; i++)
        {
            e[i] *= det_inv;
        }

        return det;
    }
    
}
