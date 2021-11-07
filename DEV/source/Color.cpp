#include "cv.h"
#include <QImage>

#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Effect.h"
#include "../include/Color.h"

/************************************************************************
* Fun��o que converte o frame para escala de cinza.
*************************************************************************
* param (E): Nenhum.
*************************************************************************
* return : Frame* - Retorna um ponteiro para um novo frame.
*************************************************************************
* Hist�rico:
* 27/06/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Frame* Color::convert2Gray(Frame* frame)
{
	IplImage *imgGray;
	Frame *frameNew;

	// Se o frame jah for cinza, vaza
	if (frame->data->nChannels == 1)
		return new Frame(frame);

	// Cria a img com as mesmas dimens�es da inst�ncia atual
	// por�m com 8 bits e 1 canal apenas
	imgGray = Frame::imgAlloc(cvGetSize(frame->data),8,1);

	// Converte pra cinza;
	cvCvtColor(frame->data, imgGray, CV_RGB2GRAY);

	frameNew = new Frame(imgGray);

	Frame::imgDealloc(imgGray);

	return (frameNew);
}

/************************************************************************
* Aplica o efeito de convers�o para escalas de cinza.
*************************************************************************
* param (E): Frame* frame => frame a ter esquema de cores alterado.
*************************************************************************
* return : frame em tons de cinza.
*************************************************************************
* Hist�rico:
* 27/06/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/

Frame* Color::applyEffect(Frame* frame)
{
	return convert2Gray(frame);
}

/************************************************************************
* Construtor que armazena o frame inicial e o frame final de um efeito 
* aplicado no v�deo.
*************************************************************************
* param (E): long frameStart => frame inicial do efeito.
* param (E): long frameEnd => frame final do efeito.
*************************************************************************
* return : Nenhum.
*************************************************************************
* Hist�rico:
* 27/06/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/

Color::Color(long frameStart, long frameEnd)
{
	this->setFrameStart(frameStart);
	this->setFrameEnd(frameEnd);
	this->setName("Conversao para Cinza");
	this->setID(EFFECT_COLOR);
}

/************************************************************************
* Construtor da classe.
*************************************************************************
* param (E): Nenhum.
*************************************************************************
* return : Nenhum.
*************************************************************************
* Hist�rico:
* 27/06/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/

Color::Color()
{
	Color(0, 0);
}
