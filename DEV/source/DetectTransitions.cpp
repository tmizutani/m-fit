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
* Função que faz a detecção das transições.
*************************************************************************
* param (E): Video* vdo => video em qual será feita a detecção
* param (E): Transition* transitions => lista de transições
*************************************************************************
* return : Lista de transições.
*************************************************************************
* Histórico:
* 11/12/08 - Fabricio Lopes de Souza
* Adaptando o controle para funcionar com threads.
* 20/10/08 - Thiago Mizutani
* Incluindo o Dissolve. Controle da posição do frame.
* 13/08/08 - Thiago Mizutani
* Implementação dos procedimentos de detecção das transições
* 04/07/08 - Thiago Mizutani
* Criação.
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
 * Função que faz validação da transição detectada com a lista de 
 * transições, evitando que 2 transições sejam apontadas para a mesma 
 * posição do vídeo.
 *************************************************************************
 * param (E): int position => Posição da nova transição a ser inserida
 * param (E): transitionList => lista de transições.
 *************************************************************************
 * return : TRUE => Transição válida
 * 			  FALSE => Há sobreposição de transições. 
 *************************************************************************
 * Histórico:
 * 20/10/08 - Thiago Mizutani
 * Criação.
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
			Log::writeLog("%s :: Transição inválida!!! Já existe uma transição na posição[%d].", __FUNCTION__, position);
			return FALSE;
		}
	}

	Log::writeLog("%s :: Transição válida!!! Posição[%d].", __FUNCTION__, position);
	// Lista está vazia ou a posição é válida.
	return TRUE;

}

/************************************************************************
 * Método de início da Thread. Inicia a detecção de todas as transições
 *************************************************************************
 * param (E): Nenhum.
 *************************************************************************
 * return : Nenhum.
 *************************************************************************
 * Histórico:
 * 10/12/08 - Fabrício Lopes de Souza
 * Criação.
 ************************************************************************/
void DetectTransitions::run()
{
	detectTransitions(currentProject->getVideo(), &currentProject->transitionList);
}
