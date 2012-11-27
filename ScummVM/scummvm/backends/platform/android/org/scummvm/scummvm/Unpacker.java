package org.scummvm.scummvm;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.widget.ProgressBar;

import java.io.IOException;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;

public class Unpacker extends Activity {
	protected final static String LOG_TAG = "ScummVM";
	// TODO don't hardcode this
	private final static boolean PLUGINS_ENABLED = false;
	private final static String META_NEXT_ACTIVITY =
		"org.scummvm.unpacker.nextActivity";
	private ProgressBar mProgress;
	private File mUnpackDest;  // location to unpack into
	private AsyncTask<String, Integer, Void> mUnpacker;
	private final static int REQUEST_MARKET = 1;

	// Android 3.1+ only
	public static final int FLAG_INCLUDE_STOPPED_PACKAGES = 32;

	private static class UnpackJob {
		public ZipFile zipfile;
		public Set<String> paths;

		public UnpackJob(ZipFile zipfile, Set<String> paths) {
			this.zipfile = zipfile;
			this.paths = paths;
		}

		public long UnpackSize() {
			long size = 0;
			for (String path: paths) {
				ZipEntry entry = zipfile.getEntry(path);
				if (entry != null) size += entry.getSize();
			}
			return size;
		}
	}

	private class UnpackTask extends AsyncTask<String, Integer, Void> {
		@Override
		protected void onProgressUpdate(Integer... progress) {
			mProgress.setIndeterminate(false);
			mProgress.setMax(progress[1]);
			mProgress.setProgress(progress[0]);
			mProgress.postInvalidate();
		}

		@Override
		protected void onPostExecute(Void result) {
			Bundle md = getMetaData();
			String nextActivity = md.getString(META_NEXT_ACTIVITY);
			if (nextActivity != null) {
				final ComponentName cn =
					ComponentName.unflattenFromString(nextActivity);
				if (cn != null) {
					final Intent origIntent = getIntent();
					Intent intent = new Intent();
					intent.setComponent(cn);
					if (origIntent.getExtras() != null)
						intent.putExtras(origIntent.getExtras());
					intent.putExtra(Intent.EXTRA_INTENT, origIntent);
					intent.setDataAndType(origIntent.getData(),
										  origIntent.getType());
					//intent.fillIn(getIntent(), 0);
					intent.addFlags(Intent.FLAG_ACTIVITY_PREVIOUS_IS_TOP);
					Log.i(LOG_TAG,
						  "Starting next activity with intent " + intent);
					startActivity(intent);
				} else {
					Log.w(LOG_TAG,
						  "Unable to extract a component name from " + nextActivity);
				}
			}

			finish();
		}

		@Override
		protected Void doInBackground(String... all_libs) {
			// This will contain all unpack jobs
			Map<String, UnpackJob> unpack_jobs =
				new HashMap<String, UnpackJob>(all_libs.length);

			// This will contain all unpack filenames (so we can
			// detect stale files in the unpack directory)
			Set<String> all_files = new HashSet<String>(all_libs.length);

			for (String lib: all_libs) {
				final Uri uri = Uri.parse(lib);
				final String pkg = uri.getAuthority();
				final String path = uri.getPath().substring(1); // skip first /

				all_files.add(new File(path).getName());

				UnpackJob job = unpack_jobs.get(pkg);
				if (job == null) {
					try {
						// getPackageResourcePath is hidden in Context,
						// but exposed in ContextWrapper...
						ContextWrapper context =
							new ContextWrapper(createPackageContext(pkg, 0));
						ZipFile zipfile =
							new ZipFile(context.getPackageResourcePath());
						job = new UnpackJob(zipfile, new HashSet<String>(1));
					} catch (PackageManager.NameNotFoundException e) {
						Log.e(LOG_TAG, "Package " + pkg +
							  " not found", e);
						continue;
					} catch (IOException e) {
						// FIXME: show some sort of GUI error dialog
						Log.e(LOG_TAG,
							  "Error opening ZIP for package " + pkg, e);
						continue;
					}
					unpack_jobs.put(pkg, job);
				}
				job.paths.add(path);
			}

			// Delete stale filenames from mUnpackDest
			for (File file: mUnpackDest.listFiles()) {
				if (!all_files.contains(file.getName())) {
					Log.i(LOG_TAG,
						  "Deleting stale cached file " + file);
					file.delete();
				}
			}

			int total_size = 0;
			for (UnpackJob job: unpack_jobs.values())
				total_size += job.UnpackSize();

			publishProgress(0, total_size);

			mUnpackDest.mkdirs();

			int progress = 0;

			for (UnpackJob job: unpack_jobs.values()) {
				try {
					ZipFile zipfile = job.zipfile;
					for (String path: job.paths) {
						ZipEntry zipentry = zipfile.getEntry(path);
						if (zipentry == null)
							throw new FileNotFoundException(
															"Couldn't find " + path + " in zip");
						File dest = new File(mUnpackDest, new File(path).getName());
						if (dest.exists() &&
							dest.lastModified() == zipentry.getTime() &&
							dest.length() == zipentry.getSize()) {
							// Already unpacked
							progress += zipentry.getSize();
						} else {
							if (dest.exists())
								Log.d(LOG_TAG,
									  "Replacing " + dest.getPath() +
									  " old.mtime=" + dest.lastModified() +
									  " new.mtime=" + zipentry.getTime() +
									  " old.size=" + dest.length() +
									  " new.size=" + zipentry.getSize());
							else
								Log.i(LOG_TAG,
									  "Extracting " + zipentry.getName() +
									  " from " + zipfile.getName() +
									  " to " + dest.getPath());

							long next_update = progress;

							InputStream in = zipfile.getInputStream(zipentry);
							OutputStream out = new FileOutputStream(dest);
							int len;
							byte[] buffer = new byte[4096];
							while ((len = in.read(buffer)) != -1) {
								out.write(buffer, 0, len);
								progress += len;
								if (progress >= next_update) {
									publishProgress(progress, total_size);
									// Arbitrary limit of 2% update steps
									next_update += total_size / 50;
								}
							}

							in.close();
							out.close();
							dest.setLastModified(zipentry.getTime());
						}
						publishProgress(progress, total_size);
					}

					zipfile.close();
				} catch (IOException e) {
					// FIXME: show some sort of GUI error dialog
					Log.e(LOG_TAG, "Error unpacking plugin", e);
				}
			}

			if (progress != total_size)
				Log.d(LOG_TAG, "Ended with progress " + progress +
					  " != total size " + total_size);

			setResult(RESULT_OK);

			return null;
		}
	}

	private class PluginBroadcastReciever extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (!intent.getAction()
				.equals(ScummVMApplication.ACTION_PLUGIN_QUERY)) {
				Log.e(LOG_TAG,
					  "Received unexpected action " + intent.getAction());
				return;
			}

			Bundle extras = getResultExtras(false);
			if (extras == null) {
				// Nothing for us to do.
				Unpacker.this.setResult(RESULT_OK);
				finish();
			}

			ArrayList<String> unpack_libs =
				extras.getStringArrayList(ScummVMApplication.EXTRA_UNPACK_LIBS);

			if (unpack_libs != null && !unpack_libs.isEmpty()) {
				final String[] libs =
					unpack_libs.toArray(new String[unpack_libs.size()]);
				mUnpacker = new UnpackTask().execute(libs);
			}
		}
	}

	private void initPlugins() {
		Bundle extras = new Bundle(1);

		ArrayList<String> unpack_libs = new ArrayList<String>(1);
		// This is the common ScummVM code (not really a "plugin" as such)
		unpack_libs.add(new Uri.Builder()
						.scheme("plugin")
						.authority(getPackageName())
						.path("mylib/armeabi/libscummvm.so")
						.toString());
		extras.putStringArrayList(ScummVMApplication.EXTRA_UNPACK_LIBS,
								  unpack_libs);

		final PackageInfo info;
		try {
			info = getPackageManager().getPackageInfo(getPackageName(), 0);
		} catch (PackageManager.NameNotFoundException e) {
			Log.e(LOG_TAG, "Error finding my own info?", e);
			return;
		}
		extras.putString(ScummVMApplication.EXTRA_VERSION, info.versionName);

		Intent intent = new Intent(ScummVMApplication.ACTION_PLUGIN_QUERY);
		// Android 3.1 defaults to FLAG_EXCLUDE_STOPPED_PACKAGES, and since
		// none of our plugins will ever be running, that is not helpful
		intent.setFlags(FLAG_INCLUDE_STOPPED_PACKAGES);
		sendOrderedBroadcast(intent, Manifest.permission.SCUMMVM_PLUGIN,
							 new PluginBroadcastReciever(),
							 null, RESULT_OK, null, extras);
	}

	@Override
	public void onCreate(Bundle b) {
		super.onCreate(b);

		mUnpackDest = ScummVMApplication.getLastCacheDir();

		setContentView(R.layout.splash);
		mProgress = (ProgressBar)findViewById(R.id.progress);

		setResult(RESULT_CANCELED);

		tryUnpack();
	}

	private void tryUnpack() {
		Intent intent = new Intent(ScummVMApplication.ACTION_PLUGIN_QUERY);
		List<ResolveInfo> plugins = getPackageManager()
			.queryBroadcastReceivers(intent, 0);
		if (PLUGINS_ENABLED && plugins.isEmpty()) {
			// No plugins installed
			AlertDialog.Builder alert = new AlertDialog.Builder(this)
				.setTitle(R.string.no_plugins_title)
				.setMessage(R.string.no_plugins_found)
				.setIcon(android.R.drawable.ic_dialog_alert)
				.setOnCancelListener(new DialogInterface.OnCancelListener() {
						public void onCancel(DialogInterface dialog) {
							finish();
						}
					})
				.setNegativeButton(R.string.quit,
								   new DialogInterface.OnClickListener() {
									   public void onClick(DialogInterface dialog, int which) {
										   finish();
									   }
								   });

			final Uri uri = Uri.parse("market://search?q=ScummVM plugin");
			final Intent market_intent = new Intent(Intent.ACTION_VIEW, uri);
			if (getPackageManager().resolveActivity(market_intent, 0) != null) {
				alert.setPositiveButton(R.string.to_market,
										new DialogInterface.OnClickListener() {
											public void onClick(DialogInterface dialog, int which) {
												dialog.dismiss();
												try {
													startActivityForResult(market_intent,
																		   REQUEST_MARKET);
												} catch (ActivityNotFoundException e) {
													Log.e(LOG_TAG,
														  "Error starting market", e);
												}
											}
										});
			}

			alert.show();

		} else {
			// Already have at least one plugin installed
			initPlugins();
		}
	}

	@Override
	public void onStop() {
		if (mUnpacker != null)
			mUnpacker.cancel(true);
		super.onStop();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode,
										Intent data) {
		switch (requestCode) {
		case REQUEST_MARKET:
			if (resultCode != RESULT_OK)
				Log.w(LOG_TAG, "Market returned " + resultCode);
			tryUnpack();
			break;
		}
	}

	private Bundle getMetaData() {
		try {
			ActivityInfo ai = getPackageManager()
				.getActivityInfo(getComponentName(), PackageManager.GET_META_DATA);
			return ai.metaData;
		} catch (PackageManager.NameNotFoundException e) {
			Log.w(LOG_TAG, "Unable to find my own meta-data", e);
			return new Bundle();
		}
	}
}
