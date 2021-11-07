#include <QThread>
#include <QObject>

#include <vector>
#include "../include/Interface.h"

#include "../include/DetectTransitions.h"

#ifndef CUT_INCLUDE
#define CUT_INCLUDE
/**
 *Classe responsável por indentificar
 *transições do tipo Corte.
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

		// Processo para a definição do limiar de corte para o processo de detecção de cortes
		int defineThreshold(int height);

		// Faz contagem de pontos do mapa de bordas para definição daquilo que é ou não um corte.
		int* countPoints(Frame* borderMap, int threshold);

		// Valida se aquilo que foi detectado é realmente um corte.
		int validateCut(Frame* visual, int position);

	protected:
		void run();


};
#endif
