#ifndef COLOR_INCLUDE
#define COLOR_INCLUDE
/************************************************************************
* Classe responsável em aplicar efeitos de cor.
*************************************************************************
* Histórico:
* 30/06/08 - Thiago Mizutani
* Inclusão do método convert2Gray(). Foi transferido da classe Frame para
* esta.
* 27/06/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/

class Color: public Effect
{
	public:

		int R;
		int G;
		int B;

		// Converte um frame de RGB para escala de cinza.
		Frame* convert2Gray(Frame *frame);
		Frame* applyEffect(Frame* frame);

		Color(long frameStart, long frameEnd);
		Color();
 
};
#endif
