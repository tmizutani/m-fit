#include "../include/Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


#ifdef __linux__
	#include <execinfo.h>
#endif

/************************************************************************
* Função que imprime as mensagems log da aplicação
* ela automaticamente concatena o timestamp.
*************************************************************************
* param (E): fmt - Formato da impressão (printf like)
* param (E): ... - Argumentos para a impressão (printf like)
************************************************************************
* Histórico:
* 27/06/08 - Fabricio Lopes de Souza
* Criação.
************************************************************************/
int Log::writeLog(const char *fmt, ...)
{
	return 0;

	va_list argList;
	// String passada por arg
	char bufArg_cy[100];
	// String que vai ser impressa no log
	char bufLog_cy[200];
	// String com a horario atual
	char bufTime_cy[20];
	// Tempo
	struct tm *paux;
	time_t now;

	FILE *fd;

	memset(bufArg_cy , '\0',  sizeof(bufArg_cy));
	memset(bufLog_cy , '\0',  sizeof(bufLog_cy));
	memset(bufTime_cy, '\0', sizeof(bufTime_cy));

/*
#ifdef __linux__
	// Backtrace
	int btSize;
	void *btArray[128];
	char** btSymbols;

	btSize = backtrace(btArray, sizeof(btArray) / sizeof(void *));

	if (btSize > 0)
	{
		btSymbols = backtrace_symbols(btArray, btSize);
		if (btSymbols)
		{
			for (int i = btSize - 1; i >= 0; --i) {
				printf("btSymbols[%d] = %s", i, btSymbols[i]);
			}
		}
	}
#endif
*/

	fd = fopen("MFIT.log", "ab");

	if (fd == NULL)
		return false;

	// Transforma os "..."
	va_start(argList, fmt);
	vsprintf(bufArg_cy, fmt ,argList);
	va_end(argList);

	// Pega a data do instante atual
	time(&now);
	paux = localtime(&now);

	// Preenche um buffer com "dia/mes/ano : hora:min:seg ::"
	sprintf(bufTime_cy, "%02d/%02d/%d : %02d:%02d:%02d :: ",
			paux->tm_mday ,(paux->tm_mon + 1) ,(paux->tm_year + 1900) ,paux->tm_hour ,paux->tm_min ,paux->tm_sec);

	// Contatena o instante com os argumentos
	strcat(bufLog_cy, bufTime_cy);
	strcat(bufLog_cy, bufArg_cy);

	//writetty(bufLog_cy);
	fprintf(fd, "%s\n",bufLog_cy);

	fclose(fd);

	return 0;

}
