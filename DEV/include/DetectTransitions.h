#include <QThread>

#include <QObject>

#include <vector>
#include "../include/Interface.h"


#ifndef DETECTTRANSITIONS_INCLUDE
#define DETECTTRANSITIONS_INCLUDE
/**
 *Classe pai de todas as detec��es
 *de transi��es.
 */
class DetectTransitions: public QThread
{
 
	Q_OBJECT
	/**
	 *Fun��o principal na detec��o
	 *de transi��es. Desencadeia
	 *a chamada da fun��o com este
	 *mesmo nome nos objetos
	 *filhos.
	 */
	signals:
		void sendMessage(char*,uint,int);

	public:
		void detectTransitions(Video* vdo, std::vector<Transition>* tansitionList);

		// Verifica se j� existem transi��es detectadas na posi��o da nova transi��o.
		bool validateTransition(long position, std::vector<Transition>* transitionList);

		void run();

};
#endif
