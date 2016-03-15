package tv.forth.broadcaster;

import android.content.Context;
import android.content.Intent;

import java.nio.ByteBuffer;

/**
 * Created by caominhtrang on 3/9/16.
 */
public class ForthBroadcasterWrapper {
    static {
        System.loadLibrary("forthwrapper");
    }

    public static native void Initialize(Context context);
    public native void setupCaptureSession(ForthBroadcasterWrapper wrapper);
    public native int startCaptureSession();
    public native void runCaptureSession(Intent intent);
    public native void pushVideoFrame(ByteBuffer frame, int width, int height, int stride);
    public native void startStreaming(String streamKey, String serverAddress);
}
