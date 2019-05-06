package com.custom.ffmpegdemo;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.custom.ffmpegdemo.listener.OnCompleteListener;
import com.custom.ffmpegdemo.listener.OnInfoListener;
import com.custom.ffmpegdemo.listener.OnStartListener;
import com.custom.ffmpegdemo.model.TimeBean;
import com.custom.ffmpegdemo.utils.MyLog;

/**
 * author：tangjianlin
 * date：2019/4/1
 * description：
 * <p>
 * ----------Dragon be here!----------/
 * 　　　┏┓　　  ┏┓
 * 　　┏┛┻━━━┛┻┓━━━
 * 　　┃　　　　　   ┃
 * 　　┃　　　━　   ┃
 * 　　┃　┳┛　┗┳
 * 　　┃　　　　　   ┃
 * 　　┃　　　┻　   ┃
 * 　　┃　　　　     ┃
 * 　　┗━┓　　　┏━┛Code is far away from bug with the animal protecting
 * 　　　　┃　　　┃    神兽保佑,代码无bug
 * 　　　　┃　　　┃
 * 　　　　┃　　　┗━━━┓
 * 　　　　┃　　　　　　   ┣┓
 * 　　　　┃　　　　　　　┏┛
 * 　　　　┗┓┓┏━┳┓┏┛━━━━━
 * 　　　　　┃┫┫　┃┫┫
 * 　　　　　┗┻┛　┗┻┛
 * ━━━━━━━━━━━神兽出没━━━━━━━━━━━━━━
 */
public class FFMpegUtil implements SurfaceHolder.Callback {


    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("native-lib");
    }

    private SurfaceView surfaceView;

    private TimeBean timeBean;

    public FFMpegUtil() {
        timeBean = new TimeBean();
    }

    public void setSurface(SurfaceView surface) {
        this.surfaceView = surface;
        display(surface.getHolder().getSurface());
        surface.getHolder().addCallback(this);
    }

    /**
     * 播放
     *
     * @param path
     */
    public native void play(String path);

    /**
     * 渲染界面
     *
     * @param surface
     */
    public native void display(Surface surface);

    /**
     * 释放资源
     */
    public native void release();

    /**
     * 停止
     */
    public native void stop();

    /**
     * 获取视频总时间
     *
     * @return
     */
    public native int getTotalTime();

    /**
     * 获取当前进度
     *
     * @return
     */
    public native double getCurrentPosition();

    /**
     * seekTo
     *
     * @param msec
     */
    public native void seekTo(int msec);

    /**
     * 快退
     */
    public native void stepBack();

    /**
     * 快进
     */
    public native void stepUp();

    private OnStartListener onStartListener;

    private OnInfoListener onInfoListener;

    private OnCompleteListener onCompleteListener;

    public void setOnCompleteListener(OnCompleteListener onCompleteListener) {
        this.onCompleteListener = onCompleteListener;
    }

    public void setOnStartListener(OnStartListener onStartListener) {
        this.onStartListener = onStartListener;
    }

    public void setOnInfoListener(OnInfoListener onInfoListener) {
        this.onInfoListener = onInfoListener;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        display(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    /**
     * 开始播放
     */
    private void onStart() {
        MyLog.d("on start");
        if (null != onStartListener) {
            onStartListener.onStart();
        }
    }

    /**
     * 进度改变监听
     */
    private void onInfo() {
        if (null != onInfoListener) {
            timeBean.setCurrentDuration((int) getCurrentPosition());
            timeBean.setTotalDuration(getTotalTime());
            onInfoListener.onInfo(timeBean);
        }
    }

    /**
     * 播放完成监听
     */
    private void onComplete() {
        if (null != onCompleteListener) {
            //释放资源，便于下次播放
            onCompleteListener.complete();
        }
    }

}
