package tv.forth.videoengine;

import android.content.Context;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraManager;
import android.os.Build;
import android.util.Log;


/**
 * Created by caominhtrang on 3/8/16.
 */
public class AndroidVideoSourceInfo {
    private static final String DEBUG_TAG = "ForthTV";
    private Context mContext;

    public AndroidVideoSourceInfo(Context context)
    {
        mContext = context;
    }

    public void getListCaptureDevices() {
        if (android.os.Build.VERSION.RELEASE.startsWith("1."))
            getListCaptureDevicesOldAPI();
        else if (android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP)
            getListCaptureDevicesOldAPI();
        else {
            getListCaptureDevicesNewAPI();
        }

    }

    private void getListCaptureDevicesOldAPI()
    {
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; i++) {
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(i, info);
            Log.d(DEBUG_TAG, "id: " + i);
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                Log.d(DEBUG_TAG, "Front Camera");
            }
            else {
                Log.d(DEBUG_TAG, "Back Camera");
            }
        }
    }

    private void getListCaptureDevicesNewAPI() {
        CameraManager manager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            String[] list = manager.getCameraIdList();
            for (int i = 0; i < list.length; i++)
            {
                Log.d("ForthTV", list[i]);
            }

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
}
