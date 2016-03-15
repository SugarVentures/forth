package tv.forth.forthbroadcasterandroiddemo;

import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;

import tv.forth.broadcaster.ForthBroadcasterWrapper;
import tv.forth.broadcaster.StreamingAsyncTask;
import tv.forth.videoengine.ScreenRecorder;

public class MainActivity extends AppCompatActivity {
    private ForthBroadcasterWrapper mBroadcasterEngine;
    private static final String TAG = "ForthTV";
    private StreamingAsyncTask mStreamingTask;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });

        ForthBroadcasterWrapper.Initialize(this);
        mBroadcasterEngine = new ForthBroadcasterWrapper();
        mBroadcasterEngine.setupCaptureSession(mBroadcasterEngine);

        Button btnRecord = (Button) findViewById(R.id.btnRecord);
        btnRecord.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mBroadcasterEngine.startCaptureSession();
            }
        });

        Button btnGoLive = (Button) findViewById(R.id.btnGoLive);
        btnGoLive.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mStreamingTask = new StreamingAsyncTask("7116f0d7-5c27-44e6-8aa4-bc4ddeea9935", "192.168.1.9", mBroadcasterEngine);
                mStreamingTask.execute();

            }
        });

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == ScreenRecorder.REQUEST_CODE && resultCode == RESULT_OK)
        {
            mBroadcasterEngine.runCaptureSession(data);
        }
    }
}
