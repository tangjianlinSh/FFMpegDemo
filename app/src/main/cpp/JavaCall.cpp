//
// Created by 唐建林 on 2019/4/1.
//

#include "JavaCall.h"

JavaCall::JavaCall(_JavaVM *vm, JNIEnv *env, jobject *jobj) {
    javaVM = vm;
    jniEnv = env;
    obj = *jobj;
    obj = env->NewGlobalRef(obj);
    jclass jlz = jniEnv->GetObjectClass(obj);
    if (!jlz) {
        return;
    }
    idStart = jniEnv->GetMethodID(jlz, "onStart", "()V");
    idInfo = jniEnv->GetMethodID(jlz, "onInfo", "()V");
    idComplete = jniEnv->GetMethodID(jlz, "onComplete", "()V");
}

JavaCall::~JavaCall() {

}

void JavaCall::onStart() {
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
//            LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
        return;
    }
    jniEnv->CallVoidMethod(obj, idStart);
    javaVM->DetachCurrentThread();
}

void JavaCall::onRelease() {
    if (javaVM != NULL) {
        javaVM = NULL;
    }
    if (jniEnv != NULL) {
        jniEnv = NULL;
    }
}

void JavaCall::onInfo() {
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
//            LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
        return;
    }
    jniEnv->CallVoidMethod(obj, idInfo);
    javaVM->DetachCurrentThread();
}

void JavaCall::onComplete() {
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
//            LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
        return;
    }
    jniEnv->CallVoidMethod(obj, idComplete);
    javaVM->DetachCurrentThread();
}