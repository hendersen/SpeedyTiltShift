package meteor.asu.edu.speedytiltshift;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

/**
 * Created by roblkw on 7/26/17.
 */

public class SpeedyTiltShift {

    static {
        System.loadLibrary("nativetiltshift-lib");
    }


//    Gaussian Function that is used to calculate the each entry of kernel function
    public static float GaussianResult (int x, int y, float sigma) {
        float G = (int) ( 1/Math.sqrt(2*Math.PI*sigma*sigma)*Math.exp((-1)*(x*x + y*y)/(2*sigma*sigma)));
        return G;
    }

    public static int getPoint(int x, int y, int[] pixels, int width) {
        int index = y*width+x;
        if(index >=0 || index <pixels.length) return pixels[index];
        else return 0;
    }

//    Gaussian blur using weight matrix approach
    public static int Weight_matrix (int r, int y, int x, int[] pixels, int width, float sigma) {
        int p_new = 0;
        int length=pixels.length;
        int height=length/width;
        for( int i=r*(-1); i<=r; i++){
            if(x+i>=0 && x+i<width){
                for( int j=r*(-1); j<=r; j++){
                    if(y+j>=0 && y+j<height){
                        p_new +=(int)GaussianResult( i, j, sigma)*getPoint(x+i, y+j, pixels, width);
                    }
                }
            }
        }
        return p_new;
    }

    public static Bitmap tiltshift_java(Bitmap in, int a0, int a1, int a2, int a3, float s_far, float s_near){
        Bitmap out;
        out=in.copy(in.getConfig(),true);

        int width=in.getWidth();
        int height=in.getHeight();

        int r_far =(int) Math.ceil(6* s_far);
        int r_near = (int) Math.ceil(6*s_near);

        Log.d("TILTSHIFT_JAVA","hey:"+width+","+height);
        int[] pixels = new int[width*height];
        int offset=0;
        int stride = width;
        in.getPixels(pixels,offset,stride,0,0,width,height);
        int[] BB=new int[width*height];
        for(int i=0;i<width*height;i++){
            BB[i]=(pixels[i]&0xff);
        }

        int[] GG=new int[width*height];
        for(int i=0;i<width*height;i++){
            GG[i]=((pixels[i]<<8)&0xff);
        }

        int[] RR=new int[width*height];
        for(int i=0;i<width*height;i++){
            RR[i]=((pixels[i]<<16)&0xff);
        }

        int[] AA=new int[width*height];
        for(int i=0;i<width*height;i++){
            AA[i]=((pixels[i]<<24)&0xff);
        }

        for (int y=0; y<height; y++){
            for (int x = 0; x<width; x++){

////                // From Google Developer: int color = (A & 0xff) << 24 | (R & 0xff) << 16 | (G & 0xff) << 16 | (B & 0xff);                int p = pixels[y*width+x];
//                int BB = p & 0xff;
//                int GG = (p<<8)& 0xff;
//                int RR = 0xff;//(p<<16)& 0xff; //set red high
//                int AA = (p<<24)& 0xff;
//                int color = (AA & 0xff) << 24 | (RR & 0xff) << 16 | (GG & 0xff) << 8 | (BB & 0xff);
//                pixels[y*width+x] = color;

//                Divide by height and apply according procedure

                if(y <= a0) {
//                    Apply guassian blur(through weighted sum
                    BB[y*width+x]=Weight_matrix(r_far, y, x, BB, width, s_far);
                    GG[y*width+x]=Weight_matrix(r_far, y, x, GG, width, s_far);
                    RR[y*width+x]=Weight_matrix(r_far, y, x, RR, width, s_far);
                    AA[y*width+x]=Weight_matrix(r_far, y, x, AA, width, s_far);
                    int color = (AA[y*width+x] & 0xff) << 24 | (RR[y*width+x] & 0xff) << 16 | (GG[y*width+x] & 0xff) << 8 | (BB[y*width+x] & 0xff);
                    pixels[y*width+x] = color;
                }
                else if(y <= a1) {
//                   Apply guassian blur(through weighted sum
                    BB[y*width+x]=Weight_matrix(r_far, y, x, BB, width, s_far*(a1-y)/(a1-a0));
                    GG[y*width+x]=Weight_matrix(r_far, y, x, GG, width, s_far*(a1-y)/(a1-a0));
                    RR[y*width+x]=Weight_matrix(r_far, y, x, RR, width, s_far*(a1-y)/(a1-a0));
                    AA[y*width+x]=Weight_matrix(r_far, y, x, AA, width, s_far*(a1-y)/(a1-a0));
                    int color = (AA[y*width+x] & 0xff) << 24 | (RR[y*width+x] & 0xff) << 16 | (GG[y*width+x] & 0xff) << 8 | (BB[y*width+x] & 0xff);
                    pixels[y*width+x] = color;
                }
                else if(y <= a2) {
//                    No nlur
                }
                else if(y <= a3) {
                    BB[y*width+x]=Weight_matrix(r_far, y, x, BB, width, s_far*(y-a2)/(a3-a2));
                    GG[y*width+x]=Weight_matrix(r_far, y, x, GG, width, s_far*(y-a2)/(a3-a2));
                    RR[y*width+x]=Weight_matrix(r_far, y, x, RR, width, s_far*(y-a2)/(a3-a2));
                    AA[y*width+x]=Weight_matrix(r_far, y, x, AA, width, s_far*(y-a2)/(a3-a2));
                    int color = (AA[y*width+x] & 0xff) << 24 | (RR[y*width+x] & 0xff) << 16 | (GG[y*width+x] & 0xff) << 8 | (BB[y*width+x] & 0xff);
                    pixels[y*width+x] = color;
                }
                else {
//                     Apply guassian blur(through weighted sum
                    BB[y*width+x]=Weight_matrix(r_far, y, x, BB, width, s_near);
                    GG[y*width+x]=Weight_matrix(r_far, y, x, GG, width, s_near);
                    RR[y*width+x]=Weight_matrix(r_far, y, x, RR, width, s_near);
                    AA[y*width+x]=Weight_matrix(r_far, y, x, AA, width, s_near);
                    int color = (AA[y*width+x] & 0xff) << 24 | (RR[y*width+x] & 0xff) << 16 | (GG[y*width+x] & 0xff) << 8 | (BB[y*width+x] & 0xff);
                    pixels[y*width+x] = color;
                }
            }
        }
        out.setPixels(pixels,offset,stride,0,0,width,height);

//        Log.d("TILTSHIFT_JAVA","hey2");
        return out;
    }
    public static Bitmap tiltshift_cpp(Bitmap in, int a0, int a1, int a2, int a3, float s_far, float s_near){
        int width = in.getWidth();
        int height = in.getHeight();
        int[] pixels = new int[width*height];
        int[] outpixels = nativeTiltShift(pixels, width, height, a0, a1, a2, a3, s_far, s_near);
        Bitmap out = in.copy(in.getConfig(), true);
        out.setPixels(outpixels, 0, width, 0, 0, width, height);
        return in;
    }
    public static Bitmap tiltshift_neon(Bitmap in, int a0, int a1, int a2, int a3, float s_far, float s_near){
        return in;
    }


    private static native int[] nativeTiltShift(int[] pixels, int imgW, int imgH, int a0, int a1, int a2, int a3, float s_far, float s_near);
    private static native int[] nativeTiltShiftNeon(int[] pixels, int imgW, int imgH, int a0, int a1, int a2, int a3, float s_far, float s_near);

}
