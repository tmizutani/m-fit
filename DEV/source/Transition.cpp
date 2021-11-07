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
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/
Transition::Transition()
{
	this->type = 0;
	this->posTransition = 0;
	this->posUserTransition = 0;
	this->selected = false;
}

/************************************************************************
* Construtor que recebe os argumentos via par�metro
*************************************************************************
* param (E): int type      -> tipo da transi��o
* param (E): posTransition -> posi��o onde ela ocorreu
* param (E): label         -> label desta transi��o
*************************************************************************
*************************************************************************
* Hist�rico:
* 01/09/08 - Fabr�cio Lopes de Souza
* Cria��o.
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
* Fun��o que armazena o tipo da transi��o 
*************************************************************************
* param (E): int type => Tipo da transi��o
*************************************************************************
* return : Nenhum 
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Transition::setType(int type)
{
	this->type = type;
}

/************************************************************************
* Fun��o que retorna o tipo da transi��o
*************************************************************************
* param (E): Nenhum 
*************************************************************************
* return : Tipo da transi��o
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

int Transition::getType()
{
	return (this->type);
}

/************************************************************************
* Fun��o que armazena a posi��o (em nro de frames) da transi��o definida 
* pelo sistema
*************************************************************************
* param (E): Time* time 
*************************************************************************
* return : nenhum
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Transition::setPosTransition(int posTransition)
{
	this->posTransition = posTransition;
}	

/************************************************************************
* Fun��o que retorna a posi��o (em nro de frames) da transi��o definida 
* pelo sistema
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Posi��o da transi��o
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

long Transition::getPosTransition()
{
	return (this->posTransition);
}

/************************************************************************
* Fun��o que armazena a posi��o da transi��o definida pelo usu�rio em 
* hh:mm:ss:ms 
*************************************************************************
* param (E): Time* timeUser
*************************************************************************
* return : nenhum
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Transition::setPosUserTransition(int posUser)
{
	this->posUserTransition = posUser;
}

/************************************************************************
* Fun��o que retorna a posi��o em (hh:mm:ss:ms) da transi��o definida 
* pelo usu�rio
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Posi��o da transi��o
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

long Transition::getPosUserTransition()
{
	return (this->posUserTransition);
}

/************************************************************************
* Fun��o que armazena a label da transi��o a ser exibida na timeline
*************************************************************************
* param (E): char* label => label a ser exibida na timeline
*************************************************************************
* return : nenhum
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Transition::setLabel(char* label)
{
	strcpy(this->label, label);
}

/************************************************************************
* Fun��o que retorna a label da transi��o
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Label da transi��o
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
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
* Hist�rico:
* 15/08/08 - Fabr�cio Lopes de Souza
* Cria��o.
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
* Fun��o que retorna a posi��o corrente da transi��o
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : Posi��o da transi��o.
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

long Transition::getPosCurrent(void) const
{
	if (this->posUserTransition > 0)
		return this->posUserTransition;
	else
		return this->posTransition;
}
