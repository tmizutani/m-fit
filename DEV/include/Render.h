#include <QObject>
#include "QDialog"

#include "../include/Interface.h"
#include "../include/Effect.h"
#include "../include/Color.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"
#include "../include/Project.h"
#include "../include/RenderThread.h"

#include "../include/Log.h"

#include "../ui_Render.h"

#ifndef RENDER_INCLUDE
#define RENDER_INCLUDE 

class Render: public QDialog
{

	Q_OBJECT

	private slots:
		void setProgress(int percent);
		void writeFrame(Frame *frame);
		void closeEvent(QCloseEvent* event);


	public:
		Render(QDialog *parent = 0);
		Ui::Render ui;
		void startRenderThread(RenderThread *renderThread);
		CvVideoWriter* videoWriter;
		RenderThread *renderThread;


};

#endif
