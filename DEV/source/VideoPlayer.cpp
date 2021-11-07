#include "../include/VideoPlayer.h"
#include "../include/Histogram.h"
#include "../include/Project.h"
#include "../include/Effect.h"
#include "../include/Color.h"

extern Project *currentProject;
extern Interface *Interface_ui;

/************************************************************************
 * Construtor
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/

VideoPlayer::VideoPlayer(void)
{
	// Variaveis internas
	this->imageData = 0x0;
	this->imgWidth  = 0x0;
	this->imgHeight = 0x0;

	this->histData = 0x0;
	this->histWidth  = 0x0;
	this->histHeight = 0x0;

	this->timelineData = 0x0;
	this->timelineWidth = 0x0;
	this->timelineHeight = 0x0;

	this->frameTimeline = 0x0;
	this->frameTimelineOriginal = 0x0;
	this->frameTimelineEdited = 0x0;
}

/************************************************************************
 * Metodo que é executado quando a thread do VideoPlayer for iniciada.
 * Pega frame a frame e emite sinal para a interface desenhar.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/

void VideoPlayer::run()
{
	Video *vdo = currentProject->getVideo();
	Frame *frame = 0x0;

	if (!vdo)
		return;

	if ((vdo->getCurrentPosition() >= vdo->getFramesTotal()))
	{
		vdo->seekFrame(0);
	}
		
	frame = vdo->getNextFrame();

	while (frame) // Enquanto houver frame vai passando o vídeo.
	{

		// Atualiza o frame do player
		updatePlayer(frame);

		// Se o histograma estiver visivel, atualiza 
		if (Interface_ui->ui.histogramLabel->isVisible())
		{
			updateHist(frame);
		}

		delete frame;

		// Damos um tempo para fazer o vídeo passar mais devagar. Parecendo um player real.
		usleep(cvRound(vdo->getFPS()*2000));

		vdo->seekFrame(cvRound((vdo->getFPS() / 10) + vdo->getCurrentPosition()));
		
		// Pega o proximo frame.
		frame = vdo->getNextFrame();

	}
}

/*************************************************************************
 * Método que emite um sinal para o connect da interface fazer a alteração
 * do frame a ser exibido no player
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani
 * Revisão - Alterado o nome da função e separação e exclusão de funções
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/

void VideoPlayer::updatePlayer(Frame *frame)
{
	// Se nao vir nada, aborta
	if (!frame)
		return;

	Frame *frameNew;

	QImage *image = 0x0;
	Video *vdo = currentProject->getVideo();

	if (currentProject->effectList.size() > 0)
	{
		frameNew = currentProject->applyEffect(frame, (long)vdo->getCurrentPosition());
	}
	else
	{
		frameNew = frame;
	}

	image = frameNew->IplImageToQImage(&imageData, &imgWidth, &imgHeight);

	// Emite o sinal para a thread principal
	// pintar a imagem
	emit setNewFrame(image);

	if (frame != frameNew)
		delete frameNew;
}

/*************************************************************************
 * Método que envia um sinal para o connect da interface fazer a atualiza
 * ção do histograma.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani
 * Diminuindo o número de funções e separando as coisas.
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/

void VideoPlayer::updateHist(Frame *frame)
{
	QImage *imageHist = 0x0;
	Frame* frameGray = 0x0;
	Frame* frameHistogram = 0x0;
	Color* color = 0x0;
	Histogram *hist = 0x0;

	if (!Interface_ui->ui.histogramLabel->isVisible())
		return;

	if (!frame)
		return;

	color = new Color;
	frameGray = color->convert2Gray(frame);
	hist = frameGray->createHistogram();

	frameHistogram = new Frame(hist->getMatrix(), 256, hist->getMax());

	imageHist = frameHistogram->IplImageToQImage(&histData, &histWidth, &histHeight);

	// Manda exibir o histograma
	emit setHistogram(imageHist);

	delete frameGray;
	delete frameHistogram;
	delete hist;
	delete color;
}

/*************************************************************************
 * Atualiza a atual posicao do cursor.
 * Serve pra quando for aplicado/removido o efeito, o video preview
 * seja atualizado no momento que uma acao for tomada
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * 03/11/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void VideoPlayer::updateCurrentFrame()
{
	Frame *frameNew;
	Video *vdo = currentProject->getVideo();

	frameNew = vdo->getCurrentFrame();

	this->updatePlayer(frameNew);
	this->updateHist(frameNew);

	delete frameNew;
}

/*************************************************************************
 * Destrutor. Libera os recursos alocados.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * 10/11/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
VideoPlayer::~VideoPlayer()
{
	if (imageData)
		delete imageData;

	if (histData)
		delete histData;

	if (timelineData)
		delete timelineData;

	if (frameTimeline)
		delete frameTimeline;

	if (frameTimelineOriginal)
		delete frameTimelineOriginal;

	if (frameTimelineEdited)
		delete frameTimelineEdited;
}
