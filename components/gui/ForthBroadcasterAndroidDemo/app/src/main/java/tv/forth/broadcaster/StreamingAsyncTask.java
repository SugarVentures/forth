package tv.forth.broadcaster;

import android.os.AsyncTask;

/**
 * Created by caominhtrang on 3/14/16.
 */
public class StreamingAsyncTask extends AsyncTask<Void, Void, Void>{
    private ForthBroadcasterWrapper mEngine;
    String mStreamKey;
    String mServerAddress;

    public StreamingAsyncTask(String streamKey, String serverAddress, ForthBroadcasterWrapper engine)
    {
        mStreamKey = streamKey;
        mServerAddress = serverAddress;
        mEngine = engine;
    }

    @Override
    protected Void doInBackground(Void... params) {
        mEngine.startStreaming(mStreamKey, mServerAddress);
        return null;
    }
}
