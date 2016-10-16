namespace DualQuaternions
{

    using namespace Quaternions;
    
    union DualQuaternion
    {
        
        struct
        {
            Quaternion real;
            Quaternion non_real;
        } part;

        Quaternion parts[2];
        
    };
    
};
