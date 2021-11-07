#include <QThread>
#include <QObject>

#include <vector>
#include "../include/Interface.h"

#include "../include/DetectTransitions.h"

/**
 *Classe responsável por detectar
 *o dissolve.
 */
class Dissolve: public DetectTransitions
{
	Q_OBJECT

	signals:
		void sendMessage(char*,uint,int);

	public:
		void detectTransitions(Video* vdo, std::vector<Transition>*);

	/**
	 *Calcula a primeira derivada de
	 *uma curva.
	 */
		int calcFirstDerivative(double , double, double, double);

	protected:
		void run();

};
