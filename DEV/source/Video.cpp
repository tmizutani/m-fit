#include "cv.h"
#include <QImage>
#include "highgui.h"

#include "../include/Histogram.h"
#include "../include/Time.h"
#include "../include/Frame.h"

#include "../include/Log.h"

#include "../include/Video.h"

/************************************************************************
* Função que realiza o carregamento do vídeo no sistema. 
*************************************************************************
* param (E): CvCapture *vdoSrc => vídeo a ser carregado.
************************************************************************
* Retorno: Video Carregado
************************************************************************
* Histórico
* 01/07/08 - Thiago Mizutani
* Criação.
************************************************************************/
CvCapture* Video::open(char *vdoSrc)
{
	CvCapture *video = 0;

	video = cvCaptureFromAVI(vdoSrc);

	if (video == NULL)
	{
		Log::writeLog("%s :: Open Failed", __FUNCTION__);
		throw "Open failed";
	}
	
	return (video);
}

/************************************************************************
* Construtor que recebe uma string que é o nome do arquivo de video 
* a ser aberto
*************************************************************************
* param (E): char *filename_cy => Nome do arquivo a ser aberto
************************************************************************
* Retorno: Video Carregado
************************************************************************
* Histórico
* 01/07/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
Video::Video(char *filename_cy)
{
	
	//char file_cy;
	//char path_cy;
	char idx_c = 0;
	char char_c;
	int i;

	// Abre o arquivo

	Log::writeLog("%s :: open filename[%s]", __FUNCTION__, filename_cy);

	this->data = open(filename_cy);

	// Atualiza as variaveis internas do objeto video
	char_c = '/';

	// Desmembra nome de arquivo - path

	// Procura pelo separador
	for (i = strlen(filename_cy) ; i >= 0 ; i--)
	{
		if (filename_cy[i] == char_c)
		{
			idx_c = i;
			break;
		}
	}

	Log::writeLog("%s :: open idx_c[%c]", __FUNCTION__, idx_c);

	memset(this->path, '\0', sizeof(this->path));
	memset(this->name, '\0', sizeof(this->name));
	
	if (idx_c) // Se encontrou o separador, desmembra
	{
		strncpy(this->path, filename_cy, idx_c);
		strncpy(this->name, &filename_cy[idx_c+1], strlen(filename_cy)-idx_c);
	}
	else // Senão, o arquivo está no diretório corrente
	{
		strcpy(this->path, "./");
		strcpy(this->name, filename_cy);
	}

	Log::writeLog("%s :: path[%s] name[%s]", __FUNCTION__, this->path, this->name);

	// Pega as propriedades do video

	// FPS
	// CV_CAP_PROP_FPS - frame rate
	this->fps = cvGetCaptureProperty(this->data, CV_CAP_PROP_FPS);
	Log::writeLog("%s :: fps[%.0f] ", __FUNCTION__, this->fps);

	// Largura dos frames
	// CV_CAP_PROP_FRAME_WIDTH - width of frames in the video stream 
	this->framesWidth = cvGetCaptureProperty(this->data, CV_CAP_PROP_FRAME_WIDTH);
	Log::writeLog("%s :: famesWidth[%.0f] ", __FUNCTION__, this->framesWidth);

	// Altura dos frames 
	// CV_CAP_PROP_FRAME_HEIGHT - height of frames in the video stream
	this->framesHeight = cvGetCaptureProperty(this->data, CV_CAP_PROP_FRAME_HEIGHT);
	Log::writeLog("%s :: famesHeight[%.0f] ", __FUNCTION__, this->framesHeight);

	// Codec
	// CV_CAP_PROP_FOURCC - 4-character code of codec.
	this->codec = cvGetCaptureProperty(this->data, CV_CAP_PROP_FOURCC);
	Log::writeLog("%s :: codec[%.0f] ", __FUNCTION__, this->codec);

	// Total de Frames que o video tem
	//CV_CAP_PROP_FRAME_COUNT - number of frames in AVI file. 
	this->framesTotal = cvGetCaptureProperty(this->data, CV_CAP_PROP_FRAME_COUNT);
	Log::writeLog("%s :: framesTotal[%.0f] ", __FUNCTION__, this->framesTotal);

	// Atributos relativos a posicao
	updatePos();

	// Inicializa o ROI
	resetROI();

}
 
/************************************************************************
* Atualiza os controles internos referente a posição atual do ponteiro
* de video
*************************************************************************
* param (E): Nenhum
************************************************************************
* return: Nenhum
************************************************************************
* Histórico
* 30/07/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void Video::updatePos()
{
	long msec;

	//CV_CAP_PROP_POS_FRAMES - 0-based index of the frame to be decoded/captured next
	this->framePos = cvGetCaptureProperty(this->data, CV_CAP_PROP_POS_FRAMES);
//	Log::writeLog("%s :: framePos[%.0f] ", __FUNCTION__, this->framePos);

	msec = (long)cvGetCaptureProperty(this->data, CV_CAP_PROP_POS_MSEC);
//	Log::writeLog("%s :: timePos[%.0f] ", __FUNCTION__, msec);

}

/************************************************************************
* Get para a variável name
*************************************************************************
* param (E): Nenhum
************************************************************************
* return: char* name
************************************************************************
* Histórico
* 30/07/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
char* Video::getName()
{
	return name;
}

/************************************************************************
* Get para a variável path
*************************************************************************
* param (E): Nenhum
************************************************************************
* return: char* path
************************************************************************
* Histórico
* 30/07/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
char* Video::getPath()
{
	return path;
}

/************************************************************************
* Get para a variável framesTotal
*************************************************************************
* param (E): Nenhum
************************************************************************
* return: double framesTotal
************************************************************************
* Histórico
* 30/07/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
double Video::getFramesTotal()
{
	return framesTotal;
}

/************************************************************************
* Captura o frame corrente e aponta para o próximo
*************************************************************************
* param (E): Nenhum
************************************************************************
* return: Frame* - frame capturado.
************************************************************************
* Histórico
* 30/07/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
Frame* Video::getNextFrame()
{
	if (data == 0x0)
		return NULL;

	if (!cvGrabFrame(this->data))
	{
		return NULL;
	}
	else
	{
		IplImage *image = cvRetrieveFrame(this->data);
		Frame *frameNew = new Frame(image);

		updatePos();

		if (this->ROI.x != -1 && this->ROI.y != 1)
		{
			IplImage* imgWide;

			// Crio uma imagem nova com o tamanho do RV sem as faixas de widescreen
			imgWide = Frame::imgAlloc(cvSize(this->ROI.width,this->ROI.height), frameNew->data->depth, frameNew->data->nChannels);

			// Pego somente a parte de interesse (sem o wide) do Frame.
			cvSetImageROI(frameNew->data,this->ROI);

			// Copio para uma imagem nova
			Frame::imgCopy(frameNew->data,imgWide);

			cvResetImageROI(frameNew->data);

			// Seto o frame sem o wide 
			frameNew->setImage(imgWide);

//			Log::writeLog("%s :: setting ROI: x[%d] y[%d] height[%d] width[%d]", __FUNCTION__ , this->ROI.x, this->ROI.y, this->ROI.height, this->ROI.width);
		}

		return (frameNew);
	}
}

/************************************************************************
 * Captura o frame anterior e passa a apontar para ele
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * return: Frame* - frame capturado.
 ************************************************************************
 * Histórico
 * 30/07/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
Frame* Video::getPreviousFrame()
{
	// Posiciono no frame anterior
	seekFrame((long)this->getCurrentPosition() - 1);

	// Capturo este frame
	Frame *frameNew = getNextFrame();

	// Posiciono no frame capturado
	seekFrame((long)this->getCurrentPosition() - 1);

	return (frameNew);
}

/************************************************************************
 * Captura o frame corrente e não move o ponteiro para o próximo
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * return: Frame* - frame capturado.
 ************************************************************************
 * Histórico
 * 30/07/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
Frame* Video::getCurrentFrame()
{
	// Pega o frame atual
	Frame *frameCurrent = getNextFrame();
	// Volta a apontar pra ele
	
	if (seekFrame((long)this->getCurrentPosition() - 1) == 1)
	{
		// Se retornar 1: erro FATAL!
		throw;
	}

	return frameCurrent;

}

/************************************************************************
 * Get para a variavel codec 
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * return: double framesWidth
 ************************************************************************
 * Histórico
 * 30/07/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
double Video::getCodec()
{
	return codec;
}

/************************************************************************
 * Get para a variavel framesWidth
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * return: double framesWidth
 ************************************************************************
 * Histórico
 * 30/07/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
double Video::getFramesWidth()
{
	return framesWidth;
}

/************************************************************************
 * Get para a variavel framesHeight
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * return: double framesHeight
 ************************************************************************
 * Histórico
 * 30/07/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
double Video::getFramesHeight()
{
	return framesHeight;
}

/************************************************************************
 * Posiciona o ponteiro do video.
 *************************************************************************
 * param (E): unsigned long posFrame - Numero do frame em questão
 ************************************************************************
 * return: 	1 - Erro: parametro passado é invalido
 *				0 - Sucesso
 ************************************************************************
 * Histórico
 * 30/07/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
int Video::seekFrame(unsigned long posFrame)
{
	if (posFrame >= this->framesTotal)
		return 1;

	if (framePos == posFrame)
		return 0;

	cvSetCaptureProperty(this->data, CV_CAP_PROP_POS_FRAMES, (double)posFrame);

	updatePos();

	return 0;
}

/************************************************************************
 * Retorna valor da posição atual do video (em qual frame está)
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * return: double framePos
 ************************************************************************
 * Histórico
 * 13/08/08 - Thiago Mizutani
 * Renomeando a função de acordo com aquilo que estava na especificação.
 * 30/07/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
double Video::getCurrentPosition()
{
	return framePos;
}

/************************************************************************
 * Retorna o valor do FPS do vídeo
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * return: double FPS 
 ************************************************************************
 * Histórico
 * 15/08/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

double Video::getFPS()
{
	return (this->fps);
}

/************************************************************************
 * Remove o wide de um video inteiro, usando a funcao de remocao
 * de wide de um frame.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Histórico
 * 05/09/08 - Fabrício Lopes de Souza
 * Criação.
 ************************************************************************/
void Video::removeWide()
{
	double currentPosition = 0;

	int sizeWide    = 0;
	int minSizeWide = 9999;

	Frame *frame = 0;

	int npoints = 0;
	int i = 0;


	npoints = cvRound(0.2 * (long)this->getFramesTotal() ); // Vou scanear 20% dos frames do video

	currentPosition = this->getCurrentPosition();

	// Aponta pro comeco do video
	this->seekFrame(0);

	for (i = 0 ; i < this->getFramesTotal() ; i += npoints)
	{
		this->seekFrame(i);

		frame = this->getCurrentFrame();

		sizeWide = frame->removeWide();

		Log::writeLog("%s :: Wide [%d]", __FUNCTION__, sizeWide);

		if (sizeWide)
		{
			if (sizeWide < minSizeWide)
				minSizeWide = sizeWide;
		}

		delete frame;

	}

	Log::writeLog("%s :: The min size of wide is [%d]", __FUNCTION__, minSizeWide);

	// Devolve ele pra posição que estava
	this->seekFrame(cvRound(currentPosition));

	if (minSizeWide > 0 && minSizeWide != 9999)
	{
		this->ROI.x = 0;
		this->ROI.y = minSizeWide;
		this->ROI.width =  cvRound(this->getFramesWidth());
		this->ROI.height = cvRound(this->getFramesHeight()-(minSizeWide*2));
	}
}

/************************************************************************
 * Remove o wide de um video inteiro, usando a funcao de remocao
 * de borda	 de um frame.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Histórico
 * 05/09/08 - Fabrício Lopes de Souza
 * Criação.
 ************************************************************************/
void Video::removeBorder()
{
	double currentPosition = 0;

	int borderSize = 0;
	int minBorderSize = 9999;

	Frame *frame = 0;

	int npoints = 0;
	int i = 0;


	npoints = cvRound(0.2 * this->getFramesTotal()); // Vou scanear 10% dos frames do video

	currentPosition = this->getCurrentPosition();

	// Aponta pro comeco do video
	this->seekFrame(0);

	for (i = 0 ; i < this->getFramesTotal() ; i += npoints)
	{
		this->seekFrame(i);

		frame = this->getCurrentFrame();

		borderSize = frame->removeBorder();

		Log::writeLog("%s :: Wide [%d]", __FUNCTION__, borderSize);

		if (borderSize)
		{
			if (borderSize < minBorderSize)
				minBorderSize = borderSize;
		}

		delete frame;

	}

	Log::writeLog("%s :: The min size of wide is [%d]", __FUNCTION__, minBorderSize);

	// Devolve ele pra posição que estava
	this->seekFrame(cvRound(currentPosition));

	if (minBorderSize > 0 && minBorderSize != 9999)
	{
		// Seta no ROI padrao do video as medidas
		this->ROI.x = minBorderSize;
		this->ROI.width = cvRound(this->getFramesWidth()-(minBorderSize*2));

		if (this->ROI.y == -1)
			this->ROI.y = 1;

		if (this->ROI.height == -1)
			this->ROI.height = cvRound(this->getFramesHeight());
	}
}

/************************************************************************
* Inicializa a ROI (Region Of Interest).
*************************************************************************
* param (E): Nenhum
************************************************************************
* return: Nenhum
************************************************************************
* Histórico
* 03/11/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void Video::resetROI()
{
	// Inicializa o ROI
	this->ROI.x      = -1;
	this->ROI.y      = -1;
	this->ROI.width  = -1; 
	this->ROI.height = -1;
}

/************************************************************************
* Destrutor. Fecha o video.
*************************************************************************
* param (E): Nenhum
************************************************************************
* return: Nenhum
************************************************************************
* Histórico
* 10/11/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
Video::~Video()
{
	if (data)
	{
		cvReleaseCapture(&data);
	}
}
