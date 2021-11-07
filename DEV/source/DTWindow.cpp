#include "../ui_DTWindow.h"
#include <stdio.h>

#include "QWidget.h"

#include "../include/Fade.h"
#include "../include/Cut.h"
#include "../include/Dissolve.h"
#include "../include/DTWindow.h"
#include "../include/DetectTransitions.h"
#include "../include/Interface.h"
#include "../include/Effect.h"
#include "../include/Color.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"
#include "../include/TransitionEdit.h"
#include "../include/Project.h"
#include "../include/VideoPlayer.h"

#include "../include/Log.h"

extern Project *currentProject;

/************************************************************************
 * Construtor da janela de DTWindowizacao
 *************************************************************************
 * param (E): QDialog -> referencia da janela que criou o objeto
 ************************************************************************
 * Hist�rico
 * 20/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
DTWindow::DTWindow(QDialog *parent) : QDialog(parent)
{
	ui.setupUi(this);
	this->percent = 0;

}

/************************************************************************
 * Recebe uma mensagem enviada pela thread
 * e monta a janela a partir dos dados recebidos
 *************************************************************************
 * param (E): char *fase_cp - Etapa da detec��o
 * param (E): uint *len_i   - Quantas transi��es j� detectadas
 * param (E): int  *percent - Percentual total da detec��o
 ************************************************************************
 * Hist�rico
 * 20/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void DTWindow::receiveMessage(char* fase_cp, uint len_i, int percent)
{

	char num_cy[30];

	if (len_i == 0)
	{
		char type_cy[30];

		switch (this->type)
		{
			case TRANSITION_FADE:
				strcpy(type_cy, "Fade");
				break;

			case TRANSITION_CUT:
				strcpy(type_cy, "Corte");
				break;

			case TRANSITION_DISSOLVE:
				strcpy(type_cy, "Dissolve");
				break;

			default:
				strcpy(type_cy, "Todas");
				break;
		}

		ui.labelType->setText(type_cy);
		return;
	}

	len_i --;

	// Etapa da detec��o
	ui.labelFase->setText(fase_cp);

	// Quantidade de transi��es jah detectadas
	sprintf(num_cy, "%d", len_i);
	ui.labelNum->setText(num_cy);

	this->percent = percent;

	ui.progressBar->setValue (this->percent);

	// Fim da detec��o
	if (this->percent >= 100)
	{
		DT->terminate();

		QMessageBox::information(
				this,
				tr("Detec��o de Transi��es"),
				tr("Fim da detec��o"),
				QMessageBox::Ok
				);

		QDialog::close();

	}
}

/************************************************************************
 * Determina qual a thread que a janela deve utilizar.
 *************************************************************************
 * param (E): DetectTransitions *DT - Objeto da detec��o
 * param (E): int type - Tipo da detec��o
 ************************************************************************
 * Hist�rico
 * 20/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void DTWindow::setDetectThread(DetectTransitions *DT, int type)
{
	this->DT = DT;
	this->type = type;

	connect(DT, SIGNAL(sendMessage (char*, unsigned int, int)),
			this, SLOT(receiveMessage(char*, unsigned int, int)),
			Qt::BlockingQueuedConnection);

}

/************************************************************************
 * Inicia a thread de detec��o e a exibi��o do progresso na janela.
 *************************************************************************
 * param (E): Nenhum
 ************************************************************************
 * Hist�rico
 * 20/11/08 - Fabricio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void DTWindow::start()
{
	switch (type)
	{
		case TRANSITION_FADE:
			((Fade*)DT)->start();
			break;

		case TRANSITION_CUT:
			((Cut*)DT)->start();
			break;

		case TRANSITION_DISSOLVE:
			((Dissolve*)DT)->start();
			break;

		default:
			DT->start();
			break;
	}
}
