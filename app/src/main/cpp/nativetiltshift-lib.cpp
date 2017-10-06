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

    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    long length = env->GetArrayLength(pixels_);
    jintArray pixelsOut = env->NewIntArray(length);
    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);

    float32_t arrayIn[length];
    float32_t arrayOut[length];

    for(int i=0;i<length; i++){
        arrayIn[i]=pixels[i];
        arrayOut[i]=pixels[i];
    }


    int arrlen=length/4;
    jfloat sig=0;

    for(int j=0;j<arrlen;j++){
        float32x4_t sum_vecA = vdupq_n_f32(0);
        float32x4_t sum_vecR = vdupq_n_f32(0);
        float32x4_t sum_vecG = vdupq_n_f32(0);
        float32x4_t sum_vecB = vdupq_n_f32(0);
        if(j<=(a0*width/4)){
            sig=s_far;
        }

        if((a0*width/4)<j&&j<=(a1*width/4)){
            int len=floor(4*j/width);
            sig=s_far*((a1-len)/(a1-a0));
        }

        if((a1*width/4)<j&&j<=(a2*width/4)){
            break;
        }

        if((a2*width/4)<j&&j<=(a3*width/4)){
            int len=floor(4*j/width);
            sig=s_near*((len-a2)/(a3-a2));
        }

        if((a3*width/4)<j){
            sig=s_far;
        }

        int r=ceil(3*sig);
        int size=(2*r+1)*(2*r+1);
        int mid=(size-1)/2;
        float32_t weight[size];
        float32_t image[size*4];

        for(jint m=0;m<2*r+1;m++){
            for(int n=0;n<2*r+1;n++){
                weight[m*r+n]=exp(-(m*m+n*n)/(2*sig*sig))/(2*3.14*sig*sig);
            }
        }//construct weight matrix

        int loc_x=4*j%width;
        int loc_y=floor(4*j/width);
        float32_t * weightPtr=(float32_t *)weight;


        for(int m=0;m<2*r+1;m++){
            for(int n=0;n<2*r+1;n++){
                if(loc_x-4*r+4*m<0||loc_x-4*r+4*m>width) {
                    for(int a=0;a<4;a++){
                        image[n*(2*r+1)+4*m+a]=0;
                    }
                }else{
                    if(loc_y-r+n<0||loc_y-r+n>height){
                        for(int a=0;a<4;a++){
                            image[n*(2*r+1)+4*m+a]=0;
                        }
                    }
                }
            }

        }//construct image matrix

        float32_t * imagePtr=(float32_t *)image;

        for(int i=0;i<(size/4);i++){
            float32x4x4_t ivec=vld4q_f32(imagePtr);
            float32x4_t wvec=vld1q_f32(weight);
            sum_vecA = vmla_f32(sum_vecA,ivec.val[0],wvec);
            sum_vecR = vmla_f32(sum_vecR,ivec.val[1],wvec);
            sum_vecG = vmla_f32(sum_vecG,ivec.val[2],wvec);
            sum_vecB = vmla_f32(sum_vecB,ivec.val[3],wvec);
        }

        float sumA=0;
        float sumR=0;
        float sumG=0;
        float sumB=0;

        sumA +=vgetq_lane_f32(sum_vecA,0);
        sumA +=vgetq_lane_f32(sum_vecA,1);
        sumA +=vgetq_lane_f32(sum_vecA,2);
        sumA +=vgetq_lane_f32(sum_vecA,3);

        sumR +=vgetq_lane_f32(sum_vecR,0);
        sumR +=vgetq_lane_f32(sum_vecR,1);
        sumR +=vgetq_lane_f32(sum_vecR,2);
        sumR +=vgetq_lane_f32(sum_vecR,3);

        sumG +=vgetq_lane_f32(sum_vecG,0);
        sumG +=vgetq_lane_f32(sum_vecG,1);
        sumG +=vgetq_lane_f32(sum_vecG,2);
        sumG +=vgetq_lane_f32(sum_vecG,3);

        sumB +=vgetq_lane_f32(sum_vecB,0);
        sumB +=vgetq_lane_f32(sum_vecB,1);
        sumB +=vgetq_lane_f32(sum_vecB,2);
        sumB +=vgetq_lane_f32(sum_vecB,3);//calculate A,R,G,B distribute

        arrayOut[j*4]=sumA;
        arrayOut[j*4+1]=sumR;
        arrayOut[j*4+2]=sumG;
        arrayOut[j*4+3]=sumB;//cppy image value to arrayout
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
