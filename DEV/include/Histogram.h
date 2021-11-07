#ifndef HISTOGRAM_INCLUDE
#define HISTOGRAM_INCLUDE
/**
 *Classe responsável por conter
 *os dados do histograma de um frame.
 */
class Histogram 
{

	private:


	 
	/**
	 *Guarda o maior valor
	 *de luminância do histograma
	 */
		float max; // Nro de pixels de valor maximo
		int maxLum; // Valor da luminancia mais presente no histograma
	 
	/**
	 *Armazena o menor valor
	 *de luminância do histograma
	 */
		float min; // Nro de pixels de menor valor
		int minLum; // Valor da luminancia menos presente no histograma

	/**
	 *Armazena os dados do histograma
	 *da imagem.
	 */

	public:
		
		double data[256];
		Histogram();
		Histogram(IplImage* img);
		double* getMatrix();
		float getMax();
		float getMin();
		int getMaxLuminance();
		int getMinLuminance();

};
#endif
