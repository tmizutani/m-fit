#ifndef TRANSITION_INCLUDE
#define TRANSITION_INCLUDE
/**
 *Classe respons�vel por representar
 *uma transi��o.
 */

#define TRANSITION_VIDEOSTART 0
#define TRANSITION_CUT        1
#define TRANSITION_FADE			2
#define TRANSITION_FADEIN     3
#define TRANSITION_FADEOUT    4
#define TRANSITION_DISSOLVE   5
#define TRANSITION_ALL			-1

class Transition
{

	private:

		/**
		 *Guarda o tipo da transi��o
		 *0 - Corte
		 *1 - Fade-In
		 *2 - Fade-Out
		 *3 - Dissolve
		 */
		int type;

		/**
		 *Posi��o do frame (em hh:mm:ss)que 
		 *marca exatamente o ponto da 
		 *transi��o no v�deo.
		 */
		long posTransition;

		/**
		 *Posi��o do frame (em hh:mm:ss)que 
		 *marca exatamente o ponto da 
		 *transi��o no v�deo, que foi
		 *redefinido pelo editor.
		 */
		long posUserTransition;

		/**
		 *Identificador da transi��o na 
		 *timeline do v�deo.
		 */
		char label[256];

	public:

		bool selected;

		// Construtores/Destrutores
		Transition();
		Transition(int type, long posTransition, char *label);

		// Get/Set
		void setType(int type);
		int getType();
		void setPosTransition(int posTransition);
		long getPosTransition();
		void setPosUserTransition(int posUser);
		long getPosUserTransition();
		void setLabel(char *label);
		char* getLabel();
		long getPosCurrent(void) const;

		// Operadores
		bool operator <(const Transition& Rhs) const;

};
#endif
