#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/CameraCalibration.h>

#include "SampleUtils.h"
#include "Texture.h"
#include "Shaders.h"
#include "Obj_lexus.h"
#include "Model.h"



#ifdef __cplusplus
extern "C"
{
#endif

// Models:
    int model_count  = 0;
    Model** models  = 0;

// Textures:
    int texture_count  = 0;
    Texture** textures  = 0;

// OpenGL ES 2.0 settings
    unsigned int shader_program_id  = 0;
    GLint vertex_handle              = 0;
    GLint normal_handle              = 0;
    GLint texture_coord_handle        = 0;
    GLint mvp_matrix_handle           = 0;


// Screen dimensions:
    unsigned int screen_width        = 0;
    unsigned int screen_height       = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
    bool is_activity_in_portrait_mode   = false;

// The projection matrix used for rendering virtual objects:
    QCAR::Matrix44F projection_matrix;

// Constants:
    static const float scale_factor = 300.f;
    static float x_pos=1.0f;
    static int lastTrackableId=-1;
    unsigned int vbo[2];


    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARCamera_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
    {
        is_activity_in_portrait_mode = isPortrait;
    }


    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARCamera_onQCARInitializedNative(JNIEnv *, jobject)
    {
        // Comment in to enable tracking of up to 2 targets simultaneously and
        // split the work over multiple frames:
        // QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
        // QCAR::setHint(QCAR::HINT_IMAGE_TARGET_MULTI_FRAME_ENABLED, 1);
    }





    void
    configureVideoBackground()
    {
        // Get the default video mode:
        QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
        QCAR::VideoMode videoMode = cameraDevice.getVideoMode(QCAR::CameraDevice::MODE_OPTIMIZE_SPEED);

        //cameraDevice.selectVideoMode(QCAR::CameraDevice::MODE_OPTIMIZE_SPEED);
        //QCAR::CameraDevice::getInstance().selectVideoMode(QCAR::CameraDevice::MODE_OPTIMIZE_SPEED);


        // Configure the video background
        QCAR::VideoBackgroundConfig config;
        config.mEnabled = true;
        config.mSynchronous = true;
        config.mPosition.data[0] = 0.0f;
        config.mPosition.data[1] = 0.0f;

        if (is_activity_in_portrait_mode)
        {
            //LOG("configureVideoBackground PORTRAIT");
            config.mSize.data[0] = videoMode.mHeight* (screen_height / (float)videoMode.mWidth);
            config.mSize.data[1] = screen_height;
        }
        else
        {
            //LOG("configureVideoBackground LANDSCAPE");
            config.mSize.data[0] = screen_width;
            config.mSize.data[1] = videoMode.mHeight
                                   * (screen_width / (float)videoMode.mWidth);
        }

        // Set the config:
        QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
    }


    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARCamera_initApplicationNative(
        JNIEnv* env, jobject obj, jint width, jint height)
    {
        LOG("Java_com_eggie5_AR_ARCamera_initApplicationNative");

        // Store screen dimensions
        screen_width = width;
        screen_height = height;

        // Handle to the activity class:
        jclass activityClass = env->GetObjectClass(obj);

        //load models
        jmethodID getModelCountMethodID = env->GetMethodID(activityClass,
                                          "getModelCount", "()I");
        if (getModelCountMethodID == 0)
        {
            LOG("Function getModelCount() not found.");
            return;
        }

        model_count = env->CallIntMethod(obj, getModelCountMethodID);
        if (!model_count)
        {
            LOG("getModelCount() returned zero.");
            return;
        }

        models = new Model*[model_count];

        //load model from java layer
        jmethodID getModelMethodID = env->GetMethodID(activityClass,  "getModel", "(I)Lcom/eggie5/AR/Model;");

        if (getModelMethodID == 0)
        {
            LOG("Function getModel() not found.");
            return;
        }

        for (int i = 0; i < model_count; ++i)
        {

            //call to get texture in javacode
            jobject modelObject = env->CallObjectMethod(obj, getModelMethodID, i);
            if (modelObject == NULL)
            {
                LOG("GetModel() returned zero pointer");
                return;
            }

            models[i]= Model::create(env, modelObject);
        }



//--------------

        //load textures
        jmethodID getTextureCountMethodID = env->GetMethodID(activityClass,
                                            "getTextureCount", "()I");
        if (getTextureCountMethodID == 0)
        {
            LOG("Function getTextureCount() not found.");
            return;
        }

        texture_count = env->CallIntMethod(obj, getTextureCountMethodID);
        if (!texture_count)
        {
            LOG("getTextureCount() returned zero.");
            return;
        }

        textures = new Texture*[texture_count];

        jmethodID getTextureMethodID = env->GetMethodID(activityClass,
                                       "getTexture", "(I)Lcom/eggie5/AR/Texture;");

        if (getTextureMethodID == 0)
        {
            LOG("Function getTexture() not found.");
            return;
        }

        // Register the textures
        for (int i = 0; i < texture_count; ++i)
        {

            //call to get texture in javacode
            jobject textureObject = env->CallObjectMethod(obj, getTextureMethodID, i);
            if (textureObject == NULL)
            {
                LOG("GetTexture() returned zero pointer");
                return;
            }

            textures[i] = Texture::create(env, textureObject);
        }
    }


    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARCamera_deinitApplicationNative(
        JNIEnv* env, jobject obj)
    {
        LOG("Java_com_eggie5_AR_ARCamera_deinitApplicationNative");

        // Release texture resources
        if (textures != 0)
        {
            for (int i = 0; i < texture_count; ++i)
            {
                delete textures[i];
                textures[i] = NULL;
            }

            delete[]textures;
            textures = NULL;

            texture_count = 0;
        }
    }


    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARCamera_startCamera(JNIEnv *,  jobject)
    {
        LOG("Java_com_eggie5_AR_ARCamera_startCamera");

        // Initialize the camera:
        if (!QCAR::CameraDevice::getInstance().init())
            return;

        // Configure the video background
        configureVideoBackground();

        // Select the default mode:
        if (!QCAR::CameraDevice::getInstance().selectVideoMode(
                    QCAR::CameraDevice::MODE_OPTIMIZE_SPEED))
            return;

        // Start the camera:
        if (!QCAR::CameraDevice::getInstance().start())
            return;


        // Start the tracker:
        QCAR::Tracker::getInstance().start();

        // Cache the projection matrix:
        const QCAR::Tracker& tracker = QCAR::Tracker::getInstance();
        const QCAR::CameraCalibration& cameraCalibration =
            tracker.getCameraCalibration();
        projection_matrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f,  5000.0f);
    }


    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARCamera_stopCamera(JNIEnv *, jobject)
    {
        LOG("Java_com_eggie5_AR_ARCamera_stopCamera");

        QCAR::Tracker::getInstance().stop();

        QCAR::CameraDevice::getInstance().stop();
        QCAR::CameraDevice::getInstance().deinit();
    }

    JNIEXPORT jboolean JNICALL
    Java_com_eggie5_AR_ARCamera_toggleFlash(JNIEnv*, jobject, jboolean flash)
    {
        return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_eggie5_AR_ARCamera_autofocus(JNIEnv*, jobject)
    {
        return QCAR::CameraDevice::getInstance().startAutoFocus()?JNI_TRUE:JNI_FALSE;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_eggie5_AR_ARCamera_setFocusMode(JNIEnv*, jobject, jint mode)
    {
        return QCAR::CameraDevice::getInstance().setFocusMode(mode)?JNI_TRUE:JNI_FALSE;
    }



    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARRenderer_initRendering(   JNIEnv* env, jobject obj)
    {
        LOG("Java_com_eggie5_AR_ARRenderer_initRendering");

        LOG("******************************");
        LOG("GL_VENDOR: %s", glGetString(GL_VENDOR));
        LOG("GL_RENDERER: %s", glGetString(GL_RENDERER));
        LOG("GL_VERSION: %s", glGetString(GL_VERSION));
        LOG("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        LOG("GL_EXTENSIONS: %s", glGetString(GL_EXTENSIONS));
        LOG("**************************_____");

        // Define clear color
        glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);

        if(model_count>0)
        {
            //VBO setup
            int _len=(models[0]->num_of_verts*3)*4;
            LOG("*** model len=%d", _len);

            glGenBuffers(2, vbo);

            glBindBuffer (GL_ARRAY_BUFFER, vbo[0]);

            //size of arry * 4 (bytes in a float)
            glBufferData (GL_ARRAY_BUFFER, _len, models[0]->vertices, GL_STATIC_DRAW);
            LOG("*** AFTER VBO SETUP CODE****");
            glBindBuffer (GL_ARRAY_BUFFER, vbo[1]);
            //size of arry * 4 (bytes in a float)
            glBufferData (GL_ARRAY_BUFFER, (models[0]->num_of_verts*2)*4, models[0]->texture_coords, GL_STATIC_DRAW);
            // glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
            //     glBufferData (GL_ELEMENT_ARRAY_BUFFER, 2*4, indices, GL_STATIC_DRAW);

            // delete Obj_LexusVerts;
            // 	 delete Obj_LexusTexCoords;
        }

        // Now generate the OpenGL texture objects and add settings
        for (int i = 0; i < texture_count; ++i)
        {
            //get id for texture
            glGenTextures(1, &(textures[i]->mTextureID));

            //active this texture
            glBindTexture(GL_TEXTURE_2D, textures[i]->mTextureID);

            //scale texture (interplate)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //set w/ image data
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[i]->mWidth,
                         textures[i]->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         (GLvoid*)  textures[i]->mData);
        }



        //compile shader
        shader_program_id     = SampleUtils::createProgramFromBuffer(vertex_shader_literal,  fragment_shader_literal);


        //get ref's to shader variables - set them in render function
        vertex_handle        = glGetAttribLocation(shader_program_id,   "vertexPosition");
        normal_handle        = glGetAttribLocation(shader_program_id, "vertexNormal");
        texture_coord_handle  = glGetAttribLocation(shader_program_id,  "vertexTexCoord");
        mvp_matrix_handle     = glGetUniformLocation(shader_program_id,    "modelViewProjectionMatrix");



    }


    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARRenderer_updateRendering(
        JNIEnv* env, jobject obj, jint width, jint height)
    {
        LOG("Java_com_eggie5_AR_ARRenderer_updateRendering");

        // Update screen dimensions
        screen_width = width;
        screen_height = height;

        // Reconfigure the video background
        configureVideoBackground();
    }


#ifdef __cplusplus
}
#endif