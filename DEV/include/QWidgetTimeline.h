#ifndef QWIDGETIMELINE_INCLUDE
#define QWIDGETIMELINE_INCLUDE
#include <QtGui>

// Tamanho de um frame na timeline
#define SIZE_FRAME_TIMELINE 75

// Quanto 1 frame da timeline representa em segundos no video
#define SIZE_SEC_FRAME 1

class QWidgetTimeline : public QWidget
{
	Q_OBJECT

	public:
		QWidgetTimeline(QWidget *parent = 0);

	protected:
		void mousePressEvent(QMouseEvent *event);
		void dragMoveEvent(QDragMoveEvent * event);
		void dragEnterEvent(QDragEnterEvent * event);
 		void dropEvent(QDropEvent *event);
		int getItemByEvent(QDropEvent *event);
		int selectDropTransition(QPoint pointEvent, int clear = 1);
		//bool event(QEvent *e);
		void dragLeaveEvent(QDragLeaveEvent *event);


};
#endif
