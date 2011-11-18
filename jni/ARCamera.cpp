#include "ARCamera.h"

#ifdef __cplusplus
extern "C"
{
#endif


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
       
	
        //renderButtons(trackable);

		//render all the models
		for (int i = 0; i < model_count; ++i)
		{
			renderModel(trackable, models[i]);
		}
       
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
