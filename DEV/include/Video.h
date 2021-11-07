#ifndef VIDEO_INCLUDE
#define VIDEO_INCLUDE

// Classe respons�vel por fazer a manipula��o de um v�deo.
class Video
{

	/**
	 *Armazena o nome do v�deo
	 */
	private:
		char name[300+1];

		// Regiao de interesse que � aplicada
		// a todo frame capturado deste video

		/**
		 *Armazena o endere�o do v�deo 
		 *carregado no sistema
		 */
		char path[300+1];


		/**
		 *Armazena o tamanho do v�deo.
		 */
		//		Time length;

		/**
		 *Armazena o codec do video.
		 *
		 */
		double codec;

		double framesHeight;

		double framesWidth;

		double framePos;

		double framesTotal;

		/**
		 *Armazena o sistema de cores que
		 *se encontra o v�deo.
		 */
		double systemColor;

		/**
		 *Armazena a taxa de frames
		 *por segundo do v�deo.
		 *
		 */
		double fps;

		/**
		 *N� da lista de tomadas.
		 */
		//	Take take;

		/**
		 *Fun��o que abre o v�deo
		 *e atualiza as vari�veis do objeto.
		 */
		CvCapture* open(char *vdoSrc); 

	public:

		CvRect ROI;

		void resetROI();

		char* getName();
		char* getPath();
		/**
		 *Armazena o v�deo carregado.
		 */
		CvCapture* data;

		Video(char *filename_cy);
		~Video();

		/**
		 *M�todo respons�vel por obter
		 *o pr�ximo frame em rela��o �
		 *posi��o atual do v�deo.
		 */
		Frame* getNextFrame();

		/**
		 *M�todo respons�vel por obter
		 *o frame anterior em rela��o �
		 *posi��o atual do v�deo.
		 */
		Frame* getPreviousFrame();

		/**
		 * M�todo respons�vel em posicionar o ponteiro do v�deo na posi��o desejada,
		 * a qual � parecida como par�metro da fun��o.
		 **/
		int seekFrame(unsigned long posFrame);

		/**
		 *Obt�m o frame atual.
		 */
		Frame* getCurrentFrame();

		double getFramesTotal();

		// Obt�m a posi��o corrente do ponteiro do v�deo.
		double getCurrentPosition();

		double getFramesWidth();
		double getFramesHeight();
		double getCodec();
		void updatePos();
		//	double getFramePos();
		
		double getFPS();
		void removeWide();
		void removeBorder();
		
};
#endif
