#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#pragma warning(disable:4996)
#define LEXEMA_SIZE 30

struct palabra {
	struct palabra *ant;
	struct palabra *sig;
	char lexema[LEXEMA_SIZE];
	char *token;
};

#define PALABRA_RESERVADA 0
#define ID_FUNCION 1
#define OPERADOR 2
#define CAR_PUNTUACION 3
#define NUMEROS 4

//definir token error

const char *tokenType[5] = { "Palabra reservada", "Identificador de funcion", "Operador","Caracter puntuacion","Numeros"};

struct palabra* nuevoNodo(char lex[LEXEMA_SIZE]) {
	struct palabra *nuevaPalabra = (struct palabra*)malloc(sizeof(struct palabra));
	nuevaPalabra->ant = NULL;
	nuevaPalabra->sig = NULL;
	strcpy(nuevaPalabra->lexema, lex);
	nuevaPalabra->token = tokenType[OPERADOR];
	return nuevaPalabra;
}

void agregarPalabraAlFinal(struct palabra **head,char lex[LEXEMA_SIZE]) {
	struct palabra **temp=head;

	if (*temp) {
		while ((*temp)->sig) temp = &(*temp)->sig;
		(*temp)->sig = nuevoNodo(lex);
		(*temp)->sig->ant = temp;
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
		fprintf("%s,%s\n", temp->lexema, temp->token);
		temp = temp->sig;
	}
	fclose(archivo);
}

void mostrarPorPantalla(struct palabra *head) {
	struct palabra *temp = head;
	
	while (temp) {
		printf("%30s\t", temp->lexema); //@Cambiar LEXEMA_ZISE manualmente
		printf("%s\n", temp->lexema);
		temp = temp->sig;
	}
	printf("\n");
}


int main(int argc, char *argv[])
{
	struct palabra *head=NULL;
	


	if (argc > 2) {
		guardarenarchivo(head,argv[1]);
	}
	else {
		mostrarporpantalla(head);
	}


	agregarPalabraAlFinal(&head, "hola");
	agregarPalabraAlFinal(&head, "hello");
	agregarPalabraAlFinal(&head, "bye");
	mostrarPorPantalla(head);
	
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
*/

