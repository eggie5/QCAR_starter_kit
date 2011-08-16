package com.eggie5.AR;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

import com.qualcomm.QCAR.QCAR;

import android.os.Message;
import android.content.Context;
import android.os.Handler;


/** The renderer class for the ARCamera sample. */
public class ARRenderer implements GLSurfaceView.Renderer
{
	// FPS counter.
   private int mFrameCount = 0;
   private long mStartTime = System.nanoTime();
    public boolean mIsActive = false;

    
    /** Native function for initializing the renderer. */
    public native void initRendering();
    
    
    /** Native function to update the renderer. */
    public native void updateRendering(int width, int height);

    
    /** Called when the surface is created or recreated. */
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        DebugLog.LOGD("GLRenderer::onSurfaceCreated");

        // Call native function to initialize rendering:
        initRendering();
        
        // Call QCAR function to (re)initialize rendering after first use
        // or after OpenGL ES context was lost (e.g. after onPause/onResume):
        QCAR.onSurfaceCreated();
    }
    
    
    /** Called when the surface changed size. */
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        DebugLog.LOGD("GLRenderer::onSurfaceChanged");
        
        // Call native function to update rendering when render surface parameters have changed:
        updateRendering(width, height);

        // Call QCAR function to handle render surface size changes:
        QCAR.onSurfaceChanged(width, height);
    }    
    
    
    /** The native render function. */    
    public native void renderFrame();
    
	// A handler object for sending messages to the main activity thread
    public static Handler mainActivityHandler;

    // Called from native to display a message
    public void displayMessage(String text)
    {
        // We use a handler because this thread cannot change the UI
    	Message message = new Message();
    	message.obj = text;
        mainActivityHandler.sendMessage(message);
    }
    
    /** Called to draw the current frame. */
    public void onDrawFrame(GL10 gl)
    {
        if (!mIsActive)
            return;

        // Call our native function to render content
         renderFrame();

	 ++mFrameCount;
	      if (mFrameCount % 50 == 0) {
	          long now = System.nanoTime();
	          double elapsedS = (now - mStartTime) / 1.0e9;
	          double msPerFrame = (1000 * elapsedS / mFrameCount);
	          DebugLog.LOGD("ms / frame: " + msPerFrame + " - fps: " + (1000 / msPerFrame));

	          mFrameCount = 0;
	          mStartTime = now;
	      }
    }
}
