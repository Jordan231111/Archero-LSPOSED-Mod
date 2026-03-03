package com.archero.mod;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.callbacks.XC_LoadPackage;
import android.util.Log;

public class ModEntry implements IXposedHookLoadPackage {
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        if ("com.habby.archero".equals(lpparam.packageName)) {
            Log.d("ArcheroMod", "Target package loaded! Loading native library...");
            System.loadLibrary("archero_mod");
        }
    }
}