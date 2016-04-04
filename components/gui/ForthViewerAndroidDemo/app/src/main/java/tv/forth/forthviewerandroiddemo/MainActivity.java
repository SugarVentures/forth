package tv.forth.forthviewerandroiddemo;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;


public class MainActivity extends Activity implements ForthCallback {
    private ForthViewer mEngine;
    private GLSurfaceView mFrameView;
    private StreamingAsyncTask streamingTask;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });

        mEngine = new ForthViewer(this);

        Button btnPlay = (Button)findViewById(R.id.playButton);
        btnPlay.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //engine = new ForthViewer();
                //engine.startStreaming("7116f0d7-5c27-44e6-8aa4-bc4ddeea9935", "192.168.1.9");
                streamingTask = new StreamingAsyncTask("7116f0d7-5c27-44e6-8aa4-bc4ddeea9935", "192.168.1.9", mEngine);
                streamingTask.execute();
            }
        });

        LinearLayout l = (LinearLayout) findViewById(R.id.frameView);
        mFrameView = new FrameView(this, mEngine);

//to add the view with your own parameters
        //l.addView(mFrameView, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

//or simply use
        l.addView(mFrameView, 0);

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
    public void frameCallback() {
        mFrameView.requestRender();
    }
}
