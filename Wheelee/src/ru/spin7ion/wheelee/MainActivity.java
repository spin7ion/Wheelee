package ru.spin7ion.wheelee;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

import android.os.Bundle;
import android.os.Handler;
import android.app.Activity;
import android.app.AlertDialog;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.Toast;
import android.bluetooth.*;
import android.content.DialogInterface;
import android.graphics.Color;

public class MainActivity extends Activity implements OnTouchListener{

	BluetoothAdapter bluetooth;
	String []boundedItems;
	 
	Handler h;
	ConnectionThread ct;
	
	BalancerView balancerView;
	
	Button wB,aB,sB,dB;
	
	// SPP UUID service
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	BluetoothSocket btSocket;
	protected static final int RECIEVE_MESSAGE = 1;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		balancerView=new BalancerView(this);
		balancerView.setBackgroundColor(Color.WHITE);

		LayoutParams params=new LayoutParams(720, 720);
		addContentView(balancerView, params);

		
		wB=(Button)findViewById(R.id.buttonW);
		aB=(Button)findViewById(R.id.buttonA);
		sB=(Button)findViewById(R.id.buttonS);
		dB=(Button)findViewById(R.id.buttonD);
		
		wB.setOnTouchListener(this);
		aB.setOnTouchListener(this);
		sB.setOnTouchListener(this);
		dB.setOnTouchListener(this);
		
		bluetooth = BluetoothAdapter.getDefaultAdapter();		
		if(bluetooth != null){
			if (!bluetooth.isEnabled()) {
			    bluetooth.enable();
			}
		}
		Set<BluetoothDevice> bounded=bluetooth.getBondedDevices();
		boundedItems=new String[bounded.size()];
		int i=0;
		for (BluetoothDevice bluetoothDevice : bounded) {
			boundedItems[i++]=bluetoothDevice.getName();
		}
		
		showListDialog();
		
		 h = new Handler() {
		        public void handleMessage(android.os.Message msg) {
		            switch (msg.what) {
		            case RECIEVE_MESSAGE:                                                   // if receive massage
		            	String line=(String)msg.obj;
		            	try{
		            		float a=Float.parseFloat(line.trim());
		            		balancerView.setAngle((float) (a-Math.PI/2.f));		      
		            	}catch (Exception e) {
		            		
						}	Log.i("rcv", line);	            	
		                break;
		            }
		        };
		    };
	}
	
	public void showListDialog(){
		AlertDialog.Builder builder = new AlertDialog.Builder(this);

		builder.setTitle("Pick a device");
		builder.setItems(boundedItems, new DialogInterface.OnClickListener() {

		   public void onClick(DialogInterface dialog, int item) {
		        connectTo(item);
		   }

		});

		AlertDialog alert = builder.create();

		alert.show();
	}

	public void connectTo(int id){
		Set<BluetoothDevice> bounded=bluetooth.getBondedDevices();
		for (BluetoothDevice bluetoothDevice : bounded) {
			if(bluetoothDevice.getName().equalsIgnoreCase(boundedItems[id])){
				try {
					btSocket=bluetoothDevice.createRfcommSocketToServiceRecord(MY_UUID);
					btSocket.connect();
					ct=new ConnectionThread(btSocket);
					ct.start();
				} catch (IOException e) {
					e.printStackTrace();
					try {
						btSocket.close();
					} catch (IOException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					showListDialog();
				}
				return;
			}
		}
		
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	private class ConnectionThread extends Thread{
		private final InputStream mmInStream;		
		private final BufferedReader br;
        private final OutputStream mmOutStream;
      
        public ConnectionThread(BluetoothSocket socket) throws IOException {
        	mmInStream =  socket.getInputStream();
        	br=new BufferedReader(new InputStreamReader(mmInStream));
            mmOutStream = socket.getOutputStream();
            
        }
      
        public void run() {
            // Keep listening to the InputStream until an exception occurs
            while (true) {                    
                		try {
                			String line=br.readLine();
							h.obtainMessage(RECIEVE_MESSAGE, line).sendToTarget();
						} catch (IOException e) {
							e.printStackTrace();
						}
                
            }
        }
        public void sendCmd(char cmd){
        	try{
        		mmOutStream.write(cmd);
        	}catch (IOException e) {
				e.printStackTrace();
			}
        }
	}

	@Override
	public boolean onTouch(View v, MotionEvent me) {
		if(me.getAction()==MotionEvent.ACTION_UP){
			ct.sendCmd('c');
			return false;
		}
		if(v==wB){
			ct.sendCmd('w');
		}else if(v==aB){
			ct.sendCmd('a');
		}else if(v==sB){
			ct.sendCmd('s');
		}else if(v==dB){
			ct.sendCmd('d');
		}
		return false;
	}
}
