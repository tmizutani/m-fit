#include "../ui_Interface.h" 
#include "cv.h"
#include "highgui.h"

#include <algorithm>

#include "QWidget.h"
#include "QDialog.h"
#include <QImage>
#include <QPainter>
#include <QtGui>
#include <QMessageBox>
#include <QPushButton>

#include "../include/DTWindow.h"

#include "../include/TransitionEdit.h"
#include "../include/DetectConfig.h"

#include "../include/Interface.h"
#include "../include/Effect.h"

#include "../include/Transition.h"
#include "../include/Project.h"
#include "../include/Log.h"
#include "../include/Color.h"
#include "../include/Histogram.h"

#include "../include/VideoPlayer.h"

#include "../include/DetectTransitions.h"
#include "../include/Cut.h"
#include "../include/Fade.h"
#include "../include/Dissolve.h"

extern Project *currentProject;
extern VideoPlayer *vdo_player;

/************************************************************************
* Construtor da interface
*************************************************************************
* param (E): QMainWindow -> referencia da janela que criou o objeto
************************************************************************
* Hist�rico
* 09/10/08 - Thiago Mizutani
* Agora utilizo um connect para cada coisa. Um para o player outro para
* o histograma.
* 29/09/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/

Interface::Interface(QMainWindow *parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	memset(recentFiles, '\0', sizeof(recentFiles));

	qRegisterMetaType<Frame>("Frame");

	// Atualiza a imagem do player assim que receber um sinal da thread
	connect(vdo_player, SIGNAL(setNewFrame(QImage *)),
			this, SLOT(updatePlayer(QImage *)));
	
	// Atualiza a imagem do histograma assim que receber um sinal da thread
	connect(vdo_player, SIGNAL(setHistogram(QImage *)),
			this, SLOT(updateHist(QImage *)));

	createRecentFilesActions();
	updateRecentFilesAct();

	// Adiciono o EXIT no final de tudo.
	ui.menuFile->addAction(ui.actionExit);
}

/************************************************************************
* Adiciona um item na lista de arquivos recentes
*************************************************************************
* param (E): QString fileName -> nome do arquivo aberto 
*************************************************************************
* Hist�rico
* 19/10/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/

void Interface::addRecentFile(QString fileName)
{
	QSettings settings("MFIT", "MFIT");
	QStringList files = settings.value("recentFileList").toStringList();
	files.removeAll(fileName); // Remove todos os arquivos recentes com o msm nome
	files.prepend(fileName);
	// Retira os ultimos at� que sobrem somente 4.
	while (files.size() > MAX_RECENT_FILES)
	{
		files.removeLast();
	}

	settings.setValue("recentFileList", files);

}

/************************************************************************
* Atualiza a lista de arquivos recentes
*************************************************************************
* param (E): Nao ha
*************************************************************************
* Hist�rico
* 19/10/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/

void Interface::updateRecentFilesAct()
{
	QSettings settings("MFIT", "MFIT");
	QStringList files = settings.value("recentFileList").toStringList();

	// Verifico o que � menor. O tamanho da lista de arquivos ou o m�ximo permitido.
	int numRecentFiles = qMin(files.size(), (int)MAX_RECENT_FILES);

	// Adiciono um separador.
	ui.menuFile->addSeparator();

	for (int i = 0; i < numRecentFiles; ++i)
	{
		// Monto a string com o n�mero (ordem de abertura) e o nome do arquivo
		QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
		recentFiles[i]->setText(text);
		recentFiles[i]->setData(files[i]);
		recentFiles[i]->setVisible(true);
	}

	// Escondo aqueles que forma acima do 4 arquivo aberto recentemente
	for (int j = numRecentFiles; j < MAX_RECENT_FILES ; ++j)
		recentFiles[j]->setVisible(false);
}

/************************************************************************
* Cria a lista de recent Files
*************************************************************************
* param (E): Nao ha
*************************************************************************
* Hist�rico
* 19/10/08 - Thiago Mizutani
* Inserindo as actions no menu. Por isso n�o estavam aparecendo. b�sico..
* 19/10/08 - Fabricio Lopes de Souza
* Cria��o.
************************************************************************/
void Interface::createRecentFilesActions()
{
	for (int i = 0; i < MAX_RECENT_FILES ; ++i)
	{
		recentFiles[i] = new QAction(this);
		recentFiles[i]->setVisible(false);
		ui.menuFile->addAction(recentFiles[i]);
		connect(recentFiles[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}
}

/************************************************************************
* Trata o evento de clicar em um dos arquivos da lista de "Recent Files".
*************************************************************************
* param (E): Nao ha
*************************************************************************
* Hist�rico
* 19/10/08 - Thiago Mizutani
* Cria��o.
************************************************************************/
void Interface::openRecentFile()
{
	char fileName_cy[100];
	char extension_cy[4];

	memset(fileName_cy,'\0',100);

	QAction *action = qobject_cast<QAction *>(sender());

	if (action)
	{
		strcpy(fileName_cy,action->data().toString());
	}
	else
	{
		return;
	}
	
	// Pego a extens�o do arquivo para verificar se � um projeto ou um v�deo.
	strcpy(extension_cy, &fileName_cy[strlen(fileName_cy)-3]);	

	// Se for um video, uso a funcao de abrir video
	if (!strcmp(extension_cy,"AVI") || !strcmp(extension_cy,"avi"))
	{
		currentProject->openVideo(fileName_cy);
		enableControls();
			
		// Ao carregar o v�deo, devo perguntar ao usu�rio se ele quer fazer a detec��o.
		if( askDetection() )
		{
			DTWindow dtwindow;

			DetectTransitions* DT = new DetectTransitions();

			dtwindow.setDetectThread(DT, TRANSITION_ALL);

			dtwindow.start();

			dtwindow.exec();

			recreateTimeline();

			delete DT;
		}
	}
	else // Senao, abro um projeto
	{
		if (currentProject != 0x0)
		{
			delete currentProject;
			currentProject = new Project();
		}

		currentProject->openProject(fileName_cy);
		updateTransitions();
		updateEffectTree();
		enableControls();
	}
}

/************************************************************************
 * Tratar o evento do bot�o PLAY. Inicia a thread do video player
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Verifica se h� v�deo, se n�o houver mostra alerta
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_playButton_clicked()
{
	// Somente para controle se h� ou n�o v�deo carregado. depois deleta o obj
	Video* vdo = currentProject->getVideo();

	if (!vdo)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	if (!vdo_player->isRunning())
		vdo_player->start();

	return;
}

/************************************************************************
 * Tratar o evento do bot�o PAUSE. Se a thread estiver rodando, mata ela
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Verifica se h� v�deo, se n�o houver mostra alerta
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_pauseButton_clicked()
{
	// Somente para controle se h� ou n�o v�deo carregado. depois deleta o obj
	Video* vdo = currentProject->getVideo();

	if (!vdo)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	if (vdo_player->isRunning())
		vdo_player->terminate();
}

/************************************************************************
 * Trata o bot�o forward. Avan�a o v�deo em um frame.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Hist�rico
 * 20/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_forwardButton_clicked()
{
	Video *vdo;
	unsigned int transitionID = 0x0;
	long pos_l;

	vdo = currentProject->getVideo();

	if (vdo == 0x0)
		return;

	if (vdo->getCurrentPosition() >= vdo->getFramesTotal())
		return;

	vdo->seekFrame((long)vdo->getCurrentPosition() + 1);

	// Vamos mostrar a transicao referente ao ponto que o usuario clicou
	pos_l = currentProject->FrameToTimelinePos((long)vdo->getCurrentPosition());

	transitionID = currentProject->getTransitionByPos((int)pos_l+9);

	if (transitionID >= 0 && transitionID < currentProject->transitionList.size())
	{
		clearTransitionHeader();
		updateTransitionHeader(transitionID);
	}

	vdo_player->updateCurrentFrame();
}

/************************************************************************
 * Trata o bot�o backward. Volta o v�deo em um frame.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Hist�rico
 * 20/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_backButton_clicked()
{
	Video *vdo;
	Frame *frame;
	unsigned int transitionID;
	long pos_l;

	vdo = currentProject->getVideo();

	if (vdo == 0x0)
		return;

	if (vdo->getCurrentPosition() >= vdo->getFramesTotal())
		return;

	frame = vdo->getPreviousFrame();

	// Vamos mostrar a transicao referente ao ponto que o usuario clicou
	pos_l = currentProject->FrameToTimelinePos((long)vdo->getCurrentPosition());

	transitionID = currentProject->getTransitionByPos((int)pos_l+9);

	if (transitionID >= 0 && transitionID < currentProject->transitionList.size())
	{
		clearTransitionHeader();
		updateTransitionHeader(transitionID);
	}

	vdo_player->updatePlayer(frame);
	vdo_player->updateHist(frame);

	delete frame;
}

/************************************************************************
 * Tratar o evento do bot�o STOP. Se a thread estiver rodando, mata ela,
 * e posiciona o ponteiro do video para seu inicio.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Verifica se h� v�deo, se n�o houver mostra alerta
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_stopButton_clicked()
{
	Video *vdo;

	// Independente de estar rodando ou nao
	// seek no frame 0
	vdo = currentProject->getVideo();

	if (!vdo)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	vdo->seekFrame(0);

	if (vdo_player->isRunning())
		vdo_player->terminate();

}

/************************************************************************
 * Tratar o evento do bot�o OpenProject.
 * TODO: Ler o XML e subir os membros da estrutura Project.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 10/10/08 - Ivan Shiguenori Machida
 * Cria��o.
 ************************************************************************/
void Interface::on_actionOpenProject_triggered()
{
	// Segundo  parametro - Mensagem que ira aparecer no topo da imagem
	// Terceiro parametro - Diretorio inicial
	// Quarto   parametro - Mensagem q ira aparecer la embaixo, e as extens�es suportadas

	if (currentProject != 0x0)
	{
		delete currentProject;
		currentProject = new Project();
	}

	if (vdo_player != 0x0)
	{
		delete vdo_player;
		vdo_player = new VideoPlayer();

		// Atualiza a imagem do player assim que receber um sinal da thread
		connect(vdo_player, SIGNAL(setNewFrame(QImage *)),
				this, SLOT(updatePlayer(QImage *)));

		// Atualiza a imagem do histograma assim que receber um sinal da thread
		connect(vdo_player, SIGNAL(setHistogram(QImage *)),
				this, SLOT(updateHist(QImage *)));
	}

	QString fileName = QFileDialog::getOpenFileName(this,
			"Abrir Projeto",
			".",
			"Projeto MFIT (*.MFIT)");

	if (!fileName.isEmpty())
	{
		currentProject->openProject(fileName);
		updateTransitions();
		updateEffectTree();
		enableControls();
	}
	else
	{
		return;
	}
}

/************************************************************************
 * Tratar o evento do bot�o LoadVideo.
 * Abre o objeto Video, atualiza a videoProperties
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani
 * Pergunta para usu�rio se deseja detectar todas as transi��es ap�s o
 * carregamento do v�deo.
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_actionLoadVideo_triggered()
{
	// Segundo  parametro - Mensagem que ira aparecer no topo da imagem
	// Terceiro parametro - Diretorio inicial
	// Quarto   parametro - Mensagem q ira aparecer la embaixo, e as extens�es suportadas
	QString fileName = QFileDialog::getOpenFileName(this,
			"Abrir V�deo",
			".",
			"V�deos Suportados (*.avi)");

	if (!fileName.isEmpty())
	{
		// Se ocorrer problema na abertura
		try
		{
			currentProject->openVideo(fileName);
		}
		catch (...) // Alerta o usuario
		{
			alertUser(ALERT_VIDEO_INCOMPATIBLE);
			return;
		}

		// Habilita os botoes
		enableControls();

		if (askDetection()) // Gero a box perguntando se deseja detectar as transi��es
		{  
			DTWindow dtwindow;

			DetectTransitions* DT = new DetectTransitions();

			dtwindow.setDetectThread(DT, TRANSITION_ALL);

			dtwindow.start();

			dtwindow.exec();

			recreateTimeline();

			delete DT;

		}
	}
	else
	{
		return;
	}
}

/************************************************************************
 * Tratar o evento do bot�o SaveAs.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_actionSaveAs_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this,
			"Salvar Projeto",
			".",
			"Projeto MFIT (*.MFIT)");

	if (!fileName.isEmpty())
	{
		currentProject->saveProject(fileName);

		this->setWindowTitle(currentProject->getName());

		return;
	}
}

/************************************************************************
 * Tratar o evento do bot�o Save.
 * TODO: Nao faz porra nenhuma pq nao temos o ParserXML
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 10/10/08 - Ivan Shiguenori Machida
 * Cria��o.
 ************************************************************************/
void Interface::on_actionSave_triggered()
{
	QString fileName;

	// Se nao for o projeto padrao aberto, usa este nome
	if (strcmp(currentProject->getName(), DEFAULT_PROJECT_NAME))
	{
		fileName = QString(currentProject->getName());
	}
	else // Se nao, pergunta o nome pro usuario
	{
		fileName = QFileDialog::getSaveFileName(this,
				"Salvar Projeto",
				".",
				"Projeto MFIT (*.MFIT)");
	}

	if (!fileName.isEmpty())
	{
		currentProject->saveProject(fileName);

		this->setWindowTitle(currentProject->getName());
	}
}

/************************************************************************
 * Altera o Title da janela principal.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::changeWindowTitle(char *string)
{
	// Se tiver algum caracter na string
	if (string[0])
	{
		QString title(string);

		// Preenche o titulo
		this->setWindowTitle(string);
	}
	else
	{
		return;
	}
}

/************************************************************************
 * Limpa a lista de propriedades do video (videoPropertiesTree)
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::clearVideoProperty()
{
	Log::writeLog("%s :: clear videoPropertiesTree", __FUNCTION__); 
	this->ui.videoPropertiesTree->clear();
}

/************************************************************************
 * Insere uma linha na lista de propriedades do v�deo.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::insertVideoProperty(char *param_cy, char *value_cy)
{
	// Cria o item referente a Tree
	QTreeWidgetItem *item = new QTreeWidgetItem(this->ui.videoPropertiesTree);

	// Cria a lista de items
	QList<QTreeWidgetItem *> itens;

	// Adiciona as colunas
	item->setText(0, param_cy);
	item->setText(1, value_cy);

	itens.append(item);

	this->ui.videoPropertiesTree->insertTopLevelItems(0, itens);
}

/************************************************************************
 * Atualiza o frame exibido na label do player
 *************************************************************************
 * param (E): QImage *image -> Imagem a ser impressa no video player
 *************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani
 * Revis�o - alterado o nome da fun��o para diminuir o nro de funcoes
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::updatePlayer(QImage *image)
{
	// Trava a thread do video_player
	vdo_player->mutex.lock();

	Video *vdo = currentProject->getVideo();

	setVideoTime(vdo->getCurrentPosition(), vdo->getFPS());

	QPixmap pix_image = QPixmap::fromImage(*image);

	// Permite ajute da imagem em rela��o ao tamanho da janela
	ui.videoLabel->setScaledContents(true);
	ui.videoLabel->setPixmap(pix_image); // Pinta a imagem

	updateTimeline();

	vdo_player->mutex.unlock();

	delete image;
}

/************************************************************************
 * Atualiza o histograma da widgetHistogram.
 *************************************************************************
 * param (E): QImage *hist-> Novo histograma 
 *************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani
 * Revis�o - alterado o nome da fun��o para diminuir o nro de funcoes e
 * e inser��o dos coment�rios
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/

void Interface::updateHist(QImage *hist)
{
	vdo_player->mutex.lock();

	QPixmap pix_image = QPixmap::fromImage(*hist);

	// Permite ajute da imagem em rela��o ao tamanho da janela
	ui.histogramLabel->setScaledContents(true);
	ui.histogramLabel->setPixmap(pix_image); // Pinta a imagem

	delete hist;

	vdo_player->mutex.unlock();
}

/************************************************************************
 * Seta a posicao do Video em HH:MM:SS na videoTime
 *************************************************************************
 * param (E): QImage *hist-> Novo histograma 
 *************************************************************************
 * Hist�rico
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::setVideoTime(double framePos, double fps)
{
	unsigned long msec;

	msec = cvRound((framePos / fps ) * 1000 );

	const QTime *time = new QTime(
			(int)( msec /3600000),     // Hora
			(int)((msec /60000) % 60), // Minuto
			(int)((msec /1000)  % 60), // Segundo
			(int)( msec % 1000));      // MileSegundo

	ui.videoTime->setTime(*time) ;

	delete time;

}

/************************************************************************
 * Cria a timeline do v�deo carregado.
 *************************************************************************
 * param (E): N�o h�
 *************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani 
 * Revis�o de c�digo. E inser��o de coment�rios
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::createTimeline(void)
{
	Video *vdo = 0x0;
	vdo = currentProject->getVideo();

	int iter_i = 0x0; // Passo da timeline (qtos frames eu devo pular para plotar o pr�ximo na timeline)
	Frame *frame = 0x0;
	Frame *frameResized = 0x0;
	Frame *frameHeader = 0x0;
	IplImage *imgHeader = 0x0;
	int i = 0;

	// Timeline sera 10x menor que a imagem
	int width =  SIZE_FRAME_TIMELINE;
	int height = SIZE_FRAME_TIMELINE;

	// Vamos pegar de 1 em 1 segundo
	int sec_i = SIZE_SEC_FRAME;

	iter_i = cvRound(sec_i * vdo->getFPS());

	vdo->seekFrame(0);

	frame = vdo->getNextFrame();	
	vdo_player->frameTimeline = frame->resize(width, height);

	delete frame;

	for (i = iter_i ; i < vdo->getFramesTotal() ; i += iter_i)
	{
		vdo->seekFrame(i);

		frame = vdo->getNextFrame();
		frameResized = frame->resize(width, height);

		if (i+iter_i >= vdo->getFramesTotal())
		{
			// O ultimo frame tem que ser proporcional a quantidade de 
			// frames restantes no video
			delete frameResized;
			int posFrame;

			posFrame = iter_i*(cvRound(vdo->getFramesTotal())-i) / SIZE_FRAME_TIMELINE;

			frameResized = frame->resize(posFrame, height);
		}

		*vdo_player->frameTimeline += *frameResized;

		delete frame;
		delete frameResized;
	}

	// Coloca o header da timeline
	imgHeader = Frame::imgAlloc(
			cvSize(vdo_player->frameTimeline->getWidth(), vdo_player->frameTimeline->getHeight() / 2 ),
			vdo_player->frameTimeline->data->depth,
			vdo_player->frameTimeline->data->nChannels);

	frameHeader = new Frame(imgHeader);

	Frame::imgDealloc(imgHeader);

	cvFillImage(frameHeader->data, 0xF4F4F4); // Preenche da cor de fundo das janelas

	frameResized = vdo_player->frameTimeline->verticalCat(frameHeader);

	// Vamos desenha o contorno da timeline

	delete vdo_player->frameTimeline;

	vdo_player->frameTimeline = frameResized;
	vdo_player->frameTimelineOriginal = new Frame(vdo_player->frameTimeline);

	this->setTimeline(vdo_player->frameTimeline);

	vdo->seekFrame(0);
}

/************************************************************************
 * Printa a imagem da timeline em sua label.
 *************************************************************************
 * param (E): N�o h�
 *************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani 
 * Revis�o de c�digo. E inser��o de coment�rios
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::setTimeline(Frame *frameTimeline)
{
	QImage *image;

	// Converte a imagem
	image = frameTimeline->IplImageToQImage(&vdo_player->timelineData, &vdo_player->timelineWidth, &vdo_player->timelineHeight);

	QPixmap pix_image = QPixmap::fromImage(*image);

	ui.timelineLabel->setScaledContents(false); // N�o permite que a imagem sofra distor��es conforme o tamanho da janela
	ui.timelineLabel->setPixmap(pix_image); // Pinta a timeline na labelTimeline

	delete image;

}

/************************************************************************
 * Restaura a TimeLine original e repinta as transi��es
 *************************************************************************
 * param (E): N�o h�
 *************************************************************************
 * 09/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::recreateTimeline()
{
	Frame::imgCopy(vdo_player->frameTimelineOriginal->data, vdo_player->frameTimeline->data);
	updateTransitions();
}

/*************************************************************************
 * Atualiza a posi��o do cursor da timeline conforme a passagem do v�deo
 *************************************************************************
 * param (E): N�o h�
 *************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani 
 * Revis�o de c�digo. E inser��o de coment�rios
 * 29/09/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::updateTimeline()
{
	Video *vdo = 0x0;
	long pos = 0x0;
	int line_point = 0x0;

	vdo = currentProject->getVideo();
	pos = (long)vdo->getCurrentPosition();

	// A formula para saber em que ponto plotar o indicador �:
	// SIZE_FRAME_TIMELINE  ---- SIZE_SEC_FRAME*vdo->getFPS()
	// pos (ponto timeline) ---- pos (ponto no video)
	//line_point  = (SIZE_FRAME_TIMELINE*cvRound(pos)) / (SIZE_SEC_FRAME*cvRound(vdo->getFPS()));
	line_point = currentProject->FrameToTimelinePos(pos);

	if (vdo_player->frameTimelineEdited != 0x0)
		delete vdo_player->frameTimelineEdited;

	CvPoint p1 = {line_point,0};
	CvPoint p2 = {line_point,SIZE_FRAME_TIMELINE+10};

	vdo_player->frameTimelineEdited = new Frame(vdo_player->frameTimeline);

	cvLine(vdo_player->frameTimelineEdited->data, p1, p2, cvScalar(0,0,0), 1);

	setTimeline(vdo_player->frameTimelineEdited);
}

/************************************************************************
 * Trata o evento gerado pelo bot�o para detec��o de todas as transi��es
 *************************************************************************
 * param (E): N�o h�
 * return : n�o h�
 *************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Verifica se h� v�deo, se n�o houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/
void Interface::on_actionAllTransitions_triggered()
{

	Video* vdo = 0x0;
	long posFrame = 0x0;
	DetectTransitions* DT;
	DTWindow dtwindow;

	vdo = currentProject->getVideo();

	if (vdo == 0x0)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	DT = new DetectTransitions();

	posFrame = (long)vdo->getCurrentPosition();

	vdo->seekFrame(0);

	dtwindow.setDetectThread(DT, TRANSITION_ALL);

	dtwindow.start();

	dtwindow.exec();

	vdo->seekFrame(posFrame);

	delete DT;

	recreateTimeline();

}

/************************************************************************
 * Trata o evento gerado pelo bot�o para detec��o de todos os cortes
 *************************************************************************
 * param (E): N�o h�
 * return : n�o h�
 *************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Verifica se h� v�deo, se n�o houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/
void Interface::on_actionOnlyCuts_triggered()
{

	Video* vdo = 0x0;
	long posFrame = 0x0;
	Cut* DTC = 0x0;
	DTWindow dtwindow;

	vdo = currentProject->getVideo();

	if (vdo == 0x0)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	DTC = new Cut();

	posFrame = (long)vdo->getCurrentPosition();

	vdo->seekFrame(0);

	dtwindow.setDetectThread(DTC, TRANSITION_CUT);

	dtwindow.start();

	dtwindow.exec();

	vdo->seekFrame(posFrame);

	delete DTC;

	recreateTimeline();

}

/************************************************************************
 * Trata o evento gerado pelo bot�o para detec��o de todos os fades
 *************************************************************************
 * param (E): N�o h�
 * return : n�o h�
 *************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Verifica se h� v�deo, se n�o houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/
void Interface::on_actionAllFades_triggered()
{

	Video* vdo = 0x0;
	long posFrame = 0x0;
	Fade* DTF = 0x0;
	DTWindow dtwindow;

	vdo = currentProject->getVideo();

	if (vdo == 0x0)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	DTF = new Fade();

	posFrame = (long)vdo->getCurrentPosition();

	vdo->seekFrame(0);

	dtwindow.setDetectThread(DTF, TRANSITION_FADE);

	dtwindow.start();

	dtwindow.exec();

	vdo->seekFrame(posFrame);

	delete DTF;

	recreateTimeline();

}

/*************************************************************************
 * Trata o evento gerado pelo bot�o para detec��o de todos os dissolve
 *************************************************************************
 * param (E): N�o h�
 **************************************************************************
 * return : n�o h�
 *************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Verifica se h� v�deo, se n�o houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/
void Interface::on_actionOnlyDissolve_triggered()
{

	Video* vdo = 0x0;
	long posFrame = 0x0;
	Dissolve* DTD = 0x0;
	DTWindow dtwindow;

	vdo = currentProject->getVideo();

	if (vdo == 0x0)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	DTD = new Dissolve();

	posFrame = (long)vdo->getCurrentPosition();

	vdo->seekFrame(0);

	dtwindow.setDetectThread(DTD, TRANSITION_DISSOLVE);

	dtwindow.start();

	dtwindow.exec();

	vdo->seekFrame(posFrame);

	delete DTD;

	recreateTimeline();

}

/**************************************************************************
 * Adiciona uma transi��o na transitionsTree
 **************************************************************************
 * param (E): Transition* transition -> transi��o a ser inserida na �rovore 
 **************************************************************************
 * return : N�o h�
 **************************************************************************
 * Hist�rico
 * 08/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::insertTransitionsTree(Transition* transition, long id_l)
{
	// Cria o item referente a Tree
	QTreeWidgetItem *item = new QTreeWidgetItem(this->ui.transitionsTree);

	// Cria a lista de items
	QList<QTreeWidgetItem *> itens;

	long pos_l = transition->getPosCurrent(); // Posi��o da transi��o detectada

	char pos_cy[10];
	char id_cy[20];

	sprintf(pos_cy, "%ld", pos_l);
	sprintf(id_cy, "%3.3ld", id_l);

	// Primeira Coluna - ID
	item->setText(0, id_cy);

	// Segunda Coluna - NOME
	item->setText(1, transition->getLabel());

	// Terceira coluna - POSICAO
	item->setText(2, pos_cy);

	itens.append(item);

	this->ui.transitionsTree->insertTopLevelItems(0, itens);
}

/**************************************************************************
 * Marca na timeline os pontos onde ocorrem as transi��es
 **************************************************************************
 * param (E): Transition* transition -> Transi��o a ser marcada na timeline 
 **************************************************************************
 * return : N�o h�
 **************************************************************************
 * Hist�rico
 * 08/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::insertTransitionsTimeline(Transition* transition)
{
	long posTransition_l = 0;
	long posTimeline_l = 0;

	// Pega o ponto da transicao
	posTransition_l = transition->getPosCurrent();

	// A formula para saber em que ponto plotar o indicador �:
	// SIZE_FRAME_TIMELINE  ---- SIZE_SEC_FRAME*vdo->getFPS()
	// pos (ponto timeline) ---- pos (ponto no video)

	//posTimeline_l = (SIZE_FRAME_TIMELINE*cvRound(posTransition_l)) / (SIZE_SEC_FRAME*cvRound(vdo->getFPS()));
	posTimeline_l = currentProject->FrameToTimelinePos(posTransition_l);

	CvPoint p1 = {posTimeline_l,0}; // Ponto inicial
	CvPoint p2 = {posTimeline_l,SIZE_FRAME_TIMELINE+10}; // Ponto final da reta + 10 (pois deve ser maior que a timeline)

	// Sempre tem que ser aplicado ao frameTimeline
	switch (transition->getType())
	{
		case TRANSITION_CUT:
			cvLine(vdo_player->frameTimeline->data, p1, p2, cvScalar(255,0,0), 2);
			break;
		case TRANSITION_FADEIN:
			cvLine(vdo_player->frameTimeline->data, p1, p2, cvScalar(255,255,0), 2);
			break;
		case TRANSITION_FADEOUT:
			cvLine(vdo_player->frameTimeline->data, p1, p2, cvScalar(0,0,255), 2);
			break;
		case TRANSITION_DISSOLVE:
			cvLine(vdo_player->frameTimeline->data, p1, p2, cvScalar(0,255,0), 2);
			break;
		default: // In�cio do v�deo.
			cvLine(vdo_player->frameTimeline->data, p1, p2, cvScalar(0,0,0), 2);
			break;
	}

	setTimeline(vdo_player->frameTimeline);

}

/*************************************************************************
 * Atualiza a lista de transi��es e marca pontos de transi��o na timeline
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : N�o h�
 *************************************************************************
 * Hist�rico
 * 08/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/
void Interface::updateTransitions()
{
	unsigned long i = 0;

	currentProject->sortTransitionList();

	clearTransitionsTree();

	Log::writeLog("%s :: updating Transitions", __FUNCTION__);

	for (i = 0 ; i < currentProject->transitionList.size() ; i ++)
	{
		insertTransitionsTree(&currentProject->transitionList.at(i), i);
		insertTransitionsTimeline(&currentProject->transitionList.at(i));
	}

	clearTransitionHeader();
	updateTimeline();

	enableSaveButton();

}

/*************************************************************************
 * Limpa a lista de transi��es - transitionsTree
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : N�o h�
 *************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/
void Interface::clearTransitionsTree()
{
	Log::writeLog("%s :: clear transitionsTree", __FUNCTION__); 
	this->ui.transitionsTree->clear();
}

/*************************************************************************
 * Trata as sele��es da lista de transi��o e manda pintar uma linha no 
 * header da timeline identificando a sua tomada. 
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : N�o h�
 *************************************************************************
 * Hist�rico
 * 16/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::on_transitionsTree_itemSelectionChanged()
{
	QList<QTreeWidgetItem *> itens;
	itens = this->ui.transitionsTree->selectedItems();

	QTreeWidgetItem *item;

	clearTransitionHeader();

	foreach(item, itens)
	{
		updateTransitionHeader(item);
	}

	updateTimeline();
}

/*************************************************************************
 * Tra�a uma linha no header da timeline referente ao item selecionado
 * na lista de transi��es.
 *************************************************************************
 * param (E): QTreeWidgetItem * item -> item selecionado 
 *************************************************************************
 * return : N�o h�
 *************************************************************************
 * Hist�rico
 * 16/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Interface::updateTransitionHeader(QTreeWidgetItem * item)
{
	char str_cy[20];
	long idx_i;
	QString text = item->text(0);

	memset(str_cy, '\0', sizeof(str_cy));

	QStringToChar(text, str_cy);

	idx_i = atoi(str_cy);

	updateTransitionHeader(idx_i);
}

/**************************************************************************
 * Cria uma caixa de di�logo perguntando ao usu�rio se este deseja realizar
 * a detec��o de transi��es logo ap�s o carregamento do v�deo.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * return : N�o h�
 **************************************************************************
 * Hist�rico
 * 09/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
int Interface::askDetection()
{
	int reply = 0; // Resposta do usu�rio

	// Crio uma box com o �cone "?"
	reply = QMessageBox::question(
			this,
			tr("MFIT"),
			tr("<p><b>Detectar todas as transi��es agora?</b>" \
				"<p>Este processo poder� levar alguns minutos."),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No
			);

	// Se clickei em detectNow, chamo a detec��o de transi��es
	if (reply == QMessageBox::Yes)
		return (TRUE);
	else
		return (FALSE);

}

/**************************************************************************
 * Fun��o que mostra uma mensagem de alerta caso o usu�io tente fazer 
 * qualquer opera��o que exija v�deo e o v�deo n�o estiver carregado.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * return : N�o h�
 **************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::alertUser(int message)
{

	char message_cy[200];

	switch(message)
	{
		case ALERT_NO_LOADED_VIDEO:
			strcpy(message_cy ,"<p><b>N�o � poss�vel fazer a detec��o de transi��es!</b><p>Carregue um v�deo antes de executar esta opera��o!");
		case ALERT_VIDEO_INCOMPATIBLE:
			strcpy(message_cy, "<p><b>N�o � poss�vel carregar o v�deo!</b><p>Formato n�o suportado!");
	}

	QMessageBox::critical(
			this,
			tr("MFIT - ERRO"),
			tr(message_cy),
			QMessageBox::Ok
			);

}

/**************************************************************************
 * Habilita todos os bot�es de controle quando carregar o v�deo.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * return : N�o h�
 **************************************************************************
 * Hist�rico
 * 13/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::enableControls()
{
	// Controles gerais
	this->ui.actionSave->setEnabled(true);
	this->ui.actionSaveAs->setEnabled(true);

	// Controles do player
	this->ui.playButton->setEnabled(true);
	this->ui.pauseButton->setEnabled(true);
	this->ui.backButton->setEnabled(true);
	this->ui.forwardButton->setEnabled(true);
	this->ui.stopButton->setEnabled(true);

	// Botao de renderizacao
	this->ui.actionRenderVideo->setEnabled(true);

	// Controles de detec��o
	this->ui.actionAllTransitions->setEnabled(true);
	this->ui.actionOnlyCuts->setEnabled(true);
	this->ui.actionAllFades->setEnabled(true);
	this->ui.actionOnlyDissolve->setEnabled(true);

	// Lista de efeitos
	this->ui.effectListTree->setEnabled(true);

}

/**************************************************************************
 * Trata evento do click sobre o menu
 **************************************************************************
 * param (E): N�o h�.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 18/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::on_actionDetectConfig_triggered()
{
	showDetectionConfigs();
}

/**************************************************************************
 * Transforma uma QString em um char*
 **************************************************************************
 * param (E): QString *string    - String do tipo QSTRING
 * param (S): char    *string_cy - String convertida para char*
 **************************************************************************
 * return : char* - String convertida
 **************************************************************************
 * Hist�rico
 * 14/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::on_actionRenderVideo_triggered()
{

	Video *vdo = currentProject->getVideo();

	if (vdo == 0x0)
	{
		alertUser(ALERT_NO_LOADED_VIDEO);
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this,
			"Renderizar Video",
			".",
			"Videos Suportados (*.avi)");

	if (!fileName.isEmpty())
	{
		char filename_cy[100];

		QStringToChar(fileName, filename_cy);

		currentProject->renderVideo(filename_cy);
	}
}

/**************************************************************************
 * Abre a janela de configura��es para detec��o de transi��es.
 **************************************************************************
 * param (E): N�o h�.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 18/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::showDetectionConfigs()
{

	DetectConfig configWindow;

	configWindow.exec();

}

/**************************************************************************
 * Transforma uma QString em um char*
 **************************************************************************
 * param (E): QString *string    - String do tipo QSTRING
 * param (S): char    *string_cy - String convertida para char*
 **************************************************************************
 * return : char* - String convertida
 **************************************************************************
 * Hist�rico
 * 14/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/

char *Interface::QStringToChar(QString string, char* string_cy)
{
	sprintf(string_cy, "%s",string.toAscii().data());

	return string_cy;
}

/**************************************************************************
 * Tra�a uma linha acima da timeline identificando a localiza��o da tomada
 **************************************************************************
 * param (E): unsigned int transitionID - Transi��o selecionada na lista.
 * param (E): int clean - flag que indica se deve apagar o tra�o ou n�o.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 14/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::updateTransitionHeader(unsigned int transitionID, int clean)
{
	long pos_l;
	long posNext_l;

	unsigned int posTimeline;
	unsigned	int posTimelineNext; 

	char frame_cy[50];

	CvFont font;

	CvScalar scalar;

	Transition *transition;
	Transition *transitionNEXT;

	if (currentProject == 0x0)
		return;

	if (currentProject->getVideo() == 0x0)
		return;

	memset(frame_cy, '\0', sizeof(frame_cy));

	transition = &currentProject->transitionList.at(transitionID);

	pos_l = transition->getPosCurrent();

	// Se a transicao escolhida for a ultima
	// Utilizamos o frame final do video como marco de fim da transi��o
	if (transitionID == (unsigned int)currentProject->transitionList.size()-1)
	{
		Video *vdo = 0x0;

		vdo = currentProject->getVideo();

		posNext_l = (long)vdo->getFramesTotal();
	}
	else // Sen�o, utlizamos o inicio da proxima transi��o
	{
		transitionNEXT = &currentProject->transitionList.at(transitionID+1);
		posNext_l = transitionNEXT->getPosCurrent();
	}

	// Printa uma linha de pos ate posNext
	posTimeline     = currentProject->FrameToTimelinePos(pos_l);
	posTimelineNext = currentProject->FrameToTimelinePos(posNext_l);

	CvPoint p1 = {posTimeline    ,SIZE_FRAME_TIMELINE+15};
	CvPoint p2 = {posTimelineNext,SIZE_FRAME_TIMELINE+15};
	CvPoint p3 = {posTimeline+4  ,SIZE_FRAME_TIMELINE+20};

	if (clean == 1)
	{  // Apaga na timelie
		scalar.val[0] = 244;
		scalar.val[1] = 244;
		scalar.val[2] = 244;
		transition->selected = false;
	}
	else
	{  // Imprime na timeline
		scalar.val[0] = 50;
		scalar.val[1] = 30;
		scalar.val[2] = 20;
		transition->selected = true;
	}

	// Escreve o ID e o nome da transi��o na timeline
	sprintf(frame_cy, "%.3d-%s", transitionID, transition->getLabel());
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3f, 0.3f, 0, 0, 0);
	cvPutText(vdo_player->frameTimeline->data, frame_cy, p3, &font, scalar);

	cvLine(vdo_player->frameTimeline->data, p1, p2, scalar, 3);

}

/**************************************************************************
 * Varre todas as transi��es e retira uma a uma do header da timeline.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 14/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::clearTransitionHeader()
{
	unsigned int i = 0;

	if (currentProject == 0x0)
		return;

	Transition *transition;
	for (i = 0 ; i < currentProject->transitionList.size() ; i ++)
	{
		transition = &currentProject->transitionList.at(i);

		updateTransitionHeader(i, 1);
	}
}

/**************************************************************************
 * Mostra no player o primeiro frame da transi��o selecionada na lista de 
 * transi��es e atualiza o cursor da timeline.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 20/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::on_transitionsTree_itemClicked(QTreeWidgetItem* item, int column)
{
	char id_cy[10];
	unsigned int id;
	long pos_l = 0;
	long timeline_l;

	Transition *transition;

	Video* vdo = currentProject->getVideo();

	QStringToChar(item->text(0), id_cy); // Pego o ID da transi��o

	id = atoi(id_cy);

	transition = &currentProject->transitionList.at(id);

	// Pega a posicao da transicao
	pos_l = transition->getPosCurrent();

	if (pos_l >= (long)vdo->getFramesTotal())
	{
		pos_l = (long)vdo->getFramesTotal() - 1;
	}

	vdo->seekFrame(pos_l); // Aponto para o frame de interesse.

	vdo_player->updateCurrentFrame(); // Atualiza o frame corrente
	updateTimeline(); // Atualizo a posi��o do cursor da timeline

	timeline_l = currentProject->FrameToTimelinePos(pos_l);

	// Move o scroll
	moveScrollArea((int)timeline_l, 0);

}

/**************************************************************************
 * Trata o evento de DoubleClick em um item na lista de transi��es.
 * Abre a janela de edi��o da transi��o.
 * Apos a janela fechar, trata as op��es escolhidas
 **************************************************************************
 * param (E): QTreeWidgetItem* - Item clicado
 * param (E): int - Coluna clicada
 **************************************************************************
 * Hist�rico
 * 09/11/08 - Fabr�cio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::on_transitionsTree_itemDoubleClicked ( QTreeWidgetItem * item, int column )
{

	char id_cy[10];
	unsigned int idx_i;
	long pos_l;
	Video *vdo;
	TransitionEdit editWindow;

	QStringToChar(item->text(0), id_cy); // Pego a posi��o da transi��o determinada pelo sistema 
	idx_i = atoi(id_cy);

	// N�o se pode editar o "Inicio de Video"
	if (idx_i == 0)
		return;

	// Define a transi��o que sera editada
	editWindow.setID(idx_i);

	// Salva a posi��o corrente do video
	vdo = currentProject->getVideo();
	pos_l = (long)vdo->getCurrentPosition();

	// Abre a janela
	editWindow.exec();

	// Restaura a posi��o do video
	vdo->seekFrame(pos_l);

	// Se cancelou, n�o faz nada
	if (editWindow.cancel)
	{
		return;
	} // Se selecionou o CHANGE, seta a posi��o da transi��o como a posi��o desejada
	else if (editWindow.change)
	{
		Transition *transition;

		transition = &currentProject->transitionList.at(idx_i);
		transition->setPosUserTransition(editWindow.pos_l);

	} // Se deseja remover a transi��o, remove :)
	else if (editWindow.del)
	{
		currentProject->removeTransition(idx_i);
	}
	else
	{
		return;
	}

	// Recria a timeline, para absorver as altera��es
	recreateTimeline();

	return;

}

/**************************************************************************
 * Insere um item na lista de efeitos do v�deo
 **************************************************************************
 * param (E): Effect *effect -> efeito a ser inserido.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 19/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::insertEffectTree(Effect *effect, int ind)
{
	char frameStart[10];
	char frameEnd[10];
	char id[10];

	// Cria o item referente a Tree
	QTreeWidgetItem *item = new QTreeWidgetItem(this->ui.effectsTree);

	// Cria a lista de items
	QList<QTreeWidgetItem *> itens;

	sprintf(frameStart, "%ld"  , effect->getFrameStart());
	sprintf(frameEnd  , "%ld"  , effect->getFrameEnd());
	sprintf(id        , "%3.3d", ind);

	// Primeira Coluna - ID do efeito
	item->setText(0, id);

	// Segunda Coluna - Nome do efeito
	item->setText(1, effect->getName());

	// Terceira coluna - Frame de start
	item->setText(2, frameStart);

	// Quarta Coluna - Frame final
	item->setText(3, frameEnd);

	itens.append(item);

	this->ui.effectsTree->insertTopLevelItems(0, itens);
}

/**************************************************************************
 * Trata evento de quando � selecionado um item na lista de efeitos.
 **************************************************************************
 * param (E): QTreeWidgetItem *item -> item selecionado (linha)
 * param (E): int column -> coluna selecionada (frame final ou inicial)
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 19/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::on_effectsTree_itemClicked(QTreeWidgetItem * item, int column)
{
	if (column > 0)
	{
		char framepos_cy[10];
		long framepos_l;
		Video *vdo;

		QStringToChar(item->text(column), framepos_cy);

		framepos_l = atol(framepos_cy);

		vdo = currentProject->getVideo();

		if (framepos_l >= vdo->getFramesTotal())
			framepos_l = (long)vdo->getFramesTotal() - 1;

		vdo->seekFrame(framepos_l);

		vdo_player->updateCurrentFrame();

	}
	else // Se clicou na coluna do nome do efeito
	{
		return;
	}
}

/**************************************************************************
 * Atualiza a lista de efeitos. 
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 19/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::updateEffectTree()
{
	unsigned int i = 0;

	effectTreeClear();

	for ( i = 0 ; i < currentProject->effectList.size() ; i++)
	{
		insertEffectTree(currentProject->effectList.at(i), i);
	}
}

/**************************************************************************
 * Limpa a lista de efeitos.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 19/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::effectTreeClear()
{
	this->ui.effectsTree->clear();
}

/**************************************************************************
 * Fecha a aplica��o.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : N�o h�.
 **************************************************************************
 * Hist�rico
 * 20/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::on_actionExit_triggered()
{
	if (currentProject->getVideo())
	{
		if (askUserSave())
		{
			QString fileName = QFileDialog::getSaveFileName(this,
					"Salvar Projeto",
					".",
					"Projeto MFIT(*.MFIT)");

			if (!fileName.isEmpty())
			{
				currentProject->saveProject(fileName);

				this->setWindowTitle(fileName.right(fileName.length() - fileName.lastIndexOf("/") - 1));

				QMainWindow::close();	
			}
		}
	}

	QMainWindow::close();
}

/**************************************************************************
 * Pergunta se o usu�rio deseja salvar o projeto antes de sair.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : TRUE - Salva e sai.
 * 			FALSE - Sai sem sair.
 **************************************************************************
 * Hist�rico
 * 20/10/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
bool Interface::askUserSave()
{
	int reply = 0; // Resposta do usu�rio

	// Crio uma box com o �cone "?"
	reply = QMessageBox::question(
			this,
			tr("MFIT"),
			tr("<p>Deseja salvar o projeto antes de sair?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No
			);

	// Se clickei em detectNow, chamo a detec��o de transi��es
	if (reply == QMessageBox::Yes)
		return (TRUE);
	else
		return (FALSE);
}

/**************************************************************************
 * Movimenta o SCROLL, posiciona a transi��o selecionada no centro da timeline
 **************************************************************************
 * param (E): x - Posicao X
 * param (E): y - Posicao Y
 **************************************************************************
 * return : Nenhum
 **************************************************************************
 * Hist�rico
 * 20/10/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::moveScrollArea(int x, int y)
{
	QRect rect(ui.widgetDockTimeline->geometry());
	int center_i;

	center_i = (int)(rect.width() / 2);

	ui.scrollArea->ensureVisible(x, y, center_i, center_i);
}


/**************************************************************************
 * Desabilita o bot�o de Save
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * Hist�rico
 * 09/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::disableSaveButton()
{
	this->ui.actionSave->setEnabled(false);
}

/**************************************************************************
 * Habilita o bot�o de Save
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * Hist�rico
 * 09/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::enableSaveButton()
{
	this->ui.actionSave->setEnabled(true);
}

/**************************************************************************
 * Trata o evento de close da aplica��o.
 * Fecha/libera todos os objetos abertos/instanciados.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * Hist�rico
 * 10/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 *************************************************************************/
void Interface::closeEvent(QCloseEvent* event)
{
	if (currentProject != 0x0)
	{
		delete currentProject;
	}

	if (vdo_player != 0x0)
	{
		delete vdo_player;
	}
}

/**************************************************************************
 * Abre novo projeto
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * Hist�rico
 * 16/11/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/
void Interface::on_actionNewProject_triggered()
{
	clearTransitionHeader();
	clearVideoProperty();
	clearTransitionsTree();	
	effectTreeClear();
	ui.timelineLabel->clear();
	ui.videoLabel->clear();

	if (currentProject != 0x0)
	{
		delete currentProject;
		currentProject = new Project();
	}

	if (vdo_player != 0x0)
	{
		delete vdo_player;
		vdo_player = new VideoPlayer();

		// Atualiza a imagem do player assim que receber um sinal da thread
		connect(vdo_player, SIGNAL(setNewFrame(QImage *)),
				this, SLOT(updatePlayer(QImage *)));

		// Atualiza a imagem do histograma assim que receber um sinal da thread
		connect(vdo_player, SIGNAL(setHistogram(QImage *)),
				this, SLOT(updateHist(QImage *)));
	}

}
