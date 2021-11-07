#include <QThread>
#include <QObject>

#include <vector>
#include "../include/Interface.h"

#include "../include/DetectTransitions.h"

#ifndef FADE_INCLUDE
#define FADE_INCLUDE
/**
 *Classe responsável por 
 *identificar todos os tipos de
 *fade.
 */
class Fade: public DetectTransitions
{
	Q_OBJECT

	signals:
		void sendMessage(char*,uint,int);

	public:
		double* calcDerivative(double *array, int size_i);
		void detectTransitions(Video* vdo, std::vector<Transition>* transitionList);

	protected:
		void run();

};
#endif
