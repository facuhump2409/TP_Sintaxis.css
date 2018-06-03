#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma warning(disable:4996)

#define LEXEMA_SIZE 30

struct palabra {
	struct palabra *ant;
	struct palabra *sig;
	char lexema[LEXEMA_SIZE];
	char *token;
};

struct idReconocido {
	char *lexema;
	int esIDdeFuncion;// = -1;
	int esIDdeVariable;// = -1;
	int estaMalDeclarado;// = 0;
	struct idReconocido *sig;
};
#define ID_FUNC 1
#define ID_VAR 2
#define MAL 3

struct idReconocido* nuevoId(char *lex, int condicion) {
	struct idReconocido *nuevo = (struct idReconocido*)malloc(sizeof(struct idReconocido));
	nuevo->lexema = lex;
	nuevo->sig = NULL;
	switch (condicion) {
	case ID_FUNC: nuevo->esIDdeFuncion = 1; nuevo->esIDdeVariable = 0; nuevo->estaMalDeclarado = 0; break;
	case ID_VAR: nuevo->esIDdeFuncion = 0; nuevo->esIDdeVariable = 1; nuevo->estaMalDeclarado = 0; break;
	case MAL: nuevo->esIDdeFuncion = 0; nuevo->esIDdeVariable = 0; nuevo->estaMalDeclarado = 1; break;
	}
	return nuevo;
}

void agregarIdentificadorAlFinal(struct idReconocido **head, char *lex, int condicion) {
	struct idReconocido **temp = head;
	struct idReconocido *nuevo = nuevoId(lex,condicion);

	while ((*temp)->sig) temp = &(*temp)->sig;

	nuevo->sig = *temp;
	*temp = nuevo;
}

#define PALABRA_RESERVADA 0
#define ID_FUNCION 1
#define IDENTIFICADOR 2
#define OPERADOR 3 
#define CAR_PUNTUACION 4
#define NUMEROS 5
#define CADENA_DE_CARACTERES 6
#define NO_IDENTIFICABLE 7

int dentro_de_cadena = 0;

const char *tokenType[8] = { 
	"PALABRA RESERVADA", //
	"IDENTIFICADOR DE FUNCION", 
	"IDENTIFICADOR",
	"OPERADOR", //
	"CARACTER DE PUNTUACION", //
	"CONSTANTE NUMERICA", //
	"CADENA DE CARACTERES", //
	"NO IDENTIFICADO"
};

#define CANT_RESERVADAS 31
const char *palabras_reservadas[CANT_RESERVADAS] = {
	"do", "while", "if", "double", "int", "struct", "for",
	"break", "else", "long", "switch",
	"case", "char", "return", "float", "void",
	"sizeof", "const", "short", "default", "unsigned",
	"continue", "signed", "goto", "volatile",
	"static","auto", "enum", "register", "extern", "union"
}; //"typedef"

int esReservada(char *lexema) {
	for (int i = 0; i < CANT_RESERVADAS; i++) if (strcmp(lexema, palabras_reservadas[i]) == 0) return 1;
	return 0;
}

#define CANT_RESERVADAS_PREPROC 11
const char *palabras_reservadas_preprocesador[CANT_RESERVADAS_PREPROC] = { //revisar despues de un #
	"include", "define", "elif", "endif",
	"pragma", "defined", "ifdef", "ifndef",
	"undef", "line", "error", 
};

int esReservadaPreproc(char *lexema) {
	for (int i = 0; i < CANT_RESERVADAS_PREPROC; i++) if (strcmp(lexema, palabras_reservadas_preprocesador[i]) == 0) return 1;
	return 0;
}

#define CANT_OPERADORES 12
const char *operadores[CANT_OPERADORES] = { // <>=+-/*!&|%^   //@QUE ES &, OPERADOR, FUNCION ,ETC???
	"<",">","=","+","-","/","*","!","&","|","%","^"
};

int esOperador(char *lexema) {
	for (int i = 0; i < CANT_OPERADORES; i++) if (strcmp(lexema, operadores[i]) == 0) return 1;
	return 0;
}

#define CANT_TIPOS_IDENTIFICADORES 10
const char *tipos_de_identificadores[CANT_TIPOS_IDENTIFICADORES] = {
	"double", "int", "struct",
	"char", "long", "float", "short"
};
int esTipoIdentificador(char *lexema) {
	for (int i = 0; i < CANT_TIPOS_IDENTIFICADORES; i++) if (strcmp(lexema, tipos_de_identificadores[i]) == 0) return 1;
	return 0;
}

//CREAR LISTA DE IDENTIFICADORES RECONOCIDOS

void analizarPalabra(struct palabra *elemento) {
	if (dentro_de_cadena && elemento->lexema[0] != '"') elemento->token = tokenType[CADENA_DE_CARACTERES];
	else if (elemento->lexema[0] == '"' && elemento->lexema[1] == '\0') { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_cadena = !dentro_de_cadena; }
	else if (esReservada(elemento->lexema)) elemento->token = tokenType[PALABRA_RESERVADA];
	else if (elemento->ant && elemento->ant->lexema[0] == '#' && esReservadaPreproc(elemento->lexema) ) elemento->token = tokenType[PALABRA_RESERVADA];
	else if (esOperador(elemento->lexema)) elemento->token = tokenType[OPERADOR]; //tiene que ir antes de ispunct
	else if (ispunct(elemento->lexema[0]) && elemento->lexema[0]!='_' && elemento->lexema[1] == '\0') elemento->token = tokenType[CAR_PUNTUACION];
	else if (isdigit(elemento->lexema[0] && !esTipoIdentificador(elemento->ant->token))) elemento->token = tokenType[NUMEROS];
}

//struct palabra* nuevoNodo(char *lex) {
struct palabra* nuevoNodo(char lex[LEXEMA_SIZE]) {
	struct palabra *nuevaPalabra = (struct palabra*)malloc(sizeof(struct palabra));
	nuevaPalabra->ant = NULL;
	nuevaPalabra->sig = NULL;
	strcpy(nuevaPalabra->lexema, lex);
	nuevaPalabra->token = NULL;
	return nuevaPalabra;
}

//void agregarPalabraAlFinal(struct palabra **head, char *lex) {
void agregarPalabraAlFinal(struct palabra **head,char lex[LEXEMA_SIZE]) {
	struct palabra **temp=head;

	if (*temp) {
		while ((*temp)->sig) temp = &(*temp)->sig;
		
		(*temp)->sig = nuevoNodo(lex);
		(*temp)->sig->ant = *temp;

		//temp->sig->sig = NULL;
		//temp->sig->token = NULL;
	}
	else {
		(*head) = nuevoNodo(lex);
	}
}

void guardarEnArchivo(struct palabra *head,char *nombreArchivo) {
	struct palabra *temp = head;
	FILE *archivo = fopen(nombreArchivo,"w");

	while (temp) {
		fprintf(archivo,"%s,%s\n", temp->lexema, temp->token);
		temp = temp->sig;
	}
	fclose(archivo);
}

void mostrarPorPantalla(struct palabra *head) {
	struct palabra *temp = head;
	
	while (temp) {
		printf("%30s\t", temp->lexema); //@Cambiar LEXEMA_ZISE manualmente
		printf("%s\n", temp->token);
		temp = temp->sig;
	}
	printf("\n");
}


void separarLexemas(struct palabra *head, char *nombreArchivo)
{
	int i = 0;
	char caracter;
	char buffer[LEXEMA_SIZE];
	FILE *archivo = fopen(nombreArchivo, "r");
	if (archivo == NULL)
	{
		printf("No se pudo abrir el archivo\n");
		return 0;
	}
	printf("lo abrio");
	caracter = getc(archivo); //leo el primer caracter y despues me fijo que es
	do
	{
		if (isalpha(caracter) || isdigit(caracter) || caracter == "_")
		{
			do
			{
				buffer[i] = caracter;
				i++;
				caracter = getc(archivo);
			} while (isalpha(caracter) || isdigit(caracter) || caracter == "_");
			i = 0;
			agregarPalabraAlFinal(&(*head), buffer);
		}
		strcpy (buffer, caracter);
		agregarPalabraAlFinal(&(*head), &caracter);
		strcpy(buffer, " ");
		caracter = getc(archivo);
	} while (caracter != EOF);
}




int main(int argc, char *argv[]) //@Ver si el archivo llega con o sin comas al principio y fin
{
	struct palabra *head=NULL;

	//agregarPalabraAlFinal(&head, "if");
	//analizarPalabra(head);
	
	//char archivoTrabajado[] = "C:\\Users\\Brenda-Vero-Frank\\Desktop\\main.txt";
	//separarLexemas(&head, archivoTrabajado);
	system("pause"); system("pause"); system("pause");


	if (argc >= 2) { //@@CAMBIAR a ==3 cuando recibamos el archivo de origen tmb
		guardarEnArchivo(head,argv[1]);
	}
	else {
		mostrarPorPantalla(head);
	}

	system("pause");
    return 0;
}


/*
struct palabra *prueba;
prueba = (struct palabra*)malloc(sizeof(struct palabra));
prueba->ant = NULL;
prueba->sig = NULL;
strcpy(prueba->lexema,"nada");
prueba->token = tokenType[ID_FUNCION];

agregarPalabraAlFinal(&head, "hola");
agregarPalabraAlFinal(&head, "hello");
agregarPalabraAlFinal(&head, "bye");
*/

