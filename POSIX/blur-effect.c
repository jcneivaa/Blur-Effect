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


using namespace cv;

struct info	//struct para pasar datos a cada hilo
{
	int inicio;
	int fin;
	int w;
	int h;
	Mat m;
	Mat tmp;
	int k;
};

void *blur(void *ap2){	//funcion (rutina) que correra cada hilo para hacer blur
	info *ap=(info *)ap2;
	int start, end, i;
	start = ap -> inicio;
	end = ap -> fin;
	int w = ap -> w;
	int h = ap->h;
	Mat m = ap ->m;
	Mat tmp = ap ->tmp;
	int kernel = ap->k;

	int sumB=0;
	int sumG=0;
	int sumR=0;
	int deno=(kernel*kernel)-1;

  for (i = start+(kernel/2); i < end+(kernel/2); ++i)	//recorrer la matriz recibida desde el punto inicial recibido.
  {
  	for (int j = 0+kernel/2; j < w+(kernel/2); ++j)
  	{
  		sumB=0;
  		sumG=0;
  		sumR=0;
  		int klimitr=i+(kernel/2);	//limites de convolucion
  		int klimitc=j+(kernel/2);	//limites de convolucion
  		if (kernel%2==0)
  		{
  			klimitr=i+(kernel/2)-1;
  			klimitc=j+(kernel/2)-1;
  		}
  		
  		for (int l = i-(kernel/2); l <= klimitr; ++l)		//convocucion
  		{
  			for (int n = j-(kernel/2); n <= klimitc; ++n)	//convolucion
  			{
  				sumB+=(tmp.at<cv::Vec3b>(l,n)[0] );		//canal azul suma de vecinos
  				sumG+=(tmp.at<cv::Vec3b>(l,n)[1] );		//canal verde suma de vecinos
  				sumR+=(tmp.at<cv::Vec3b>(l,n)[2] );		//canal rojo suma de vecinos
  			}
  		}
  		sumB-=(tmp.at<cv::Vec3b>(i,j)[0] );				//canal azul resta de px a tratar
  		sumG-=(tmp.at<cv::Vec3b>(i,j)[1] );				//canal verde resta de px a tratar
  		sumR-=(tmp.at<cv::Vec3b>(i,j)[2] );				//canal rojo resta de px a tratar
  		
  		ap->m.at<cv::Vec3b>(i,j)[0] = sumB/deno;		//guardando datos
  		ap->m.at<cv::Vec3b>(i,j)[1] = sumG/deno;
  		ap->m.at<cv::Vec3b>(i,j)[2] = sumR/deno;
  		
  	}
  }
}



int main(int argc, char const *argv[])
{

int Ksize=atoi(argv[3]);
IplImage* img = cvLoadImage(argv[1]);	//leer imagen dos veces para evitar shallow copy
IplImage* img2 = cvLoadImage(argv[1]);	
  cv::Mat m = cv::cvarrToMat(img); 		//convirtiendo imagen en matriz
  cv::Mat tmp = cv::cvarrToMat(img2);
  CvSize dim = cvGetSize(img);
  std::string filename=argv[1];
  int filenameSize=filename.size();
  std::string blurredFilename=filename.substr(0,filenameSize-4);
  int h=dim.height; 	//altura
  int w=dim.width;		//ancho
  
  	copyMakeBorder( m, m, Ksize/2, Ksize/2, Ksize/2, Ksize/2, BORDER_REPLICATE);		//expandir imagen como manejo de bordes
  	copyMakeBorder( tmp, tmp, Ksize/2, Ksize/2, Ksize/2, Ksize/2, BORDER_REPLICATE);

  	Mat blurred=tmp;
	int i, cont, rango;
	int numHilos = atoi(argv[2]);		//leer numero de hilos

	rango = h/numHilos;		//fin trabajo primer hilo
	cont = 0;				//inicio trabajo primer hilo
	struct info array[numHilos];	//ingresando datos en struct para cada hilo
	for (i = 0; i < numHilos; ++i)
	{
		array[i].inicio = cont;
		array[i].fin = rango;
		array[i].w=w;
		array[i].h=h;
		array[i].m=m;
		array[i].tmp=tmp;
		array[i].k=Ksize;
		cont = rango; 	//inicio de tabajo de cada hilo hilo
		rango = rango + (h/numHilos); //fin de trabajo de cada hilo
	}

	pthread_t hilos[numHilos];
	int r, j, *retval;
	for (i = 0; i < numHilos; ++i)	//crear hilos
	{
		r = pthread_create(&hilos[i], NULL, &blur, (void *)&array[i]);
		if (r != 0)
		{
			perror("Error al crear el hilo");
			exit(-1);
		}
	}

	for (j = 0; j < numHilos; ++j)		//join hilos
	{
		r = pthread_join(hilos[j], (void **)&retval);
	}
	Mat cropedImage = m(Rect(Ksize/2,Ksize/2,w,h));
	int y=0;
	int copyLimit=h/numHilos;
	for (int x = 0; x < numHilos; ++x)		//join data
	{
		for (y; y < copyLimit; ++y)
		{
			for (int z = 0; z < w; ++z)
			{
				blurred.at<cv::Vec3b>(y,z)[0] = array[x].m.at<cv::Vec3b>(y,z)[0];
				blurred.at<cv::Vec3b>(y,z)[1] = array[x].m.at<cv::Vec3b>(y,z)[1];
				blurred.at<cv::Vec3b>(y,z)[2] = array[x].m.at<cv::Vec3b>(y,z)[2];
			}
		}
		
		copyLimit+=h/numHilos;
		
	}
	//guardar imagen procesada y recortada
	cv::imwrite( blurredFilename+"-blurred- #threads="+std::to_string(numHilos)+", kernel size= "+std::to_string(Ksize)+".jpg", cropedImage);
}