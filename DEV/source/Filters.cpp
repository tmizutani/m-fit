#include "cv.h"
#include "highgui.h"
#include <QImage>

#include "../include/Log.h"

#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Filters.h"

/************************************************************************
* Fun��o que realiza a segmentacao da imagem usando um limiar.
*************************************************************************
* param (E): frame     - Frame a ser aplicado o efeito 
* param (E): threshold - Valor do limiar a ser aplicado
* return   : Frame*    - Ponteiro para um novo frame com o efeito
************************************************************************
* Hist�rico:
* 26/06/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
void Filters::segment(Frame* frame, int threshold)
{
	IplImage* img_dst;
//	Frame *frameNew;

	Log::writeLog("%s :: param: frame[%x] threshold[%d]", __FUNCTION__, frame, threshold);

	// Faz uma copia da imagem
	img_dst = Frame::imgAlloc(cvGetSize(frame->data),frame->data->depth,frame->data->nChannels);

	Log::writeLog("%s :: cvThreshold: frame_src[%x] frame_dst[%d] thresh[%d] maxValue[%d] CvThresh[%s]", __FUNCTION__, frame->data, img_dst, threshold, 255, "CV_THRESH_BINARY_INV");

	// Aplica o limiar
	cvThreshold(frame->data, img_dst, threshold, 255, CV_THRESH_BINARY_INV);

//	frameNew = new Frame(img_dst);

	frame->setImage(img_dst);

	// Nao preciso mais da imagem, posso (devo) desalocar
	//Frame::imgDealloc(img_dst);

	// Retorna um objeto encapsulado do tipo frame
//	return (frameNew);
}

/************************************************************************
* Fun��o que aplica o Operador Sobel nas imagens.
*************************************************************************
* param (E): Frame* frame: Imagem � qual ser� aplicado o operador.
* param (E): int direction:
* 				 0 - Vertical
* 				 1 - Horizontal
* 				 2 - Vertical e Horizontal
************************************************************************
* Retorno: Frame com o efeito aplicado (img_dst)
************************************************************************
* Autor: Thiago Mizutani
************************************************************************
* Hist�rico:
* 23/06/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Filters::Sobel(Frame* frame, int direction)
{
   IplImage* img_dst;

	Log::writeLog("%s :: param: frame[%x] direction[%d]", __FUNCTION__, frame, direction);
	
	img_dst = Frame::imgAlloc(cvGetSize(frame->data),frame->data->depth,frame->data->nChannels);

   switch(direction)
   {

		//origem, destino, vert, horiz, abertura.
      case 0: //Vertical

			Log::writeLog("%s :: cvSobel : frame_src[%x] frame_dst[%d] dx[%d] dy[%d] apertureSize[%d]",
				__FUNCTION__, frame->data, img_dst, 1, 0, 3);

         cvSobel(frame->data,img_dst,1,0,3);
         break;
      case 1: //Horizontal

			Log::writeLog("%s :: cvSobel : frame_src[%x] frame_dst[%d] dx[%d] dy[%d] apertureSize[%d]",
					__FUNCTION__, frame->data, img_dst, 0, 1, 3);

         cvSobel(frame->data,img_dst,0,1,3);
			break;
      case 2: //Both

			Log::writeLog("%s :: cvSobel : frame_src[%x] frame_dst[%d] dx[%d] dy[%d] apertureSize[%d]",
					__FUNCTION__, frame->data, img_dst, 1, 1, 3);

         cvSobel(frame->data,img_dst,1,1,3);
			break;

   }	

	frame->setImage(img_dst);
   
}

/************************************************************************
* Fun��o que aplica o Filtro de Canny nas imagens.
*************************************************************************
* cvCanny( const CvArr* image, CvArr* edges, double threshold1, 
* 			  double threshold2, int aperture_size=3 );
*************************************************************************
* param (E): Frame* frame: Imagem � qual ser� aplicado o operador.
* param (E): double thresholdMin: menor limiar para compara��o de bordas
* param (E): double thresholdMax: maior limiar para compara��o de bordas
* param (E): int size: abertura da m�scara
*************************************************************************
* Retorno: Frame com a detec��o de bordas realizada.
*************************************************************************
* Autor: Thiago Mizutani
*************************************************************************
* Hist�rico:
* 30/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Filters::Canny(Frame* frame, double thresholdMin, double thresholdMax, int size)
{
	IplImage* img_dst;

	img_dst = Frame::imgAlloc(frame);

	Log::writeLog("%s :: thresholdMin[%lf], thresholdMax[%lf], size[%d]", __FUNCTION__, thresholdMin, thresholdMax, size);

	cvCanny(frame->data, img_dst, thresholdMin, thresholdMax, size);

	frame->setImage(img_dst);
}

/************************************************************************
* Fun��o que aplica o filtro passa-baixa na imagem.
*************************************************************************
* param (E): Frame* frame: Imagem � qual ser� aplicado o operador.
* param (E): int size: Tamanho da matriz.
* 				 3 - 3x3
* 				 5 - 5x5
* 				 7 - 7x7
* 				 n - nxn
************************************************************************
* Retorno: Frame com o efeito aplicado (img_dst)
************************************************************************
* Autor: Thiago Mizutani
************************************************************************
* Hist�rico
* 24/06/08 - Thiago Mizutani
* Altera��o da montagem do kernel do filtro.
* 23/06/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Filters::lowPass(Frame* frame, int size)
{
   IplImage* imgDst = 0;
   IplImage* imgAux = 0;
	IplImage* imgNew = 0;

	Frame* frameAux;

	Log::writeLog("%s :: param: frame[%x] size[%d]", __FUNCTION__, frame, size);

	//Ajuste do tamanho da matriz.
	if (size > 9)
		size = 9;

	int cols_i = size;
	int rows_i = size;
	int total_size = 0;
	CvMat *filter = 0;

	total_size=(int)pow(size,2);

	// M�scara para realizar o processo de convolu��o.
	double convMask[total_size];
	
	// Cria uma imagem com os mesmos par�metros da original.
	frameAux = new Frame(frame);

	imgDst = Frame::imgAlloc(frameAux);
	imgAux = Frame::imgAlloc(frameAux);
	imgNew = Frame::imgAlloc(frameAux);

	// Monta a m�scara com o tamanho que foi passado como par�metro.
	for (int i=0; i<total_size; i++)	
		convMask[i] = (double)1/(double)total_size;

	imgAux->imageData = frameAux->data->imageData;
	imgAux->widthStep = frameAux->data->width;

	imgDst->imageData = imgAux->imageData;
	imgDst->widthStep = imgAux->width;

	filter = cvCreateMatHeader(rows_i, cols_i, CV_64FC1);

	cvSetData(filter, convMask, cols_i*8);

	cvFilter2D(imgAux, imgDst, filter, cvPoint(-1,-1));

	Frame::imgCopy(imgDst, imgNew);

	frame->setImage(imgNew);

	// Desaloca os tempor�rios
	Frame::imgDealloc(imgAux);
	Frame::imgDealloc(imgDst);

	delete frameAux;

}

/************************************************************************
* Fun��o que aplica o filtro passa-alta na imagem.
*************************************************************************
* param (E): Frame* frame: Imagem � qual ser� aplicado o operador.
* param (E): int typeMas: Tipo da m�scara a ser utilizada (0, 1 ou 2).
* 	==========================================================
* 	|		opcao 0			||		opcao 1		||		opcao 2		|
* 	|  	-1 -1 -1 		|| 	0 -1  0 		||		1 -2  1 		|
* 	|  	-1  8 -1 		||   -1  4 -1 		||   -2  4 -2 		|
* 	|  	-1 -1 -1 		|| 	0 -1  0 		||	   1 -2  1 		|	
* 	==========================================================
************************************************************************
* Retorno: Frame com o efeito aplicado (img_dst)
************************************************************************
* Autor: Thiago Mizutani
************************************************************************
* Hist�rico
* 21/08/08 - Thiago Mizutani
* Fun��o n�o retorna mais um ponteiro para Frames. Recebe o frame de 
* retorno como par�metro.
* 29/06/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Filters::highPass(Frame* frame, int typeMask)
{
	IplImage* imgDst = 0;
	IplImage* imgAux = 0;
	IplImage* imgNew = 0;

	Frame* frameAux = 0;

	int cols = 0;
	int rows = 0;
	CvMat *filter = 0;

	// M�scaras de convolu��o
	double masks[][9] = {
		// Tipo 1
		{ 
			-1, -1, -1,
			-1,  8, -1,
			-1, -1, -1,
		},
		// Tipo 2
		{
			0, -1,  0,
			-1, 4, -1,
			0, -1,  0,
		},
		// Tipo 3
		{
			1 , -2,  1,
			-2,  4, -2,
			1 , -2,  1,
		},
	};

	// Cria uma img com as mesmas propriedades da imagem de par�metro
	imgAux = Frame::imgAlloc(frame);
	imgDst = Frame::imgAlloc(frame);
	imgNew = Frame::imgAlloc(frame);

	frameAux = new Frame(frame);

	// Se n�o estiver dentro do range
	// de m�scaras v�lidas, atribui valor default
	if (typeMask > 3 || typeMask < 0)
		typeMask = 0;

	Log::writeLog("%s :: param: frame[%x] typeMask[%d]", __FUNCTION__, frame, typeMask);

	cols = 3;
	rows = 3;

	imgAux->imageData = frameAux->data->imageData;
	imgAux->widthStep = frameAux->data->width;

	imgDst->imageData = imgAux->imageData;
	imgDst->widthStep = imgAux->width;

	Log::writeLog("%s :: cvCreateMatHeader: rows[%d] cols[%d] CV_64FC1", __FUNCTION__, rows, cols);

	filter = cvCreateMatHeader(rows, cols, CV_64FC1);

	Log::writeLog("%s :: cvSetData: filter[%x] masks[%x] cols[%d]", __FUNCTION__, filter, masks[typeMask], cols*8);

	cvSetData(filter, masks[typeMask], cols*8);

	Log::writeLog("%s :: cvFilter2D: imgAux[%x] imgDst[%x] filter[%x] cvPoint[%x]", __FUNCTION__, imgAux, imgDst, filter, cvPoint(-1,-1));

	cvFilter2D(imgAux, imgDst, filter, cvPoint(-1,-1));

	// Fa�o uma c�pia, por seguran�a pois alteramos muito as refer�ncias
	Frame::imgCopy(imgDst, imgNew);

	frame->setImage(imgNew);

	// Desaloco tudo
	Frame::imgDealloc(imgAux);
	Frame::imgDealloc(imgDst);
	delete frameAux;

}

