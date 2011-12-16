#include "ARCamera.h"

#ifdef __cplusplus
extern "C"
{
#endif

    QCAR::Vec2F cameraPointToScreenPoint(QCAR::Vec2F cameraPoint)
    {
        QCAR::VideoMode videoMode = QCAR::CameraDevice::getInstance().getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);
        QCAR::VideoBackgroundConfig config = QCAR::Renderer::getInstance().getVideoBackgroundConfig();

        int xOffset = ((int) screen_width - config.mSize.data[0]) / 2.0f + config.mPosition.data[0];
        int yOffset = ((int) screen_width - config.mSize.data[1]) / 2.0f - config.mPosition.data[1];

        if (false	)//isActivityInPortraitMode
        {
            // camera image is rotated 90 degrees
            int rotatedX = videoMode.mHeight - cameraPoint.data[1];
            int rotatedY = cameraPoint.data[0];

            return QCAR::Vec2F(rotatedX * config.mSize.data[0] / (float) videoMode.mHeight + xOffset,
                               rotatedY * config.mSize.data[1] / (float) videoMode.mWidth + yOffset);
        }
        else
        {
            return QCAR::Vec2F(cameraPoint.data[0] * config.mSize.data[0] / (float) videoMode.mWidth + xOffset,
                               cameraPoint.data[1] * config.mSize.data[1] / (float) videoMode.mHeight + yOffset);
        }
    }

    void screenshot(const QCAR::Trackable* trackable, JNIEnv * env, jobject obj)
    {
        const QCAR::ImageTarget* target = static_cast<const QCAR::ImageTarget*>(trackable);
        QCAR::Vec2F target_size = target->getSize();
        //LOG("size=%f,%f", target_size.data[0], target_size.data[1]);
        GLfloat x= target_size.data[0]/2;
        GLfloat y = target_size.data[1]/2;
        const QCAR::Tracker& tracker = QCAR::Tracker::getInstance();
        const QCAR::CameraCalibration& cameraCalibration = tracker.getCameraCalibration();
        QCAR::Vec2F cameraPoint_top_left = QCAR::Tool::projectPoint(cameraCalibration, trackable->getPose(), QCAR::Vec3F(-x,y,0));
        QCAR::Vec2F top_left = cameraPointToScreenPoint(cameraPoint_top_left);

        QCAR::Vec2F cameraPoint_bottom_right = QCAR::Tool::projectPoint(cameraCalibration, trackable->getPose(), QCAR::Vec3F(x,-y,0));
        QCAR::Vec2F bottom_right = cameraPointToScreenPoint(cameraPoint_bottom_right);

        if(top_left.data[0]<0 || top_left.data[1]<0 || bottom_right.data[0]<0 || bottom_right.data[1]<0)
        {
            LOG("NEGATIVE INDICIES");
            return;
        }

        float width=bottom_right.data[0]-top_left.data[0];
        float height=bottom_right.data[1]-top_left.data[1];

        if(width<0 || height<0)
        {
            LOG("NEGATIVE SIZE");
            return;
        }


        LOG("top_left=(%f,	%f)  bottom_right=(%f,  %f) --- width:%f, height:%f",
            top_left.data[0], top_left.data[1], bottom_right.data[0], bottom_right.data[1], width, height);

        //send xy to java.
        jclass javaClass = env->GetObjectClass(obj);
        jmethodID method = env->GetMethodID(javaClass, "screenshot", "(FFFF)V");
        env->CallVoidMethod(obj, method, top_left.data[0], top_left.data[1], bottom_right.data[0], bottom_right.data[1]);


    }

    int cc=0;
    void outlineTarget(const QCAR::Trackable* trackable, JNIEnv * env, jobject obj)
    {
        SampleUtils::checkGlError("method start");
        //glEnable(GL_TEXTURE_2D);
        //SampleUtils::checkGlError("GL_TEXTURE_2D");
        glEnable(GL_BLEND);
        SampleUtils::checkGlError("GL_BLEND");
        glBlendFunc(GL_ONE, GL_SRC_COLOR);
        SampleUtils::checkGlError("get GL_ONE, GL_SRC_COLOR");

        model_view_matrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());
        SampleUtils::checkGlError("get model_view_matrix");

        const QCAR::ImageTarget* target = static_cast<const QCAR::ImageTarget*>(trackable);



        QCAR::Vec2F target_size = target->getSize();
        //LOG("size=%f,%f", target_size.data[0], target_size.data[1]);
        GLfloat x= target_size.data[0]/2;
        GLfloat y = target_size.data[1]/2;

        //LOG("x: %f  y: %f", x, y);

        GLfloat vbVertices[12];
        vbVertices[0]=-x;
        vbVertices[1]=y;
        vbVertices[2]=0.0f;
        vbVertices[3]=-x;
        vbVertices[4]=-y;
        vbVertices[5]=0.0f;
        vbVertices[6]=x;
        vbVertices[7]=y;
        vbVertices[8]=0.0f;
        vbVertices[9]=x;
        vbVertices[10]=-y;
        vbVertices[11]=0.0f;

        const QCAR::Tracker& tracker = QCAR::Tracker::getInstance();




        SampleUtils::checkGlError("create vbverticies");

        //mv + projection matrix
        QCAR::Matrix44F modelViewProjection;
        SampleUtils::multiplyMatrix(&projection_matrix.data[0], &model_view_matrix.data[0] ,  &modelViewProjection.data[0]);

        glUseProgram(shader_program_id);

        glVertexAttribPointer(vertex_handle, 3, GL_FLOAT, GL_FALSE,  0, (const GLvoid*) &vbVertices[0]);

        glEnableVertexAttribArray(vertex_handle);

        glUniformMatrix4fv(mvp_matrix_handle, 1, GL_FALSE,  (GLfloat*)&modelViewProjection.data[0]);

        glLineWidth(5.0f);
        glDrawArrays(GL_LINES, 0, 4);

        SampleUtils::checkGlError("gldrawarrays");

        glDisableVertexAttribArray(vertex_handle);




    }



    void
    renderModel(const QCAR::Trackable* trackable, const Model* m)
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        const Texture* const thisTexture = textures[0];

        //mv + projection matrix
        QCAR::Matrix44F modelViewProjection;

        glUseProgram(shader_program_id);

        //set vars in shader program
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(vertex_handle, 3, GL_FLOAT, GL_FALSE,  0, 0);
        glVertexAttribPointer(normal_handle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &m->normals[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glVertexAttribPointer(texture_coord_handle, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(vertex_handle);
        glEnableVertexAttribArray(normal_handle);
        glEnableVertexAttribArray(texture_coord_handle);

        glActiveTexture(GL_TEXTURE0);

        model_view_matrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());
        QCAR::Matrix44F transform = SampleMath::Matrix44FIdentity();
        float* transformPtr = &transform.data[0];


        SampleUtils::scalePoseMatrix(scale_factor, scale_factor, scale_factor, &model_view_matrix.data[0]);
        SampleUtils::translatePoseMatrix(-0.5f, 0.0f, 0.0f,  &model_view_matrix.data[0]);
        SampleUtils::multiplyMatrix(&projection_matrix.data[0], &model_view_matrix.data[0] ,  &modelViewProjection.data[0]);
        glUniformMatrix4fv(mvp_matrix_handle, 1, GL_FALSE,  (GLfloat*)&modelViewProjection.data[0]);

        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);


        glDrawArrays(GL_TRIANGLES, 0, m->num_of_verts);

        SampleUtils::checkGlError("ARPlayer renderFrame");

        glDisable(GL_DEPTH_TEST);

        glDisableVertexAttribArray(vertex_handle);
        glDisableVertexAttribArray(normal_handle);
        glDisableVertexAttribArray(texture_coord_handle);

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

        SampleUtils::checkGlError("renderFrame:  glEnable(GL_CULL_FACE);");


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
            SampleUtils::checkGlError("renderFrame: getActiveTrackable");

            outlineTarget(trackable, env, obj);

            if (trackable->getId() != lastTrackableId )
            {   cc++;
                if(cc>100) {
                    cc=0;
                    jstring js = env->NewStringUTF(trackable->getName());
                    env->CallVoidMethod(obj, method, js);
                    lastTrackableId = trackable->getId();

                    //take screenshot!
                    screenshot(trackable, env, obj);
                }

            }



            //renderButtons(trackable);

            //render all the models
            // for (int i = 0; i < model_count; ++i)
            // 		{
            // 			renderModel(trackable, models[i]);
            // 		}

        }


        QCAR::Renderer::getInstance().end();
    }



    JNIEXPORT void JNICALL
    Java_com_eggie5_AR_ARCamera_nativeTouch(JNIEnv *, jobject)
    {
        x_pos=1;
    }


#ifdef __cplusplus
}
#endif
