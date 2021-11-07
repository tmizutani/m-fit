/**
 *Classe que agrupa filtros de
 *processamento de imagens digitais.
 */
#ifndef FILTERS_INCLUDE
#define FILTERS_INCLUDE

#define SOBEL_VERTICAL	  0
#define SOBEL_HORIZONTAL  1
#define SOBEL_COMPLETE	  2

class Filters 
{

	public:

		void Sobel(Frame* frame, int direction);
		void lowPass(Frame* frame, int size);
		void highPass(Frame* frame, int typeMask);
		void segment(Frame* frame, int threshold);
		void Canny(Frame* frame, double thresholdMin, double thresholdMax, int size);

};
#endif
