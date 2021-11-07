#include "cv.h"
#include "highgui.h"
#include <stdexcept>
#include <vector>
#include <QImage>

#include "../include/Time.h"
#include "../include/Log.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"

#include "../include/Interface.h"

#include "../include/Video.h"
#include "../include/Transition.h"
#include "../include/DetectTransitions.h"
#include "../include/VisualRythim.h"

#include "../include/Dissolve.h"
#include "../include/Effect.h"
#include "../include/Color.h"
#include "../include/Filters.h"

#include "../include/Project.h"

extern Project *currentProject;

#define RIGHT	1
#define LEFT	2

/************************************************************************
* Função que detecta as transições do tipo Dissolve
*************************************************************************
* param (E): Video* vdo -> video à detectar as transições
* param (S): Transition* transitions -> Posicão corrente da lista de transições
*************************************************************************
* return : Nenhum
*************************************************************************
* Histórico:
* 16/10/08 - Mauricio Hirota / Ivan Shiguenori Machida
* Criação.
* 08/11/08 - Ivan Shiguenori Machida
* Utilizacao da funcao calcFirstDerivative
************************************************************************/
void Dissolve::detectTransitions(Video* vdo, std::vector<Transition>* transitionList)
{
	double *array_vrh;
	int k, i;
	double videoSize;

	double *detect, *orientation, thresholdMax=0, thresholdMin=0, threshold_fade = 0;

	VisualRythim* vrh = new VisualRythim();

	videoSize = cvRound(vdo->getFramesTotal());

	detect = (double *) malloc(sizeof(double)*cvRound(videoSize));
	orientation = (double *) malloc(sizeof(double)*cvRound(videoSize));
	array_vrh = (double *) malloc(sizeof(double)*cvRound(videoSize));

	memset(detect, '\0',     sizeof(double)*cvRound(videoSize));
	memset(orientation, '\0',     sizeof(double)*cvRound(videoSize));
	memset(array_vrh, '\0',     sizeof(double)*cvRound(videoSize));

	emit sendMessage("Iniciando Detecção", 0, 0);

	//Coleta o ritmo visual dos frames
	emit sendMessage("Criando VRH", transitionList->size(), 10);
	array_vrh = vrh->createVRH(vdo);

	thresholdMin = 0.25;
	thresholdMax = 4.0;

	//Verifica ponto de dissolve
	for(k=0;k<videoSize;k++)
	{
		if((array_vrh[k]<array_vrh[k+1]) &&
		   (array_vrh[k+1]<array_vrh[k+2]) &&
		   (array_vrh[k+2]<array_vrh[k+3]) &&
		   (array_vrh[k+3]<array_vrh[k+4]) &&
		   (array_vrh[k+4]<array_vrh[k+5]) &&
		   (array_vrh[k+5]<array_vrh[k+6]) &&
		   (array_vrh[k+6]<array_vrh[k+7]))
		{
			//aplica 1. derivada
			if((calcFirstDerivative(array_vrh[k],array_vrh[k+1],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+1],array_vrh[k+2],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+2],array_vrh[k+3],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+3],array_vrh[k+4],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+4],array_vrh[k+5],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+5],array_vrh[k+6],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+6],array_vrh[k+7],thresholdMax,thresholdMin)))
			{
				orientation[k]=RIGHT;
				detect[k]=1;
				k=k+6;
			}
		}

		if((array_vrh[k]>array_vrh[k+1]) &&
		   (array_vrh[k+1]>array_vrh[k+2]) &&
		   (array_vrh[k+2]>array_vrh[k+3]) &&
		   (array_vrh[k+3]>array_vrh[k+4]) &&
		   (array_vrh[k+4]>array_vrh[k+5]) &&
		   (array_vrh[k+5]>array_vrh[k+6]) &&
		   (array_vrh[k+6]>array_vrh[k+7]))
		{
			//aplica 1. derivada
			if((calcFirstDerivative(array_vrh[k+1],array_vrh[k],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+2],array_vrh[k+1],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+3],array_vrh[k+2],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+4],array_vrh[k+3],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+5],array_vrh[k+4],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+6],array_vrh[k+5],thresholdMax,thresholdMin)) &&
			  (calcFirstDerivative(array_vrh[k+7],array_vrh[k+6],thresholdMax,thresholdMin)))
			{
				orientation[k]=LEFT;
				detect[k]=1;
				k=k+6;
			}
		}
	
		emit sendMessage("Analizando Derivadas", transitionList->size(), cvRound(10 + ((((float)k / (float)videoSize ) * 100) * 0.4)));
	}

	//Retirando redundancias
	emit sendMessage("Retirando Redundâncias", transitionList->size(), 50);

	for(k=0;k<videoSize;k++)
	{
		if(detect[k])
		{
			while(detect[k+7]==1)
			{
				detect[k+7]=0;
				k=k+7;
			}
		}
	}

	//Verifica se é um fade
	emit sendMessage("Verificando Fades", transitionList->size(), 60);

	i=0;
	threshold_fade = 6.0;

	for(k=0;k<videoSize;k++)
	{
		if(detect[k]==1)
		{
			i=k;

			if(orientation[k]==RIGHT)
			{
				while(((array_vrh[i]<=array_vrh[i+1]) || (array_vrh[i]<=array_vrh[i+2])) && (i<videoSize))
				{
//					Log::writeLog("RIGHT %d - %lf", k, array_vrh[i]);
					if(array_vrh[i]<threshold_fade)
					{
						detect[k]=0;
						break;
					}
					i++;
				}
			}
			else if(orientation[k]==LEFT)
			{
				while(((array_vrh[i]>=array_vrh[i+1]) || (array_vrh[i]>=array_vrh[i+2])) && (i<videoSize))
				{
//					Log::writeLog("LEFT %d - %lf", k, array_vrh[i]);
					if(array_vrh[i]<threshold_fade)
					{
						detect[k]=0;
						break;
					}
					i++;
				}
			}
		}
	}

	//Transportando para matriz de detecção
	emit sendMessage("Validando Dissolve", transitionList->size(), 80);

	for(k=0;k<videoSize;k++)
	{
		if(detect[k]==1)
		{
			Transition *transition = new Transition();

			transition = new Transition(TRANSITION_DISSOLVE, k, "Dissolve");

			if(this->validateTransition(k, transitionList))
				transitionList->push_back(*transition);
		}
	}

	emit sendMessage("Fim do Dissolve", transitionList->size(), 100);
}

/************************************************************************
* Função que calcula a segunda derivada através do histograma do ritmo 
* visual
*************************************************************************
* param (E): vetor de entrada
*************************************************************************
* return : float -> valor da segunda derivada do histograma do ritmo 
* visual
*************************************************************************
* Histórico:
* 18/08/08 - Ivan Shiguenori Machida
* Criação.
************************************************************************/
int Dissolve::calcFirstDerivative(double firstFrame, double secondFrame, double thresholdMax, double thresholdMin)
{
	if(((secondFrame-firstFrame) < thresholdMax) && ((secondFrame-firstFrame) > thresholdMin))
		return(1);
	else
		return(0);
}

/*************************************************************************
 * Método de início da Thread. Inicia a detecção das transições
 *************************************************************************
 * param (E): Nenhum.
 *************************************************************************
 * return : Nenhum.
 *************************************************************************
 * Histórico:
 * 10/12/08 - Fabrício Lopes de Souza
 * Criação.
 ************************************************************************/
void Dissolve::run()
{
	detectTransitions(currentProject->getVideo(), &currentProject->transitionList);
}
