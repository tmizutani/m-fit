#include "cv.h"
#include <QImage>

#include "../include/Histogram.h"
#include "../include/Frame.h"

#include "../include/Morphology.h"

#include "../include/Log.h"

/************************************************************************
* Fun��o que aplica o efeito de Dilata��o no Frame
*************************************************************************
* param (E): Frame* frame - Frame onde o efeito vai ser aplicado
************************************************************************
* return   : Frame* - Ponteiro para um novo frame com o efeito aplicado
************************************************************************
* Hist�rico:
* 26/06/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Frame* Morphology::dilate(Frame* frame)
{
	IplImage *imgDst;
	Frame *frameNew;

	imgDst = Frame::imgAlloc(cvGetSize(frame->data),frame->data->depth,frame->data->nChannels);

	Log::writeLog("%s :: cvDilate frame[%x] imgDst[%x] IplConvKernel[%d] iterations[%d]", __FUNCTION__, frame, imgDst, 0, 1);

	cvDilate(frame->data, imgDst, 0, 1);

	frameNew = new Frame(imgDst);

	Frame::imgDealloc(imgDst);

	return (frameNew);
}

/************************************************************************
* Fun��o que aplica o efeito de Eros�o no Frame
*************************************************************************
* param (E): Frame* frame - Frame onde o efeito vai ser aplicado
************************************************************************
* return   : Frame* - Ponteiro para um novo frame com o efeito aplicado
************************************************************************
* Hist�rico:
* 26/06/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Frame* Morphology::erode(Frame* frame)
{
	IplImage *imgDst;
	Frame *frameNew;

	imgDst = Frame::imgAlloc(cvGetSize(frame->data),frame->data->depth,frame->data->nChannels);

	Log::writeLog("%s :: cvErode frame[%x] imgDst[%x] IplConvKernel[%d] iterations[%d]", __FUNCTION__, frame, imgDst, 0, 1);

	cvErode(frame->data, imgDst, 0, 1);

	frameNew = new Frame(imgDst);

	Frame::imgDealloc(imgDst);

	return (frameNew);
}
