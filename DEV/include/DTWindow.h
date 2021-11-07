#include <QObject>
#include "QDialog"

#include "../include/Interface.h"
#include "../include/Effect.h"
#include "../include/Color.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"
#include "../include/Project.h"
#include "../include/DetectTransitions.h"

#include "../include/Log.h"

#include "../ui_DTWindow.h"

#ifndef DTWINDOW_INCLUDE
#define DTWINDOW_INCLUDE

class DTWindow: public QDialog
{

	Q_OBJECT

	private slots:
		void receiveMessage(char*,uint,int);

	public:
		DTWindow(QDialog *parent = 0);
		Ui::DTWindow ui;

		void setDetectThread(DetectTransitions *DT, int type);
		void start();

		int percent;

		DetectTransitions *DT;
		int type;
};

#endif
