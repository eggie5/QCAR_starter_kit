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
#include <QCAR/ImageTarget.h>
#include <QCAR/CameraCalibration.h>

#include "SampleUtils.h"
#include "Texture.h"
#include "Shaders.h"
#include "Model.h"
#include "SampleMath.h"


//model ref
extern float** Obj_LexusNormals;
extern unsigned int Obj_LexusNumVerts;

// Models:
extern int model_count;
extern Model** models;

// Textures:
extern int texture_count;
extern Texture** textures;

// OpenGL ES 2.0 settings
extern unsigned int shader_program_id ;
extern GLint vertex_handle;
extern GLint normal_handle;
extern GLint texture_coord_handle;
extern GLint mvp_matrix_handle;


// Screen dimensions:
extern unsigned int screen_width;
extern unsigned int screen_height;



// Indicates whether screen is in portrait (true) or landscape (false) mode
//bool is_activity_in_portrait_mode   = false;

// The projection matrix used for rendering virtual objects:
extern QCAR::Matrix44F projection_matrix;
QCAR::Matrix44F model_view_matrix;

// Constants:
static const float scale_factor = 300.f;
static float x_pos=1.0f;
static int lastTrackableId=-1;
extern unsigned int vbo[2];

