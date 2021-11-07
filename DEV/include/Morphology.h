#ifndef MORPHOLOGY_INCLUDE
#define MORPHOLOGY_INCLUDE
/**
 *Classe que agrupa filtros de 
 *morfologia matem�tica.
 */
class Morphology
{
 
	public:
	 
		Frame* erode(Frame* frame);
		Frame* dilate(Frame* frame);
	 
};
#endif
