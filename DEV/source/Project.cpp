#include "cv.h"
#include "highgui.h"


#include <QImage>
#include <vector>

#include "../include/VideoPlayer.h"
#include "../include/Render.h"
#include "../include/RenderThread.h"
#include "../include/Interface.h"
#include "../include/Effect.h"
#include "../include/Color.h"
#include "../include/VisualRythim.h"
#include "../include/Transition.h"
#include "../include/Project.h"
#include "../include/Xml.h"
#include "../include/Log.h"

#include "../include/main.h"

extern VideoPlayer *vdo_player;

/************************************************************************
* Abre um arquivo XML de projeto e carrega as estruturas necess�rias.
*************************************************************************
* param (E): QString fileName => nome do arquivo a ser aberto.
*************************************************************************
* Hist�rico
* 29/09/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
int Project::openProject(QString fileName)
{

	char filename_cy[100];
	int ret,i;
	Xml *fileXml = new Xml();

	// Abre um projeto ja existente
	Interface::QStringToChar(fileName, filename_cy);

	ret = fileXml->openXml(filename_cy);

	if(!ret)
	{
		//apaga lista de transicoes
		this->clearTransitionList();

		//apaga lista de efeitos
		this->clearEffectList();

		fileXml->setItemNumber(0);
		fileXml->readXml("name");

		Interface_ui->changeWindowTitle(fileXml->getText());

		fileXml->setItemNumber(0);
		fileXml->readXml("video");

		openVideo(fileXml->getText());

		// Recupera a lista de transicoes detectadas.
		for(i=0;i<fileXml->getSizeNodes();i++)
		{
			fileXml->setItemNumber(i);
			fileXml->readXml("transition");

			if(fileXml->getSizeNodes())
			{
				Transition *transition = fileXml->getTransition();
				this->transitionList.push_back(*transition);
			}
		}

		// Recupera a lista de efeitos aplicados.
		for(i=0;i<fileXml->getSizeNodes();i++)
		{
			fileXml->setItemNumber(i);
			fileXml->readXml("effect");

			if(fileXml->getSizeNodes())
			{
				Effect *effect = fileXml->getEffect();
				this->effectList.push_back(effect);
			}
		}

		fileXml->setItemNumber(0);
		fileXml->readXml("userCutThreshold");
		this->userCutThreshold = atoi(fileXml->getText());

		fileXml->setItemNumber(0);
		fileXml->readXml("userMinCanny");
		this->userMinCanny = atoi(fileXml->getText());

		fileXml->setItemNumber(0);
		fileXml->readXml("userMaxCanny");
		this->userMaxCanny = atoi(fileXml->getText());

		Interface_ui->addRecentFile(fileName);
		Interface_ui->updateRecentFilesAct();
	}

	fileXml->closeXml();

	Interface_ui->disableSaveButton();

	return true;
}

/************************************************************************
 * Salva o projeto em um arquivo XML
 *************************************************************************
 * param (E): QString fileName => Nome do arquivo a ser salvo
 *************************************************************************
 * Hist�rico
 * 09/11/08 - Thiago Mizutani
 * Revis�o de c�digo, altera��o dos coment�rios
 * 14/10/08 - Ivan Shiguenori Machida
 * Cria��o.
 ************************************************************************/
int Project::saveProject(QString fileName)
{
	char filename_cy[256];
	char path_cy[256];
	int ret;
	Xml *fileXml = new Xml();

	QFileInfo pathInfo( fileName );

	QString file( pathInfo.fileName());
	QString path( pathInfo.absoluteFilePath() );

	Interface::QStringToChar(path, path_cy);
	Interface::QStringToChar(file, filename_cy);

	ret = fileXml->createXml(path_cy, this);

	strcpy(this->filename_cy, filename_cy);
	strcpy(this->path_cy, path_cy);

	Interface_ui->disableSaveButton();

	return true;
}

/************************************************************************
 * Inicia um novo projeto
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
Project::Project(void)
{

	// Inicializa os ponteiros com 0
	this->vdo = '\0';
	this->frame = '\0';
	this->userCutThreshold = '\0';
	this->userMinCanny = '\0';
	this->userMaxCanny = '\0';

	strcpy(this->filename_cy, DEFAULT_PROJECT_NAME);

	// Troca o titulo da janela
	Interface_ui->changeWindowTitle(this->filename_cy);

	// Limpa a lista de transicoes
	this->clearTransitionList();

}

/*************************************************************************
 * Destrutor. Libera/fecha todos os recursos abertos/instanciados
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 10/11/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/
Project::~Project(void)
{
	if (vdo)
		delete vdo;

	if (frame)
		delete frame;

	// Limpa lista de transi��es
	transitionList.clear();

	// Limpa lista de efeitos
	effectList.clear();
}

/************************************************************************
 * Realiza a abertura de um video novo.
 * Preenche a videoPropertiesTree com informacoes do video.
 * Caso ja tenha um video aberto, mata ele e limpa a videoPropertiesTree.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
int Project::openVideo(QString fileName)
{
	char param_cy[50];
	char filename_cy[256];

	Log::writeLog("%s :: vdo[%x]", __FUNCTION__, this->vdo);

	if (this->vdo != 0x0)
	{
		Log::writeLog("%s :: cleaning vdo[%x]", __FUNCTION__, this->vdo);
		delete this->vdo;

		//apaga lista de transicoes
		this->clearTransitionList();
		this->clearEffectList();

		Interface_ui->clearVideoProperty();
		Interface_ui->clearTransitionsTree();
		Interface_ui->effectTreeClear();
	}

	// Transformando QString em char*
	Interface::QStringToChar(fileName, filename_cy);

	this->vdo = new Video(filename_cy);

	// Pega as propriedades do video e exibe na lista
	memset(param_cy, '\0', sizeof(param_cy));

	// Nome
	Interface_ui->insertVideoProperty("Name", this->vdo->getName());

	// Path
	Interface_ui->insertVideoProperty("Path", this->vdo->getPath());

	// Codec
	memset(param_cy, '\0', sizeof(param_cy));
	sprintf(param_cy, "%.0lf", this->vdo->getCodec());
	Interface_ui->insertVideoProperty("Codec", param_cy);

	// framesHeight
	memset(param_cy, '\0', sizeof(param_cy));
	sprintf(param_cy, "%.0lf", this->vdo->getFramesHeight());
	Interface_ui->insertVideoProperty("Height", param_cy); 

	// framesWidth
	memset(param_cy, '\0', sizeof(param_cy));
	sprintf(param_cy, "%.0lf", this->vdo->getFramesHeight());
	Interface_ui->insertVideoProperty("Width",  param_cy);

	// FPS
	memset(param_cy, '\0', sizeof(param_cy));
	sprintf(param_cy, "%.2lf", this->vdo->getFPS());
	Interface_ui->insertVideoProperty("FPS",  param_cy);

	// Frames
	memset(param_cy, '\0', sizeof(param_cy));
	sprintf(param_cy, "%.0lf", this->vdo->getFramesTotal());
	Interface_ui->insertVideoProperty("Frames",  param_cy);

	Interface_ui->createTimeline();
	Interface_ui->updateTransitions();

	Interface_ui->addRecentFile(fileName);
	Interface_ui->updateRecentFilesAct();

	vdo_player->updateCurrentFrame();

	return true;
}

/************************************************************************
 * Retorna o video corrente do projeto
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Video* -> video do projeto
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/

Video* Project::getVideo()
{
	return vdo;
}

/************************************************************************
 * Define o limiar do projeto com o valor de entrada do usu�rio.
 *************************************************************************
 * param (E): int threshold -> valor do limiar. 
 *************************************************************************
 * return : Nenhum. 
 *************************************************************************
 * Hist�rico
 * 14/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

void Project::setUserThreshold(int threshold)
{
	this->userCutThreshold = threshold;
}

/************************************************************************
 * Retorna o valor do limiar definido pelo usu�rio no projeto.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : int userCutThreshold -> limiar definido pelo usu�rio. 
 *************************************************************************
 * Hist�rico
 * 14/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

int Project::getUserThreshold()
{
	return (this->userCutThreshold);
}

/*************************************************************************
 * Fun��o set para limiar m�nimo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E) : int userMinCanny-> limiar minimo definido pelo usu�rio. 
 *************************************************************************
 * return : nenhum
 *************************************************************************
 * Hist�rico
 * 18/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

void Project::setUserMinCanny(int userMinCanny)
{
	this->userMinCanny = userMinCanny;
}

/*************************************************************************
 * Fun��o get para limiar m�nimo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : int userMinCanny -> limiar minimo definido pelo usu�rio. 
 *************************************************************************
 * Hist�rico
 * 18/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

int Project::getUserMinCanny()
{
	return (this->userMinCanny);
}

/*************************************************************************
 * Fun��o set para limiar m�ximo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E):int userMaxCanny -> limiar maximo definido pelo usu�rio 
 *************************************************************************
 * return : n�o h�. 
 *************************************************************************
 * Hist�rico
 * 18/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

void Project::setUserMaxCanny(int userMaxCanny)
{
	this->userMaxCanny = userMaxCanny;
}

/*************************************************************************
 * Fun��o get para limiar m�ximo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : int userMaxCanny-> limiar m�ximo definido pelo usu�rio. 
 *************************************************************************
 * Hist�rico
 * 18/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

int Project::getUserMaxCanny()
{
	return (this->userMaxCanny);
}

/*************************************************************************
 * Ordena a lista de transi��s
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Nenhum. 
 *************************************************************************
 * Hist�rico
 * 17/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/

void Project::sortTransitionList(void)
{
	sort(this->transitionList.begin(), this->transitionList.end() );
}

/*************************************************************************
 * Obt�m, a partir de um n�mero de frame a posi��o em que o cursor da 
 * timeline deve aparecer.
 *************************************************************************
 * param (E): long frame -> n�mero do frame corrente.
 *************************************************************************
 * return : Posi��o do cursor na timeline. 
 *************************************************************************
 * Hist�rico
 * 17/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/

long Project::FrameToTimelinePos(long frame)
{
	// A formula para saber em que ponto plotar o indicador �:
	// SIZE_FRAME_TIMELINE  ---- SIZE_SEC_FRAME*vdo->getFPS()
	// pos (ponto timeline) ---- pos (ponto no video)
	return (SIZE_FRAME_TIMELINE*cvRound(frame)) / (SIZE_SEC_FRAME*cvRound(this->vdo->getFPS()));
}

/*************************************************************************
 * Converte a posicao clicada na timeline para a posicao no frame
 *************************************************************************
 * param (E): long pos -> posi��o da timeline em que foi clicado. 
 *************************************************************************
 * return : N�mero do frame correspondente. 
 *************************************************************************
 * Hist�rico
 * 17/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/
long Project::TimelinePosToFrame(long pos)
{
	// A ideia aqui e a seguinte:
	// Tendo em maos as seguintes variaveis:
	// Todo FRAME da timeline tem o tamanho fixo de SIZE_FRAME_TIMELINE
	// E cada um desse FRAME compreende SIZE_SEC_FRAME segundos do video
	// captando a posicao em que o usuario clicou, podemos fazer a seguinte regra de 3:
	// SIZE_FRAME_TIMELINE ---------- SIZE_SEC_FRAME*vdo->getFPS()
	// Posicao clicada (x) ---------- X (posicao no frame)
	return (cvRound(SIZE_SEC_FRAME*this->vdo->getFPS())*pos / SIZE_FRAME_TIMELINE);
}

/*************************************************************************
 * Renderiza o video com os efeitos aplicados
 *************************************************************************
 * param (E): char *filename_cy 
 *************************************************************************
 * return : nenhum.
 *************************************************************************
 * Hist�rico
 * 16/10/08 - Fabr�cio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Project::renderVideo(char *filename_cy)
{
	Render renderUI;
	RenderThread *renderThread;

	// Cria a thread
	renderThread = new RenderThread(filename_cy);

	// Inicia a thread
	renderUI.startRenderThread(renderThread);

	// Mostra a janela
	renderUI.exec();

	delete renderThread;
}


/*************************************************************************
 * Limpa a lista de transi��es
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Hist�rico
 * 18/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/
void Project::clearTransitionList()
{
	Transition *transition;

	//apaga lista de transicoes
	this->transitionList.clear();

	// Cria transicao de inicio de video
	transition = new Transition(TRANSITION_VIDEOSTART, 0, "Inicio do Video");

	this->transitionList.push_back(*transition);

}

/*************************************************************************
 * Aplica todos os efeitos possiveis no frame
 *************************************************************************
 * param (E) : Frame *frame -> Frame em que ser� aplicado o efeito
 * param (E) : long pos -> posic�o do frame no video
 *************************************************************************
 * return : N�mero do frame correspondente. 
 *************************************************************************
 * Hist�rico
 * 19/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/

Frame* Project::applyEffect(Frame *frame, long pos)
{
	Frame *frameEffect = 0x0;
	Frame *frameNew = new Frame(frame);
	Effect *effect = 0x0;
	unsigned int i = 0;

	for ( i = 0 ; i < effectList.size() ; i++)
	{
		effect = effectList.at(i);

		// Se o frame corrente for maior que o primeiro frame do efeito
		if (pos >= effect->getFrameStart())
		{
			if (pos < effect->getFrameEnd())
			{
				// Aplica o efeito no frame
				frameEffect = effect->applyEffect(frameNew);

				delete frameNew;

				frameNew = frameEffect;

				continue;
			}
		}
	}

	return frameNew;
}

/*************************************************************************
 * Limpa a lista de efeitos.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Hist�rico
 * 19/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/

void Project::clearEffectList()
{
	this->effectList.clear();
}

/*************************************************************************
 * Obt�m a transi��o correspondente ao ponto clicado na timeline e marca
 * esta tomada.
 *************************************************************************
 * param (E): int pos_x => posi��o do cursor da timeline.
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Hist�rico
 * 19/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/

int Project::getTransitionByPos(int pos_x)
{
	int x;
	long pos;
	unsigned int i;
	// Por padrao deixamos o size, pois se nao achar nenhum
	// nao entra nas condicoes
	unsigned int transitionID = transitionList.size();

	Transition* transition = 0x0;

	x = pos_x - 9 ;
	pos = TimelinePosToFrame(x);

	// Temos que achar se tem alguma transicao no ponto indicado
	// Vamos de traz pra frente ate achar uma transicao
	// onde o ponto clicado eh maior que o ponto de inicio da transicao
	for (i = transitionList.size()-1 ; (signed)i >= 0 ; i--)
	{
		transition = &transitionList.at(i);

		if (pos >= transition->getPosCurrent())
		{
			// Achamos
			transitionID = i;
			break;
		}	
	}

	return transitionID;
}

/*************************************************************************
 * Remove um efeito do v�deo e da lista de efeitos.
 *************************************************************************
 * param (E): int ind => n�mero do efeito.
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Hist�rico
 * 19/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/

void Project::removeEffect(int ind)
{
	Effect *effect = effectList.at(ind);

	delete effect;

	effectList.erase(effectList.begin( ) + ind);

	Interface_ui->updateEffectTree();
}

/*************************************************************************
 * Remove uma transi��o apontada pelo sistema da lista de transi��es.
 *************************************************************************
 * param (E): int ind => n�mero do efeito.
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Hist�rico
 * 19/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/

void Project::removeTransition(int ind)
{
	Transition *transition = &transitionList.at(ind);

	delete transition;

	transitionList.erase(transitionList.begin( ) + ind);

	sortTransitionList();
}


/*************************************************************************
 * Get para a vari�vel: path_cy
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : char* - Path do projeto
 *************************************************************************
 * Hist�rico
 * 19/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/
char* Project::getPath()
{
	return path_cy;
}

/*************************************************************************
 * Get para a vari�vel: filename_cy
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : char* - Nome do projeto
 *************************************************************************
 * Hist�rico
 * 19/10/08 - Fabr�cio Lopes de Souza 
 * Cria��o.
 ************************************************************************/
char* Project::getName()
{
	return filename_cy;
}
