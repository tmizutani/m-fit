#ifndef VISUALRYTHIM
#define VISUALRYTHIM
/**
 *Classe responsável por criar
 *os dois tipos de ritmo visual.
 */
class VisualRythim 
{

	public:

		/**
		 *Cria o Ritmo Visual por sub-amostragem.
		 */
		Frame *createVR(Video* vdo);

		/**
		 *Retorna o Ritmo Visual por 
		 *Histograma  do vídeo
		 */
		double* createVRH(Video* vdo); 

		/**
		 *Calcula primeira derivada da curva do
		 *gráfico do ritmo visual.
		 */
		float calcDerivatives();
};
#endif
