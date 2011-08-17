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

#ifdef __cplusplus
extern "C"
{
#endif

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


JNIEXPORT void JNICALL
Java_com_eggie5_AR_ARRenderer_renderFrame(JNIEnv * env, jobject obj)
{
    // Clear color and depth buffer 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render video background:
    QCAR::State state = QCAR::Renderer::getInstance().begin();
    
  	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


	jclass javaClass = env->GetObjectClass(obj);
	jmethodID method = env->GetMethodID(javaClass, "displayMessage", "(Ljava/lang/String;)V");
	
	if(state.getNumActiveTrackables()<1 &&  lastTrackableId!=-1)
	{
		jstring js = env->NewStringUTF("");
		env->CallVoidMethod(obj, method, js);
		lastTrackableId  =-1;
	}

  

    // Did we find any trackables this frame?
    for(int tIdx = 0; tIdx < state.getNumActiveTrackables(); tIdx++)
    {
        // Get the trackable:
        const QCAR::Trackable* trackable = state.getActiveTrackable(tIdx);
		
        if (trackable->getId() != lastTrackableId) 
		{    
				jstring js = env->NewStringUTF(trackable->getName());
				env->CallVoidMethod(obj, method, js);
				lastTrackableId = trackable->getId();
		}
        QCAR::Matrix44F model_view_matrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());        
        
        //we only have 1 tex loaded so just choose the first
        const Texture* const thisTexture = textures[0];

        SampleUtils::translatePoseMatrix(0.0f, 0.0f, 0.0f,   &model_view_matrix.data[0]);
        SampleUtils::rotatePoseMatrix(x_pos, 0.0f, 0.0f, 1.0f,  &model_view_matrix.data[0]);
        x_pos++;
        
        SampleUtils::scalePoseMatrix(scale_factor, scale_factor, scale_factor, &model_view_matrix.data[0]);
        
        //mv + projection matrix
        QCAR::Matrix44F modelViewProjection;
                                             
        SampleUtils::multiplyMatrix(&projection_matrix.data[0], &model_view_matrix.data[0] ,  &modelViewProjection.data[0]);

        glUseProgram(shader_program_id);
         
        //set vars in shader program
       // glVertexAttribPointer(vertex_handle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &Obj_LexusVerts[0]);
		//VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer(vertex_handle, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glVertexAttribPointer(normal_handle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &Obj_LexusNormals[0]);
        
		//glVertexAttribPointer(texture_coord_handle, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &Obj_LexusTexCoords[0]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glVertexAttribPointer(texture_coord_handle, 2, GL_FLOAT, GL_FALSE, 0, 0);
		
        //set modelViewProjectionMatrix var in shader
        glUniformMatrix4fv(mvp_matrix_handle, 1, GL_FALSE,  (GLfloat*)&modelViewProjection.data[0] );
        
        glEnableVertexAttribArray(vertex_handle);
        glEnableVertexAttribArray(normal_handle);
        glEnableVertexAttribArray(texture_coord_handle);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        
        
        glDrawArrays(GL_TRIANGLES, 0, Obj_LexusNumVerts);

        SampleUtils::checkGlError("ARCamera renderFrame");


    }

    glDisable(GL_DEPTH_TEST);


    glDisableVertexAttribArray(vertex_handle);
    glDisableVertexAttribArray(normal_handle);
    glDisableVertexAttribArray(texture_coord_handle);


    QCAR::Renderer::getInstance().end();
}



JNIEXPORT void JNICALL
Java_com_eggie5_AR_ARCamera_nativeTouch(JNIEnv *, jobject)
{ 
   x_pos=1;
}


void
configureVideoBackground()
{
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


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
Java_com_eggie5_AR_ARCamera_startCamera(JNIEnv *,
                                                                         jobject)
{
    LOG("Java_com_eggie5_AR_ARCamera_startCamera");

    // Initialize the camera:
    if (!QCAR::CameraDevice::getInstance().init())
        return;

    // Configure the video background
    configureVideoBackground();

    // Select the default mode:
    if (!QCAR::CameraDevice::getInstance().selectVideoMode(
                                QCAR::CameraDevice::MODE_DEFAULT))
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
    projection_matrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f,
                                            2000.0f);
}


JNIEXPORT void JNICALL
Java_com_eggie5_AR_ARCamera_stopCamera(JNIEnv *,
                                                                   jobject)
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

	//VBO setup
	glGenBuffers(2, vbo);
	
    glBindBuffer (GL_ARRAY_BUFFER, vbo[0]);
	//size of arry * 4 (bytes in a float)
    glBufferData (GL_ARRAY_BUFFER, (3*Obj_LexusNumVerts)*4, Obj_LexusVerts, GL_STATIC_DRAW);

	glBindBuffer (GL_ARRAY_BUFFER, vbo[1]);
	//size of arry * 4 (bytes in a float)
    glBufferData (GL_ARRAY_BUFFER, (2*Obj_LexusNumVerts)*4, Obj_LexusTexCoords, GL_STATIC_DRAW);
    // glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
    //     glBufferData (GL_ELEMENT_ARRAY_BUFFER, 2*4, indices, GL_STATIC_DRAW);

	// delete Obj_LexusVerts; Obj_LexusVerts=NULL;
	// delete Obj_LexusTexCoords; Obj_LexusTexCoords=NULL;
    
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
