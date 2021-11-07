#ifndef VIDEO_INCLUDE
#define VIDEO_INCLUDE

// Classe responsável por fazer a manipulação de um vídeo.
class Video
{

	/**
	 *Armazena o nome do vídeo
	 */
	private:
		char name[300+1];

		// Regiao de interesse que é aplicada
		// a todo frame capturado deste video

		/**
		 *Armazena o endereço do vídeo 
		 *carregado no sistema
		 */
		char path[300+1];


		/**
		 *Armazena o tamanho do vídeo.
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
		 *se encontra o vídeo.
		 */
		double systemColor;

		/**
		 *Armazena a taxa de frames
		 *por segundo do vídeo.
		 *
		 */
		double fps;

		/**
		 *Nó da lista de tomadas.
		 */
		//	Take take;

		/**
		 *Função que abre o vídeo
		 *e atualiza as variáveis do objeto.
		 */
		CvCapture* open(char *vdoSrc); 

	public:

		CvRect ROI;

		void resetROI();

		char* getName();
		char* getPath();
		/**
		 *Armazena o vídeo carregado.
		 */
		CvCapture* data;

		Video(char *filename_cy);
		~Video();

		/**
		 *Método responsável por obter
		 *o próximo frame em relação à
		 *posição atual do vídeo.
		 */
		Frame* getNextFrame();

		/**
		 *Método responsável por obter
		 *o frame anterior em relação à
		 *posição atual do vídeo.
		 */
		Frame* getPreviousFrame();

		/**
		 * Método responsável em posicionar o ponteiro do vídeo na posição desejada,
		 * a qual é parecida como parâmetro da função.
		 **/
		int seekFrame(unsigned long posFrame);

		/**
		 *Obtém o frame atual.
		 */
		Frame* getCurrentFrame();

		double getFramesTotal();

		// Obtêm a posição corrente do ponteiro do vídeo.
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
