package ru.spin7ion.wheelee;

import android.graphics.Canvas;
import android.view.SurfaceHolder;

public class ViewThread extends Thread {
    private BalancerView balancerView;
    private SurfaceHolder mHolder;
    private boolean mRun = false;
 
    public ViewThread(BalancerView gameView) {
    	balancerView = gameView;
        mHolder = balancerView.getHolder();
    }
 
    public void setRunning(boolean run) {
        mRun = run;
    }
 
    @Override
    public void run() {
        Canvas canvas = null;
        while (mRun) {
        	 try {  
                 canvas = mHolder.lockCanvas();  
                 synchronized (mHolder) {  
                	 balancerView.postInvalidate();           
                 }  
               }  
               finally {  
                    if (canvas != null) {  
                    	mHolder.unlockCanvasAndPost(canvas);  
                         }  
               }  
        	 try {  
                 Thread.sleep(50); // Wait some time till I need to display again  
            } catch (InterruptedException e) {  
                 // TODO Auto-generated catch block  
                 e.printStackTrace();  
            }    
        }
    }
}