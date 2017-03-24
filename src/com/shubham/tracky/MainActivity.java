package com.shubham.tracky;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.List;
import java.util.Scanner;
import java.util.Set;
import java.util.UUID;


import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.Vibrator;
import android.app.Activity;
import android.app.AlertDialog;
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
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
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
	Button alarm;
	Button track;
	ToggleButton off;
	boolean alert;
	boolean mode = false;
	MediaPlayer media = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.activity_main);
		handler = new Handler();
		txt = (TextView) findViewById(R.id.textView1);
		alarm = (Button) findViewById(R.id.button1);
		track = (Button) findViewById(R.id.button2);
		off = (ToggleButton)findViewById(R.id.toggleButton2);
		alarm.setEnabled(false);
		track.setEnabled(false);
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
	}
	

	class BluetoothServer {
		BluetoothAdapter mBluetoothAdapter = null;
		String data = null;

		final Runnable updateUI;
		private BluetoothSocket btSocket = null;
		private final UUID uuid = UUID
				.fromString("00001101-0000-1000-8000-00805F9B34FB");

		public BluetoothServer(Handler handler, Runnable updateUI,
				String mac_address, Context cur) {
			this.updateUI = updateUI;

			mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
			BluetoothDevice device = mBluetoothAdapter
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
					txt.setText("Connected");
				}
			});
		}

		public String getBluetoothServer() {
			return data;
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
			//alarm.setEnabled(true);
			//track.setEnabled(true);

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
	}

}
