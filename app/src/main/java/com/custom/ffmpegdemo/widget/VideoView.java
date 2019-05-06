package com.custom.ffmpegdemo.widget;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;

import com.custom.ffmpegdemo.FFMpegUtil;

/**
 * author：tangjianlin
 * date：2019/3/28
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
public class VideoView extends SurfaceView {

    public VideoView(Context context) {
        this(context, null);
    }

    public VideoView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }
}

