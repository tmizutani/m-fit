#include <QThread>

#include <QObject>

#include <vector>
#include "../include/Interface.h"


#ifndef DETECTTRANSITIONS_INCLUDE
#define DETECTTRANSITIONS_INCLUDE
/**
 *Classe pai de todas as detecções
 *de transições.
 */
class DetectTransitions: public QThread
{
 
	Q_OBJECT
	/**
	 *Função principal na detecção
	 *de transições. Desencadeia
	 *a chamada da função com este
	 *mesmo nome nos objetos
	 *filhos.
	 */
	signals:
		void sendMessage(char*,uint,int);

	public:
		void detectTransitions(Video* vdo, std::vector<Transition>* tansitionList);

		// Verifica se já existem transições detectadas na posição da nova transição.
		bool validateTransition(long position, std::vector<Transition>* transitionList);

		void run();

};
#endif
