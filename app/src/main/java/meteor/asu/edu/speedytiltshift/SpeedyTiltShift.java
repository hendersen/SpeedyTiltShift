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
    public static int GaussianResult (int x, int y, float sigma) {
        int G = (int) ( 1/Math.sqrt(2*Math.PI*sigma*sigma)*Math.exp((-1)*(x*x + y*y)/(2*sigma*sigma)));
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
        for(int i = r*(-1); i<= r ; i++) {
            p_new += GaussianResult(i, (-1)*r, sigma)*getPoint(x-r, y+i, pixels, width);
        }
        for(int i = r*(-1); i<= r ; i++) {
            p_new += GaussianResult(i, (-1)*r, sigma)*getPoint(x, y+i, pixels, width);
        }
        for(int i = r*(-1); i<= r ; i++) {
            p_new += GaussianResult(i, (-1)*r, sigma)*getPoint(x+r, y+i, pixels, width);
        }
        return p_new;
    }

    public static Bitmap tiltshift_java(Bitmap in, int a0, int a1, int a2, int a3, float s_far, float s_near){
        Bitmap out;
        out=in.copy(in.getConfig(),true);

        int width=in.getWidth();
        int height=in.getHeight();

        int r_far =(int) Math.max(6, s_far);
        int r_near = (int) Math.max(6, s_near);

        Log.d("TILTSHIFT_JAVA","hey:"+width+","+height);
        int[] pixels = new int[width*height];
        int offset=0;
        int stride = width;
        in.getPixels(pixels,offset,stride,0,0,width,height);
        for (int y=0; y<height; y++){
            for (int x = 0; x<width; x++){
//                // From Google Developer: int color = (A & 0xff) << 24 | (R & 0xff) << 16 | (G & 0xff) << 16 | (B & 0xff);
//                int p = pixels[y*width+x];
//                int BB = p & 0xff;
//                int GG = (p<<8)& 0xff;
//                int RR = 0xff;//(p<<16)& 0xff; //set red high
//                int AA = (p<<24)& 0xff;
//                int color = (AA & 0xff) << 24 | (RR & 0xff) << 16 | (GG & 0xff) << 8 | (BB & 0xff);
//                pixels[y*width+x] = color;

//                Divide by height and apply according procedure
                if(y <= a0) {
//                    Apply guassian blur(through weighted sum
                    Weight_matrix(r_far, y, x, pixels, width, s_far);
                }
                else if(y <= a1) {
//                   Apply guassian blur(through weighted sum
                    Weight_matrix(r_far, y, x, pixels, width, s_far*(a1-y)/(a1-a0));
                }
                else if(y <= a2) {
//                    No nlur
                }
                else if(y <= a3) {
                    Weight_matrix(r_near, y, x, pixels, width, s_far*(y-a2)/(a3-a2));
                }
                else {
//                     Apply guassian blur(through weighted sum
                    Weight_matrix(r_near, y, x, pixels, width, s_near);
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
