package com.testapp.example;

import android.os.Bundle;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.widget.Toast;

import com.google.androidgamesdk.GameActivity;

public class MainActivity extends GameActivity {
    static {
        System.loadLibrary("testapp");
    }

    TestClass C;

    @Override
    protected void onCreate(Bundle savedInstance)
    {
        super.onCreate(savedInstance);
        setContentView(R.layout.mainactivity);

        WindowInsetsController controller = getWindow().getInsetsController();
        controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        controller.hide(WindowInsets.Type.systemBars());

        findViewById(R.id.button).setOnClickListener(view -> nativeMethod());
    }

    void Toaster(int Count, String Text, String[] Strings, short[] Shorts)
    {
        if (C != null)
        {
            TestClass.S = Text;
            runOnUiThread( () -> Toast.makeText(this, TestClass.S,Toast.LENGTH_LONG).show());
        }
    }

    native void nativeMethod();
}