package com.shubham.tracky;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.List;
import java.util.Scanner;
import java.util.Set;
import java.util.UUID;


import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.Vibrator;
import android.app.Activity;
import android.app.Dialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.RotateAnimation;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ToggleButton;

public class MainActivity extends Activity {

	BluetoothAdapter mainAdapter;
	BroadcastReceiver findDevices = null;
	boolean activated = false;
	List<BluetoothDevice> list;
	ConnectedThread mConnectedThread = null;
	Dialog listDialog;
	String[] val2;
	Context curr;
	TextView txt;
	Handler handler;
	String msgBuffer;
	public InputStream mmInStream;
	public OutputStream mmOutStream;
	ToggleButton off;
	boolean alert;
	boolean mode = false;
	MediaPlayer media = null;
	SensorManager sm;
	Sensor magnet;
	float [] sensorData = new float[3];
	ImageView image;
	float fromDegrees = 0.f;
	double longitude;
	double latitude;
	LocationManager lm;
	LocationListener ll;
	boolean sm_b;
	boolean lm_b;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.activity_main);
		handler = new Handler();
		txt = (TextView) findViewById(R.id.textView1);
		off = (ToggleButton)findViewById(R.id.toggleButton2);
		image = (ImageView)findViewById(R.id.imageView2);
		sm = (SensorManager) getSystemService(SENSOR_SERVICE);
		lm = (LocationManager)getSystemService(Context.LOCATION_SERVICE);
		off.setEnabled(false);
		curr = this;
		
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	public void bluetooth_switch(View v) {
		// TODO Auto-generated method stub
		mainAdapter = BluetoothAdapter.getDefaultAdapter();

		if (!activated) {
			Intent startBluetooth = new Intent(
					BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivity(startBluetooth);
			activated = true;
			bluetooth_switch(v);
		} else if (activated) {
			if (mainAdapter != null) {
				if (mainAdapter.isDiscovering()) {
					mainAdapter.cancelDiscovery();
				}
			}
			Set<BluetoothDevice> set = mainAdapter.getBondedDevices();
			showdialog(set);
		}
	}

	public void send_activate(View v) {
		mode = off.isChecked();
		if (mode){
			msgBuffer = "1";
			Thread t = new Thread(new write_message());
			t.start();
		}else{
			alert = mode;
			msgBuffer = "0";
			Thread t = new Thread(new write_message());
			t.start();
			if(media != null){
				media.stop();
				media = null;
			}
			if(sm_b){
				sm.unregisterListener(sel);
				sm_b = false;
			}
		
		}

	}

	private void showdialog(Set<BluetoothDevice> set) {
		String[] val = new String[set.size()];
		val2 = new String[set.size()];
		int i = 0;

		for (BluetoothDevice bt : set) {
			val[i] = new String(bt.getName());
			val2[i++] = new String(bt.getAddress());
		}

		listDialog = new Dialog(this);
		listDialog.setTitle("Select Device");
		LayoutInflater li = (LayoutInflater) this
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View v = li.inflate(R.layout.bluetooth_devices, null, false);
		listDialog.setContentView(v);
		listDialog.setCancelable(true);

		ListView list1 = (ListView) listDialog.findViewById(R.id.listing);
		list1.setOnItemClickListener(new clickable());
		list1.setAdapter(new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_1, val));

		listDialog.show();
	}
	SensorEventListener sel = new SensorEventListener(){	
		@Override
		public void onSensorChanged(SensorEvent arg0) {
			// TODO Auto-generated method stub
			float [] val = arg0.values;
			sensorData[0] = val[0];
			sensorData[1] = val[1];
			sensorData[2] = val[2];
			txt.setText(new String(val[0] + " " + val[1] + " " + val[2]));
			RotateAnimation rotate = new RotateAnimation(fromDegrees, -(val[0]-90),Animation.RELATIVE_TO_SELF,0.5f,Animation.RELATIVE_TO_SELF,0.5f);
			rotate.setDuration(100);
			image.startAnimation(rotate);
			fromDegrees = -(val[0]-90);
		}
		
		@Override
		public void onAccuracyChanged(Sensor arg0, int arg1) {
			// TODO Auto-generated method stub
		}
	};
	public void data(){
		magnet = sm.getDefaultSensor(Sensor.TYPE_ORIENTATION);
		sm_b = sm.registerListener(sel, magnet,
				SensorManager.SENSOR_DELAY_GAME);
	}
	
	public class clickable implements OnItemClickListener {

		@Override
		public void onItemClick(AdapterView<?> arg0, View arg1, final int arg2,
				long arg3) {
			// TODO Auto-generated method stub
			String mac_address = val2[arg2];
			BluetoothServer blue = new BluetoothServer(handler, null,
					mac_address, curr);
			listDialog.dismiss();
		}
	}
	
	private void func() {
		// TODO Auto-generated method stub
		
		media = MediaPlayer.create(curr, R.raw.alarm );
		media.start();
		Vibrator vib = (Vibrator) curr.getSystemService(Context.VIBRATOR_SERVICE);
		vib.vibrate(5000);
		
		msgBuffer = "2";
		Thread t = new Thread(new write_message());
		t.start();
		
		
		// data() should be called
		data();
		
		
		// enable gps location
		ll = new locater();
		lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 10000, 5, ll);
	}
	

	class BluetoothServer {
		BluetoothAdapter mBluetoothAdapter = null;
		String data = null;

		private BluetoothSocket btSocket = null;
		private final UUID uuid = UUID
				.fromString("00001101-0000-1000-8000-00805F9B34FB");

		public BluetoothServer(Handler handler, Runnable updateUI,
				String mac_address, Context cur) {

			mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
			final BluetoothDevice device = mBluetoothAdapter
					.getRemoteDevice(mac_address);
			try {
				btSocket = device.createRfcommSocketToServiceRecord(uuid);
				btSocket.connect();

			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			mConnectedThread = new ConnectedThread(btSocket);
			handler.post(new Runnable() {

				@Override
				public void run() {
					// TODO Auto-generated method stub
					txt.setText("Connected :" + device.getName());
				}
			});
		}
	}

	public class read_message implements Runnable {
		byte[] buff = new byte[8];

		@Override
		public void run() {
			// TODO Auto-generated method stub
			Scanner s = new Scanner(mmInStream);
			while (true) {
				final String result = s.hasNext() ? s.next() : "";
				handler.post(new Runnable() {

					@Override
					public void run() {
						// TODO Auto-generated method stub
						if (result.equals(new String("ALERT"))) {
							txt.setText(new String("Alert Mode"));
							alert = true;
						}

						if (alert && mode){
							func();

						}
						//txt.setText(result);
					}
				});
			}

		}
	}

	public class write_message implements Runnable {
		PrintWriter print;

		@Override
		public void run() {
			// TODO Auto-generated method stub
			print = new PrintWriter(mmOutStream);
			print.print(msgBuffer);
			print.flush();
		}
	}

	private class ConnectedThread {

		// creation of the connect thread
		public ConnectedThread(BluetoothSocket socket) {
			InputStream tmpIn = null;
			OutputStream tmpOut = null;

			try {
				// Create I/O streams for connection
				tmpIn = socket.getInputStream();
				tmpOut = socket.getOutputStream();
			} catch (IOException e) {
				Toast.makeText(getApplicationContext(), "Server Socket Error",
						Toast.LENGTH_SHORT).show();
			}

			mmInStream = tmpIn;
			mmOutStream = tmpOut;
			Thread t = new Thread(new read_message());
			t.start();
			off.setEnabled(true);

		}
	}
	
	public class locater implements LocationListener{

		@Override
		public void onLocationChanged(Location arg0) {
			// TODO Auto-generated method stub
			latitude = arg0.getLatitude();
			longitude = arg0.getLongitude();
			txt.setText("LAT "+latitude + " ,LON " + longitude);
		}

		@Override
		public void onProviderDisabled(String arg0) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void onProviderEnabled(String arg0) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
			// TODO Auto-generated method stub
			
		}
	
	}
	
	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
		if(media != null){
			media.stop();
			media = null;
		}
	}
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		try {
			finalize();
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		if(media != null){
			media.stop();
			media = null;
		}
		if(sm_b){
			sm.unregisterListener(sel);
		}
	}

}
