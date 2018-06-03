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
	int esID;
	struct idReconocido *sig;
};
#define ID_FUNC 1
#define ID_VAR 2
#define ID_MAL 3

struct idReconocido* nuevoId(char *lex, int condicion) {
	struct idReconocido *nuevo = (struct idReconocido*)malloc(sizeof(struct idReconocido));
	nuevo->lexema = lex;
	nuevo->sig = NULL;
	switch (condicion) {
	case ID_FUNC: nuevo->esID = ID_FUNC; break;
	case ID_VAR: nuevo->esID = ID_VAR; break;
	case ID_MAL: nuevo->esID = ID_MAL; break;
	}
	return nuevo;
}

void agregarIdentificadorAlFinal(struct idReconocido **head, char *lex, int condicion) {
	struct idReconocido **temp = head;
	struct idReconocido *nuevo = nuevoId(lex, condicion);

	while (*temp) temp = &(*temp)->sig;

	nuevo->sig = *temp;
	*temp = nuevo;
}

#define PALABRA_RESERVADA 0
#define IDENTIFICADOR_FUNCION 1
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

#define CANT_TIPOS_DE_IDENTIFICADORES 8
const char *tipos_de_identificadores[CANT_TIPOS_DE_IDENTIFICADORES] = {
	"double", "int", "struct", "void",
	"char", "long", "float", "short"
};
int esTipoDeIdentificador(char *lexema) {
	for (int i = 0; i < CANT_TIPOS_DE_IDENTIFICADORES; i++) {
		if (strcmp(lexema, tipos_de_identificadores[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

int esIdReconocido(struct idReconocido *head, char *lex) {
	while (head) {
		if (strcmp(head->lexema, lex) == 0) {
			if (head->esID == ID_FUNC) return ID_FUNC;
			else if (head->esID == ID_VAR) return ID_VAR;
			else return ID_MAL;
		}
		head = head->sig;
	}
	return 0;
}

void analizarPalabra(struct palabra *elemento, struct idReconocido **headIdentificadores) { 
	//@@cuando revisamos de que tipo es el anterior seria mas facil ver el token en vez de recorrer toda la lista de cosas
	//@@FALTA: cuando hay comentarios, tanto con // como /**/
	int tipo = -1;

	if (dentro_de_cadena && elemento->lexema[0] != '"') elemento->token = tokenType[CADENA_DE_CARACTERES];

	else if (elemento->lexema[0] == '"' && elemento->lexema[1] == '\0') { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_cadena = !dentro_de_cadena; }

	else if (esReservada(elemento->lexema)) elemento->token = tokenType[PALABRA_RESERVADA];

	else if (elemento->ant && elemento->ant->lexema[0] == '#' && esReservadaPreproc(elemento->lexema) ) elemento->token = tokenType[PALABRA_RESERVADA];

	else if (esOperador(elemento->lexema)) elemento->token = tokenType[OPERADOR]; //tiene que ir antes de ispunct

	else if (ispunct(elemento->lexema[0]) && elemento->lexema[0]!='_' && elemento->lexema[1] == '\0') elemento->token = tokenType[CAR_PUNTUACION];

	else if (isdigit(elemento->lexema[0]) && elemento->ant && !esTipoDeIdentificador(elemento->ant->lexema)) elemento->token = tokenType[NUMEROS];

	else if ((tipo = esIdReconocido(*headIdentificadores, elemento->lexema))) {
		switch (tipo) {
		case ID_FUNC: elemento->token = tokenType[IDENTIFICADOR_FUNCION]; break;
		case ID_VAR: elemento->token = tokenType[IDENTIFICADOR]; break;
		case ID_MAL: elemento->token = tokenType[NO_IDENTIFICABLE]; break;
		} }

	else if (elemento->ant && ( esTipoDeIdentificador(elemento->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->lexema)==ID_VAR ) && elemento->sig && elemento->sig->lexema[0] == '(') {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR_FUNCION]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_FUNC); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }
	
	else if (elemento->ant && (esTipoDeIdentificador(elemento->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->lexema) == ID_VAR) && elemento->sig && elemento->sig->lexema[0] == '(' && elemento->ant->ant && elemento->ant->ant->lexema[0] == '*') {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR_FUNCION]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_FUNC); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }

	else if (elemento->ant && (esTipoDeIdentificador(elemento->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->lexema) == ID_VAR)) {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_VAR); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }
	
	else if (elemento->ant && elemento->ant->ant && (esTipoDeIdentificador(elemento->ant->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->ant->lexema) == ID_VAR) && elemento->ant->lexema[0] == '*') {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_VAR); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); }	}
	else elemento->token = tokenType[NO_IDENTIFICABLE];
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

//@@@@Una vez dentro de un "" el programa no debe cortar strings, por ej el %30s aparece como % 30s separado
void separarLexemas(struct palabra **head, char *nombreArchivo) {

	int i = 0;
	int j = 0;
	char unCaracter[2] = "\0";
	char buffer[LEXEMA_SIZE] = "\0";
	FILE *archivo = fopen(nombreArchivo, "r");
	if (archivo == NULL) {
		printf("No se pudo abrir el archivo\n");
		return 0;
	}
	
	unCaracter[0] = getc(archivo); //leo el primer caracter y despues me fijo que es
	do {
		if (isalpha(unCaracter[0]) || isdigit(unCaracter[0]) || unCaracter[0] == "_") {
			do {
				buffer[i] = unCaracter[0];
				i++;
				unCaracter[0] = getc(archivo);
			} while (isalpha(unCaracter[0]) || isdigit(unCaracter[0]) || unCaracter[0] == "_");
			i = 0;
			agregarPalabraAlFinal(&(*head), buffer);
		}
		//strcpy (buffer, caracter);
		if (isspace(unCaracter[0]) == 0) agregarPalabraAlFinal(&(*head), unCaracter);

		for (j = 0; j<LEXEMA_SIZE; j++) { buffer[j] = '\0'; }
		unCaracter[0] = getc(archivo);
	} while (unCaracter[0] != EOF);
	fclose(archivo);
}

void analizadorLexico(struct palabra *elemento, struct idReconocido **headIdentificadores) {
	while (elemento) {
		analizarPalabra(elemento, &(*headIdentificadores));
		elemento = elemento->sig;
	}
}

int main(int argc, char *argv[]) //@Ver si el archivo llega con o sin comas al principio y fin
{
	struct palabra *head=NULL;
	struct idReconocido *headIdentificadores=NULL;

	//separarLexemas(&head, argv[1]);
	separarLexemas(&head, "C:\\Users\\Brenda-Vero-Frank\\Desktop\\main.txt" );

	analizadorLexico(head,&headIdentificadores);

	//@ ==3 cuando recibis el archivo de origen y el de salida
	if (argc >= 3) guardarEnArchivo(head,argv[2]);
	else mostrarPorPantalla(head); 

	system("pause");
	//@@@@LIBERAR TODA LA MEMORIA????
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

