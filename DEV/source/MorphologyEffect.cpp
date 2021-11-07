#include "cv.h"
#include <QImage>

#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Effect.h"
#include "../include/Morphology.h"
#include "../include/MorphologyEffect.h"

/************************************************************************
* Construtor que inicializa os atributos do objeto. 
*************************************************************************
* param (E): Nenhum
*************************************************************************
* Hist�rico
* 08/11/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Erode::Erode()
{
	Erode(0, 0);
}

/************************************************************************
* Construtor que inicializa os atributos do objeto. 
*************************************************************************
* param (E): Nenhum
*************************************************************************
* Hist�rico
* 08/11/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Dilate::Dilate()
{
	Dilate(0, 0);
}

/************************************************************************
* Construtor que inicializa os atributos do objeto, seta os intervalos de
* in�cio e fim de aplica��o do efeito.
*************************************************************************
* param (E): Nenhum
*************************************************************************
* Hist�rico
* 08/11/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Dilate::Dilate(long frameStart, long frameEnd)
{
	this->setName("Dilatacao");
	this->setID(EFFECT_DILATE);
	this->setFrameStart(frameStart);
	this->setFrameEnd(frameEnd);
}

/************************************************************************
* Construtor que inicializa os atributos do objeto, seta os intervalos de
* in�cio e fim de aplica��o do efeito.
*************************************************************************
* param (E): Nenhum
*************************************************************************
* Hist�rico
* 08/11/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Erode::Erode(long frameStart, long frameEnd)
{
	this->setName("Erosao");
	this->setID(EFFECT_ERODE);
	this->setFrameStart(frameStart);
	this->setFrameEnd(frameEnd);
}


/************************************************************************
* Aplica o efeito de eros�o no Frame.
*************************************************************************
* param (E): Frame* - Frame onde ser� aplicado o efeito.
*************************************************************************
* return : Frame* - Frame com o efeito aplicado
*************************************************************************
* Hist�rico
* 08/11/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Frame* Erode::applyEffect(Frame* frame)
{
	Frame *frameResult;
	Morphology *morph = new Morphology();

	frameResult = morph->erode(frame);

	delete morph;

	return frameResult;
}

/************************************************************************
* Aplica o efeito de dilata��o no Frame.
*************************************************************************
* param (E): Frame* - Frame onde ser� aplicado o efeito.
*************************************************************************
* return : Frame* - Frame com o efeito aplicado
*************************************************************************
* Hist�rico
* 08/11/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
Frame* Dilate::applyEffect(Frame* frame)
{
	Frame *frameResult;
	Morphology *morph = new Morphology();

	frameResult = morph->dilate(frame);

	delete morph;

	return frameResult;
}
