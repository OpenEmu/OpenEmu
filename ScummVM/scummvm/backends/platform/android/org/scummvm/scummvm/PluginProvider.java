package org.scummvm.scummvm;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import java.util.ArrayList;

public class PluginProvider extends BroadcastReceiver {
	private final static String LOG_TAG = "ScummVM";

	public final static String META_UNPACK_LIB =
		"org.scummvm.scummvm.meta.UNPACK_LIB";

	public void onReceive(Context context, Intent intent) {
		if (!intent.getAction().equals(ScummVMApplication.ACTION_PLUGIN_QUERY))
			return;

		Bundle extras = getResultExtras(true);

		final ActivityInfo info;
		final PackageInfo pinfo;
		try {
			info = context.getPackageManager()
				.getReceiverInfo(new ComponentName(context, this.getClass()),
									PackageManager.GET_META_DATA);
			pinfo = context.getPackageManager()
				.getPackageInfo(context.getPackageName(), 0);
		} catch (PackageManager.NameNotFoundException e) {
			Log.e(LOG_TAG, "Error finding my own info?", e);
			return;
		}

		String host_version = extras.getString(ScummVMApplication.EXTRA_VERSION);
		if (!pinfo.versionName.equals(host_version)) {
			Log.e(LOG_TAG, "Plugin version " + pinfo.versionName + " is not equal to ScummVM version " + host_version);
			return;
		}

		String mylib = info.metaData.getString(META_UNPACK_LIB);
		if (mylib != null) {
			ArrayList<String> all_libs =
				extras.getStringArrayList(ScummVMApplication.EXTRA_UNPACK_LIBS);
			all_libs.add(new Uri.Builder()
							.scheme("plugin")
							.authority(context.getPackageName())
							.path(mylib)
							.toString());

			extras.putStringArrayList(ScummVMApplication.EXTRA_UNPACK_LIBS,
										all_libs);
		}

		setResultExtras(extras);
	}
}
