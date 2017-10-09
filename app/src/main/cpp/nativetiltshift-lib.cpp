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
JNIEXPORT jintArray JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_nativeTiltShiftNeon(JNIEnv *env,
                                                                    jobject This,
                                                                    jintArray pixels_,
                                                                    jint width, jint height,
                                                                    jint a0, jint a1, jint a2, jint a3, jfloat s_far, jfloat s_near
) {

    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    long length = env->GetArrayLength(pixels_);

    pixels = env->GetIntArrayElements(pixels_, NULL);
    jintArray pixelsOut = env->NewIntArray(length);
    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);

    uint8_t arrayIn[length];





    for(int i=0;i<length; i++){
        arrayIn[i]=pixels[i];
    }


    jint *h=new jint[length];//store result

    for(int i=0;i<length; i++){
        h[i]=pixels[i];
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
            if(sig<0.7){
                continue;
            }
        }

        if((a0*width/4)<j&&j<=(a1*width/4)){
            int len=floor(4*j/width);
            sig=s_far*((a1-len)/(a1-a0));
            if(sig<0.7){
                continue;
            }
        }

        if((a1*width/4)<j&&j<=(a2*width/4)){
            continue;
        }

        if((a2*width/4)<j&&j<=(a3*width/4)){
            int len=floor(4*j/width);
            sig=s_near*((len-a2)/(a3-a2));
            if(sig<0.7){
                continue;
            }
        }

        if((a3*width/4)<j){
            sig=s_far;
            if(sig<0.7){
                continue;
            }
        }

        int r=ceil(3*sig);
        int size=(2*r+1)*(2*r+1);

        float32_t weight[size];
        float32_t imageA[size];
        float32_t imageR[size];
        float32_t imageG[size];
        float32_t imageB[size];

        for(int m=0;m<2*r+1;m++){
            for(int n=0;n<2*r+1;n++){
                float mf=(float) m-r;
                float nf=(float) n-r;
                weight[m*(2*r+1)+n]=exp(-(mf*mf+nf*nf)/(2*sig*sig))/(2*3.14*sig*sig);
            }
        }//construct weight matrix

        float32_t * weightPtr=(float32_t *)weight;

        int loc_x=4*j%width;
        int loc_y=floor(4*j/width);



        for(int n=0;n<2*r+1;n++){
            for(int m=0;m<2*r+1;m++){
                if(loc_x-4*r+4*m<0||loc_x-4*r+4*m>width) {

                        imageA[n*(2*r+1)+m]=0;
                        imageR[n*(2*r+1)+m]=0;
                        imageG[n*(2*r+1)+m]=0;
                        imageB[n*(2*r+1)+m]=0;

                }else{
                    if(loc_y-r+n<0||loc_y-r+n>height){

                        imageA[n*(2*r+1)+m]=0;
                        imageR[n*(2*r+1)+m]=0;
                        imageG[n*(2*r+1)+m]=0;
                        imageB[n*(2*r+1)+m]=0;
                    }else{
                        imageA[n*(2*r+1)+m]=arrayIn[(loc_y-r+n)*width+(loc_x-4*r+4*m)];
                        imageR[n*(2*r+1)+m]=arrayIn[(loc_y-r+n)*width+(loc_x-4*r+4*m)+1];
                        imageG[n*(2*r+1)+m]=arrayIn[(loc_y-r+n)*width+(loc_x-4*r+4*m)+2];
                        imageB[n*(2*r+1)+m]=arrayIn[(loc_y-r+n)*width+(loc_x-4*r+4*m)+3];
                    }

                }


            }

        }//construct image matrix

        float32_t * imageAPtr=(float32_t *)imageA;
        float32_t * imageRPtr=(float32_t *)imageR;
        float32_t * imageGPtr=(float32_t *)imageG;
        float32_t * imageBPtr=(float32_t *)imageB;


        for(int i=0;i<(size/4);i++){
            float32x4_t Avec = vld1q_f32(imageAPtr);
            float32x4_t Rvec = vld1q_f32(imageRPtr);
            float32x4_t Gvec = vld1q_f32(imageGPtr);
            float32x4_t Bvec = vld1q_f32(imageBPtr);
            float32x4_t wvec = vld1q_f32(weightPtr);



           sum_vecA = vmlaq_f32(sum_vecA,Avec,wvec);
           sum_vecR = vmlaq_f32(sum_vecR,Rvec,wvec);
           sum_vecG = vmlaq_f32(sum_vecG,Gvec,wvec);
           sum_vecB = vmlaq_f32(sum_vecB,Bvec,wvec);
            weightPtr +=4;
            imageAPtr += 4;
            imageRPtr += 4;
            imageGPtr += 4;
            imageBPtr += 4;
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



            h[j*4]=(jint)sumA;
            h[j*4+1]=(jint)sumR;
            h[j*4+2]=(jint)sumG;
            h[j*4+3]=(jint)sumB;

    }


    env->SetIntArrayRegion(pixelsOut,0,length,h);
    return pixelsOut;
}
}
