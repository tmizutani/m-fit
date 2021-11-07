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
* Histórico
* 08/11/08 - Fabricio Lopes de Souza
* Criação.
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
* Histórico
* 08/11/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
Dilate::Dilate()
{
	Dilate(0, 0);
}

/************************************************************************
* Construtor que inicializa os atributos do objeto, seta os intervalos de
* início e fim de aplicação do efeito.
*************************************************************************
* param (E): Nenhum
*************************************************************************
* Histórico
* 08/11/08 - Fabricio Lopes de Souza
* Criação.
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
* início e fim de aplicação do efeito.
*************************************************************************
* param (E): Nenhum
*************************************************************************
* Histórico
* 08/11/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
Erode::Erode(long frameStart, long frameEnd)
{
	this->setName("Erosao");
	this->setID(EFFECT_ERODE);
	this->setFrameStart(frameStart);
	this->setFrameEnd(frameEnd);
}


/************************************************************************
* Aplica o efeito de erosão no Frame.
*************************************************************************
* param (E): Frame* - Frame onde será aplicado o efeito.
*************************************************************************
* return : Frame* - Frame com o efeito aplicado
*************************************************************************
* Histórico
* 08/11/08 - Fabricio Lopes de Souza
* Criação.
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
* Aplica o efeito de dilatação no Frame.
*************************************************************************
* param (E): Frame* - Frame onde será aplicado o efeito.
*************************************************************************
* return : Frame* - Frame com o efeito aplicado
*************************************************************************
* Histórico
* 08/11/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
Frame* Dilate::applyEffect(Frame* frame)
{
	Frame *frameResult;
	Morphology *morph = new Morphology();

	frameResult = morph->dilate(frame);

	delete morph;

	return frameResult;
}
