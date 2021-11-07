#include "cv.h"
#include "highgui.h"
#include <vector>

#include "../include/Log.h"
#include "../include/Time.h"

#include "../include/Histogram.h"
#include "../include/Frame.h"
#include "../include/Video.h"

#include "../include/Effect.h"
#include "../include/Color.h"

#include "../include/Filters.h"
#include "../include/Morphology.h"

#include "../include/VisualRythim.h"

#include "../include/Transition.h"
#include "../include/DetectTransitions.h"
#include "../include/Cut.h"
#include "../include/Fade.h"
#include "../include/Fadein.h"
#include "../include/Fadeout.h"
#include "../include/Dissolve.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <time.h>

#define EFFECT_NAME_SIZE 30
#define MAX_OPTIONS 30

void usage();

/************************************************************************
* Função main.
*************************************************************************
* param (E): argc e argv.
*************************************************************************
* return : nenhum.
*************************************************************************
* Histórico:
* 18/08/08 - Ivan Shiguenori Machida
* Criando parametro 'd' para modo video (dissolve).
* 29/07/08 - Thiago Mizutani
* Retirando alguns parametros desnecessários.
* Algum dia alguém criou. Coloquem historico a partir daqui.
************************************************************************/

int main(int argc, char* argv[])
{

	Frame *frame; // Frame original

	Frame *frameGray;	// Frame original em escala de cinza

	Frame *frameEffect; 	// Frame original com o efeito aplicado

	Color *color; 	// Objeto de efeitos de cor

	Video *vdo;

	//CvCapture* video = 0; 

	std::vector<Transition> transitionList;

	char filename_cy[100];	// Nome do arquivo

	char extension_cy[4];

	char *effectName;	// Nome do efeito aplicado

	int i, aux_i, effectCount = 0;

	//int detectNow = 0;

	enum inputType { IMAGE, VIDEO }; // Tipo da entrada

	inputType input;

	/** 
	 * Estrutura para armazenar a lista de efeitos aplicados
	 *  possibilitando assim aplicar efeitos sobre efeitos,
	 *  e aumentando a memória usada :)
	 **/
	struct 
	{
		Frame *frame;
		char  *name ;
	} effectsList[MAX_OPTIONS];


	// Se nao mandarem o nome do arquivo como primeiro parametro
	// adota default
	if (argc > 1)
	{
		strcpy(filename_cy, argv[1]);

		// Pega somente a extensao do arquivo
		strcpy(extension_cy, &filename_cy[strlen(filename_cy)-3]);

		Log::writeLog("%s :: file extension[%s]", __FUNCTION__, extension_cy);

		// Verifica se o arquivo carregado é um vídeo ou imagem.
		if (!strcmp(extension_cy,"AVI") || !strcmp(extension_cy,"avi"))
		{
			input = VIDEO;
		}
		else
		{
			input = IMAGE;
		}
	}
	else
	{
		strcpy(filename_cy, "teste.jpg");
	}

	// Tenta abrir o arquivo
	try
	{

		if (input == VIDEO)
		{
			Log::writeLog("%s :: new Video filename[%s]", __FUNCTION__, filename_cy);

			vdo = new Video(filename_cy); // Instancia um objeto da classe video

			Log::writeLog("%s :: new Video Loaded [%x]", __FUNCTION__, vdo);
		}
		else
		{
			Log::writeLog("%s :: new Frame filename[%s]", __FUNCTION__, filename_cy);

			frame = new Frame(filename_cy); // Instancia um objeto da classe video

			Log::writeLog("%s :: new Frame Loaded [%x]", __FUNCTION__, frame);
		}
	}
	catch (char *str)
	{
		Log::writeLog("%s :: Exception [%s]", __FUNCTION__, str);
		usage();
		return 1;
	}
	catch (...)
	{
		Log::writeLog("%s :: Unhandled Exception", __FUNCTION__);
		usage();
		return 1;
	}

	// Todos os tratamentos usamos img em cinza, então vamos 
	// transformar nosso frame em cinza.

	if (input == IMAGE) // É imagem (isso vai sumir depois... só vai servir de exemplo.)
	{
		color = new Color();

		// converte a imagem para tons de cinza.
		frameGray = color->convert2Gray(frame);

		// Faço a varredura nos parametros passados pela linha de comando
		for (i = 2 ; i < argc ; i++)
		{

			if (argv[i][0] != '-')
				continue;

			Log::writeLog("%s :: param[%d]", __FUNCTION__, argc);
			Log::writeLog("%s :: param: argv[%d] = [%s]", __FUNCTION__, i, argv[i]);

			// Cria uma nova string para colocar na lista de efeitos
			effectName = new char[ sizeof(char)*EFFECT_NAME_SIZE ];

			// Zera a variável auxiliar
			aux_i = 0;

			switch (argv[i][1])
			{
				case 'r':
					{
						Frame* frameWide = new Frame(frameGray);

						sprintf(effectName, "Remove Wide");

						frameWide->removeWide();

						frameEffect = frameWide;

						break;
					}
				case 't':
					{

						Filters *filters = new Filters();

						aux_i = atoi(argv[++i]);

						sprintf(effectName, "Treshold t=%d", aux_i);

						frameEffect = frameGray;
						
						filters->segment(frameEffect, aux_i);

						break;

					}
				case 'v':
					{

						Frame *frame2;
						Frame *frame3;
						Frame *frameNew;
						IplImage *imgDst;
						char filename_cy[100];

						strcpy(effectName, "Concatenate_Vertical");

						frame2 = new Frame(frame);
						frame3 = new Frame(argv[i+1]);



						imgDst = Frame::imgAlloc(
								cvSize(frame2->getWidth(), frame2->getHeight() + 3 + frame3->getHeight()),
								frame2->data->depth,
								frame2->data->nChannels);

						Log::writeLog("%s :: img_dst width[%d] height[%d]", __FUNCTION__, imgDst->width, imgDst->height);

						Log::writeLog("%s :: this x[%d] y[%d] width[%d] height[%d]", __FUNCTION__, 0, 0, frame2->getWidth(), frame2->getHeight());

						// Na imagem destino, seto a area de interesse o espaco que a primeira imagem ira ocupar
						cvSetImageROI(imgDst,
								cvRect
								(
								 0,
								 0,
								 frame2->getWidth(),
								 frame2->getHeight())
								);

						// então copio esta imagem para esta área
						frame2->imgCopy(frame2->data,imgDst);

						Log::writeLog("%s :: this x[%d] y[%d] width[%d] height[%d]", __FUNCTION__, 0, frame3->getHeight()+2, frame3->getWidth(), frame3->getHeight());

						// Agora, seto a area de interesse o espaco que a segunda imagem irá ocupar
						cvSetImageROI(imgDst,
								cvRect
								(
								 0,
								 frame2->getHeight()+2,
								 frame3->getWidth(),
								 frame3->getHeight())
								);

						// Copia a segunda parte do frame
						frame2->imgCopy(frame3->data,imgDst);

						// Removo as áreas de interesse da imagem
						cvResetImageROI(imgDst);

						cvLine(imgDst, cvPoint(0,frame2->getHeight()+2), cvPoint(frame2->getWidth(),frame2->getHeight()+2), CV_RGB(0, 0, 0));

						frameNew = new Frame(imgDst);

						frameEffect = frameNew;

						Frame::imgDealloc(imgDst);

						strcpy(filename_cy, argv[i-1]);
						strcat(filename_cy, " x ");
						strcat(filename_cy, argv[i+1]);

						frameNew->write(filename_cy);

						break;

					}

				case 'd':
					{
						Morphology *morph = new Morphology();

						strcpy(effectName, "Dilate");

						frameEffect = morph->dilate(frameGray);

						break;
					}
				case 'f':
					{
						strcpy(effectName, "Diagonal");

						Frame *frame2;

						frame2 = frame->getDiagonal();

						frameEffect = frame2;

						break;
					}
				case 'z':
					{
						Frame *a;
						Frame b;

						strcpy(effectName, "operator=");

						a = new Frame(frame);

						b = *a;

						Log::writeLog("%s :: b[%x]", __FUNCTION__, &b);

						frameEffect = &b;

						frameEffect->write(effectName);

						break;
					}
				case 'e':
					{

						Morphology *morph = new Morphology();

						strcpy(effectName, "Erode");

						frameEffect = morph->erode(frameGray);

						break;

					}
				case 'c':
					{

						Frame *frame2;
						Frame *frame3;

						strcpy(effectName, "Concatenate");

						frame2 = new Frame(argv[i+1]);
						frame3 = new Frame(frame);

						Log::writeLog("%s :: frame3[%x] + frame2[%x]", __FUNCTION__, frame3, frame2);
						Log::writeLog("%s :: frame3->data[%x]", __FUNCTION__, frame3->data);

						for (i=0 ; i <= 10 ; i++)
						{
							*frame3 += *frame2;
						}

						Log::writeLog("%s :: frame3[%x] + frame2[%x]", __FUNCTION__, frame3, frame2);
						Log::writeLog("%s :: frame3->data[%x]", __FUNCTION__, frame3->data);

						frameEffect = frame3;

						break;

					}
				case 's':       
					{

						Filters *filters = new Filters();

						frameEffect = frameGray;

						if (argv[i][2] == 'v')
						{
							strcpy(effectName, "Vertival Sobel");
							filters->Sobel(frameEffect, SOBEL_VERTICAL);
						}
						else if (argv[i][2] == 'h')
						{
							strcpy(effectName, "Horizontal Sobel");
							filters->Sobel(frameEffect, SOBEL_HORIZONTAL);               
						}
						else
						{
							strcpy(effectName, "Complete Sobel");
							filters->Sobel(frameEffect, SOBEL_COMPLETE);
						}

						break;
					}

				case 'C':
					{
						Filters* filters = new Filters();

						strcpy(effectName, "Canny");

						frameEffect = frameGray;

						filters->Canny(frameEffect, 100, 200, 3);

						break;
					}

				case 'l':
					{

						Filters *filters = new Filters();

						Frame *frameLP = new Frame(frameGray);

						strcpy(effectName, "Low-Pass Filter");

						//Se for passado algum argumento como valor para tamanho da máscara
						//será = tamanho passado, senão assume por default o valor 5.

						if ( i < (argc-1) ) // O ultimo argumento é argc-1
						{
							if (argv[i+1][0] <= '9' && argv[i+1][0] >= '3')
							{
								Log::writeLog("%s :: LowPass param[%s]", __FUNCTION__, argv[i+1]);

								aux_i = atoi(argv[++i]); //Passo o valor do proximo parametro passado
							}
						}

						// 5 é o padrão
						if (!aux_i)
						{
							aux_i = 5;
						}

						
						filters->lowPass(frameLP, aux_i); 

						frameEffect = frameLP;

						break;
					}

				case 'H':
					{
						Filters *filters = new Filters();
						// Se o usuário não escolher o tipo de matriz a ser usada o sistema adota
						// uma como padrão.

						if (('0' <= atoi(argv[i+1]) <= '9') && (i <= argc))	
						{
							aux_i = atoi(argv[++i]); // Incrementa i para que a proxima analise do for não pegue o mesmo parametro
						}
						else
						{
							aux_i = 1;
						}

						sprintf(effectName, "High-Pass kernel [%d]", aux_i);
						frameEffect = frameGray;
							
						filters->highPass(frameEffect, aux_i);

						break;
					}
				case 'h':
					{

						Histogram *hist;
						Frame *frameAux;

						// Se ja foi aplicado algum tipo de efeito, realizamo o histograma do ultimo efeito aplicado
						if (effectCount >= 1)
						{

							frameAux = effectsList[effectCount-1].frame;

							sprintf(effectName, "Histogram of %s", effectsList[effectCount-1].name);

							Log::writeLog("%s :: Histogram from effect[%d] [%s]\n",
									__FUNCTION__, effectCount-1, effectsList[effectCount-1].name);
						}
						else // Senão, aplicamos na imagem original mesmo
						{
							frameAux = frameGray;
							sprintf(effectName, "Histogram of %s", filename_cy);
						}

						//	Histogram *hist = new Histogram(frameAux->data);
						hist = frameAux->createHistogram();

						Log::writeLog("%s :: min[%d]: [%.0f], max[%d]: [%.0f]",
								__FUNCTION__, hist->getMinLuminance(), hist->getMin(), hist->getMaxLuminance(), hist->getMax());

						frameEffect = new Frame(hist->getMatrix(), 256, hist->getMax());

						break;
					}

				case 'b':
					{
						Frame* frameAux = new Frame(frameGray);

						int maxLum = 0;

						sprintf(effectName,"Binarize");

						frameEffect = frameAux;
							
						maxLum = frameEffect->getMaxLum();

						frameEffect->binarizeImage(maxLum/4);

						break;
					}

				case 'w':
					if (effectCount >= 1)
					{
						char imgname_cy[50];

						strcpy(imgname_cy, filename_cy);
						// Tira a extensao
						imgname_cy[strlen(imgname_cy) - 4] = '\0';

						sprintf(imgname_cy, "%s_%s.jpg", imgname_cy, effectsList[effectCount-1].name);

//						Log::writeLog("%s :: Writing effect [%s] in file [%s]\n", __FUNCTION__, effectsList[effectCount-1], imgname_cy);

						if(!cvSaveImage(imgname_cy,effectsList[effectCount-1].frame->data))
						{
							printf("Could not save: %s\n",imgname_cy);
						}
					}

					continue;

				case 'g':
					sprintf(effectName, "%s Gray", filename_cy);
					frameEffect = frameGray;
					break;

				case 'p':
					strcpy(effectName, filename_cy);
					frameEffect = frame;
					break;

				case '?':
				default:
					usage();
					return -1;
					break;

			}


			Log::writeLog("%s :: Effect[%d] : [%s] = [%x]", __FUNCTION__, effectCount, effectName, frameEffect);

			// Salva os dados na estrutura, para poder aplicar efeitos na imagem anterior
			effectsList[effectCount].name  = effectName;
			effectsList[effectCount].frame = frameEffect;

			Log::writeLog("%s :: effectsList[%d].frame = [%x]", __FUNCTION__, effectCount, effectsList[effectCount].frame);
			Log::writeLog("%s :: effectsList[%d].name= [%s]", __FUNCTION__, effectCount, effectsList[effectCount].name);

			// Imprime na tela
			cvNamedWindow(effectsList[effectCount].name, 1);
			cvShowImage(effectsList[effectCount].name, effectsList[effectCount].frame->data);

			effectCount++;

		}

		cvWaitKey(0);

		// Limpa a memória
		for (i=0 ; i < effectCount ; i++)
		{
			Log::writeLog("%s :: Destroy[%d] : [%s]", __FUNCTION__, i, effectsList[i].name);

			cvDestroyWindow(effectsList[i].name);
			delete effectsList[i].name;
			delete effectsList[i].frame;
		}
	}
	else if (input == VIDEO) // É vídeo
	{
		//cvNamedWindow(vdo->getName(), 1);

		for (i = 2 ; i < argc ; i++)
		{

			if (argv[i][0] != '-')
				continue;

			Log::writeLog("%s :: param[%d]", __FUNCTION__, argc);
			Log::writeLog("%s :: param: argv[%d] = [%s]", __FUNCTION__, i, argv[i]);

			// Zera a variável auxiliar
			aux_i = 0;

			switch (argv[i][1])
			{
				case 'w':
					{
						vdo->removeWide();
						break;
					}
				case 'h':
					{
						char imgname_cy[100];

						// Monta o ritmo visual por histograma
						VisualRythim *vrh = new VisualRythim();
						Frame *frameVRH;

						int vdoSize = 0;
						double *visual;

						Log::writeLog("%s :: createVRH", __FUNCTION__);

						visual = vrh->createVRH(vdo);

						vdoSize = cvRound(vdo->getFramesTotal());

						Log::writeLog("%s :: create VRH frame", __FUNCTION__);

						frameVRH = new Frame(visual, vdoSize, 256);

						sprintf(imgname_cy, "vrh_%s.jpg", vdo->getName());

						frameVRH->write(imgname_cy);

						cvShowImage(vdo->getName(), frameVRH->data);

						cvWaitKey(0);

						break;
					}
				case 'r':
					{
						// Monta o ritmo-visual
						char imgname_cy[100];

						VisualRythim *vr = new VisualRythim();
						Frame *vr_frame = new Frame();

						Log::writeLog("%s :: VisualRythim video[%s]", __FUNCTION__, vdo->getName());

						Log::writeLog("%s :: createVR[%x]", __FUNCTION__, vdo);

						vr_frame = vr->createVR(vdo);

						cvShowImage(vdo->getName(), vr_frame->data);

						sprintf(imgname_cy, "vr_%s.jpg", vdo->getName());

						vr_frame->write(imgname_cy);

						cvWaitKey(0);

						delete vr_frame;
						delete vr;

						break;
					}
				case 'n':
					{
						// Navigate
						char c;
						char frame_cy[20];
						Frame *frameVideo;
						Frame *frameHistogram;
						Histogram *hist;
						CvFont font;

						cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 0.5, 0.5, 0, 1, CV_AA);
						vdo->removeWide();
						vdo->removeBorder();

						frameVideo = vdo->getCurrentFrame();
						cvNamedWindow("Histograma"  , 1);
						cvNamedWindow(vdo->getName(), 1);

						do
						{

							c = cvWaitKey(0);

							Log::writeLog("%s :: pressed[%d][%c]", __FUNCTION__, c, c);

							// 44 = "<"
							if (c == 44)
							{
								frameVideo = vdo->getPreviousFrame();
								
							} // 46 = ">"
							else if (c == 46)
							{
								frameVideo = vdo->getNextFrame();


							} // 103 - g - go to
							else if ( c == 103 )
							{
								char d = 0;
								int  i = 0;
								char qnt_cy[10];
								int  z = 0;
								int pos;
								int pos_max;

								pos_max = cvRound(vdo->getFramesTotal());

								// Digitar a seguir o numero desejado e dar enter
								printf("Please type the pos and press enter\n");

								while ( d != 13 && d != -1 && d != 27) // 13 = ENTER 
								{

									if (i == 9)
									{
										break;
									}

									d = cvWaitKey(0);
									qnt_cy[i] = d;
									Log::writeLog("%s :: Entered : [%d] [%c] total [%d]", __FUNCTION__, d, d, z);

									i++;
								}

								qnt_cy[i-1] = 0;

								pos = atoi(qnt_cy);

								Log::writeLog("%s :: GoTo [%d] [%s] max[%d]", __FUNCTION__, pos, qnt_cy, pos_max);

								// Se for digitado mais do que tem, truncamos para o ultimo frame
								if (pos >= pos_max)
								{
									Log::writeLog("%s :: pos = pos_max", __FUNCTION__);
									pos = pos_max - 1;
								}

								vdo->seekFrame(pos);

								frameVideo = vdo->getCurrentFrame();

								// Se for o untimo frame tem q parar.
								if (!frameVideo)
									continue;

							} // -1 = Fechar a janela 27 = ESC 3 = CTRL+C
							else if (c == -1 || c == 27 || c == 3)
							{
								Log::writeLog("%s :: End key pressed [%d]", __FUNCTION__, c);
								break;
							}
							else
							{
								continue;
							}

							if (frameVideo == NULL)
								continue;

							frameGray = color->convert2Gray(frameVideo);

							hist = frameGray->createHistogram();

							Log::writeLog("%s :: min[%d]: [%.0f], max[%d]: [%.0f]",
		 						__FUNCTION__, hist->getMinLuminance(), hist->getMin(), hist->getMaxLuminance(), hist->getMax());


							// Printa no frame a posicao corrente
							// os valores dos pontos foram colocados mediante tentativa e erro
							//---
							cvRectangle(frameVideo->data,
									cvPoint(frameVideo->getWidth()-51 ,20),
									cvPoint(frameVideo->getWidth()-26 ,35),
									CV_RGB(255,255,255), -1);

							sprintf(frame_cy, "%0.0lf", vdo->getCurrentPosition());

							cvPutText(frameVideo->data, frame_cy,
									cvPoint(frameVideo->getWidth()-44,25),
									&font,
									CV_RGB(0,0,0));
							//---


							frameHistogram = new Frame(hist->getMatrix(), 256, hist->getMax());

							cvShowImage(vdo->getName(), frameVideo->data);

							cvShowImage("Histograma", frameHistogram->data);

							delete frameVideo;
							delete frameGray;
							delete frameHistogram;

						}
						while(true);


						break;
					}
				case 'f':
					{
						//Transition* transitions = new Transition();
						Fade* DTF = new Fade();
						
						DTF->detectTransitions(vdo, &transitionList);

						/*
					{
						Fade *fade;
						Frame *frameFADE;
						VisualRythim *vrh;
						int len_i;
						int i;
						int j;
						double *array_dy;
						double *array_vrh;

						double *array_fade;

						// Fade
						//
						

						double last_point = 0;
						int equal_points = 0;
						int plateau = 0 ;
						int fade_start = 0;
						int fade_end = 0 ;
						int variacao = 0;
						double fade_max = 0;
						int fade_max_idx = 0;
						double aux = 0;

						char imgname_cy[50];
Cançao do Alien.avi
						fade = new Fade();

						array_vrh = vrh->createVRH(vdo);

						len_i = cvRound(vdo->getFramesTotal());

						array_dy = fade->calcDerivative(array_vrh, len_i);

						// Ideia do ivan
						// para que fique visivel as partes negativas
						//for (i=0 ; i<len_i ; i++)
						//	array_dy[i]+= 50;

						//frameFADE = new Frame(array_dy, len_i, 256);

						//cvShowImage(vdo->getName(), frameFADE->data);

						for ( i=0 ; i < len_i ; i++)
						{

							Log::writeLog("%s :: last_point [%lf] = array_dy[%d][%lf]", __FUNCTION__, last_point, i, array_dy[i]);

							if (last_point == array_dy[i])
							{

								if (fade_start == 1)
									fade_start = 0;

								equal_points++;
							}
							else
							{
								if (fade_start == 0)
								{
									Log::writeLog("%s :: comecou uma variacao em : %d", __FUNCTION__, i);
									fade_start = i;
								}
								else if (array_dy[i] == 0.0)
								{
									fade_end = i;

									variacao = fade_end - fade_start;

									Log::writeLog("%s :: variacao de %d ateh %d, total de %d pontos", __FUNCTION__, fade_start, fade_end, variacao);

									// Se for maior que um limiar
									// esse limiar foi definido realizando alguns experimentos onde apareciam
									// transicoes diferentes de fade.
									if (variacao> 5)
									{

										for (j=fade_start ; j<fade_end; j++)
										{
											if (fabs(array_dy[j]) > fade_max)
											{
												fade_max = array_dy[j];
												fade_max_idx = j;
											}

										}

										Log::writeLog("%s :: pico do fade : idx : %d valor %lf", __FUNCTION__, fade_max_idx, fade_max);

										if (fade_max < 0)
											Log::writeLog("%s :: fade in em : %d", __FUNCTION__, fade_max_idx);
										else
											Log::writeLog("%s :: fade out em : %d", __FUNCTION__, fade_max_idx);

										fade_max = 0;
										fade_max_idx = 0;

									}
									else
									{
											Log::writeLog("%s :: Variacao muito pequena para ser considerada um fade [%d] < [%d]", __FUNCTION__, variacao, 5);
									}

									fade_start = fade_end = variacao = 0;
								}
							}

							last_point = array_dy[i];
						}

						//sprintf(imgname_cy, "vrh_derivative_%s.jpg", vdo->getName());

						//frameFADE->write(imgname_cy);

						//cvShowImage(vdo->getName(), frameFADE->data);

						//cvWaitKey(0);

						delete array_vrh;
						delete array_dy;
						delete vrh;
						delete fade;
						break;

					}
					*/
						break;
					}
				case 'c':
					{
						// Detecção de cortes
						
						//Transition* transitions = new Transition();
						Cut* DTC = new Cut();
						
						DTC->detectTransitions(vdo, &transitionList);
						
						break;
					}
				case 'd':
					{
						// Detecção de dissolve
						
						Dissolve* dissolve = new Dissolve(vdo);
						
						dissolve->calcVariance();
						
						break;
					}
			}

			break;
		}

		cvDestroyWindow(vdo->getName());
		delete vdo;

		if (transitionList.size() > 0)
		{
			unsigned int i = 0;
			Transition *transition;

			for (i = 0 ; i < transitionList.size() ; i++)
			{
				transition = &(transitionList.at(i));

				Log::writeLog("%s :: Find a transiction    ", __FUNCTION__);
				Log::writeLog("%s :: type :              %d", __FUNCTION__, transition->getType());
				Log::writeLog("%s :: posTransition :     %d", __FUNCTION__, transition->getPosTransition());
				Log::writeLog("%s :: posUserTransition : %d", __FUNCTION__, transition->getPosUserTransition());
				Log::writeLog("%s :: label :             %s", __FUNCTION__, transition->getLabel());
			}
		}

	}

	cvWaitKey(0);

	transitionList.clear();
}

void usage()
{
	int i;
	char *usage_cy[] = {
		"Use: MFIT.exe img/video -[t|d|e|s|h|p|g|l|H|w|n|z|f]  - Apply filter in img/video\n",
		"MFIT image -t T Segmentation of the image with the threshold T",
		"MFIT image -C   Apply Canny in the image",
		"MFIT image -d   Dilate the image",
		"MFIT image -e   Erode the image",
		"MFIT image -sh  Apply Sobel (Horizontal) filter in the image",
		"MFIT image -sv  Apply Sobel (Vertial) filter in the image",
		"MFIT image -s   Apply Sobel (Both) filter in the image",
		"MFIT image -h   Print the histogram of the last img",
		"MFIT image -p   Print the img_src",
		"MFIT image -g   Print the img_src in gray",
		"MFIT image -l M Apply low-pass filter with the kernel M",
		"MFIT image -H M Apply high-pass filter with the kernel M",
		"MFIT image -w   Write the last effect on a file",
		"MFIT image -z   Simulate a '=' operation with frame",
		"MFIT image -f   Get a image diagonal",
		"MFIT image -c image2 Concatenate image and image2 (horizontal)",
		"MFIT image -v image2 Concatenate image and image2 (Vertical)",

		"MFIT video -n   Navigate using '<' and '>' in a video file",
		"MFIT video -r   Generate a Visual Rythm",
		"MFIT video -h   Generate a Visual Rythm Histogram",
		"MFIT video -c   Detect cut transitions",
		""
	};

	for (i=0 ; usage_cy[i][0] != '\0' ; i++)
	{
		printf("%s\n", usage_cy[i]);
	}
}
