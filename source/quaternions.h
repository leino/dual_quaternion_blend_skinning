namespace Quaternions
{

    union Quaternion
    {
    
        struct
        {
            float x;
            float y;
            float z;
            float w;
        } component;

        struct
        {
            Vec3 vector;
            float scalar;
        } part;

        float components[4];

    };
    
}
