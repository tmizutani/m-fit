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
* Abre um arquivo XML de projeto e carrega as estruturas necessárias.
*************************************************************************
* param (E): QString fileName => nome do arquivo a ser aberto.
*************************************************************************
* Histórico
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
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
 * Histórico
 * 09/11/08 - Thiago Mizutani
 * Revisão de código, alteração dos comentários
 * 14/10/08 - Ivan Shiguenori Machida
 * Criação.
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
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Histórico
 * 10/11/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/
Project::~Project(void)
{
	if (vdo)
		delete vdo;

	if (frame)
		delete frame;

	// Limpa lista de transições
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
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/

Video* Project::getVideo()
{
	return vdo;
}

/************************************************************************
 * Define o limiar do projeto com o valor de entrada do usuário.
 *************************************************************************
 * param (E): int threshold -> valor do limiar. 
 *************************************************************************
 * return : Nenhum. 
 *************************************************************************
 * Histórico
 * 14/10/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

void Project::setUserThreshold(int threshold)
{
	this->userCutThreshold = threshold;
}

/************************************************************************
 * Retorna o valor do limiar definido pelo usuário no projeto.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : int userCutThreshold -> limiar definido pelo usuário. 
 *************************************************************************
 * Histórico
 * 14/10/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

int Project::getUserThreshold()
{
	return (this->userCutThreshold);
}

/*************************************************************************
 * Função set para limiar mínimo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E) : int userMinCanny-> limiar minimo definido pelo usuário. 
 *************************************************************************
 * return : nenhum
 *************************************************************************
 * Histórico
 * 18/10/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

void Project::setUserMinCanny(int userMinCanny)
{
	this->userMinCanny = userMinCanny;
}

/*************************************************************************
 * Função get para limiar mínimo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : int userMinCanny -> limiar minimo definido pelo usuário. 
 *************************************************************************
 * Histórico
 * 18/10/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

int Project::getUserMinCanny()
{
	return (this->userMinCanny);
}

/*************************************************************************
 * Função set para limiar máximo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E):int userMaxCanny -> limiar maximo definido pelo usuário 
 *************************************************************************
 * return : não há. 
 *************************************************************************
 * Histórico
 * 18/10/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

void Project::setUserMaxCanny(int userMaxCanny)
{
	this->userMaxCanny = userMaxCanny;
}

/*************************************************************************
 * Função get para limiar máximo (definido pelo usuario) do filtro de canny 
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : int userMaxCanny-> limiar máximo definido pelo usuário. 
 *************************************************************************
 * Histórico
 * 18/10/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

int Project::getUserMaxCanny()
{
	return (this->userMaxCanny);
}

/*************************************************************************
 * Ordena a lista de transiçõs
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Nenhum. 
 *************************************************************************
 * Histórico
 * 17/10/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/

void Project::sortTransitionList(void)
{
	sort(this->transitionList.begin(), this->transitionList.end() );
}

/*************************************************************************
 * Obtém, a partir de um número de frame a posição em que o cursor da 
 * timeline deve aparecer.
 *************************************************************************
 * param (E): long frame -> número do frame corrente.
 *************************************************************************
 * return : Posição do cursor na timeline. 
 *************************************************************************
 * Histórico
 * 17/10/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/

long Project::FrameToTimelinePos(long frame)
{
	// A formula para saber em que ponto plotar o indicador é:
	// SIZE_FRAME_TIMELINE  ---- SIZE_SEC_FRAME*vdo->getFPS()
	// pos (ponto timeline) ---- pos (ponto no video)
	return (SIZE_FRAME_TIMELINE*cvRound(frame)) / (SIZE_SEC_FRAME*cvRound(this->vdo->getFPS()));
}

/*************************************************************************
 * Converte a posicao clicada na timeline para a posicao no frame
 *************************************************************************
 * param (E): long pos -> posição da timeline em que foi clicado. 
 *************************************************************************
 * return : Número do frame correspondente. 
 *************************************************************************
 * Histórico
 * 17/10/08 - Fabrício Lopes de Souza 
 * Criação.
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
 * Histórico
 * 16/10/08 - Fabrício Lopes de Souza
 * Criação.
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
 * Limpa a lista de transições
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Histórico
 * 18/10/08 - Fabrício Lopes de Souza 
 * Criação.
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
 * param (E) : Frame *frame -> Frame em que será aplicado o efeito
 * param (E) : long pos -> posicão do frame no video
 *************************************************************************
 * return : Número do frame correspondente. 
 *************************************************************************
 * Histórico
 * 19/10/08 - Fabrício Lopes de Souza 
 * Criação.
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
 * Histórico
 * 19/10/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/

void Project::clearEffectList()
{
	this->effectList.clear();
}

/*************************************************************************
 * Obtém a transição correspondente ao ponto clicado na timeline e marca
 * esta tomada.
 *************************************************************************
 * param (E): int pos_x => posição do cursor da timeline.
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Histórico
 * 19/10/08 - Fabrício Lopes de Souza 
 * Criação.
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
 * Remove um efeito do vídeo e da lista de efeitos.
 *************************************************************************
 * param (E): int ind => número do efeito.
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Histórico
 * 19/10/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/

void Project::removeEffect(int ind)
{
	Effect *effect = effectList.at(ind);

	delete effect;

	effectList.erase(effectList.begin( ) + ind);

	Interface_ui->updateEffectTree();
}

/*************************************************************************
 * Remove uma transição apontada pelo sistema da lista de transições.
 *************************************************************************
 * param (E): int ind => número do efeito.
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Histórico
 * 19/10/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/

void Project::removeTransition(int ind)
{
	Transition *transition = &transitionList.at(ind);

	delete transition;

	transitionList.erase(transitionList.begin( ) + ind);

	sortTransitionList();
}


/*************************************************************************
 * Get para a variável: path_cy
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : char* - Path do projeto
 *************************************************************************
 * Histórico
 * 19/10/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/
char* Project::getPath()
{
	return path_cy;
}

/*************************************************************************
 * Get para a variável: filename_cy
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : char* - Nome do projeto
 *************************************************************************
 * Histórico
 * 19/10/08 - Fabrício Lopes de Souza 
 * Criação.
 ************************************************************************/
char* Project::getName()
{
	return filename_cy;
}
