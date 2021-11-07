#include <QThread>
#include <QMutex>
#include "QMainWindow"

#include "../include/Interface.h"
#include "../include/Frame.h"
#include "../include/Effect.h"
#include "../include/Transition.h"
#include "../include/Project.h"

#ifndef VIDEOPLAYER_INCLUDE
#define VIDEOPLAYER_INCLUDE

class VideoPlayer: public QThread
{
	Q_OBJECT 

	protected:
		void run();

	signals:
		void setNewFrame(QImage *image); // emite sinal para interface atualizar o frame do player
		void setHistogram(QImage *image); // emite sinal para interface atualziar o histograma

	public:
		QMutex mutex;
		uchar* imageData;
		uchar* histData;
		uchar* timelineData;
		VideoPlayer();
		~VideoPlayer();

		int imgWidth, imgHeight;
		int histWidth, histHeight;
		int timelineWidth, timelineHeight;
		void updateHist(Frame *frame);
		void updatePlayer(Frame *frame);

		Frame* frameTimelineOriginal;
		Frame* frameTimeline;
		Frame* frameTimelineEdited;

		void updateCurrentFrame();
};
#endif
