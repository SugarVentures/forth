package tv.forth.videoengine;

import android.content.Context;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Build;
import android.util.Log;
import android.util.Range;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;


/**
 * Created by caominhtrang on 3/8/16.
 */
public class AndroidVideoSourceInfo {
    private static final String DEBUG_TAG = "ForthTV";
    private final static double NANO_SECONDS_PER_SECOND = 1.0e9;

    private static boolean isFrontFacing(Camera.CameraInfo info) {
        return info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT;
    }

    private static boolean isFrontFacing(CameraCharacteristics characteristics) {
        Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
        if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
            return true;
        }
        return false;
    }

    private static String deviceUniqueName(int index, Camera.CameraInfo info) {
        return "Camera " + index +", Facing " +
                (isFrontFacing(info) ? "front" : "back") +
                ", Orientation "+ info.orientation;
    }

    private static String deviceUniqueName(String index, CameraCharacteristics characteristics)
    {
        return "Camera " + index + ", Facing " + (isFrontFacing(characteristics) ? "front" : "back") +
                ", Orientation "+ characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
    }

    private static void getListCaptureDevices(Object context) {
        if (android.os.Build.VERSION.RELEASE.startsWith("1."))
            getListCaptureDevicesOldAPI();
        else if (android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP)
            getListCaptureDevicesOldAPI();
        else {
            getListCaptureDevicesNewAPI((Context)context);
        }

    }

    private static String getListCaptureDevicesOldAPI()
    {
        try {
            int numberOfCameras = Camera.getNumberOfCameras();
            JSONArray devices = new JSONArray();
            for (int i = 0; i < numberOfCameras; i++) {
                Camera.CameraInfo info = new Camera.CameraInfo();
                Camera.getCameraInfo(i, info);
                String deviceName = deviceUniqueName(i, info);
                JSONObject cameraDict = new JSONObject();
                devices.put(cameraDict);
                List<Camera.Size> supportedSizes;
                List<int[]> supportedFpsRanges;
                Camera camera = null;
                try {
                    camera = Camera.open(i);
                    Camera.Parameters parameters = camera.getParameters();
                    supportedSizes = parameters.getSupportedPreviewSizes();
                    supportedFpsRanges = parameters.getSupportedPreviewFpsRange();
                    Log.d(DEBUG_TAG, deviceName);
                } catch (RuntimeException e) {
                    Log.e(DEBUG_TAG, "Failed to open " + deviceName + ", skipping", e);
                    continue;
                } finally {
                    if (camera != null) {
                        camera.release();
                    }
                }

                JSONArray sizes = new JSONArray();
                for (Camera.Size supportedSize : supportedSizes) {
                    JSONObject size = new JSONObject();
                    size.put("width", supportedSize.width);
                    size.put("height", supportedSize.height);
                    sizes.put(size);
                }

                boolean is30fpsRange = false;
                boolean is15fpsRange = false;
                // If there is constant 30 fps mode, but no 15 fps - add 15 fps
                // mode to the list of supported ranges. Frame drop will be done
                // in software.
                for (int[] range : supportedFpsRanges) {
                    if (range[Camera.Parameters.PREVIEW_FPS_MIN_INDEX] == 30000 &&
                            range[Camera.Parameters.PREVIEW_FPS_MAX_INDEX] == 30000) {
                        is30fpsRange = true;
                    }
                    if (range[Camera.Parameters.PREVIEW_FPS_MIN_INDEX] == 15000 &&
                            range[Camera.Parameters.PREVIEW_FPS_MAX_INDEX] == 15000) {
                        is15fpsRange = true;
                    }
                }
                if (is30fpsRange && !is15fpsRange) {
                    Log.d(DEBUG_TAG, "Adding 15 fps support");
                    int[] newRange = new int [Camera.Parameters.PREVIEW_FPS_MAX_INDEX + 1];
                    newRange[Camera.Parameters.PREVIEW_FPS_MIN_INDEX] = 15000;
                    newRange[Camera.Parameters.PREVIEW_FPS_MAX_INDEX] = 15000;
                    for (int j = 0; j < supportedFpsRanges.size(); j++ ) {
                        int[] range = supportedFpsRanges.get(j);
                        if (range[Camera.Parameters.PREVIEW_FPS_MAX_INDEX] >
                                newRange[Camera.Parameters.PREVIEW_FPS_MAX_INDEX]) {
                            supportedFpsRanges.add(j, newRange);
                            break;
                        }
                    }
                }

                JSONArray mfpsRanges = new JSONArray();
                for (int[] range : supportedFpsRanges) {
                    JSONObject mfpsRange = new JSONObject();
                    // Android SDK deals in integral "milliframes per second"
                    // (i.e. fps*1000, instead of floating-point frames-per-second) so we
                    // preserve that through the Java->C++->Java round-trip.
                    mfpsRange.put("min_mfps", range[Camera.Parameters.PREVIEW_FPS_MIN_INDEX]);
                    mfpsRange.put("max_mfps", range[Camera.Parameters.PREVIEW_FPS_MAX_INDEX]);
                    mfpsRanges.put(mfpsRange);
                }

                cameraDict.put("id", i);
                cameraDict.put("name", deviceName);
                cameraDict.put("front_facing", isFrontFacing(info))
                        .put("orientation", info.orientation)
                        .put("sizes", sizes)
                        .put("mfpsRanges", mfpsRanges);
            }
            String ret = devices.toString(2);
            Log.d(DEBUG_TAG, ret);
            return ret;
        }
        catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    private static String getListCaptureDevicesNewAPI(Context context) {
        try {
            CameraManager manager = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
            JSONArray devices = new JSONArray();
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics
                        = manager.getCameraCharacteristics(cameraId);
                String deviceName = deviceUniqueName(cameraId, characteristics);
                JSONObject cameraDict = new JSONObject();
                devices.put(cameraDict);

                final Range<Integer>[] fpsRanges =
                        characteristics.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);

                final StreamConfigurationMap streamMap =
                        characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                final android.util.Size[] sizes = streamMap.getOutputSizes(ImageFormat.YUV_420_888);
                if (sizes == null) {
                    throw new RuntimeException("ImageFormat.YUV_420_888 not supported.");
                }

                JSONArray jsonSizes = new JSONArray();
                for (android.util.Size size : sizes) {
                    JSONObject jsonSize = new JSONObject();
                    jsonSize.put("width", size.getWidth());
                    jsonSize.put("height", size.getHeight());
                    jsonSizes.put(size);
                }

                JSONArray mfpsRanges = new JSONArray();
                for (Range<Integer> fpsRange : fpsRanges) {
                    JSONObject mfpsRange = new JSONObject();
                    mfpsRange.put("min_mfps", 0);
                    mfpsRange.put("max_mfps", fpsRange.getUpper());
                    mfpsRanges.put(mfpsRange);
                }

                cameraDict.put("id", cameraId);
                cameraDict.put("name", deviceName);
                cameraDict.put("front_facing", isFrontFacing(characteristics));
                cameraDict.put("orientation", characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION))
                        .put("sizes", jsonSizes)
                        .put("mfpsRanges", mfpsRanges);

            }

            String ret = devices.toString(2);
            Log.d(DEBUG_TAG, ret);
            return ret;

        } catch (CameraAccessException e) {
            e.printStackTrace();
            throw new RuntimeException(e);
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }

    }
}
