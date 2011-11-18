
#ifndef _QCAR_Model_H_
#define _QCAR_Model_H_

// Include files
#include <jni.h>

// Forward declarations

/// A utility class for Models.
class Model
{
public:

    /// Constructor
    Model();

    /// Destructor.
    ~Model();


    /// Create a Model from a jni object:
    static Model* create(JNIEnv* env, jobject ModelObject);



    /// The pointer to the raw Model data.
    unsigned char* mData;
    float* vertices;
    float* normals;
    float* texture_coords;
    /// The width of the texture.
    int num_of_verts;
    /// The ID of the texture
    unsigned int mVerticesBufferID;

};


#endif //_QCAR_Model_H_