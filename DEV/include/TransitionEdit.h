#include <QObject>
#include "QDialog"

#include "../include/Interface.h"
#include "../include/Effect.h"
#include "../include/Color.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"
#include "../include/Project.h"

#include "../include/Log.h"

#include "../ui_transitionEdit.h"

#ifndef TRANSITIONEDIT_INCLUDE
#define TRANSITIONEDIT_INCLUDE 

class TransitionEdit: public QDialog
{

	Q_OBJECT

	private slots:
		void on_changeButton_clicked();
		void on_deleteButton_clicked();
		void on_cancelButton_clicked();
		void on_okButton_clicked();
		void on_backButton_clicked();
		void on_forwardButton_clicked();

	public:
		TransitionEdit(QDialog *parent = 0);

		unsigned int id;

		bool del;
		bool change;
		bool cancel;

		long pos_l;

		void setID(unsigned int id);
		void updatePreview();

		Ui::transitionEditWidget ui;

};

#endif
