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
* Histórico
* 09/10/08 - Thiago Mizutani
* Agora utilizo um connect para cada coisa. Um para o player outro para
* o histograma.
* 29/09/08 - Fabricio Lopes de Souza
* Criação.
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
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/

void Interface::addRecentFile(QString fileName)
{
	QSettings settings("MFIT", "MFIT");
	QStringList files = settings.value("recentFileList").toStringList();
	files.removeAll(fileName); // Remove todos os arquivos recentes com o msm nome
	files.prepend(fileName);
	// Retira os ultimos até que sobrem somente 4.
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
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/

void Interface::updateRecentFilesAct()
{
	QSettings settings("MFIT", "MFIT");
	QStringList files = settings.value("recentFileList").toStringList();

	// Verifico o que é menor. O tamanho da lista de arquivos ou o máximo permitido.
	int numRecentFiles = qMin(files.size(), (int)MAX_RECENT_FILES);

	// Adiciono um separador.
	ui.menuFile->addSeparator();

	for (int i = 0; i < numRecentFiles; ++i)
	{
		// Monto a string com o número (ordem de abertura) e o nome do arquivo
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
* Histórico
* 19/10/08 - Thiago Mizutani
* Inserindo as actions no menu. Por isso não estavam aparecendo. básico..
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
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
* Histórico
* 19/10/08 - Thiago Mizutani
* Criação.
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
	
	// Pego a extensão do arquivo para verificar se é um projeto ou um vídeo.
	strcpy(extension_cy, &fileName_cy[strlen(fileName_cy)-3]);	

	// Se for um video, uso a funcao de abrir video
	if (!strcmp(extension_cy,"AVI") || !strcmp(extension_cy,"avi"))
	{
		currentProject->openVideo(fileName_cy);
		enableControls();
			
		// Ao carregar o vídeo, devo perguntar ao usuário se ele quer fazer a detecção.
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
 * Tratar o evento do botão PLAY. Inicia a thread do video player
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Verifica se há vídeo, se não houver mostra alerta
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::on_playButton_clicked()
{
	// Somente para controle se há ou não vídeo carregado. depois deleta o obj
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
 * Tratar o evento do botão PAUSE. Se a thread estiver rodando, mata ela
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Verifica se há vídeo, se não houver mostra alerta
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::on_pauseButton_clicked()
{
	// Somente para controle se há ou não vídeo carregado. depois deleta o obj
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
 * Trata o botão forward. Avança o vídeo em um frame.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Histórico
 * 20/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Trata o botão backward. Volta o vídeo em um frame.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Histórico
 * 20/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Tratar o evento do botão STOP. Se a thread estiver rodando, mata ela,
 * e posiciona o ponteiro do video para seu inicio.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Verifica se há vídeo, se não houver mostra alerta
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Tratar o evento do botão OpenProject.
 * TODO: Ler o XML e subir os membros da estrutura Project.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 10/10/08 - Ivan Shiguenori Machida
 * Criação.
 ************************************************************************/
void Interface::on_actionOpenProject_triggered()
{
	// Segundo  parametro - Mensagem que ira aparecer no topo da imagem
	// Terceiro parametro - Diretorio inicial
	// Quarto   parametro - Mensagem q ira aparecer la embaixo, e as extensões suportadas

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
 * Tratar o evento do botão LoadVideo.
 * Abre o objeto Video, atualiza a videoProperties
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani
 * Pergunta para usuário se deseja detectar todas as transições após o
 * carregamento do vídeo.
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::on_actionLoadVideo_triggered()
{
	// Segundo  parametro - Mensagem que ira aparecer no topo da imagem
	// Terceiro parametro - Diretorio inicial
	// Quarto   parametro - Mensagem q ira aparecer la embaixo, e as extensões suportadas
	QString fileName = QFileDialog::getOpenFileName(this,
			"Abrir Vídeo",
			".",
			"Vídeos Suportados (*.avi)");

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

		if (askDetection()) // Gero a box perguntando se deseja detectar as transições
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
 * Tratar o evento do botão SaveAs.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Tratar o evento do botão Save.
 * TODO: Nao faz porra nenhuma pq nao temos o ParserXML
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 10/10/08 - Ivan Shiguenori Machida
 * Criação.
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
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::clearVideoProperty()
{
	Log::writeLog("%s :: clear videoPropertiesTree", __FUNCTION__); 
	this->ui.videoPropertiesTree->clear();
}

/************************************************************************
 * Insere uma linha na lista de propriedades do vídeo.
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Histórico
 * 09/10/08 - Thiago Mizutani
 * Revisão - alterado o nome da função para diminuir o nro de funcoes
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::updatePlayer(QImage *image)
{
	// Trava a thread do video_player
	vdo_player->mutex.lock();

	Video *vdo = currentProject->getVideo();

	setVideoTime(vdo->getCurrentPosition(), vdo->getFPS());

	QPixmap pix_image = QPixmap::fromImage(*image);

	// Permite ajute da imagem em relação ao tamanho da janela
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
 * Histórico
 * 09/10/08 - Thiago Mizutani
 * Revisão - alterado o nome da função para diminuir o nro de funcoes e
 * e inserção dos comentários
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/

void Interface::updateHist(QImage *hist)
{
	vdo_player->mutex.lock();

	QPixmap pix_image = QPixmap::fromImage(*hist);

	// Permite ajute da imagem em relação ao tamanho da janela
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
 * Histórico
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Cria a timeline do vídeo carregado.
 *************************************************************************
 * param (E): Não há
 *************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani 
 * Revisão de código. E inserção de comentários
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::createTimeline(void)
{
	Video *vdo = 0x0;
	vdo = currentProject->getVideo();

	int iter_i = 0x0; // Passo da timeline (qtos frames eu devo pular para plotar o próximo na timeline)
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
 * param (E): Não há
 *************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani 
 * Revisão de código. E inserção de comentários
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::setTimeline(Frame *frameTimeline)
{
	QImage *image;

	// Converte a imagem
	image = frameTimeline->IplImageToQImage(&vdo_player->timelineData, &vdo_player->timelineWidth, &vdo_player->timelineHeight);

	QPixmap pix_image = QPixmap::fromImage(*image);

	ui.timelineLabel->setScaledContents(false); // Não permite que a imagem sofra distorções conforme o tamanho da janela
	ui.timelineLabel->setPixmap(pix_image); // Pinta a timeline na labelTimeline

	delete image;

}

/************************************************************************
 * Restaura a TimeLine original e repinta as transições
 *************************************************************************
 * param (E): Não há
 *************************************************************************
 * 09/11/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::recreateTimeline()
{
	Frame::imgCopy(vdo_player->frameTimelineOriginal->data, vdo_player->frameTimeline->data);
	updateTransitions();
}

/*************************************************************************
 * Atualiza a posição do cursor da timeline conforme a passagem do vídeo
 *************************************************************************
 * param (E): Não há
 *************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani 
 * Revisão de código. E inserção de comentários
 * 29/09/08 - Fabricio Lopes de Souza
 * Criação.
 ************************************************************************/
void Interface::updateTimeline()
{
	Video *vdo = 0x0;
	long pos = 0x0;
	int line_point = 0x0;

	vdo = currentProject->getVideo();
	pos = (long)vdo->getCurrentPosition();

	// A formula para saber em que ponto plotar o indicador é:
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
 * Trata o evento gerado pelo botão para detecção de todas as transições
 *************************************************************************
 * param (E): Não há
 * return : não há
 *************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Verifica se há vídeo, se não houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Criação.
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
 * Trata o evento gerado pelo botão para detecção de todos os cortes
 *************************************************************************
 * param (E): Não há
 * return : não há
 *************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Verifica se há vídeo, se não houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Criação.
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
 * Trata o evento gerado pelo botão para detecção de todos os fades
 *************************************************************************
 * param (E): Não há
 * return : não há
 *************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Verifica se há vídeo, se não houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Criação.
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
 * Trata o evento gerado pelo botão para detecção de todos os dissolve
 *************************************************************************
 * param (E): Não há
 **************************************************************************
 * return : não há
 *************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Verifica se há vídeo, se não houver mostra alerta
 * 06/10/08 - Thiago Mizutani
 * Criação.
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
 * Adiciona uma transição na transitionsTree
 **************************************************************************
 * param (E): Transition* transition -> transição a ser inserida na árovore 
 **************************************************************************
 * return : Não há
 **************************************************************************
 * Histórico
 * 08/10/08 - Thiago Mizutani
 * Criação.
 *************************************************************************/
void Interface::insertTransitionsTree(Transition* transition, long id_l)
{
	// Cria o item referente a Tree
	QTreeWidgetItem *item = new QTreeWidgetItem(this->ui.transitionsTree);

	// Cria a lista de items
	QList<QTreeWidgetItem *> itens;

	long pos_l = transition->getPosCurrent(); // Posição da transição detectada

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
 * Marca na timeline os pontos onde ocorrem as transições
 **************************************************************************
 * param (E): Transition* transition -> Transição a ser marcada na timeline 
 **************************************************************************
 * return : Não há
 **************************************************************************
 * Histórico
 * 08/10/08 - Thiago Mizutani
 * Criação.
 *************************************************************************/
void Interface::insertTransitionsTimeline(Transition* transition)
{
	long posTransition_l = 0;
	long posTimeline_l = 0;

	// Pega o ponto da transicao
	posTransition_l = transition->getPosCurrent();

	// A formula para saber em que ponto plotar o indicador é:
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
		default: // Início do vídeo.
			cvLine(vdo_player->frameTimeline->data, p1, p2, cvScalar(0,0,0), 2);
			break;
	}

	setTimeline(vdo_player->frameTimeline);

}

/*************************************************************************
 * Atualiza a lista de transições e marca pontos de transição na timeline
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Não há
 *************************************************************************
 * Histórico
 * 08/10/08 - Thiago Mizutani
 * Criação.
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
 * Limpa a lista de transições - transitionsTree
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Não há
 *************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/
void Interface::clearTransitionsTree()
{
	Log::writeLog("%s :: clear transitionsTree", __FUNCTION__); 
	this->ui.transitionsTree->clear();
}

/*************************************************************************
 * Trata as seleções da lista de transição e manda pintar uma linha no 
 * header da timeline identificando a sua tomada. 
 *************************************************************************
 * param (E): Nenhum
 *************************************************************************
 * return : Não há
 *************************************************************************
 * Histórico
 * 16/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Traça uma linha no header da timeline referente ao item selecionado
 * na lista de transições.
 *************************************************************************
 * param (E): QTreeWidgetItem * item -> item selecionado 
 *************************************************************************
 * return : Não há
 *************************************************************************
 * Histórico
 * 16/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Cria uma caixa de diálogo perguntando ao usuário se este deseja realizar
 * a detecção de transições logo após o carregamento do vídeo.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * return : Não há
 **************************************************************************
 * Histórico
 * 09/10/08 - Thiago Mizutani
 * Criação.
 *************************************************************************/
int Interface::askDetection()
{
	int reply = 0; // Resposta do usuário

	// Crio uma box com o ícone "?"
	reply = QMessageBox::question(
			this,
			tr("MFIT"),
			tr("<p><b>Detectar todas as transições agora?</b>" \
				"<p>Este processo poderá levar alguns minutos."),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No
			);

	// Se clickei em detectNow, chamo a detecção de transições
	if (reply == QMessageBox::Yes)
		return (TRUE);
	else
		return (FALSE);

}

/**************************************************************************
 * Função que mostra uma mensagem de alerta caso o usuáio tente fazer 
 * qualquer operação que exija vídeo e o vídeo não estiver carregado.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * return : Não há
 **************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Criação.
 *************************************************************************/
void Interface::alertUser(int message)
{

	char message_cy[200];

	switch(message)
	{
		case ALERT_NO_LOADED_VIDEO:
			strcpy(message_cy ,"<p><b>Não é possível fazer a detecção de transições!</b><p>Carregue um vídeo antes de executar esta operação!");
		case ALERT_VIDEO_INCOMPATIBLE:
			strcpy(message_cy, "<p><b>Não é possível carregar o vídeo!</b><p>Formato não suportado!");
	}

	QMessageBox::critical(
			this,
			tr("MFIT - ERRO"),
			tr(message_cy),
			QMessageBox::Ok
			);

}

/**************************************************************************
 * Habilita todos os botões de controle quando carregar o vídeo.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * return : Não há
 **************************************************************************
 * Histórico
 * 13/10/08 - Thiago Mizutani
 * Criação.
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

	// Controles de detecção
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
 * param (E): Não há.
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 18/10/08 - Thiago Mizutani
 * Criação.
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
 * Histórico
 * 14/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Abre a janela de configurações para detecção de transições.
 **************************************************************************
 * param (E): Não há.
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 18/10/08 - Thiago Mizutani
 * Criação.
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
 * Histórico
 * 14/10/08 - Fabricio Lopes de Souza
 * Criação.
 *************************************************************************/

char *Interface::QStringToChar(QString string, char* string_cy)
{
	sprintf(string_cy, "%s",string.toAscii().data());

	return string_cy;
}

/**************************************************************************
 * Traça uma linha acima da timeline identificando a localização da tomada
 **************************************************************************
 * param (E): unsigned int transitionID - Transição selecionada na lista.
 * param (E): int clean - flag que indica se deve apagar o traço ou não.
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 14/10/08 - Fabricio Lopes de Souza
 * Criação.
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
	// Utilizamos o frame final do video como marco de fim da transição
	if (transitionID == (unsigned int)currentProject->transitionList.size()-1)
	{
		Video *vdo = 0x0;

		vdo = currentProject->getVideo();

		posNext_l = (long)vdo->getFramesTotal();
	}
	else // Senão, utlizamos o inicio da proxima transição
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

	// Escreve o ID e o nome da transição na timeline
	sprintf(frame_cy, "%.3d-%s", transitionID, transition->getLabel());
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3f, 0.3f, 0, 0, 0);
	cvPutText(vdo_player->frameTimeline->data, frame_cy, p3, &font, scalar);

	cvLine(vdo_player->frameTimeline->data, p1, p2, scalar, 3);

}

/**************************************************************************
 * Varre todas as transições e retira uma a uma do header da timeline.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 14/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Mostra no player o primeiro frame da transição selecionada na lista de 
 * transições e atualiza o cursor da timeline.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 20/10/08 - Thiago Mizutani
 * Criação.
 *************************************************************************/
void Interface::on_transitionsTree_itemClicked(QTreeWidgetItem* item, int column)
{
	char id_cy[10];
	unsigned int id;
	long pos_l = 0;
	long timeline_l;

	Transition *transition;

	Video* vdo = currentProject->getVideo();

	QStringToChar(item->text(0), id_cy); // Pego o ID da transição

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
	updateTimeline(); // Atualizo a posição do cursor da timeline

	timeline_l = currentProject->FrameToTimelinePos(pos_l);

	// Move o scroll
	moveScrollArea((int)timeline_l, 0);

}

/**************************************************************************
 * Trata o evento de DoubleClick em um item na lista de transições.
 * Abre a janela de edição da transição.
 * Apos a janela fechar, trata as opções escolhidas
 **************************************************************************
 * param (E): QTreeWidgetItem* - Item clicado
 * param (E): int - Coluna clicada
 **************************************************************************
 * Histórico
 * 09/11/08 - Fabrício Lopes de Souza
 * Criação.
 *************************************************************************/
void Interface::on_transitionsTree_itemDoubleClicked ( QTreeWidgetItem * item, int column )
{

	char id_cy[10];
	unsigned int idx_i;
	long pos_l;
	Video *vdo;
	TransitionEdit editWindow;

	QStringToChar(item->text(0), id_cy); // Pego a posição da transição determinada pelo sistema 
	idx_i = atoi(id_cy);

	// Não se pode editar o "Inicio de Video"
	if (idx_i == 0)
		return;

	// Define a transição que sera editada
	editWindow.setID(idx_i);

	// Salva a posição corrente do video
	vdo = currentProject->getVideo();
	pos_l = (long)vdo->getCurrentPosition();

	// Abre a janela
	editWindow.exec();

	// Restaura a posição do video
	vdo->seekFrame(pos_l);

	// Se cancelou, não faz nada
	if (editWindow.cancel)
	{
		return;
	} // Se selecionou o CHANGE, seta a posição da transição como a posição desejada
	else if (editWindow.change)
	{
		Transition *transition;

		transition = &currentProject->transitionList.at(idx_i);
		transition->setPosUserTransition(editWindow.pos_l);

	} // Se deseja remover a transição, remove :)
	else if (editWindow.del)
	{
		currentProject->removeTransition(idx_i);
	}
	else
	{
		return;
	}

	// Recria a timeline, para absorver as alterações
	recreateTimeline();

	return;

}

/**************************************************************************
 * Insere um item na lista de efeitos do vídeo
 **************************************************************************
 * param (E): Effect *effect -> efeito a ser inserido.
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 19/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Trata evento de quando é selecionado um item na lista de efeitos.
 **************************************************************************
 * param (E): QTreeWidgetItem *item -> item selecionado (linha)
 * param (E): int column -> coluna selecionada (frame final ou inicial)
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 19/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * return : Não há.
 **************************************************************************
 * Histórico
 * 19/10/08 - Fabricio Lopes de Souza
 * Criação.
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
 * return : Não há.
 **************************************************************************
 * Histórico
 * 19/10/08 - Fabricio Lopes de Souza
 * Criação.
 *************************************************************************/
void Interface::effectTreeClear()
{
	this->ui.effectsTree->clear();
}

/**************************************************************************
 * Fecha a aplicação.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : Não há.
 **************************************************************************
 * Histórico
 * 20/10/08 - Thiago Mizutani
 * Criação.
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
 * Pergunta se o usuário deseja salvar o projeto antes de sair.
 **************************************************************************
 * param (E): Nenhum.
 **************************************************************************
 * return : TRUE - Salva e sai.
 * 			FALSE - Sai sem sair.
 **************************************************************************
 * Histórico
 * 20/10/08 - Thiago Mizutani
 * Criação.
 *************************************************************************/
bool Interface::askUserSave()
{
	int reply = 0; // Resposta do usuário

	// Crio uma box com o ícone "?"
	reply = QMessageBox::question(
			this,
			tr("MFIT"),
			tr("<p>Deseja salvar o projeto antes de sair?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No
			);

	// Se clickei em detectNow, chamo a detecção de transições
	if (reply == QMessageBox::Yes)
		return (TRUE);
	else
		return (FALSE);
}

/**************************************************************************
 * Movimenta o SCROLL, posiciona a transição selecionada no centro da timeline
 **************************************************************************
 * param (E): x - Posicao X
 * param (E): y - Posicao Y
 **************************************************************************
 * return : Nenhum
 **************************************************************************
 * Histórico
 * 20/10/08 - Fabricio Lopes de Souza
 * Criação.
 *************************************************************************/
void Interface::moveScrollArea(int x, int y)
{
	QRect rect(ui.widgetDockTimeline->geometry());
	int center_i;

	center_i = (int)(rect.width() / 2);

	ui.scrollArea->ensureVisible(x, y, center_i, center_i);
}


/**************************************************************************
 * Desabilita o botão de Save
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * Histórico
 * 09/11/08 - Fabricio Lopes de Souza
 * Criação.
 *************************************************************************/
void Interface::disableSaveButton()
{
	this->ui.actionSave->setEnabled(false);
}

/**************************************************************************
 * Habilita o botão de Save
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * Histórico
 * 09/11/08 - Fabricio Lopes de Souza
 * Criação.
 *************************************************************************/
void Interface::enableSaveButton()
{
	this->ui.actionSave->setEnabled(true);
}

/**************************************************************************
 * Trata o evento de close da aplicação.
 * Fecha/libera todos os objetos abertos/instanciados.
 **************************************************************************
 * param (E): Nenhum
 **************************************************************************
 * Histórico
 * 10/11/08 - Fabricio Lopes de Souza
 * Criação.
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
 * Histórico
 * 16/11/08 - Thiago Mizutani
 * Criação.
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
