#include "../include/QWidgetTimeline.h"
#include "cv.h"
#include "highgui.h"

#include "../include/Transition.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Time.h"
#include "../include/Log.h"
#include "../include/Video.h"
#include "../include/Effect.h"
#include "../include/Project.h"
#include "../include/VideoPlayer.h"
#include "../include/MorphologyEffect.h"
#include "../include/Color.h"

extern Project* currentProject;
extern VideoPlayer* vdo_player;
extern Interface* Interface_ui;

/************************************************************************
* Construtor
*************************************************************************
* param (E): QWidget *parent - Widget parente.
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
QWidgetTimeline::QWidgetTimeline(QWidget *parent) : QWidget(parent) { }

/************************************************************************
* Trata o evento de pressionar o mouse.
*************************************************************************
* param (E): QMouseEvent* - Evento do mouse.
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void QWidgetTimeline::mousePressEvent(QMouseEvent *event)
{
	// Pega os pontos
	QPoint point(this->pos());
	QPoint pointEvent(event->pos());
	QRect  rect(this->geometry());

	// Se o player estiver rodando, ignora
	if (vdo_player->isRunning())
		return;

	// Se nao tiver video carregado, ignora
	if (currentProject->getVideo() == 0x0)
		return;

	Log::writeLog("%s :: widget x[%d] y[%d] press x[%d] y[%d]",
			__FUNCTION__,
			point.x(), point.y(),
			pointEvent.x(), pointEvent.y());

	// Se o click foi no intervalo tratado.
	// Lembrando que a area visivel começa no 9 pixel
	if (pointEvent.x() >= 9 && pointEvent.x() <= rect.width() - 9)
	{
		Video *vdo;
		Frame *frame;

		int posFrame;
		int x;
		unsigned int transitionID;

		// A widget tem 9 pixels antes de comecar realmente
		// a imagem da timeline
		x = pointEvent.x() - 9 ;

		vdo = currentProject->getVideo();

		// Converte a posicao clicada para a posicao no Video.
		posFrame = currentProject->TimelinePosToFrame(x);

		Log::writeLog("%s :: setting frame to pos [%d]", __FUNCTION__, posFrame);

		// Posiciona o ponteiro do Video na posição clicada.
		vdo->seekFrame(posFrame);

		frame = vdo->getCurrentFrame();

		// Atualiza o player com a posicao clicada
		vdo_player->updatePlayer(frame);
		vdo_player->updateHist(frame);

		// Vamos mostrar a transicao referente ao ponto que o usuario clicou
		transitionID = currentProject->getTransitionByPos(pointEvent.x());

		// Se o ponto clicado contiver uma transicao
		// Desenha o Header da timeline
		if (transitionID >= 0 && transitionID < currentProject->transitionList.size())
		{
			Interface_ui->clearTransitionHeader();
			Interface_ui->updateTransitionHeader(transitionID);
			Interface_ui->updateTimeline();
		}

		delete frame;

	}
}

/************************************************************************
* Trata o evento de DROP na timeline.
* Aplica o efeito arrastado no intervalo da transição escolhida.
*************************************************************************
* param (E): QDropEvent* - Evento de drop
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void QWidgetTimeline::dropEvent(QDropEvent *event)
{
	Effect *effect = 0x0;
	Transition *transition = 0x0;
	Video *vdo = 0x0;
	unsigned int i = 0;
	long start = -1;
	long end = 0;

	// Procuro na lista de transicoes a transicao que esta selecionada.
	for (i = 0; i < currentProject->transitionList.size() ; i++)
	{
		transition = &currentProject->transitionList.at(i);

		if (start >= 0)
		{
			if (transition->selected == false)
			{
				end = transition->getPosCurrent();
				break;
			}
			else
			{
				continue;
			}
		}

		if (start == -1)
		{
			if (transition->selected == true)
			{
				start = transition->getPosCurrent();
				continue;
			}	
		}

	}

	// Se nao achou uma transicao seguinte, utilizamos o fim do
	// video como marco para o fim da transicao;
	if (end == 0x0)
	{
		vdo = currentProject->getVideo();
		end = (long)vdo->getFramesTotal();
	}

	start++;
	end++;

	// Define qual efeito foi arrastado
	switch(getItemByEvent(event))
	{
		case EFFECT_COLOR:
			effect = new Color(start, end);
			break;

		case EFFECT_ERODE:
			effect = new Erode(start, end);
			break;

		case EFFECT_DILATE:
			effect = new Dilate(start, end);
			break;

		default:
			break;

	}

	// Se nao aplicou nenhum efeito, retorna
	if (effect == 0x0)
		return;

	// Empilha o efeito
	currentProject->effectList.push_back(effect);

	// Atualiza a Tree
	Interface_ui->updateEffectTree();

	// Atualiza o cabecalho da Timeline
	Interface_ui->clearTransitionHeader();

	// Atualiza o cursor
	vdo_player->updateCurrentFrame();

	// Habilita salvar
	Interface_ui->enableSaveButton();

}

/************************************************************************
* Trata o evento de DragEnter.
* Este evento ocorre quando um objeto que está sendo arrastado entra
* no range de tratamento da timeline.
*************************************************************************
* param (E): QDragEnterEvent* - Evento
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void QWidgetTimeline::dragEnterEvent(QDragEnterEvent * event)
{
	int ret_i = 0;

	// Se nao tem timeline
	if (vdo_player->frameTimeline == 0)
		return;

	// Seleciona a transicao de acordo com as coordenadas do evento
	ret_i = selectDropTransition(event->pos());

	// Se modificamos a timeline, aceita o evento
	if (ret_i == 0)
		event->acceptProposedAction();

	return;

}

/************************************************************************
* Trata o evento de DragMove.
* Este evento ocorre quando um objeto que está sendo arrastado, que já
* está dentro do range da timeline, se movimenta.
*************************************************************************
* param (E): QDragMoveEvent * - Evento
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void QWidgetTimeline::dragMoveEvent(QDragMoveEvent *event)
{
	int ret_i = 0;

	// Se nao tem timeline
	if (vdo_player->frameTimeline == 0)
		return;

	// Seleciona a transicao de acordo com as coordenadas do evento
	// Se tem o CTRL pressionado, nao apagamos o anterior
	if (event->keyboardModifiers() & Qt::ControlModifier)
	{
		ret_i = selectDropTransition(event->pos(), 0);
	}
	else
	{
		ret_i = selectDropTransition(event->pos());
	}

	// Se modificamos a timeline, aceita o evento
	if (ret_i == 0)
		event->acceptProposedAction();

	return;
}

/************************************************************************
* Trata o evento de DragLeave.
* Este evento ocorre quando um objeto que está sendo arrastado, sai
* do range da timeline.
*************************************************************************
* param (E): QDragLeaveEvent * - Evento de DragLeave
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void QWidgetTimeline::dragLeaveEvent(QDragLeaveEvent *event)
{
	// Simpelsmente limpa o cabecalho da timeline
	Interface_ui->clearTransitionHeader();
	// E repinta
	Interface_ui->updateTimeline();
}

/************************************************************************
* A partir do evento, retorna qual Efeito vai ser aplicado.
*************************************************************************
* param (E): QDropEvent* - Evento de Drop
*************************************************************************
* return (E): int - ID do efeito a ser aplicado
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
int QWidgetTimeline::getItemByEvent(QDropEvent *event)
{
	int item = -1;

	// Se for do tipo de QTreeItem
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		QByteArray itemData = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
		QDataStream stream(&itemData, QIODevice::ReadOnly);

		int r, c;
		QMap<int, QVariant> v;
		stream >> r >> c >> v;

		item = r;
	}

	return item;

}


/************************************************************************
* A partir de um ponto, pinta o Header da timeline
* referente a uma transição.
*************************************************************************
* param (E): QPoint - Ponto da transição
* param (E): int    - 1 para limpar antes de pintar
*************************************************************************
* return (E): int - 0 - Pintou / 1 - Nao pintou
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
int QWidgetTimeline::selectDropTransition(QPoint pointEvent, int clear)
{
	unsigned int transitionID;

	// Se nao tem timeline
	if (vdo_player->frameTimeline == 0)
		return 1;

	transitionID = currentProject->getTransitionByPos(pointEvent.x());

	if (clear == 1)
	{
		Interface_ui->clearTransitionHeader();
	}

	if (transitionID < currentProject->transitionList.size())
	{
		Interface_ui->updateTransitionHeader(transitionID, 0);
		Interface_ui->updateTimeline();
		return 0;
	}

	return 1;
}
