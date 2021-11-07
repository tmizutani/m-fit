#include "cv.h"
#include "highgui.h"
#include <vector>
#include <QImage>

#include "../include/Histogram.h"
#include "../include/Time.h"
#include "../include/Frame.h"

#include "../include/Video.h"
#include "../include/Transition.h"
#include "../include/DetectTransitions.h"

#include "../include/Project.h"

#include "../include/Cut.h"
#include "../include/Fade.h"
#include "../include/Dissolve.h"

#include "../include/Log.h"

extern Project *currentProject;

/************************************************************************
* Fun��o que faz a detec��o das transi��es.
*************************************************************************
* param (E): Video* vdo => video em qual ser� feita a detec��o
* param (E): Transition* transitions => lista de transi��es
*************************************************************************
* return : Lista de transi��es.
*************************************************************************
* Hist�rico:
* 11/12/08 - Fabricio Lopes de Souza
* Adaptando o controle para funcionar com threads.
* 20/10/08 - Thiago Mizutani
* Incluindo o Dissolve. Controle da posi��o do frame.
* 13/08/08 - Thiago Mizutani
* Implementa��o dos procedimentos de detec��o das transi��es
* 04/07/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void DetectTransitions::detectTransitions(Video* vdo, std::vector<Transition>* transitionList)

{
	Cut* DTC = new Cut();
	Fade* DTF = new Fade();
	Dissolve* DTD = new Dissolve();
	long posFrame = 0x0;	

	posFrame = (long)vdo->getCurrentPosition();

	emit sendMessage("Inicio", 0, 0);

	vdo->seekFrame(0);

	emit sendMessage("Detectando Corte", transitionList->size(), 0);

	DTC->start(); // Cortes

	while (DTC->isRunning()) {};

	vdo->seekFrame(0);

	emit sendMessage("Detectando Fade", transitionList->size(), 33);

	DTF->start(); // Fade

	while (DTF->isRunning()) {};

	vdo->seekFrame(0);

	emit sendMessage("Detectando Dissolve", transitionList->size(), 66);

	DTD->start(); // Dissolve

	while (DTD->isRunning()) {};

	vdo->seekFrame(posFrame);

	emit sendMessage("Fim", transitionList->size(), 100);

	delete DTC;
	delete DTF;
	delete DTD;
}

/************************************************************************
 * Fun��o que faz valida��o da transi��o detectada com a lista de 
 * transi��es, evitando que 2 transi��es sejam apontadas para a mesma 
 * posi��o do v�deo.
 *************************************************************************
 * param (E): int position => Posi��o da nova transi��o a ser inserida
 * param (E): transitionList => lista de transi��es.
 *************************************************************************
 * return : TRUE => Transi��o v�lida
 * 			  FALSE => H� sobreposi��o de transi��es. 
 *************************************************************************
 * Hist�rico:
 * 20/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/
bool DetectTransitions::validateTransition(long position, std::vector<Transition>* transitionList)
{
	unsigned long i;
	Transition* currentTransition;

	Log::writeLog("%s :: position = %ld", __FUNCTION__, position);

	for(i=0; i<transitionList->size(); i++)
	{
		currentTransition = &transitionList->at(i);
		Log::writeLog("%s :: currentTransition = %ld", __FUNCTION__, currentTransition->getPosTransition());

		if (position == currentTransition->getPosTransition())
		{
			Log::writeLog("%s :: Transi��o inv�lida!!! J� existe uma transi��o na posi��o[%d].", __FUNCTION__, position);
			return FALSE;
		}
	}

	Log::writeLog("%s :: Transi��o v�lida!!! Posi��o[%d].", __FUNCTION__, position);
	// Lista est� vazia ou a posi��o � v�lida.
	return TRUE;

}

/************************************************************************
 * M�todo de in�cio da Thread. Inicia a detec��o de todas as transi��es
 *************************************************************************
 * param (E): Nenhum.
 *************************************************************************
 * return : Nenhum.
 *************************************************************************
 * Hist�rico:
 * 10/12/08 - Fabr�cio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void DetectTransitions::run()
{
	detectTransitions(currentProject->getVideo(), &currentProject->transitionList);
}
