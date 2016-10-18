package luofeng.com.ffmpegdemo;

/**
 * Created by luofe on 2016/9/24.
 */
public class FFmpegUtils {

    static{
        System.loadLibrary("avutil-55");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("ffmpegutils");
    }

    public native int getFFmpegInfo1(String input,String output);

    public native int getFFmpegInfo2(String input,String output);

    public native String getFFmpegInfo3(int cmdnum,String[] args);

    public native String getFFmpegInfo4();

    public native String getFFmpegInfo5();

    public native String getFFmpegInfo6();



}
