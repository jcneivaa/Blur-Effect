#include <omp.h>
#include <stdio.h>
#include <opencv/cvaux.h>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <opencv/cv.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

using namespace cv;

int w = 0;
int h = 0;
int channels = 0;

int main(int argc, char *argv[])
{
    IplImage *img = cvLoadImage(argv[1]);
    IplImage *img2 = cvLoadImage(argv[1]);
    int kernel = atoi(argv[3]);
    int num_threads = atoi(argv[2]);
    cv::Mat m = cv::cvarrToMat(img);
    cv::Mat m2 = cv::cvarrToMat(img2);
    CvSize dim = cvGetSize(img);
    h = dim.height;
    w = dim.width;
    channels = img->nChannels;

    if (!img)
    {
        printf("Image can NOT Load!!!\n");
        return 1;
    }


    int sumB = 0;
    int sumG = 0;
    int sumR = 0;
    int deno = (kernel * kernel) - 1;
    int thread;

    omp_set_num_threads(num_threads); 
    #pragma omp parallel shared(m,m2) private(sumB,sumG,sumR)
    {   
        //copyMakeBorder( m, m, kernel/2, kernel/2, kernel/2, kernel/2, BORDER_REPLICATE);		//expandir imagen como manejo de bordes
  	    //copyMakeBorder( m2, m2, kernel/2, kernel/2, kernel/2, kernel/2, BORDER_REPLICATE);
        #pragma omp for schedule (static)
        for (int i = (kernel / 2); i < (h - (kernel / 2)); ++i) //recorrer la matriz recibida desde el punto inicial recibido.
        {
            for (int j = (kernel / 2); j < (w - (kernel / 2)); ++j)
            {
                sumB = 0;
                sumG = 0;
                sumR = 0;
                int klimitr = i + (kernel / 2); //limites de convolucion
                int klimitc = j + (kernel / 2); //limites de convolucion
                if (kernel % 2 == 0)
                {
                    klimitr = i + (kernel / 2) - 1;
                    klimitc = j + (kernel / 2) - 1;
                }

                for (int l = i - (kernel / 2); l <= klimitr; ++l) //convocucion
                {
                    for (int n = j - (kernel / 2); n <= klimitc; ++n) //convolucion
                    {   
                        sumB += (m.at<cv::Vec3b>(l, n)[0]); //canal azul suma de vecinos
                        sumG += (m.at<cv::Vec3b>(l, n)[1]); //canal verde suma de vecinos
                        sumR += (m.at<cv::Vec3b>(l, n)[2]); //canal rojo suma de vecinos
                    }
                }
                
                sumB -= (m.at<cv::Vec3b>(i, j)[0]); //canal azul resta de px a tratar
                sumG -= (m.at<cv::Vec3b>(i, j)[1]); //canal verde resta de px a tratar
                sumR -= (m.at<cv::Vec3b>(i, j)[2]); //canal rojo resta de px a tratar          
        
                m2.at<cv::Vec3b>(i, j)[0] = sumB / deno; //guardando datos
                m2.at<cv::Vec3b>(i, j)[1] = sumG / deno;
                m2.at<cv::Vec3b>(i, j)[2] = sumR / deno;
            }
        }   
    }
    //Mat m3 = m2(Rect(kernel/2,kernel/2,w,h));
    //cv::imwrite(" blur-recortada.jpg", m3);
    cv::imwrite(" blur.jpg", m2);
    return 0;
}