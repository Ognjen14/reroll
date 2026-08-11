package com.topicdev.reroll;

import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.view.Window;

import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import org.qtproject.qt.android.bindings.QtActivity;
import android.util.Log;
public class RerollActivity extends QtActivity {

    private static RerollActivity m_instance;
    @SuppressWarnings("deprecation")
    @Override
    public void onCreate(Bundle savedInstanceState) {
        m_instance = this;

        super.onCreate(savedInstanceState);
        int orientId = getResources().getIdentifier("portrait_only", "bool", getPackageName());
        if (getResources().getBoolean(orientId)) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_FULL_USER);
        }
    }
    @Override
    protected void onDestroy() {
        m_instance = null;
        super.onDestroy();
    }

    public static void setLightSystemBars(final boolean light) {
        if (m_instance == null) return;

        m_instance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Window window = m_instance.getWindow();
                WindowInsetsControllerCompat controller = new WindowInsetsControllerCompat(window, window.getDecorView());
                // If light is true, we want dark icons (for light theme)
                // If light is false, we want light icons (for dark theme)
                controller.setAppearanceLightStatusBars(light);
                controller.setAppearanceLightNavigationBars(light);
            }
        });
    }

}