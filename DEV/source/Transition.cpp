#include <stdlib.h>
#include <string.h>

#include "../include/Transition.h"

/************************************************************************
* Construtor
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Nenhum 
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/
Transition::Transition()
{
	this->type = 0;
	this->posTransition = 0;
	this->posUserTransition = 0;
	this->selected = false;
}

/************************************************************************
* Construtor que recebe os argumentos via parâmetro
*************************************************************************
* param (E): int type      -> tipo da transição
* param (E): posTransition -> posição onde ela ocorreu
* param (E): label         -> label desta transição
*************************************************************************
*************************************************************************
* Histórico:
* 01/09/08 - Fabrício Lopes de Souza
* Criação.
************************************************************************/
Transition::Transition(int type, long posTransition, char *label)
{
	Transition();

	this->type = type;
	this->posTransition = posTransition;
	this->posUserTransition = 0;
	this->selected = false;
	strcpy(this->label, label);
}

/************************************************************************
* Função que armazena o tipo da transição 
*************************************************************************
* param (E): int type => Tipo da transição
*************************************************************************
* return : Nenhum 
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

void Transition::setType(int type)
{
	this->type = type;
}

/************************************************************************
* Função que retorna o tipo da transição
*************************************************************************
* param (E): Nenhum 
*************************************************************************
* return : Tipo da transição
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

int Transition::getType()
{
	return (this->type);
}

/************************************************************************
* Função que armazena a posição (em nro de frames) da transição definida 
* pelo sistema
*************************************************************************
* param (E): Time* time 
*************************************************************************
* return : nenhum
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

void Transition::setPosTransition(int posTransition)
{
	this->posTransition = posTransition;
}	

/************************************************************************
* Função que retorna a posição (em nro de frames) da transição definida 
* pelo sistema
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Posição da transição
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

long Transition::getPosTransition()
{
	return (this->posTransition);
}

/************************************************************************
* Função que armazena a posição da transição definida pelo usuário em 
* hh:mm:ss:ms 
*************************************************************************
* param (E): Time* timeUser
*************************************************************************
* return : nenhum
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

void Transition::setPosUserTransition(int posUser)
{
	this->posUserTransition = posUser;
}

/************************************************************************
* Função que retorna a posição em (hh:mm:ss:ms) da transição definida 
* pelo usuário
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Posição da transição
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

long Transition::getPosUserTransition()
{
	return (this->posUserTransition);
}

/************************************************************************
* Função que armazena a label da transição a ser exibida na timeline
*************************************************************************
* param (E): char* label => label a ser exibida na timeline
*************************************************************************
* return : nenhum
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

void Transition::setLabel(char* label)
{
	strcpy(this->label, label);
}

/************************************************************************
* Função que retorna a label da transição
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Label da transição
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

char* Transition::getLabel()
{
	return (this->label);
}

/************************************************************************
* Sobrecarga do operador '<'
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : True/False
*************************************************************************
* Histórico:
* 15/08/08 - Fabrício Lopes de Souza
* Criação.
************************************************************************/

bool Transition::operator <(const Transition &Rhs) const
{  
	long pos;
	long posAUX;

	pos = getPosCurrent();
	posAUX = Rhs.getPosCurrent();

	return (pos < posAUX);        
}

/************************************************************************
* Função que retorna a posição corrente da transição
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Posição da transição.
*************************************************************************
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

long Transition::getPosCurrent(void) const
{
	if (this->posUserTransition > 0)
		return this->posUserTransition;
	else
		return this->posTransition;
}
