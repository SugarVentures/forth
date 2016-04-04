package tv.forth.forthviewerandroiddemo;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

/**
 * Created by caominhtrang on 4/4/16.
 */
public class ForthViewer {
    public native int startStreaming(String streamKey, String serverAddress);
    public static native int setup(int w, int h);
    public static native  int init();
    public static native void render();
    public native void setContext(EGLContext context, EGLDisplay display, EGLConfig config);

    private ForthCallback mForthCallback;

    static {
        System.loadLibrary("forthwrapper");
    }

    public ForthViewer(ForthCallback cb)
    {
        mForthCallback = cb;
    }

    public void frameCallback()
    {
        mForthCallback.frameCallback();
    }
}

