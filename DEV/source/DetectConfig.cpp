#include "../ui_detectConfig.h"

#include "QWidget.h"
#include "QDialog.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QtGui>

#include "../include/Interface.h"
#include "../include/Effect.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"
#include "../include/DetectConfig.h"
#include "../include/Project.h"

#include "../include/Log.h"

extern Project *currentProject;

/************************************************************************
* Construtor da janela de configura��es 
*************************************************************************
* param (E): QDialog -> referencia da janela que criou o objeto
************************************************************************
* Hist�rico
* 14/10/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

DetectConfig::DetectConfig(QDialog *parent) : QDialog(parent)
{
	ui.setupUi(this);

	ui.listDetectConfig->setCurrentRow(0);
	ui.detectProperties->setCurrentIndex(0);

	/**
	 * Verifico se existem valores definidos pelo usu�rio. Se sim, utilizo 
	 * estes valores como valor inicial, sen�o utilizo o valor padr�o.
	**/
	if (currentProject->getUserThreshold()) // Limiar de corte
		ui.spinPercentage->setValue(currentProject->getUserThreshold());

	if (currentProject->getUserMinCanny()) // Limiar m�nimo do Canny
		ui.spinMinCanny->setValue(currentProject->getUserMinCanny());

	if (currentProject->getUserMaxCanny()) // Limiar m�ximo do Canny
		ui.spinMaxCanny->setValue(currentProject->getUserMaxCanny());

	// Se bot�o "Cancelar" for apertado, fecho a janela sem fazer nada.
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));

	// Se alterar o item da lista (menu com os tipos de transi��es), mudo a pagina de configura��es.
	connect(ui.listDetectConfig, SIGNAL(itemClicked(QListWidgetItem *)),
 			  this, SLOT(changePage())
			 );

}

/************************************************************************
* Salva as configura��es e fecha a janela.
*************************************************************************
* param (E): N�o h�.
*************************************************************************
* return: nenhum.
*************************************************************************
* Hist�rico
* 18/10/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void DetectConfig::on_okButton_clicked()
{
	int userCutThreshold = ui.spinPercentage->value();
	int userMinCanny = ui.spinMinCanny->value();
	int userMaxCanny= ui.spinMaxCanny->value();

	if ( userCutThreshold == DEFAULT_CUT_THRESHOLD )
		currentProject->setUserThreshold(0);
	else
		currentProject->setUserThreshold(userCutThreshold);

	if ( userMinCanny == DEFAULT_MIN_CANNY )
		currentProject->setUserMinCanny(0);
	else
		currentProject->setUserMinCanny(userMinCanny);

	if ( userMaxCanny == DEFAULT_MAX_CANNY )
		currentProject->setUserMaxCanny(0);
	else
		currentProject->setUserMaxCanny(userMaxCanny);

	QDialog::close();
}

/************************************************************************
* Altera as p�ginas da janela conforme item selecionado no menu.
*************************************************************************
* param (E): N�o h�.
*************************************************************************
* return: nenhum.
*************************************************************************
* Hist�rico
* 18/10/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void DetectConfig::changePage()
{

	ui.detectProperties->setCurrentIndex(ui.listDetectConfig->currentRow());

}
