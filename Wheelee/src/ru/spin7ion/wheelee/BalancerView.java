package ru.spin7ion.wheelee;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Rect;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class BalancerView extends SurfaceView implements SurfaceHolder.Callback  {

	private float angle = 1.5f;
	private int cx,cy;
	
	ViewThread vt;
	
	private SurfaceHolder holder;     // This is no instantiation. Just saying that the holder  
    // will be of a class implementing SurfaceHolder  
	public BalancerView(Context context) {
		super(context);
		holder = getHolder();          // Holder is now the internal/private mSurfaceHolder inherit   
        // from the SurfaceView class, which is from an anonymous  
        // class implementing SurfaceHolder interface.  
		holder.addCallback(this);  
		cx=720/2;
		cy=720/2;
	}

	
	public void draw(Canvas canvas) {

		Paint paint=new Paint();
		paint.setStrokeWidth(3);
		//canvas.save();
		canvas.rotate((float) (angle*180.f/Math.PI), cx, cy);		
		
		paint.setColor(Color.BLACK);        
        canvas.drawRect(cx-15, cy-150, cx+15, cy, paint);
        paint.setColor(Color.WHITE);        
        canvas.drawRect(cx-12, cy-147, cx+12, cy-3, paint);
        
        paint.setColor(Color.BLACK);        
        canvas.drawCircle(cx, cy, 30, paint);
        paint.setColor(Color.WHITE);        
        canvas.drawCircle(cx, cy, 25, paint);
        
        
        //canvas.restore();
		
	}

	public float getAngle() {
		return angle;
	}

	public void setAngle(float angle) {
		this.angle = angle;
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		cx=width/2;
		cy=height/2;
		if (vt==null){  
			vt = new ViewThread(this);  
			vt.setRunning(true);  
			//vt.setSurfaceSize(width, height);  
			vt.start();  
       }  
		
	}

	@Override
	public void surfaceCreated(SurfaceHolder arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder arg0) {
		// TODO Auto-generated method stub
		
	}

}
