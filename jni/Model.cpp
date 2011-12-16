

// Include files
#include "Model.h"
#include "SampleUtils.h"

#include <string.h>

Model::Model() :
    vertices(0),
    normals(0),
    texture_coords(0),
    mVerticesBufferID(0)
{}


Model::~Model()
{
    if (vertices != 0)
        delete[]vertices;
    if (normals != 0)
        delete[]normals;
    if (texture_coords != 0)
        delete[]texture_coords;
}


Model*
Model::create(JNIEnv* env, jobject ModelObject)
{

    Model* newModel = new Model();

    // Handle to the Model class:
    jclass ModelClass = env->GetObjectClass(ModelObject);

//TODO: fix super bad duplication in this class

    // get the getVertices java method refreence:
    jmethodID vertBufferMethodId = env->GetMethodID(ModelClass , "getVertices", "()[F");
    if (!vertBufferMethodId)
    {
        LOG("Function GetVertices() not found.");
        return 0;
    }

    //get Model bytes from Java Model.getData
    jfloatArray vertexBuffer = (jfloatArray)env->CallObjectMethod(ModelObject, vertBufferMethodId);
    if (vertexBuffer == NULL)
    {
        LOG("Get image buffer returned zero pointer");
        return 0;
    }

    int len = (int)env->GetArrayLength(vertexBuffer);
    newModel->num_of_verts =len/3;

    jboolean isCopy; //should be NULL
    float * vertices = env->GetFloatArrayElements(vertexBuffer, &isCopy);
    if (vertices == NULL)
    {
        LOG("Failed to get Model buffer.");
        return 0;
    }

    //all copied over!
    newModel->vertices =  vertices;

    // Release:
    // informs the VM that the native code no longer needs access to vertices ref
    env->ReleaseFloatArrayElements(vertexBuffer, vertices, 0);


////////// for normals
    // get the getVertices java method refreence:
    jmethodID normBufferMethodId = env->GetMethodID(ModelClass , "getNormals", "()[F");
    if (!normBufferMethodId)
    {
        LOG("Function GetNormals() not found.");
        return 0;
    }

    //get Model bytes from Java Model.getData
    jfloatArray normalBuffer = (jfloatArray)env->CallObjectMethod(ModelObject, normBufferMethodId);
    if (normalBuffer == NULL)
    {
        LOG("Get norm buffer returned zero pointer");
        return 0;
    }

    float * normals = env->GetFloatArrayElements(normalBuffer, NULL);
    if (normals == NULL)
    {
        LOG("Failed to get Model buffer.");
        return 0;
    }

    //all copied over!
    newModel->normals =  normals;

    // Release:
    // informs the VM that the native code no longer needs access to vertices ref
    env->ReleaseFloatArrayElements(normalBuffer, normals, 0);
////////// end normals


////////// for texture_coords
    // get the getVertices java method refreence:
    jmethodID text_coordsBufferMethodId = env->GetMethodID(ModelClass , "getTextureCoords", "()[F");
    if (!text_coordsBufferMethodId)
    {
        LOG("Function GetNormals() not found.");
        return 0;
    }

    //get Model bytes from Java Model.getData
    jfloatArray texture_coordBuffer = (jfloatArray)env->CallObjectMethod(ModelObject, text_coordsBufferMethodId);
    if (texture_coordBuffer == NULL)
    {
        LOG("Get norm buffer returned zero pointer");
        return 0;
    }

    float * texture_coords = env->GetFloatArrayElements(texture_coordBuffer, NULL);
    if (texture_coords == NULL)
    {
        LOG("Failed to get Model buffer.");
        return 0;
    }

    //all copied over!
    newModel->texture_coords =  texture_coords;

    // Release:
    // informs the VM that the native code no longer needs access to vertices ref
    env->ReleaseFloatArrayElements(texture_coordBuffer, texture_coords, 0);
    ////////// end texture_coords


    return newModel;
}

