#include <QThread>
#include <QObject>

#include <vector>
#include "../include/Interface.h"

#include "../include/DetectTransitions.h"

#ifndef CUT_INCLUDE
#define CUT_INCLUDE
/**
 *Classe respons�vel por indentificar
 *transi��es do tipo Corte.
 */
class Cut: public DetectTransitions
{

	Q_OBJECT

	signals:
		void sendMessage(char*,uint,int);
 
	private:
		int threshold;
	 
	/**
	 *Cria o mapa de bordas.
	 */
	public:

		Cut(); // Construtor

		// Passa o filtro de Canny sobre o Ritmo Visual
		void createBorderMap(Frame* visualRythim);

		void detectTransitions(Video* vdo, std::vector<Transition>* tansitionList);

		int getThreshold();

		int setThreshold(int threshold);

		// Processo para a defini��o do limiar de corte para o processo de detec��o de cortes
		int defineThreshold(int height);

		// Faz contagem de pontos do mapa de bordas para defini��o daquilo que � ou n�o um corte.
		int* countPoints(Frame* borderMap, int threshold);

		// Valida se aquilo que foi detectado � realmente um corte.
		int validateCut(Frame* visual, int position);

	protected:
		void run();


};
#endif
