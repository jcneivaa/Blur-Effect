#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <opencv/cvaux.h>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <string>
#include <string.h>
#include <iostream>
#include <cuda_runtime.h>
struct pixel{
	int R,G,B;
};
__global__ void blur(const pixel *src_img, pixel *dst_img,int Ksize, int h, int w, int numElements, int threads){


	int index = (blockDim.x * blockIdx.x) + threadIdx.x;
        if (index<w-Ksize)
    {
	int n;
	int l;
	int i;
	int j;
    int sumR=0;
    int sumG=0;
    int sumB=0;
    int klimitr;	//limites de convolucion
	int klimitc;	//limites de convolucion  
    int aux=((h-(2*(Ksize/2)))/threads);
    int start= index*aux;
    int end=start+aux;
    int Km=Ksize/2;

if (threads>(h-(2*Km)))
        {
             aux=1;
        }

		//i=index+(Ksize/2);
		for (i = start+Ksize/2; i < end+Ksize/2; ++i)
        {
            /* code */
        
	  	for (j = Ksize/2; j < w-Ksize/2; ++j)
	  	{
	  		sumB=0;
	  		sumG=0;
	  		sumR=0;
	  		
	  		if (Ksize%2==0)
	  		{
	  			klimitr=i+(Ksize/2)-1;
	  			klimitc=j+(Ksize/2)-1;
	  		}else{
                klimitr=i+(Ksize/2);    //limites de convolucion
                klimitc=j+(Ksize/2);    //limites de convolucion
            }
	  		for (l = i-(Ksize/2); l <= klimitr; ++l)		//convocucion
	  		{
	  			for (n = j-(Ksize/2); n <= klimitc; ++n)	//convolucion
	  			{
	  				sumR+=src_img[l*w+n].R;
	                sumG+=src_img[l*w+n].G;
	                sumB+=src_img[l*w+n].B;		//canal rojo suma de vecinos
	  			}
	  		}
	  		sumR-=src_img[i*w+j].R;
	        sumG-=src_img[i*w+j].G;
	        sumB-=src_img[i*w+j].B;			//canal rojo resta de px a tratar
	  		
	  		dst_img[i*w+j].R=sumR/((Ksize*Ksize)-1);
	        dst_img[i*w+j].G=sumG/((Ksize*Ksize)-1);
	        dst_img[i*w+j].B=sumB/((Ksize*Ksize)-1);
	  		}
	  	}
  	}
  }








using namespace cv;

int main(int argc, char const *argv[])
{
	Mat mat = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	int Ksize=atoi(argv[2]);
	int h=mat.rows;
	int w=mat.cols;
	
/*  if (!img)
  {
    printf("Image: can NOT Load!!!\n");
    return 1;
  }*/
  copyMakeBorder( mat, mat, Ksize/2, Ksize/2, Ksize/2, Ksize/2, BORDER_REPLICATE);
  	int tmpH=mat.rows;
	int tmpW=mat.cols;
  	cudaError_t err = cudaSuccess;
	int numElements = tmpH*tmpW;
	int size = numElements*sizeof(pixel);
	
	pixel * h_img = (pixel*)malloc(size);



	if (h_img == NULL)
    {
        fprintf(stderr, "Failed to allocate host vectors!\n");
        exit(EXIT_FAILURE);
    }


  for (int i = 0; i < tmpH; ++i)
  {
	for (int j = 0; j < tmpW; ++j)
	{
	  h_img[(tmpW*i)+j].B=(mat.at<cv::Vec3b>(i,j)[0]);
	  h_img[(tmpW*i)+j].G=(mat.at<cv::Vec3b>(i,j)[1]);
	  h_img[(tmpW*i)+j].R=(mat.at<cv::Vec3b>(i,j)[2]);
    }
}



	pixel * d_img;
	err = cudaMalloc((void**)&d_img, size);
	if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector d_img (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    pixel * d_imgDst;

	err = cudaMalloc((void**)&d_imgDst, size);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector D_imgDst (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }


    err = cudaMemcpy(d_img, h_img, size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector h_R from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

 

    int threadsPerBlock = atoi(argv[3]);
    int blocks=atoi(argv[4]);
    if (blocks==0){
    	blocks= (h/threadsPerBlock)+1;
    }
    int threads=blocks*threadsPerBlock;
    printf("CUDA kernel launch with %d blocks of %d threads\n", blocks, threadsPerBlock);
    blur<<<blocks,threadsPerBlock>>>(d_img, d_imgDst, Ksize, tmpH, tmpW, numElements, threads);

    err = cudaGetLastError();

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to launch blur kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(h_img, d_imgDst, size, cudaMemcpyDeviceToHost);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector d_dstR from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_img);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector d_img (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_imgDst);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to free device vector d_imgDst (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }



    /*
	implementacion de vectores resultantes de cuda blur kernel a Mat m
    */
    for (int i = 0; i < tmpH; ++i)
        {
        for (int j = 0; j < tmpW; ++j)
            {
            (mat.at<cv::Vec3b>(i,j)[0])=h_img[(tmpW*i)+j].B;
            (mat.at<cv::Vec3b>(i,j)[1])=h_img[(tmpW*i)+j].G;
            (mat.at<cv::Vec3b>(i,j)[2])=h_img[(tmpW*i)+j].R;

        }
    }

    Mat cropedImage = mat(Rect(Ksize/2,Ksize/2,w,h));
    std::string filename=argv[1];
    filename=filename.substr(0,sizeof(argv[1]));
    cv::imwrite( filename+" --NOCROPPED-- .jpg", mat);
    cv::imwrite( filename+" --blurred-- .jpg", cropedImage);

    free(h_img);


    err = cudaDeviceReset();

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to deinitialize the device! error=%s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    printf("Done\n");
    return 0;

}