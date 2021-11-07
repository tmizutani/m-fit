#include "cv.h"
#include "highgui.h"

#include "../include/RenderThread.h"
#include "../include/Histogram.h"
#include "../include/Project.h"
#include "../include/Effect.h"
#include "../include/Color.h"

extern Project *currentProject;
extern Interface *Interface_ui;

/************************************************************************
 * Construtor
 *************************************************************************
 * param (E): char* - nome do video a ser salvo
 *************************************************************************
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
RenderThread::RenderThread(char *filename_cy)
{
	strcpy(this->filename_cy, filename_cy);
}

/************************************************************************
 * Metodo que é executado quando a thread da Renderização
 * Pega frame a frame, aplica o efeito, e manda escrever
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 20/11/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void RenderThread::run()
{
	long totalFrame = 0x0;
	long i = 0;
	long currentPos = 0;
	unsigned int j = 0;
	unsigned int numEffect = 0;

	Frame *frame;
	Frame *frameEffect;
	Video *vdo;

	// CODECS
	// CV_FOURCC('H','F','Y','U') // HuffYUV  ~3:1 compression.
	// CV_FOURCC('P','I','M','1') // MPEG 1 (if you have it)
	// CV_FOURCC('M','J','P','G') // Motion-jpeg
	// CV_FOURCC('D','I','B',' ') // Uncompressed
	// CV_FOURCC('C','D','V','C') // Canopus DV
	// CV_FOURCC('D','I','V','X') // divx

	vdo = currentProject->getVideo();

#define UNCOMPRESSED 541215044

	// Posiciona o ponteiro no comeco do video
	currentPos = (long)vdo->getCurrentPosition();

	vdo->seekFrame(0);

	totalFrame = (long)vdo->getFramesTotal();

	// Quantos efeitos temos
	numEffect = currentProject->effectList.size();

	// Reseta as ROI
	vdo->ROI.x = -1;
	vdo->ROI.y = -1;

	// Varremos Frame a Frame do video
	for ( i = 0 , j = 0 ; i < totalFrame ; i ++)
	{
		frame = vdo->getNextFrame();

		if (numEffect > 0)
		{
			frameEffect = currentProject->applyEffect(frame, i);
			delete frame;
		}
		else
		{
			frameEffect = frame;
		}

		// Atualiza a barra de progresso
		sendProgress(i, totalFrame);

		// Escreve o frame no video final
		emit writeFrameThread(frameEffect);

		while (true)
		{
			if (!mutex.locked())
				break;

			usleep(10000);
		}

		delete frameEffect;
	}

	// Devolve o ponteiro pra posicao inicial
	vdo->seekFrame(currentPos);

	emit setProgressThread(100);
}


/************************************************************************
 * Envia para a janela o progresso atual.
 *************************************************************************
 * param (E): int  - Posicao atual
 * param (E): long - Total de posições
 *************************************************************************
 * Histórico
 * 20/11/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void RenderThread::sendProgress(int i, long total)
{

	int percent;

	percent = ( i * 100 / total );

	emit setProgressThread(percent);
}
