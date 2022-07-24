#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Flight.h"
#include "fileHelper.h"

void	initFlight(Flight* pFlight, const AirportManager* pManager)
{
	Airport* pPortOr = setAiportToFlight(pManager, "Enter name of origin airport:");
	pFlight->nameSource = _strdup(pPortOr->name);
	int same;
	Airport* pPortDes;
	do {
		pPortDes = setAiportToFlight(pManager, "Enter name of destination airport:");
		same = isSameAirport(pPortOr, pPortDes);
		if (same)
			printf("Same origin and destination airport\n");
	} while (same);
	pFlight->nameDest = _strdup(pPortDes->name);
	initPlane(&pFlight->thePlane);
	getCorrectDate(&pFlight->date);
}

int		isFlightFromSourceName(const Flight* pFlight, const char* nameSource)
{
	if (strcmp(pFlight->nameSource, nameSource) == 0)
		return 1;
		
	return 0;
}


int		isFlightToDestName(const Flight* pFlight, const char* nameDest)
{
	if (strcmp(pFlight->nameDest, nameDest) == 0)
		return 1;

	return 0;


}

int		isPlaneCodeInFlight(const Flight* pFlight, const char*  code)
{
	if (strcmp(pFlight->thePlane.code, code) == 0)
		return 1;
	return 0;
}

int		isPlaneTypeInFlight(const Flight* pFlight, ePlaneType type)
{
	if (pFlight->thePlane.type == type)
		return 1;
	return 0;
}


void	printFlight(const Flight* pFlight)
{
	printf("Flight From %s To %s\t",pFlight->nameSource, pFlight->nameDest);
	printDate(&pFlight->date);
	printPlane(&pFlight->thePlane);
}

void	printFlightV(const void* val)
{
	const Flight* pFlight = *(const Flight**)val;
	printFlight(pFlight);
}


Airport* setAiportToFlight(const AirportManager* pManager, const char* msg)
{
	char name[MAX_STR_LEN];
	Airport* port;
	do
	{
		printf("%s\t", msg);
		myGets(name, MAX_STR_LEN,stdin);
		port = findAirportByName(pManager, name);
		if (port == NULL)
			printf("No airport with this name - try again\n");
	} while(port == NULL);

	return port;
}

void	freeFlight(Flight* pFlight)
{
	free(pFlight->nameSource);
	free(pFlight->nameDest);
	free(pFlight);
}


int saveFlightToFile(const Flight* pF, FILE* fp)
{
	if (!writeStringToFile(pF->nameSource, fp, "Error write flight source name\n"))
		return 0;

	if (!writeStringToFile(pF->nameDest, fp, "Error write flight destination name\n"))
		return 0;

	if (!savePlaneToFile(&pF->thePlane,fp))
		return 0;

	if (!saveDateToFile(&pF->date,fp))
		return 0;

	return 1;
}


int loadFlightFromFile(Flight* pF, const AirportManager* pManager, FILE* fp)
{

	pF->nameSource = readStringFromFile(fp, "Error reading source name\n");
	if (!pF->nameSource)
		return 0;

	pF->nameDest = readStringFromFile(fp, "Error reading destination name\n");
	if (!pF->nameDest)
		return 0;

	if (!loadPlaneFromFile(&pF->thePlane, fp))
		return 0;

	if (!loadDateFromFile(&pF->date, fp))
		return 0;

	return 1;
}

int	compareFlightBySourceName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameSource, pFlight2->nameSource);
}

int	compareFlightByDestName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameDest, pFlight2->nameDest);
}

int	compareFlightByPlaneCode(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->thePlane.code, pFlight2->thePlane.code);
}

int		compareFlightByDate(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;


	return compareDate(&pFlight1->date, &pFlight2->date);
	

	return 0;
}

int		saveFlightToBinaryFileCompressed(FILE* fp, Flight* st)
{
	BYTE data[2] = { 0 };
	BYTE dataTwo[3] = { 0 };
	BYTE dataThree[1] = { 0 };

	int lenSrc = (int)strlen(st->nameSource);
	int lenDst = (int)strlen(st->nameDest);
	data[0] = lenSrc << 3 | lenDst >> 2;
	data[1] = lenDst << 6 | st->thePlane.type << 4;
	data[1] = data[1] | st->date.month;

	if (fwrite(&data, sizeof(BYTE), 2, fp) != 2)
		return 0;

	char c = st->thePlane.code[0];
	int num = c - 65;
	char c1 = st->thePlane.code[1];
	int num1 = c1 - 65;
	char c2 = st->thePlane.code[2];
	int num2 = c2 - 65;
	char c3 = st->thePlane.code[3];
	int num3 = c3 - 65;

	dataTwo[0] = num << 3;
	dataTwo[0] = dataTwo[0] | (num1 >> 2);

	dataTwo[1] = (num1 & 0x3) << 6;
	dataTwo[1] = dataTwo[1] | ((num2 & 0x1F) << 1);
	dataTwo[1] = dataTwo[1] | ((num3 & 0x10) >> 4);
	dataTwo[2] = ((num3 & 0xF) << 4) | (st->date.year & 0xF);

	if (fwrite(&dataTwo, sizeof(BYTE), 3, fp) != 3)
		return 0;

	dataThree[0] = st->date.day;



	if (fwrite(&dataThree, sizeof(BYTE), 1, fp) != 1)
		return 0;



	if (fwrite(st->nameSource, sizeof(char), lenSrc, fp) != lenSrc)
		return 0;
	if (fwrite(st->nameDest, sizeof(char), lenDst, fp) != lenDst)
		return 0;

	return 1;
}
int readFlightFromCompressBinFile(FILE* fp, Flight* st) {

	BYTE data[2] = { 0 };
	BYTE dataTwo[3] = { 0 };
	BYTE dataThree[1] = { 0 };

	if (fread(&data, sizeof(BYTE), 2, fp) != 2)
	{
		fclose(fp);
		return 0;
	}

	int lenSrc = (data[0] >> 3);
	int lenDst = (((data[0] & 0x7) << 2) | (data[1] >> 6));
	st->thePlane.type = (data[1] >> 4) & 0x3;
	st->date.month = data[1] & 0xF;



	if (fread(&dataTwo, sizeof(BYTE), 3, fp) != 3)
		return 0;

	int codeDec = ((dataTwo[0] & 0xF8) >> 3);
	char codeChar = codeDec + 65;
	st->thePlane.code[0] = codeChar;

	codeDec = (((dataTwo[0]) & (0x7)) << 2) | ((dataTwo[1] & 0xC0) >> 6);
	codeChar = codeDec + 65;
	st->thePlane.code[1] = codeChar;

	codeDec = (dataTwo[1] >> 1) & (0x1F);
	codeChar = codeDec + 65;
	st->thePlane.code[2] = codeChar;

	codeDec = ((dataTwo[1] & 0x1) << 4) | ((dataTwo[2] & 0xF0) >> 4);
	codeChar = codeDec + 65;
	st->thePlane.code[3] = codeChar;
	st->thePlane.code[4] = 0;


	st->date.year = (dataTwo[2] & 0xF) + 2016;



	if (fread(&dataThree, sizeof(BYTE), 1, fp) != 1)
		return 0;
	st->date.day = dataThree[0] & 0x1F;

	st->nameSource = (char*)calloc(lenSrc + 1, sizeof(char));
	if (!st->nameSource)
		return 0;

	if (fread(st->nameSource, sizeof(char), lenSrc, fp) != lenSrc)
	{
		free(st->nameSource);
		return 0;
	}

	st->nameDest = (char*)calloc(lenDst + 1, sizeof(char));
	if (!st->nameDest)
		return 0;

	if (fread(st->nameDest, sizeof(char), lenDst, fp) != lenDst)
	{
		free(st->nameDest);
		return 0;
	}


	return 1;

}
