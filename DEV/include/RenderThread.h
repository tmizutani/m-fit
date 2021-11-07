#include <QThread>
#include <QMutex>
#include "QMainWindow"

#include "../include/Interface.h"
#include "../include/Frame.h"
#include "../include/Effect.h"
#include "../include/Transition.h"
#include "../include/Project.h"

#ifndef RENDERTHREAD_INCLUDE
#define RENDERTHREAD_INCLUDE 

class RenderThread: public QThread
{
	Q_OBJECT 

	protected:
		void run();

	signals:
		void setProgressThread(int percent);
		void writeFrameThread(Frame *frame);

	public:
		RenderThread(char *filename_cy);
		char filename_cy[256];
		void sendProgress(int i, long total);
		QMutex mutex;


};
#endif
