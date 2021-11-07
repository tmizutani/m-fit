/************************************************************************
* Classe que armazena infomações sobre um único frame.
*************************************************************************
* Histórico:
* 30/06/08 - Thiago Mizutani
* Retirada do método convert2Gray(). Agora este faz parte da classe Color
* 27/06/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
#ifndef FRAME_INCLUDE
#define FRAME_INCLUDE
class Frame 
{
 
	/**
	 *Armazena uma imagem, no caso o 
	 *frame do tipo IplImage
	 *
	 */
	private:

		/**
	 	*Largura do frame
	 	*/
		int *width;

		/**
	 	*Altura do frame
	 	*/
		int *height;

	public:

		IplImage* data;

		// Construtor/Destrutor
		Frame();
		Frame(char *filename_cy);
		Frame(IplImage *img_src);
		Frame(Frame *frame);
		Frame(double *matrix, int len_i, float max_f, bool QT = true);
		~Frame();

		// Funcoes
		QImage* IplImageToQImage( uchar **qImageBuffer, int *last_width, int *last_height, double mini = 0, double maxi = 0);
		Histogram* createHistogram();
		Frame* getDiagonal();
		Frame* verticalCat(Frame* frame);
		Frame* resize(int width, int height);
		int removeWide();
		int removeBorder();
		void binarizeImage(int treshold);	
		void initAttr();
		void write(char *filename_cy);
		double calcLuminanceAvarage();

		// Funcoes estaticas
		static void imgCopy(IplImage *imgSrc, IplImage *imgDst);
		static void imgDealloc(IplImage* img);
		static IplImage* imgAlloc(CvSize size, int depth, int channels); 
		static IplImage* imgAlloc(Frame *frame);

		// Gets/Sets
		int getWidth();
		int getHeight();
		int getPixel(int x, int y);
		int getMaxLum();
		void setPixel(int x, int y, int lum);
		void setImage(IplImage *imgNew);

		// Operadores
		Frame& operator+=(Frame& frame);
		Frame&  operator=(Frame& frame);


};
#endif
