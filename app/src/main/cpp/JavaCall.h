//
// Created by 唐建林 on 2019/4/1.
//

#ifndef FFMPEGDEMO_JAVACALL_H
#define FFMPEGDEMO_JAVACALL_H

#include <jni.h>
#include <stdint.h>

class JavaCall {

public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject obj;
    jmethodID idStart;
    jmethodID idInfo;
    jmethodID idComplete;

public:
    JavaCall(_JavaVM *javaVM, JNIEnv *env, jobject *jobj);

    ~JavaCall();

    void onStart();

    void onRelease();

    void onInfo();

    void onComplete();
};


#endif //FFMPEGDEMO_JAVACALL_H
