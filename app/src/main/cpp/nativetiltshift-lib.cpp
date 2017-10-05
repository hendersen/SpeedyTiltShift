#include <jni.h>
#include <string>
#include <arm_neon.h>
#include <math.h>
using namespace std;
extern "C" {

JNIEXPORT jintArray JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_nativeTiltShift(JNIEnv *env,
                                                                    jobject This,
                                                                    jintArray pixels_,
                                                                    jint width, jint height,
                                                                    jint a0, jint a1, jint a2, jint a3, jfloat s_far, jfloat s_near
                                                                    ) {
    int32x4_t sum_vec = vdupq_n_s32(0);
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    long length = env->GetArrayLength(pixels_);
    jintArray pixelsOut = env->NewIntArray(length);

    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);
    env->ReleaseIntArrayElements(pixels_, pixels, 0);
    return pixelsOut;
}

JNIEXPORT jintArray JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_nativeTiltShiftNeon(JNIEnv *env,
                                                                    jobject This,
                                                                    jintArray pixels_,
                                                                    jint width, jint height,
                                                                    jint a0, jint a1, jint a2, jint a3, jfloat s_far, jfloat s_near
) {
    int32x4_t sum_vec = vdupq_n_s32(0);
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    long length = env->GetArrayLength(pixels_);
    jintArray pixelsOut = env->NewIntArray(length);
    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);

    float32_t arrayIn[length];
    float32_t arrayOut[length];

    for(int i=0;i<length; i++){
        arrayIn[i]=pixels[i];
    }

    int arrlen=length/4;
    float32_t AA[arrlen];
    float32_t RR[arrlen];
    float32_t GG[arrlen];
    float32_t BB[arrlen];


    float32_t * arrayInPtr = (float32_t *)arrayIn;
    for(int i=0;i<arrlen;i++){
        float32x4x4_t vecs=vld4q_f32(arrayInPtr);
        float32x4_t A=vecs.val[0];
        float32x4_t R=vecs.val[1];
        float32x4_t G=vecs.val[2];
        float32x4_t B=vecs.val[3];
        AA[i]=A;
        RR[i]=R;
        GG[i]=G;
        BB[i]=B;
        arrayInPtr+=16;
    }

    int sig=0;
    float32_t temp0;
    float32_t temp1;
    float32_t temp2;
    float32_t temp3;
    for(int j=0;j<arrlen;j++){
        if(j<(a0*width/4)){
            sig=s_far;
            int r=ceil(3*sig);
            int size=(2*r+1)*(2*r+1);
            int mid=(size-1)/2;
            temp0=0;
            temp1=0;
            temp2=0;
            temp3=0;
            float32_t weight[size];
            for(int m=0;m<2*r+1;m++){
                for(int n=0;n<2*r+1;n++){
                    weight[m*r+n]=exp(-(m*m+n*n)/(2*sig*sig))/(2*3.14*sig*sig);
                }
            }
            for(int a=mid;a<size&&a+j-mid<arrlen;a++) {
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            for(int a=mid-1;a>0&&a>mid-j;a--){
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            arrayOut[4*j]=temp0;
            arrayOut[4*j+1]=temp1;
            arrayOut[4*j+2]=temp2;
            arrayOut[4*j+3]=temp3;
            break;
        }
        if(j<a1*width/4){
            int len=floor(4*j/width);
            sig=s_far*((a1-len)/(a1-a0));
            int r=ceil(sig*3);
            int size=(2*r+1)*(2*r+1);
            int mid=(size-1)/2;
            temp0=0;
            temp1=0;
            temp2=0;
            temp3=0;
            float32_t weight[size];
            for(int m=0;m<2*r+1;m++){
                for(int n=0;n<2*r+1;n++){
                    weight[m*r+n]=exp(-(m*m+n*n)/(2*sig*sig))/(2*3.14*sig*sig);
                }
            }
            for(int a=mid;a<size&&a+j-mid<arrlen;a++) {
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            for(int a=mid-1;a>0&&a>mid-j;a--){
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            arrayOut[4*j]=temp0;
            arrayOut[4*j+1]=temp1;
            arrayOut[4*j+2]=temp2;
            arrayOut[4*j+3]=temp3;
            break;
        }
        if(j<a2*width/4){
            arrayOut[4*j]=AA[j];
            arrayOut[4*j+1]=RR[j];
            arrayOut[4*j+2]=GG[j];
            arrayOut[4*j+3]=BB[j];
            break;
        }
        if(j<a3*width/4){
            int len=floor(4*j/width);
            sig=s_near*((len-a2)/(a3-a2));
            int r=ceil(sig*3);
            int size=(2*r+1)*(2*r+1);
            int mid=(size-1)/2;
            temp0=0;
            temp1=0;
            temp2=0;
            temp3=0;
            float32_t weight[size];
            for(int m=0;m<2*r+1;m++){
                for(int n=0;n<2*r+1;n++){
                    weight[m*r+n]=exp(-(m*m+n*n)/(2*sig*sig))/(2*3.14*sig*sig);
                }
            }
            for(int a=mid;a<size&&a+j-mid<arrlen;a++) {
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            for(int a=mid-1;a>0&&a>mid-j;a--){
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            arrayOut[4*j]=temp0;
            arrayOut[4*j+1]=temp1;
            arrayOut[4*j+2]=temp2;
            arrayOut[4*j+3]=temp3;
            break;

        }
        else{
            sig=s_near;
            int r=ceil(3*sig);
            int size=(2*r+1)*(2*r+1);
            int mid=(size-1)/2;
            temp0=0;
            temp1=0;
            temp2=0;
            temp3=0;
            float32_t weight[size];
            for(int m=0;m<2*r+1;m++){
                for(int n=0;n<2*r+1;n++){
                    weight[m*r+n]=exp(-(m*m+n*n)/(2*sig*sig))/(2*3.14*sig*sig);
                }
            }
            for(int a=mid;a<size&&a+j-mid<arrlen;a++) {
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            for(int a=mid-1;a>0&&a>mid-j;a--){
                temp0=vmla_f32(temp0,AA[j+a-mid],weight[a]);
                temp1=vmla_f32(temp1,RR[j+a-mid],weight[a]);
                temp2=vmla_f32(temp2,GG[j+a-mid],weight[a]);
                temp3=vmla_f32(temp3,BB[j+a-mid],weight[a]);
            }
            arrayOut[4*j]=temp0;
            arrayOut[4*j+1]=temp1;
            arrayOut[4*j+2]=temp2;
            arrayOut[4*j+3]=temp3;
        }

    }
    jint *h=new jint[length];
    for(int i=0;i<length;i++){
        h[i]=(jint)arrayOut[i];
    }

    env->SetIntArrayRegion(pixelsOut,0,length,h);


    env->ReleaseIntArrayElements(pixels_, pixels, 0);
    return pixelsOut;
}
}
