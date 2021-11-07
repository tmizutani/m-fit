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
* Histórico:
* 15/08/08 - Thiago Mizutani
* Criação.
************************************************************************/

Cut::Cut()
{
	this->threshold = 0;	
}

/************************************************************************
* Função que faz a detecção das transições do tipo corte.
*************************************************************************
* param (E): Video* vdo => vídeo onde será feita a detecção
* param (E): Transition *transitions => lista de transições
*************************************************************************
* return : Lista de transições.
*************************************************************************
* Histórico:
* 16/08/08 - Thiago Mizutani
* Inclusão das chamadas das funções para processamento da imagem do RV.
* Criação do RV, suavização, mapa de bordas, binarização e contagem dos
* pontos.
* 13/08/08 - Thiago Mizutani
* Chamadas das funções que completam o processo de detecção de cortes.
* 06/07/08 - Thiago Mizutani
* Ordenação das chamadas de função.
* 04/07/08 - Thiago Mizutani
* Criação.
************************************************************************/

void Cut::detectTransitions(Video* vdo, std::vector<Transition>* transitionList)
{ 
	Frame* visualRythim= new Frame();

	VisualRythim *vr = new VisualRythim();

	Log::writeLog("%s :: iniciando detecção de cortes\n");

	emit sendMessage("Iniciando Deteccao", 0, 0);

	int threshold = 0;
	int thresholdBin = 0;
	int *trans = 0;
	long i = 0;
	double totalFrames = vdo->getFramesTotal();

	// Se a posicao do video não for a inicial, aponto para o primeiro frame.
	if (vdo->getCurrentPosition() != 0)
		vdo->seekFrame(0);

	// Crio o Ritmo VIsual do vídeo
	emit sendMessage("Criando VR", transitionList->size(), 0);
	visualRythim = vr->createVR(vdo);

	emit sendMessage("Removendo Wide", transitionList->size(), 5);
	visualRythim->removeWide();

//	visualRythim->write("RV.jpg");

	// Crio uma cópia do frame original para realizar a validação dos cortes detectados posteriormente.
	Frame* visual = new Frame(visualRythim);

	// Passo o filtro de Canny no RV suavizado para destacar as bordas
	emit sendMessage("Criando Mapa de Bordas", transitionList->size(), 10);
	this->createBorderMap(visualRythim);

//	visualRythim->write("RV Canny.jpg");
	
	// Verifico se foi setado algum valor de limiar pelo usuário 
	threshold = this->defineThreshold(visualRythim->getHeight());

	// Defino o limiar para binarização da imagem.
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
				 * Verifico se na posição em que eu detectei um corte já não foi considerada
				 * outro tipo de transição. Isso evita que o sistema diga que em uma mesma posição
				 * existam 2 transições diferentes.
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
 * Função que retorna o mapa de bordas de um Ritmo Visual
 *************************************************************************
 * param (E): Frame* visualRythim => Ritmo Visual do qual será gerado o
 *  											mapa de bordas
 *************************************************************************
 * return : mapa de bordas.
 *************************************************************************
 * Histórico:
 * 13/08/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

void Cut::createBorderMap(Frame* visualRythim)
{
	Filters* canny = new Filters();
	int minCannyThreshold = 100;
	int maxCannyThreshold = 200;

	// Se o usuário alterou o valor do mínimo do Canny devo usá-lo
	if( currentProject->getUserMinCanny() )
		minCannyThreshold = currentProject->getUserMinCanny();

	// Se o usuário alterou o valor do máximo do Canny devo usá-lo
	if( currentProject->getUserMaxCanny() )
		maxCannyThreshold = currentProject->getUserMaxCanny();

	// Crio o mapa de bordas do RV com o operador Canny.

	Log::writeLog("%s :: min = %d", __FUNCTION__, minCannyThreshold);
	Log::writeLog("%s :: max = %d", __FUNCTION__, maxCannyThreshold);

	canny->Canny(visualRythim, (double)minCannyThreshold, (double)maxCannyThreshold, 3);

	delete canny;

}

/************************************************************************
 * Função que retorna um novo limiar para detecção de cortes. Caso o 
 * usuário tenha setado um novo valor retorna este valor, senão retorna o 
 * valor default
 *************************************************************************
 * param (E): Frame* visualRythim => Ritmo Visual do qual será gerado o
 *  											mapa de bordas
 *************************************************************************
 * return : mapa de bordas.
 *************************************************************************
 * Histórico:
 * 13/08/08 - Thiago Mizutani
 * Criação.
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
 * Função que faz contagem dos pontos das bordas verticais detectadas.	
 *************************************************************************
 * param (E): Frame* borderMap => mapa de bordas do qual será feita a 
 * 											contagem dos pontos
 * param (E): int threshold => Valor do limiar para considerações de onde
 * 										existem cortes
 *************************************************************************
 * return : array preenchido com 1 ou 0, onde houver corte será preenchido
 * com 1 e onde não houver com 0. Cada coluna do mapa de bordas representa
 * 1 frame do vídeo. Com isso será possível saber onde estão os cortes.
 *************************************************************************
 * Histórico:
 * 13/08/08 - Thiago Mizutani
 * Criação.
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
	 *	o pixel é branco. Se for branco, significa que faz parte da borda.
	 *	Ao término da contagem de pontos de uma coluna, verifico se o numero
	 *	de pontos da borda é maior ou igual ao valor do limiar. Se for marco 
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
		// Se o nro de pontos da reta for > que o limiar, então é corte.
		transitions[column] = points >= threshold ? 1 : 0;

		points = 0;
	}

	return (transitions);	
}

/*************************************************************************
 * Função que retorna o valor do limiar escolhido para a contagem de pontos
 **************************************************************************
 * param (E): nenhum
 **************************************************************************
 * return : Treshould
 **************************************************************************
 * Histórico:
 * 13/08/08 - Thiago Mizutani
 * Criação.
 *************************************************************************/

int Cut::getThreshold(void)
{

	return (this->threshold);

}

/************************************************************************
 * Função que seta o valor do limiar (private threshold) da classe CUT
 *************************************************************************
 * param (E): int threshold => Valor do limiar
 *************************************************************************
 * return :  nenhum.
 *************************************************************************
 * Histórico:
 * 13/08/08 - Thiago Mizutani
 * Criação.
 ************************************************************************/

int Cut::setThreshold(int threshold)
{
	this->threshold = threshold;

	return 0;
}

/************************************************************************
 * Função que valida se a transição encontrada é realmente um corte.
 *************************************************************************
 * param (E): Frame* visual : Ritmo Visual original para análise
 * param (E): int position : Posição em que foi encontrado o suposto corte
 *************************************************************************
 * return :  TRUE OU FALSE.
 *************************************************************************
 * Histórico:
 * 06/08/08 - Thiago Mizutani
 * Criação.
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

	// Verifico os próximos 2 frames
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
	 *  Como posso ter a transição de uma cena mais clara para uma mais escura
	 *  ainda preciso considerar estas diferenças. Portanto se a diferença for
	 *  negativa, até certo ponto ainda é válida, por exemplo, uma cena possui média
	 *  50 e vai para uma em que a média da luminosidade é 40, a diferença será -10,
	 *  porém isto não é um fade-in, então considero como corte.
	 **/
	if( difference < -15 )
	{
		/**
		 *	Como posso ter o corte entre uma cena extremamente clara (media da luminancia 
		 *	acima de 100) para uma em que a média é extremamente baixa, devo considerar 
		 *	que a diferença vai ser absurda, porém não deixa de ser um corte.
		 *	Os valores aqui usados foram baseados em testes deste trabalho. Não há menção disso
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
	 *	Se a diferença entre as médias for muito alta, ou a média da luminância dos frames
	 *	anteriores for muito baixa, significa que está vindo de um fade-out, portanto não
	 *	podemos considerar isto como um corte.
	 * **/
	else if( previousAvarage < 10 ) // 10 será o padrão. TALVEZ será possível que o usuário altere isso.	
	{
		// Veio de fade-out
		return (FALSE);
	}
	/**
	 *	Se a diferença das luminâncias for 0 ou muito baixa, a probabilidade de ter
	 *	ocorrido um erro é significamente alta, visto que em uma única cena a luz não
	 *	varia tanto (com excessão de alguns casos), portanto não podemos considerar
	 *	um corte se a alteração da luminância não for significativa.
	 * **/
	else if(difference >= 0 && difference < 20) // Chute!
	{
		// Objeto de cena
		return (FALSE);
	}

	// Diferença de luminâncias válida se encontra entre 31,99.
	return (TRUE);

}

/************************************************************************
 * Método de início da Thread. Inicia a detecção das transições
 *************************************************************************
 * param (E): Nenhum.
 *************************************************************************
 * return : Nenhum.
 *************************************************************************
 * Histórico:
 * 10/12/08 - Fabrício Lopes de Souza
 * Criação.
 ************************************************************************/
void Cut::run()
{
	detectTransitions(currentProject->getVideo(), &currentProject->transitionList);
}

