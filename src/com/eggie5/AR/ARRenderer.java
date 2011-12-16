package com.eggie5.AR;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.IntBuffer;

import android.opengl.GLSurfaceView;

import com.qualcomm.QCAR.QCAR;

import android.os.Message;
import android.content.Context;
import android.os.Handler;

import android.widget.Toast;
import android.os.Environment;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;


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

	public void screenshot(float top_left_x, float top_left_y, float bottom_right_x, float bottom_right_y)
	{
		DebugLog.LOGI("screenshot go ("+top_left_x+"," +top_left_y+")"+" - ("+bottom_right_x+", "+bottom_right_y+")");
		SavePNG((int)top_left_x, (int)top_left_y, (int)bottom_right_x-(int)top_left_x, (int)bottom_right_y-(int)top_left_y, mFrameCount+"_screen.png", gl_handle);
	}

    public void SavePNG(int x, int y, int w, int h, String name, GL10 gl)
    {
        Bitmap bmp=SavePixels(x,y,w,h,gl);
        try
        {
        	String path = Environment.getExternalStorageDirectory() + "/Android/data/com.qualcomm.tween/screens/";
            File file = new File(path, name);
            
            //File file = new File("/sdcard", name);
            try
            {
                file.createNewFile();
            }
            catch (IOException e1)
            {
                e1.printStackTrace();
            }
            FileOutputStream fos=new FileOutputStream(file);
            bmp.compress(CompressFormat.PNG, 100, fos);
            try
            {
                fos.flush();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
            try
            {
                fos.close();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }

        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        } 
    }

    public Bitmap SavePixels(int x, int y, int w, int h, GL10 gl)
    { 
        int b[]=new int[w*(y+h)];
        int bt[]=new int[w*h];
        IntBuffer ib=IntBuffer.wrap(b);
        ib.position(0);
        gl.glReadPixels(x, 0, w, y+h, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, ib);

        for(int i=0, k=0; i<h; i++, k++)
        {//remember, that OpenGL bitmap is incompatible with Android bitmap
            //and so, some correction need. 
            for(int j=0; j<w; j++)
            {
                int pix=b[i*w+j];
                int pb=(pix>>16)&0xff;
                int pr=(pix<<16)&0x00ff0000;
                int pix1=(pix&0xff00ff00) | pr | pb;
                bt[(h-k-1)*w+j]=pix1;
            }
        }

        Bitmap sb=Bitmap.createBitmap(bt, w, h, Bitmap.Config.ARGB_8888);
        return sb;
    }

	private GL10 gl_handle=null;
    
    /** Called to draw the current frame. */
    public void onDrawFrame(GL10 gl)
    {
        if (!mIsActive)
            return;

		gl_handle=gl;

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
