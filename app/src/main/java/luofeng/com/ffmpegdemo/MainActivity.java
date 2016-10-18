package luofeng.com.ffmpegdemo;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import org.red5.client.Red5Client;
import org.red5.client.net.rtmp.RTMPClient;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final String rootpath = Environment.getExternalStorageDirectory().getAbsolutePath().toString();
        final FFmpegUtils fFmpegUtils = new FFmpegUtils();
        Button btn = (Button) findViewById(R.id.btn);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(){
                    @Override
                    public void run() {
                        System.out.println("开始解码");
                        fFmpegUtils.getFFmpegInfo2(rootpath+ File.separator+"wind.flv","rtmp://192.168.1.21/oflaDemo/stt34545");
                        System.out.println("解码完成");
                    }
                }.start();
            }
        });
    }
}
