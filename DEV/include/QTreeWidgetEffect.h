#ifndef QTREEWIDGETEFFECT_INCLUDE
#define QTREEWIDGETEFFECT_INCLUDE
#include <QtGui>


class QTreeWidgetEffect : public QTreeWidget
{
	Q_OBJECT

	public:
		QTreeWidgetEffect(QWidget *parent = 0);
		QAction *deleteAct;

	protected:
		void contextMenuEvent(QContextMenuEvent *event);

	private slots:
		void deleteItem(void);

};
#endif
