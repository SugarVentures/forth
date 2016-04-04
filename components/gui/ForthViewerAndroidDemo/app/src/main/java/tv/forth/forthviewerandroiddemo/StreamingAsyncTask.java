package tv.forth.forthviewerandroiddemo;

import android.os.AsyncTask;

/**
 * Created by caominhtrang on 4/4/16.
 */

public class StreamingAsyncTask extends AsyncTask<Void, Void, Void> {
    String mStreamKey;
    String mServerAddress;
    private ForthViewer mEngine;

    public StreamingAsyncTask(String streamKey, String serverAddress, ForthViewer engine)
    {
        this.mStreamKey = streamKey;
        this.mServerAddress = serverAddress;
        this.mEngine = engine;
    }

    @Override
    protected void onPreExecute() {

    }

    @Override
    protected Void doInBackground(Void... params) {
        mEngine.startStreaming(mStreamKey, mServerAddress);
        return null;
    }

}