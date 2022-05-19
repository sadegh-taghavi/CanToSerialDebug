package org.tdevelopers.serialdebug;

import java.nio.charset.StandardCharsets;
import android.os.Bundle;
import android.content.Intent;
import android.app.PendingIntent;
import org.qtproject.qt5.android.bindings.QtApplication;
import org.qtproject.qt5.android.bindings.QtActivity;
import android.util.Log;
import android.content.Context;
import java.util.List;
import android.os.Handler;
import android.os.Looper;
import java.util.concurrent.Executors;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbDeviceConnection;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;


public class CustomMainActivity extends QtActivity implements SerialInputOutputManager.Listener
{
    public static native void serialDataReady(String data);

    private SerialInputOutputManager usbIoManager;
    private UsbSerialPort port;
    private Handler mainLooper;

    @Override
    public void onNewData(byte[] data) {
       mainLooper.post(() -> {
           String str = new String(data, StandardCharsets.UTF_8);
           Log.d("org.tdevelopers", "###############onNewData" + str);
           serialDataReady( str );
       });
    }

    @Override
    public void onRunError(Exception e) {
       mainLooper.post(() -> {
           Log.d("org.tdevelopers", "###############onRunError");
       });
    }

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        mainLooper = new Handler(Looper.getMainLooper());
    }

    @Override
    protected void onResume(){      
        super.onResume();
    }

    @Override
    protected void onPause(){
        super.onPause();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
    }

    public boolean sendData( String data )
    {
        try {
            port.write(data.getBytes(), 100);
        }catch (Exception e) {
            Log.d("org.tdevelopers", "###############Exception sendData");
            return false;
        }
    return true;
    }

    public boolean openDevice()
    {
        // Find all available drivers from attached devices.
        UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);
        if (availableDrivers.isEmpty())
        {
            Log.d("org.tdevelopers", "###############availableDrivers.isEmpty()");

            return false;
        }

        // Open a connection to the first available driver.
        UsbSerialDriver driver = availableDrivers.get(0);
        if( !manager.hasPermission(driver.getDevice()) )
        {
            PendingIntent usbPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent("usbPermissionIntent"), 0);
            manager.requestPermission(driver.getDevice(), usbPermissionIntent);
        }
        UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
        if (connection == null)
        {
            Log.d("org.tdevelopers", "###############connection == null");
            return false;
        }

        port = driver.getPorts().get(0); // Most devices have just one port (port 0)
        try {
            port.open(connection);
            port.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
        }
        catch (Exception e) {
            Log.d("org.tdevelopers", "###############Exception");
            return false;
        }
        usbIoManager = new SerialInputOutputManager(port, this);
        Executors.newSingleThreadExecutor().submit(usbIoManager);

        return true;
    }
}
