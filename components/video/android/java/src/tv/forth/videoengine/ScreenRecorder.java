package tv.forth.videoengine;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.Image;
import android.media.ImageReader;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Handler;
import android.os.Looper;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.WindowManager;

import java.nio.ByteBuffer;

import tv.forth.broadcaster.ForthBroadcasterWrapper;

/**
 * Created by caominhtrang on 3/10/16.
 */
public class ScreenRecorder {
    private static final String TAG = "ForthTV ScreenRecorder";
    private static final String FORTH_SCREEN_RECORDER_NAME = "forthscreenrecorder";
    public static final int REQUEST_CODE = 100;
    private static final int VIRTUAL_DISPLAY_FLAGS = DisplayManager.VIRTUAL_DISPLAY_FLAG_OWN_CONTENT_ONLY | DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC;

    private static MediaProjection sMediaProjection;

    private MediaProjectionManager mProjectionManager;
    private Context mApplicationContext;
    private ImageReader mImageReader;
    private Handler mHandler;
    private Display mDisplay;
    private VirtualDisplay mVirtualDisplay;
    private int mDensity;
    private int mWidth;
    private int mHeight;
    private int mRotation;
    private OrientationChangeCallback mOrientationChangeCallback;
    private ForthBroadcasterWrapper mBroadcasterEngine;

    public ScreenRecorder(Context context, ForthBroadcasterWrapper engine)
    {
        mApplicationContext = context;
        mBroadcasterEngine = engine;
    }

    public void initialize()
    {
        mProjectionManager = (MediaProjectionManager) mApplicationContext.getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        // start capture handling thread
        new Thread() {
            @Override
            public void run() {
                Looper.prepare();
                mHandler = new Handler();
                Looper.loop();
            }
        }.start();
    }

    public void start() {
        Log.d(TAG, "Start recording");
        ((Activity) mApplicationContext).startActivityForResult(mProjectionManager.createScreenCaptureIntent(), REQUEST_CODE);
    }

    public void stop() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (sMediaProjection != null) {
                    sMediaProjection.stop();
                }
            }
        });
    }

    private class OrientationChangeCallback extends OrientationEventListener {
        public OrientationChangeCallback(Context context) {
            super(context);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            synchronized (this) {
                final int rotation = mDisplay.getRotation();
                if (rotation != mRotation) {
                    mRotation = rotation;
                    try {
                        // clean up
                        if(mVirtualDisplay != null) mVirtualDisplay.release();
                        if(mImageReader != null) mImageReader.setOnImageAvailableListener(null, null);

                        // re-create virtual display depending on device width / height
                        createVirtualDisplay();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    private class ImageAvailableListener implements ImageReader.OnImageAvailableListener {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = null;

            try {
                image = mImageReader.acquireLatestImage();
                if (image != null) {
                    Image.Plane[] planes = image.getPlanes();
                    ByteBuffer buffer = planes[0].getBuffer();
                    int pixelStride = planes[0].getPixelStride();
                    int rowStride = planes[0].getRowStride();
                    int rowPadding = rowStride - pixelStride * mWidth;
                    Log.e(TAG, "info: " + planes.length + " " + image.getWidth() + " " + image.getHeight());

                    mBroadcasterEngine.pushVideoFrame(buffer, image.getWidth(), image.getHeight(), rowStride);
                }

            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if (image!=null) {
                    image.close();
                }
            }
        }
    }


    private void createVirtualDisplay() {
        // get width and height
        Point size = new Point();
        mDisplay.getSize(size);
        mWidth = size.x;
        mHeight = size.y;


        // start capture reader
        mImageReader = ImageReader.newInstance(mWidth, mHeight, PixelFormat.RGBA_8888, 2);
        mVirtualDisplay = sMediaProjection.createVirtualDisplay(FORTH_SCREEN_RECORDER_NAME, mWidth, mHeight, mDensity, VIRTUAL_DISPLAY_FLAGS, mImageReader.getSurface(), null, mHandler);
        mImageReader.setOnImageAvailableListener(new ImageAvailableListener(), mHandler);
    }

    private void onActivityOnResult(Intent data)
    {
        Log.d(TAG, "onActivityOnResult");
        sMediaProjection = mProjectionManager.getMediaProjection(-1, data);
        if (sMediaProjection != null) {
            // display metrics
            DisplayMetrics metrics = mApplicationContext.getResources().getDisplayMetrics();
            mDensity = metrics.densityDpi;
            WindowManager windowManager = (WindowManager) mApplicationContext
                    .getSystemService(Context.WINDOW_SERVICE);
            windowManager.getDefaultDisplay().getMetrics(metrics);
            mDisplay = windowManager.getDefaultDisplay();

            // create virtual display depending on device width / height
            createVirtualDisplay();

            // register orientation change callback
            /*mOrientationChangeCallback = new OrientationChangeCallback(mApplicationContext);
            if (mOrientationChangeCallback.canDetectOrientation()) {
                mOrientationChangeCallback.enable();
            }*/

            // register media projection stop callback
            sMediaProjection.registerCallback(new MediaProjectionStopCallback(), mHandler);
        }
    }

    private class MediaProjectionStopCallback extends MediaProjection.Callback {
        @Override
        public void onStop() {
            Log.d(TAG, "stopping projection.");
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mVirtualDisplay != null) mVirtualDisplay.release();
                    if (mImageReader != null) mImageReader.setOnImageAvailableListener(null, null);
                    if (mOrientationChangeCallback != null) mOrientationChangeCallback.disable();
                    sMediaProjection.unregisterCallback(MediaProjectionStopCallback.this);
                }
            });
        }
    }



}
