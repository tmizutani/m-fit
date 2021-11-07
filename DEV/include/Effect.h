#ifndef EFFECT_INCLUDE
#define EFFECT_INCLUDE
/**
 *Classe respons�vel por centralizar
 *os m�todos dos efeitos.
 */
class Effect
{
	/**
	 *Fun��o gen�rica para aplica��o
	 *de efeito. Ser� implementada
	 *por todas as classes filhas
	 *deste objeto.
	 */
	private:
		char name_cy[50];
		int ID;
		long frameStart;
		long frameEnd;

	public:


		virtual Frame* applyEffect(Frame *frame) = 0;

		void setFrameStart(long pos);
		long getFrameStart();

		void setFrameEnd(long pos);
		long getFrameEnd();

		void setName(char *name);
		char *getName();

		void setID(int ID);
		int getID();

		// Define de TODOS os EFEITOS
		#define EFFECT_COLOR  0
		#define EFFECT_ERODE  1
		#define EFFECT_DILATE 2
};
#endif
