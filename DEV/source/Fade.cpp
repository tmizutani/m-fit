#include "cv.h"
#include "highgui.h"
#include <QImage>
#include <vector>

#include "../include/Time.h"
#include "../include/Histogram.h"
#include "../include/Frame.h"

#include "../include/Video.h"
#include "../include/Transition.h"
#include "../include/VisualRythim.h"
#include "../include/DetectTransitions.h"

#include "../include/Project.h"

#include "../include/Fade.h"

#include "../include/Log.h"

extern Project *currentProject;


/************************************************************************
 * Função que calcula a derivada no exito y de um dado array
 * retorna um array com esta derivada.
 *************************************************************************
 * param (E): double array -> array a ser derivado no eixo y
 *************************************************************************
 * return : double* -> array com seus valores derivados
 *************************************************************************
 * Histórico:
 * 14/08/08 - Fabrício Lopes de Souza
 * Criação.
 ************************************************************************/
double* Fade::calcDerivative(double *array, int size_i)
{
	// Para calcular a derivada no eixo y , calculamos para cada ponto fy:
	// dy = ((f(y+1) - f(y-1)) / 2)

	//double dy;
	int i = 0;
	double *array_dy;

	Log::writeLog("%s :: array[%x] size_i[%d]", __FUNCTION__, array, size_i);

	array_dy = (double*)malloc(sizeof(double)*size_i);

	// Calculo para a posicao 0
	array_dy[i] = ( 0 - array[i] ) / 2;
	Log::writeLog("%s :: array_y[%d] = %lf array_dy[%d] = %lf", __FUNCTION__, i, array[i], i, array_dy[i]);

	for ( i = 1 ; i < size_i ; i++ )
	{
		array_dy[i] = (( array[i+1] - array[i-1] ) / 2);
		Log::writeLog("%s :: array_y[%d] = %lf array_dy[%d] = %lf", __FUNCTION__, i, array[i], i, array_dy[i]);
	}

	return array_dy;
}

/************************************************************************
 * Função que detecta as transições do tipo FADE (i.e. : fade-in e fade-out)
 *************************************************************************
 * param (E): Video* vdo -> video à detectar as transições
 * param (S): Transition* transitions -> Posicão corrente da lista de transições
 *************************************************************************
 * return : Nenhum
 *************************************************************************
 * Histórico:
 * 14/08/08 - Fabrício Lopes de Souza
 * Criação.
 ************************************************************************/
void Fade::detectTransitions(Video* vdo, std::vector<Transition>* transitionList)
{

	// Objeto para o RVH
	VisualRythim *vrh;

	// Tamanho do video em frames
	int len_i;

	// Contadores
	int i;

	// Porcentagem atual do processo de detecção
	int percent;

	// Array do VRH
	double *array_vrh;

	// Array da primeira derivada
	double *array_dy;

	// Ultimo ponto analizado
	double last_point = 0;

	// Poicao em que comecamos a analizar um fade
	int fade_start = 0;

	// Posicao aonde encerramos a analise de um fade
	int fade_end = 0 ;

	// Quantidade de frames presentes na transição
	int var = 0;

	// Variacao do ponto atual com o ponto anterior
	double var_d = 0.0;

	// Posicao do fade
	int fade_pos = 0;

	// Objeto Transicao
	Transition *transition = 0x0;

	// Tipo do fade: 0 - fadein / 1 - fadeout
	int type;

	// Label do fade
	char label[100];

	emit sendMessage("Iniciando Deteccao", 0, 0);

	// Processo de detecção de transições do tipo FADE
	vrh = new VisualRythim();

	// Removemos partes indesejaveis
	emit sendMessage("Removendo Wide", transitionList->size(), 0);
	vdo->removeWide();
	emit sendMessage("Removendo Borda", transitionList->size(), 5);
	vdo->removeBorder();

	emit sendMessage("Criando VRH", transitionList->size(), 10);

	// 1- Pegamos o video e criamos o Ritmo Visual por Histograma
	array_vrh = vrh->createVRH(vdo);

	len_i = cvRound(vdo->getFramesTotal());

	emit sendMessage("Arredondando VRH", transitionList->size(), 15);

	// Arredondamos os valores do VRH
	for (i = 0 ; i < len_i ; i++)
	{
		array_vrh[i] = cvRound(array_vrh[i]);
	}

	emit sendMessage("Calculando Derivadas", transitionList->size(), 20);

	// 2- Tiramos a derivada
	array_dy = calcDerivative(array_vrh, len_i);

	// Descomentar para obter uma imagem do RVH derivado
	/*******************************************************/
	/*
	{
		int i;
		double *array_dy_aux;
		double *array_vrh_aux;

		Frame *frameVRH;
		Frame *frameVRHD;

		array_dy_aux = (double*)malloc(sizeof(double)*len_i);
		array_vrh_aux = (double*)malloc(sizeof(double)*len_i);

		for (i = 0 ; i < len_i ; i++)
		{
			array_dy_aux[i] = array_dy[i] + 50;
			array_vrh_aux[i] = array_vrh[i] + 50;
		}

		frameVRH  = new Frame(array_vrh_aux, len_i, 256, false);
		frameVRHD = new Frame(array_dy_aux , len_i, 256, false);

		frameVRH-> write("vrh_dump.jpg");
		frameVRHD->write("vrhd_dump.jpg");

		delete array_dy_aux;
		delete array_vrh_aux;
		delete frameVRH;
		delete frameVRHD;
	}
	*/
	/*******************************************************/

	// A partir dai é necessário fazer uma análize na função derivada.
	// O que iremos fazer é o seguinte:
	// Varreremos todo o array em buscas de variações muito que tenham uma duração relativamente grande
	// essa é a característica de um fade.
	// Se essa variação terminar com um frame preto, então temos um fade-out, caso ela comece com um frame preto,
	// temos um fade-in

	// Desconsideramos o primeiro ponto da derivada
	// pois ele sempre será '0'
	last_point = array_dy[1];

	for ( i=2 ; i < len_i ; i++)
	{

		percent = cvRound(20 + ((((float)i / (float)len_i ) * 100) * 0.8));

		if (percent != 100)
		{
			emit sendMessage("Analizando Derivadas", transitionList->size(), percent);
		}

		// Reseta controles
		fade_end = var = 0;

		var_d = fabs(fabs(array_dy[i]) - fabs(last_point));

		Log::writeLog("%s :: var_d[%0.3lf] i[%d] array_dy[%0.3lf] array_vrh[%0.3lf]", __FUNCTION__, var_d, i, array_dy[i], array_vrh[i] );

		// Condições para se iniciar a verificação de um FADE
		if ( fade_start == 0 && ( var_d > 0.0 && var_d < 10.0 && array_dy[i-1] == 0))
		{
			Log::writeLog("%s :: fade_start in %d", __FUNCTION__, i);

			fade_start = i-1;

			fade_end = var = 0;

		} // Condições para se terminar um fade, o valor quebrado foi adotado pois os vídeos comerciais
		// tem muito ruído, então a variação dificilmente chega a ser exatamente 0
		else if ( fade_start > 0 && ( fabs(array_dy[i]) <= 0.003) )
		{
			fade_end = i;

			Log::writeLog("%s :: fade_end in %d", __FUNCTION__, fade_end);

			var = fade_end - fade_start;

			Log::writeLog("%s :: %d - %d, total points : %d", __FUNCTION__, fade_start, fade_end, var);

			// Temos que ter mais do que 10 frames na variação para considerarmos um fade e
			// a variação não pode sermuito abrupta ( tem que ser menor que 10 ). Esses dois pontos
			// eliminam 100% os cortes da detecção do fade.
			if (var < 10 || var_d >= 10.0)
			{
				// Não é um fade. Reseta os controles.
				fade_start = 0;
				Log::writeLog("%s :: [%d] < [%d] - not a fade, var_d [%lf]", __FUNCTION__, var, 10, var_d);

				// Voltamos um Frame, pois podemos ter "comido" um possivel início fade
				i --;

				continue;
			}

			if (array_vrh[fade_start] <= 5.0)
			{
				type = TRANSITION_FADEIN;
				strcpy(label, "Fade-In");
				fade_pos = fade_start;
				Log::writeLog("%s :: fade in: %d", __FUNCTION__, fade_pos);
			}
			else if (array_vrh[fade_end] <= 5.0)
			{
				type = TRANSITION_FADEOUT;
				strcpy(label, "Fade-Out");
				fade_pos = fade_end;
				Log::writeLog("%s :: fade out: %d", __FUNCTION__, fade_pos);
			}
			else
			{
				Log::writeLog("%s :: nothing: %d", __FUNCTION__);
				fade_start = 0;
				continue;
			}

			// Criamos a transição
			transition = new Transition(type, fade_pos, label);

			/**
			 * Verifico se na posição em que eu detectei um corte já não foi considerada
			 * outro tipo de transição. Isso evita que o sistema diga que em uma mesma posição
			 * existam 2 transições diferentes.
			 **/
			if( this->validateTransition((long)fade_pos, transitionList) )
				transitionList->push_back(*transition);

			fade_start = 0;

		}

		// Ultimo ponto, igual a ponto corrente
		last_point = array_dy[i];
	}

	// Reseta o ROI craido pelo removeWide e removeBorder
	vdo->resetROI();

	emit sendMessage("Fim do Corte", transitionList->size(), 100);

	delete array_vrh;
	delete array_dy;
	delete vrh;
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
void Fade::run()
{
	detectTransitions(currentProject->getVideo(), &currentProject->transitionList);
}
