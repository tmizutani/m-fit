#include "../ui_transitionEdit.h"

#include "QWidget.h"

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
extern VideoPlayer *vdo_player;

/************************************************************************
* Construtor da janela de configurações 
*************************************************************************
* param (E): QDialog -> referencia da janela que criou o objeto
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
TransitionEdit::TransitionEdit(QDialog *parent) : QDialog(parent)
{
	ui.setupUi(this);

	this->cancel = false;
	this->del = false;
	this->cancel = false;
	this->pos_l = 0;
}

/************************************************************************
* Define o ID da transicao que sera Editada
*************************************************************************
* param (E): unsigned int - ID da transicao
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::setID(unsigned int id)
{
	Transition* transition;
	Video* vdo;
	long pos_l;
	char pos_cy[10];

	// Pega a transicao desejada
	transition = &currentProject->transitionList.at(id);
	vdo = currentProject->getVideo();
	pos_l = (long)transition->getPosCurrent();

	sprintf(pos_cy, "%ld", pos_l);

	// Escreve na label o label da transicao
	ui.labelTransitionName->setText(transition->getLabel());

	// Escreve a posicao da transicao
	ui.labelFramePos->setText(pos_cy);

	// Aponta o ponteiro do Video para a posicao da transicao.
	vdo->seekFrame(pos_l);

	// Atualiza o preview
	updatePreview();

}

/************************************************************************
* Atualiza a label de Preview com o Frame corrente do video.
*************************************************************************
* param (E): Nenhum
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::updatePreview()
{

	Frame *frameNew;
	QImage *image = 0x0;
	Video *vdo = currentProject->getVideo();
	long pos_l;
	char pos_cy[10];

	// Pega o Frame corrente
	frameNew = vdo->getCurrentFrame();

	// Transforma para QImage
	image = frameNew->IplImageToQImage(&vdo_player->imageData, &vdo_player->imgWidth, &vdo_player->imgHeight);

	QPixmap pix_image = QPixmap::fromImage(*image);

	// Permite ajute da imagem em relação ao tamanho da janela
	ui.labelPreview->setScaledContents(true);
	ui.labelPreview->setPixmap(pix_image); // Pinta a imagem

	// Atualiza a label da posição corrente do video
	pos_l = (long)vdo->getCurrentPosition();
	sprintf(pos_cy, "%ld", pos_l);
	ui.labelCurrentPos->setText(pos_cy);

	delete frameNew;
	delete image;
}

/************************************************************************
* Evento do botak "Change".
* Ativa a flag avisando que o ponto da transição deve ser modificado
* e salva a posição corrente do Video.
*************************************************************************
* param (E): Nenhum
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::on_changeButton_clicked()
{
	Video *vdo;
	long pos_l;
	char pos_cy[10];
	vdo = currentProject->getVideo();

	pos_l = (long)vdo->getCurrentPosition();
	sprintf(pos_cy, "%ld", pos_l);
	ui.labelFramePos->setText(pos_cy);

	this->pos_l =  pos_l;
	this->change = true;
}

/************************************************************************
* Evento do botao "Delete".
* Ativa a flag avisando que a transição deve ser apagada.
*************************************************************************
* param (E): Nenhum
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::on_deleteButton_clicked()
{
	this->del = true;
	this->change = false;
}

/************************************************************************
* Evento do botao "Cancel".
* Ativa a flag avisando que nenhuma alteração deve ser efetivada.
*************************************************************************
* param (E): Nenhum
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::on_cancelButton_clicked()
{
	this->cancel = true;
	QDialog::close();
}

/************************************************************************
* Evento do botao "Back".
* Move o ponteiro do Video para o Frame anterior e pede para atualizar
* o preview
*************************************************************************
* param (E): Nenhum
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::on_backButton_clicked()
{
	Video *vdo;
	vdo = currentProject->getVideo();

	vdo->seekFrame((long)vdo->getCurrentPosition()-1);

	updatePreview();
}

/************************************************************************
* Evento do botao "Forward".
* Move o ponteiro do Video para o Frame posterior e pede para atualizar
* o preview
*************************************************************************
* param (E): Nenhum
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::on_forwardButton_clicked()
{
	Video *vdo;
	vdo = currentProject->getVideo();

	vdo->seekFrame((long)vdo->getCurrentPosition()+1);

	updatePreview();
}

/************************************************************************
* Evento do botao "Ok".
* Ativa a flag sinalizando que deve efetivar as alterações do usuário.
*************************************************************************
* param (E): Nenhum
************************************************************************
* Histórico
* 19/10/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
void TransitionEdit::on_okButton_clicked()
{
	this->cancel = false;
	QDialog::close();

}
