#include <stdio.h>
#include <vector>
#include <QThread>
#include <QObject>
#include <QImage>

#include "cv.h"
#include "highgui.h"

#include "../include/Time.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"

#include "../include/Transition.h"
#include "../include/VisualRythim.h"

#include "../include/DetectTransitions.h"

#include "../include/Effect.h"
#include "../include/Project.h"

#include "../include/Cut.h"

#include "../include/Filters.h"
#include "../include/Log.h"

extern Project *currentProject;

/************************************************************************
* Construtor
*************************************************************************
* param (E): nenhum
*************************************************************************
* return : nenhum
*************************************************************************
* Hist�rico:
* 15/08/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

Cut::Cut()
{
	this->threshold = 0;	
}

/************************************************************************
* Fun��o que faz a detec��o das transi��es do tipo corte.
*************************************************************************
* param (E): Video* vdo => v�deo onde ser� feita a detec��o
* param (E): Transition *transitions => lista de transi��es
*************************************************************************
* return : Lista de transi��es.
*************************************************************************
* Hist�rico:
* 16/08/08 - Thiago Mizutani
* Inclus�o das chamadas das fun��es para processamento da imagem do RV.
* Cria��o do RV, suaviza��o, mapa de bordas, binariza��o e contagem dos
* pontos.
* 13/08/08 - Thiago Mizutani
* Chamadas das fun��es que completam o processo de detec��o de cortes.
* 06/07/08 - Thiago Mizutani
* Ordena��o das chamadas de fun��o.
* 04/07/08 - Thiago Mizutani
* Cria��o.
************************************************************************/

void Cut::detectTransitions(Video* vdo, std::vector<Transition>* transitionList)
{ 
	Frame* visualRythim= new Frame();

	VisualRythim *vr = new VisualRythim();

	Log::writeLog("%s :: iniciando detec��o de cortes\n");

	emit sendMessage("Iniciando Deteccao", 0, 0);

	int threshold = 0;
	int thresholdBin = 0;
	int *trans = 0;
	long i = 0;
	double totalFrames = vdo->getFramesTotal();

	// Se a posicao do video n�o for a inicial, aponto para o primeiro frame.
	if (vdo->getCurrentPosition() != 0)
		vdo->seekFrame(0);

	// Crio o Ritmo VIsual do v�deo
	emit sendMessage("Criando VR", transitionList->size(), 0);
	visualRythim = vr->createVR(vdo);

	emit sendMessage("Removendo Wide", transitionList->size(), 5);
	visualRythim->removeWide();

//	visualRythim->write("RV.jpg");

	// Crio uma c�pia do frame original para realizar a valida��o dos cortes detectados posteriormente.
	Frame* visual = new Frame(visualRythim);

	// Passo o filtro de Canny no RV suavizado para destacar as bordas
	emit sendMessage("Criando Mapa de Bordas", transitionList->size(), 10);
	this->createBorderMap(visualRythim);

//	visualRythim->write("RV Canny.jpg");
	
	// Verifico se foi setado algum valor de limiar pelo usu�rio 
	threshold = this->defineThreshold(visualRythim->getHeight());

	// Defino o limiar para binariza��o da imagem.
	thresholdBin = (visualRythim->getMaxLum())/4;

	// Binarizo a imagem (transformo tudo em preto e branco)
	emit sendMessage("Binarizando RV", transitionList->size(), 10);
	visualRythim->binarizeImage(thresholdBin);

	// Realizo a contagem dos pontos das bordas que foram encontradas
	emit sendMessage("Contando Pontos", transitionList->size(), 10);
	trans = countPoints(visualRythim, threshold);
	
	for(i=0; i<(long)totalFrames; i++ )
	{
		if(trans[i]) 
		{
			
			if(validateCut(visual, i))
			{
				Transition* newTransition = new Transition(TRANSITION_CUT,i,"Corte");

				/**
				 * Verifico se na posi��o em que eu detectei um corte j� n�o foi considerada
				 * outro tipo de transi��o. Isso evita que o sistema diga que em uma mesma posi��o
				 * existam 2 transi��es diferentes.
				**/
			   if(this->validateTransition(i, transitionList))
					transitionList->push_back(*newTransition);

				emit sendMessage("Validando Cortes", transitionList->size(), 60);
			}
		}
	}

	emit sendMessage("Fim do Corte", transitionList->size(), 100);

	delete visualRythim;
	delete vr;

}

/************************************************************************
 * Fun��o que retorna o mapa de bordas de um Ritmo Visual
 *************************************************************************
 * param (E): Frame* visualRythim => Ritmo Visual do qual ser� gerado o
 *  											mapa de bordas
 *************************************************************************
 * return : mapa de bordas.
 *************************************************************************
 * Hist�rico:
 * 13/08/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

void Cut::createBorderMap(Frame* visualRythim)
{
	Filters* canny = new Filters();
	int minCannyThreshold = 100;
	int maxCannyThreshold = 200;

	// Se o usu�rio alterou o valor do m�nimo do Canny devo us�-lo
	if( currentProject->getUserMinCanny() )
		minCannyThreshold = currentProject->getUserMinCanny();

	// Se o usu�rio alterou o valor do m�ximo do Canny devo us�-lo
	if( currentProject->getUserMaxCanny() )
		maxCannyThreshold = currentProject->getUserMaxCanny();

	// Crio o mapa de bordas do RV com o operador Canny.

	Log::writeLog("%s :: min = %d", __FUNCTION__, minCannyThreshold);
	Log::writeLog("%s :: max = %d", __FUNCTION__, maxCannyThreshold);

	canny->Canny(visualRythim, (double)minCannyThreshold, (double)maxCannyThreshold, 3);

	delete canny;

}

/************************************************************************
 * Fun��o que retorna um novo limiar para detec��o de cortes. Caso o 
 * usu�rio tenha setado um novo valor retorna este valor, sen�o retorna o 
 * valor default
 *************************************************************************
 * param (E): Frame* visualRythim => Ritmo Visual do qual ser� gerado o
 *  											mapa de bordas
 *************************************************************************
 * return : mapa de bordas.
 *************************************************************************
 * Hist�rico:
 * 13/08/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

int Cut::defineThreshold(int height)
{
	double userThreshold = 0.0;
	double sysThreshold = 0.0;

	userThreshold = (double)currentProject->getUserThreshold();

	//	Log::writeLog("%s :: userThreshold = %d", __FUNCTION__, userThreshold);
	//	Log::writeLog("%s :: height = %d", __FUNCTION__, height);
	//	Log::writeLog("%s :: teste = %lf", __FUNCTION__, height*(userThreshold/100.0));

	if (userThreshold)
		userThreshold = ((double)height * (userThreshold/100.0));
	else
		sysThreshold = height * 0.45;

	setThreshold(userThreshold > 0 ? (int)userThreshold : (int)sysThreshold);

	//	Log::writeLog("%s :: threshold(%d) ", __FUNCTION__, this->threshold);

	return (getThreshold());

}

/************************************************************************
 * Fun��o que faz contagem dos pontos das bordas verticais detectadas.	
 *************************************************************************
 * param (E): Frame* borderMap => mapa de bordas do qual ser� feita a 
 * 											contagem dos pontos
 * param (E): int threshold => Valor do limiar para considera��es de onde
 * 										existem cortes
 *************************************************************************
 * return : array preenchido com 1 ou 0, onde houver corte ser� preenchido
 * com 1 e onde n�o houver com 0. Cada coluna do mapa de bordas representa
 * 1 frame do v�deo. Com isso ser� poss�vel saber onde est�o os cortes.
 *************************************************************************
 * Hist�rico:
 * 13/08/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

int* Cut::countPoints(Frame* borderMap, int threshold)
{
	// Coluna do mapa de bordas que estou analisando.
	int column = 0;
	int points = 0;
	int width = borderMap->getWidth();
	int height = borderMap->getHeight();

	int* transitions;
	int luminance = 0;	

	transitions = (int*)malloc(sizeof(int)*width);
	memset(transitions,'\0',width);

	//Log::writeLog("%s :: threshold[%d] ", __FUNCTION__, threshold);
	//	Log::writeLog("%s :: width[%d] ", __FUNCTION__, width);
	//	Log::writeLog("%s :: height[%d] ", __FUNCTION__, height);

	/**
	 *	Varro toda a imagem coluna por coluna, pixel a pixel, verificando se
	 *	o pixel � branco. Se for branco, significa que faz parte da borda.
	 *	Ao t�rmino da contagem de pontos de uma coluna, verifico se o numero
	 *	de pontos da borda � maior ou igual ao valor do limiar. Se for marco 
	 *	no vetor transitions que aquela coluna representa um corte.	
	 * **/
	for (column = 0; column < width; column++)	
	{
		for (int y=0; y < height; y++)
		{
			luminance = borderMap->getPixel(column,y);
			if(luminance == 255)
			{
				points++;	
			}
		}
		// Se o nro de pontos da reta for > que o limiar, ent�o � corte.
		transitions[column] = points >= threshold ? 1 : 0;

		points = 0;
	}

	return (transitions);	
}

/*************************************************************************
 * Fun��o que retorna o valor do limiar escolhido para a contagem de pontos
 **************************************************************************
 * param (E): nenhum
 **************************************************************************
 * return : Treshould
 **************************************************************************
 * Hist�rico:
 * 13/08/08 - Thiago Mizutani
 * Cria��o.
 *************************************************************************/

int Cut::getThreshold(void)
{

	return (this->threshold);

}

/************************************************************************
 * Fun��o que seta o valor do limiar (private threshold) da classe CUT
 *************************************************************************
 * param (E): int threshold => Valor do limiar
 *************************************************************************
 * return :  nenhum.
 *************************************************************************
 * Hist�rico:
 * 13/08/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

int Cut::setThreshold(int threshold)
{
	this->threshold = threshold;

	return 0;
}

/************************************************************************
 * Fun��o que valida se a transi��o encontrada � realmente um corte.
 *************************************************************************
 * param (E): Frame* visual : Ritmo Visual original para an�lise
 * param (E): int position : Posi��o em que foi encontrado o suposto corte
 *************************************************************************
 * return :  TRUE OU FALSE.
 *************************************************************************
 * Hist�rico:
 * 06/08/08 - Thiago Mizutani
 * Cria��o.
 ************************************************************************/

int Cut::validateCut(Frame* visual, int position)
{
	int width = visual->getWidth();
	int height = visual->getHeight();

	long totalPixels = height*2;
	long totalNextLum = 0; // Soma total da luminancia dos pixels dos proximos 2 frames.
	long totalPreviousLum = 0;

	long nextAvarage = 0;
	long previousAvarage = 0;

	long difference = 0;

	//	Log::writeLog("%s :: position[%d]", __FUNCTION__, position);	

	// Verifico os pr�ximos 2 frames
	for(int x=position+1; x<=position+2 && x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			totalNextLum = totalNextLum + visual->getPixel(x,y);
		}
	}

	// Se for Fade-In, totalNextLum = 0, ou valor muito baixo.

	nextAvarage = totalNextLum/totalPixels;

	//	Log::writeLog("%s :: totalNextLum = %ld, nextAvarage = %ld", __FUNCTION__, totalNextLum, nextAvarage);	

	for(int x=position-1; x>=position-2 && x>0; x--)
	{
		for(int y=0; y<height; y++)
		{
			totalPreviousLum = totalPreviousLum + visual->getPixel(x,y);
		}
	}

	previousAvarage = totalPreviousLum/totalPixels;

	//	Log::writeLog("%s :: totalPreviousLum = %ld, previousAvarage = %ld", __FUNCTION__, totalPreviousLum, previousAvarage);	

	difference = nextAvarage - previousAvarage;

	//	Log::writeLog("%s :: difference = %ld", __FUNCTION__, difference);	

	/**
	 *  Como posso ter a transi��o de uma cena mais clara para uma mais escura
	 *  ainda preciso considerar estas diferen�as. Portanto se a diferen�a for
	 *  negativa, at� certo ponto ainda � v�lida, por exemplo, uma cena possui m�dia
	 *  50 e vai para uma em que a m�dia da luminosidade � 40, a diferen�a ser� -10,
	 *  por�m isto n�o � um fade-in, ent�o considero como corte.
	 **/
	if( difference < -15 )
	{
		/**
		 *	Como posso ter o corte entre uma cena extremamente clara (media da luminancia 
		 *	acima de 100) para uma em que a m�dia � extremamente baixa, devo considerar 
		 *	que a diferen�a vai ser absurda, por�m n�o deixa de ser um corte.
		 *	Os valores aqui usados foram baseados em testes deste trabalho. N�o h� men��o disso
		 *	em nenhum dos trabalhos relacionados estudados.
		 * **/
		if (previousAvarage >= 100 && nextAvarage >= 10)		
		{
			return (TRUE);
		}
		// Saiu de uma cena e foi para um FADE-IN
		return (FALSE);
	}
	/**
	 *	Se a diferen�a entre as m�dias for muito alta, ou a m�dia da lumin�ncia dos frames
	 *	anteriores for muito baixa, significa que est� vindo de um fade-out, portanto n�o
	 *	podemos considerar isto como um corte.
	 * **/
	else if( previousAvarage < 10 ) // 10 ser� o padr�o. TALVEZ ser� poss�vel que o usu�rio altere isso.	
	{
		// Veio de fade-out
		return (FALSE);
	}
	/**
	 *	Se a diferen�a das lumin�ncias for 0 ou muito baixa, a probabilidade de ter
	 *	ocorrido um erro � significamente alta, visto que em uma �nica cena a luz n�o
	 *	varia tanto (com excess�o de alguns casos), portanto n�o podemos considerar
	 *	um corte se a altera��o da lumin�ncia n�o for significativa.
	 * **/
	else if(difference >= 0 && difference < 20) // Chute!
	{
		// Objeto de cena
		return (FALSE);
	}

	// Diferen�a de lumin�ncias v�lida se encontra entre 31,99.
	return (TRUE);

}

/************************************************************************
 * M�todo de in�cio da Thread. Inicia a detec��o das transi��es
 *************************************************************************
 * param (E): Nenhum.
 *************************************************************************
 * return : Nenhum.
 *************************************************************************
 * Hist�rico:
 * 10/12/08 - Fabr�cio Lopes de Souza
 * Cria��o.
 ************************************************************************/
void Cut::run()
{
	detectTransitions(currentProject->getVideo(), &currentProject->transitionList);
}

