#ifndef XML_INCLUDE
#define XML_INCLUDE

#define	NODE		1
#define	ATTR		2
#define	VALUE		3
#define	ROOTNAME	5

#include <QXmlSimpleReader>
#include <QIODevice>
#include <QXmlInputSource>
#include <QtXml>
#include <QXmlReader>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/**
 *Classe responsável por detectar
 *o dissolve.
 */
class Xml
{
	private:
		QDomDocument doc;
		char *xmlName;


		Transition *transition;
		Effect *effect;

	public:


		char userCutThreshold[10];
		char text[30];

		int		sizeNodes;
		int		itemNumber;

		QFile *file;

		int openXml(char *);
		int readXml(char *);
		int createXml(char *, Project *project);
		int closeXml();

		char *getText();

		char *getUserCutThreshold();
		int getSizeNodes();
		void setItemNumber(int);

		Transition *getTransition();
		Effect *getEffect();
};
#endif
