package tv.forth.forthbroadcasterandroiddemo;

import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import tv.forth.broadcaster.ForthBroadcasterWrapper;
import tv.forth.videoengine.AndroidVideoSourceInfo;

public class MainActivity extends AppCompatActivity {

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

        ForthBroadcasterWrapper.Initialize();

        AndroidVideoSourceInfo videoSourceInfo = new AndroidVideoSourceInfo(this);
        videoSourceInfo.getListCaptureDevices();

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

    public static boolean androidMinimum(int verCode) {
        if (android.os.Build.VERSION.RELEASE.startsWith("1.0"))
            return verCode == 1;
        else if (android.os.Build.VERSION.RELEASE.startsWith("1.1")) {
            return verCode <= 2;
        } else if (android.os.Build.VERSION.RELEASE.startsWith("1.5")) {
            return verCode <= 3;
        } else {
            return android.os.Build.VERSION.SDK_INT >= verCode;
        }
    }
}
