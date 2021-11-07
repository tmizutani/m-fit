#ifndef VISUALRYTHIM
#define VISUALRYTHIM
/**
 *Classe respons�vel por criar
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
		 *Histograma  do v�deo
		 */
		double* createVRH(Video* vdo); 

		/**
		 *Calcula primeira derivada da curva do
		 *gr�fico do ritmo visual.
		 */
		float calcDerivatives();
};
#endif
