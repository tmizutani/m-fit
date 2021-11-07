#include <QObject>
#include "QMainWindow"
#include "cv.h"
#include "highgui.h"

#include "../ui_Interface.h" 

#include "../include/Time.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"
#include "../include/Effect.h"
#include "../include/Transition.h"

#ifndef INTERFACE_INCLUDE
#define INTERFACE_INCLUDE

#define MAX_RECENT_FILES 4

#define ALERT_NO_LOADED_VIDEO 0
#define ALERT_VIDEO_INCOMPATIBLE 1

class Interface: public QMainWindow
{
	Q_OBJECT

	private slots:
		void on_actionOpenProject_triggered();
		void on_actionSave_triggered();
		void on_actionSaveAs_triggered();
		void on_actionLoadVideo_triggered();

		void on_playButton_clicked(); // d� play no v�deo
		void on_pauseButton_clicked(); // pausa o v�deo
		void on_stopButton_clicked(); // para o v�deo
		void on_backButton_clicked(); // volta um frame
		void on_forwardButton_clicked(); // avan�a um frame

		void updatePlayer(QImage *image);
		void updateHist(QImage *image);

		void on_transitionsTree_itemSelectionChanged(void);

		void on_actionAllTransitions_triggered(); // detecta todas as transi��es
		void on_actionOnlyCuts_triggered(); // detecta somente cortes
		void on_actionAllFades_triggered(); // detecta somente fades
		void on_actionOnlyDissolve_triggered(); // detecta somente dissolve
		void on_actionRenderVideo_triggered();   // renderiza o v�deo
		void on_actionDetectConfig_triggered(); // abre janela de configura��es da transi��o
		void on_actionExit_triggered(); // fecha aplica��o
		void on_actionNewProject_triggered(); 

		int askDetection(); // Pergunta se o usu�rio quer detectar todas as transi��es ap�s carregar o v�deo.
		bool askUserSave(); // Pergunta pro usu�rio se deseja salvar o projeto.
	

		void enableControls(); // Habilita todos os comandos (bot�es) ap�s carregar um v�deo.

		void showDetectionConfigs(); // Abre a janela de configura��es para detec��o de transi��es

		void openRecentFile(); // Abre um arquivo (projeto ou v�deo) selecionado no recent files

		// Mostra frame inicial/final de onde foi aplicado o efeito no v�deo.
		void on_effectsTree_itemClicked(QTreeWidgetItem * item, int column);
		void on_transitionsTree_itemClicked(QTreeWidgetItem* item, int column);

		void on_transitionsTree_itemDoubleClicked ( QTreeWidgetItem * item, int column );

		void alertUser(int message); // Mostra uma mensagem de alerta se o usu�rio tentar fazer qqr coisa antes de dar um load num v�deo

	private:

		QAction *recentFiles[MAX_RECENT_FILES]; // Array que guarda o nome dos arquivos recentes

	public:
		Interface(QMainWindow *parent = 0);

		void addRecentFile(QString fileName);
		void updateRecentFilesAct();
		void createRecentFilesActions();

		void changeWindowTitle(char *string);
		void insertVideoProperty(char *param_cy, char *value_cy);
		void clearVideoProperty();
		
		void setVideoTime(double framePos, double fps);
		void createTimeline(void);
		void setTimeline(Frame *timeLine);
		void updateTimeline();

		void insertTransitionsTree(Transition* transition, long id_l); // Insere um item na lista de transi��es
		void insertTransitionsTimeline(Transition* transition); // Marca a timeline com uma linha onde houver transi��o.

		void updateTransitions();
		void clearTransitionsTree();

		static char* QStringToChar(QString string, char* string_cy);

		void updateTransitionHeader(unsigned int transitionID, int clean = 0);
		void updateTransitionHeader(QTreeWidgetItem * item);
		void clearTransitionHeader();

		void insertEffectTree(Effect *effect, int ind);
		void updateEffectTree();
		void effectTreeClear();
		void moveScrollArea(int x, int y);

		void enableSaveButton();
		void disableSaveButton();

		void recreateTimeline();

		void closeEvent(QCloseEvent* event);

		Ui::MainWindow ui;

};

#endif
