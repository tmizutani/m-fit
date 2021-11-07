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
* Construtor da janela de configurações 
*************************************************************************
* param (E): QDialog -> referencia da janela que criou o objeto
************************************************************************
* Histórico
* 14/10/08 - Thiago Mizutani
* Criação.
************************************************************************/

DetectConfig::DetectConfig(QDialog *parent) : QDialog(parent)
{
	ui.setupUi(this);

	ui.listDetectConfig->setCurrentRow(0);
	ui.detectProperties->setCurrentIndex(0);

	/**
	 * Verifico se existem valores definidos pelo usuário. Se sim, utilizo 
	 * estes valores como valor inicial, senão utilizo o valor padrão.
	**/
	if (currentProject->getUserThreshold()) // Limiar de corte
		ui.spinPercentage->setValue(currentProject->getUserThreshold());

	if (currentProject->getUserMinCanny()) // Limiar mínimo do Canny
		ui.spinMinCanny->setValue(currentProject->getUserMinCanny());

	if (currentProject->getUserMaxCanny()) // Limiar máximo do Canny
		ui.spinMaxCanny->setValue(currentProject->getUserMaxCanny());

	// Se botão "Cancelar" for apertado, fecho a janela sem fazer nada.
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));

	// Se alterar o item da lista (menu com os tipos de transições), mudo a pagina de configurações.
	connect(ui.listDetectConfig, SIGNAL(itemClicked(QListWidgetItem *)),
 			  this, SLOT(changePage())
			 );

}

/************************************************************************
* Salva as configurações e fecha a janela.
*************************************************************************
* param (E): Não há.
*************************************************************************
* return: nenhum.
*************************************************************************
* Histórico
* 18/10/08 - Thiago Mizutani
* Criação.
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
* Altera as páginas da janela conforme item selecionado no menu.
*************************************************************************
* param (E): Não há.
*************************************************************************
* return: nenhum.
*************************************************************************
* Histórico
* 18/10/08 - Thiago Mizutani
* Criação.
************************************************************************/

void DetectConfig::changePage()
{

	ui.detectProperties->setCurrentIndex(ui.listDetectConfig->currentRow());

}
