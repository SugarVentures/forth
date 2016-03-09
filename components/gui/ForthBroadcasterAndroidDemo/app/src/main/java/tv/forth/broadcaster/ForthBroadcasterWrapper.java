package tv.forth.broadcaster;

/**
 * Created by caominhtrang on 3/9/16.
 */
public class ForthBroadcasterWrapper {
    static {
        System.loadLibrary("forthwrapper");
    }

    public static native void Initialize();
}
