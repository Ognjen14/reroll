package com.topicdev.reroll;

import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.view.Window;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;

import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import org.qtproject.qt.android.bindings.QtActivity;
import android.util.Log;
public class RerollActivity extends QtActivity {

    private static RerollActivity m_instance;
    private OnBackInvokedCallback m_backCallback;

    @SuppressWarnings("deprecation")
    @Override
    public void onCreate(Bundle savedInstanceState) {
        m_instance = this;

        Window window = getWindow();
        WindowCompat.setDecorFitsSystemWindows(window, false);
        if (Build.VERSION.SDK_INT < 35) {
            window.setStatusBarColor(Color.TRANSPARENT);
            window.setNavigationBarColor(Color.TRANSPARENT);
        }
        // Going edge-to-edge makes Android draw a translucent scrim behind the
        // system bars so their icons stay legible over arbitrary app content.
        // We already set the bar icon polarity from the app theme
        // (setLightSystemBars), so opt out of the scrim and let AppTheme show
        // through instead.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            window.setNavigationBarContrastEnforced(false);
            window.setStatusBarContrastEnforced(false);
        }

        super.onCreate(savedInstanceState);
        int orientId = getResources().getIdentifier("portrait_only", "bool", getPackageName());
        if (getResources().getBoolean(orientId)) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_FULL_USER);
        }

        // API 33+: register after super so ours wins over Qt's default callback.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            m_backCallback = () -> {
                try { nativeOnBack(); } catch (UnsatisfiedLinkError e) { finish(); }
            };
            getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                OnBackInvokedDispatcher.PRIORITY_DEFAULT, m_backCallback);
        }
    }

    /** Fallback for API < 33; API 33+ is handled by the OnBackInvokedCallback above. */
    @SuppressWarnings("deprecation")
    @Override
    public void onBackPressed() {
        nativeOnBack();
    }

    private static native void nativeOnBack();

    @Override
    protected void onDestroy() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU && m_backCallback != null) {
            getOnBackInvokedDispatcher().unregisterOnBackInvokedCallback(m_backCallback);
            m_backCallback = null;
        }
        m_instance = null;
        super.onDestroy();
    }

    public static void minimizeApp() {
        if (m_instance == null) return;
        m_instance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_instance.moveTaskToBack(true);
            }
        });
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