#include "cv.h"
#include <QImage>
#include "highgui.h"

#include "../include/Time.h"

#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"
#include "../include/VisualRythim.h"

#include "../include/Effect.h"
#include "../include/Color.h"

#include "../include/Log.h"

#include <stdio.h>

/************************************************************************
* Fun��o que cria o Ritmo Visual por amostragem de um v�deo.
*************************************************************************
* param (E): Video* vdo => v�deo do qual ser� montado o ritmo visual.
*************************************************************************
* return : Ritmo Visual do v�deo.
*************************************************************************
* Hist�rico:
* 29/07/08 - Thiago Mizutani
* Retirando coisas obsoletas.
* 06/07/08 - Thiago Mizutani
* Implementa��o do processo que monta as colunas do ritmo visual atraves
* da extracao dos pixels da diagonal de um frame.
* 04/07/08 - Thiago Mizutani
* Cria��o.
************************************************************************/
Frame* VisualRythim::createVR(Video* vdo)
{

	Frame *frameVR = 0;
	Frame* frame = 0;
	Frame* frameGray = 0;
	Frame *frameAux = 0;

	Color *color = 0;

	//int lum;

	double width;
	double heigth;

	Log::writeLog("%s :: param: Video[%x]", __FUNCTION__, vdo);

	color = new Color();

	// Pego o primeiro frame do v�deo e sua diagonal	
	width =  vdo->getFramesWidth();
	heigth = vdo->getFramesHeight();
	frame =  vdo->getNextFrame();
	frameVR = new Frame(frame->getDiagonal());

	while(true)
	{
		frame = vdo->getNextFrame();

		if (frame == NULL)
			break;

//		Log::writeLog("%s :: Convert2Gray Frame[%x] Frame->data[%x]", __FUNCTION__, frame, frame->data);

		// Converto o frame para escala de cinza.
		frameGray = color->convert2Gray(frame);

		// Pego a diagonal (pixel por pixel) e ploto este pixel na coluna f do RV.
		frameAux = frameGray->getDiagonal();

		/** Utilizando sobrecarga de operador para concatenar uma nova coluna ao frame
		 * do Ritmo Visual.
		**/

		*frameVR += *frameAux;

		// Desaloca os temporarios
		delete frameAux;
		delete frame;
		delete frameGray;

	}

	Log::writeLog("%s :: max_frames[%.f]", __FUNCTION__, vdo->getFramesTotal());
	return (frameVR);
}

/************************************************************************
* Fun��o que cria o Ritmo Visual por histograma de um v�deo.
*************************************************************************
* param (E): Video* vdo => v�deo do qual ser� montado o ritmo visual.
*************************************************************************
* return : Ritmo visual por histograma (um array).
*************************************************************************
* Hist�rico:
* 18/08/08 - Ivan Shiguenori Machida
* Convertendo o frame para tons de cinza antes de obter a diagonal
* 29/07/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

double* VisualRythim::createVRH(Video* vdo)
{
	Frame* frame = new Frame(); 
	Histogram* histogram;
	Color *color = new Color();
	Frame* frameGray = 0;
	Frame *frameDiagonal = 0;

	double totalFrames = vdo->getFramesTotal();

	int posic = 0;

	/**
	 * Crio um array com o numero de posicoes = ao nro de frames do video
	 * para guardar o valor m�ximo de cada histograma que ser� gerado.
	 * Obs.: Ser� gerado 1 histograma por frame.
	**/
	double* hist; // Este � o RVH.

	hist = (double*)malloc(sizeof(double)*cvRound(totalFrames));

	memset(hist, '\0',     sizeof(double)*cvRound(totalFrames));

	// Pego o primeiro frame
	frame = vdo->getNextFrame();	

	while(frame != NULL)
	{

		if (posic >= totalFrames)
		{
			Log::writeLog("%s :: BOOOOOOM!", __FUNCTION__);
		}

		// Converto o frame para escala de cinza.
		frameGray = color->convert2Gray(frame);

		// Pega a diagonal
		frameDiagonal = frameGray->getDiagonal();

		// Pega o histograma da diagonal
		histogram = frameDiagonal->createHistogram();

		// Pega o pico deste histograma.
		//hist[posic] = histogram->getMaxLuminance();

		// Guardo a media do valor de luminancia da diagonal.
		hist[posic] = frameGray->calcLuminanceAvarage();
	
//		Log::writeLog("%s :: hist[%d] = %lf", __FUNCTION__, posic, hist[posic]);

		/**
		 * Deleto os objetos criados anteriormente para desalocamento de
		 * memoria.
		**/
		delete frameDiagonal;
		delete frameGray;
		delete histogram;
		delete frame;
		
		// Pego o proximo
		frame = vdo->getNextFrame();

		posic++;

	}

	delete color;
	// Retorno o array com os valores do RVH

	return (hist);
}
