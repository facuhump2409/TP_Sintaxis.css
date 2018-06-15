#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEXEMA_SIZE 30

struct palabra {
	struct palabra *ant;
	struct palabra *sig;
	int nroLinea;
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

//struct palabra* nuevoNodo(char *lex) {
struct palabra* nuevoNodo(char lex[LEXEMA_SIZE],int nroLinea) {
	struct palabra *nuevaPalabra = (struct palabra*)malloc(sizeof(struct palabra));
	nuevaPalabra->ant = NULL;
	nuevaPalabra->sig = NULL;
	nuevaPalabra->nroLinea = nroLinea;
	strcpy(nuevaPalabra->lexema, lex);
	nuevaPalabra->token = NULL;
	return nuevaPalabra;
}

//void agregarPalabraAlFinal(struct palabra **head, char *lex,int nroLinea) {
void agregarPalabraAlFinal(struct palabra **head, char lex[LEXEMA_SIZE], int nroLinea) {
	struct palabra **temp = head;

	if (*temp) {
		while ((*temp)->sig) temp = &(*temp)->sig;

		(*temp)->sig = nuevoNodo(lex,nroLinea);
		(*temp)->sig->ant = *temp;
	}
	else (*head) = nuevoNodo(lex,nroLinea);
}

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

const char *tokenType[8] = {
	"PALABRA RESERVADA",
	"IDENTIFICADOR DE FUNCION",
	"IDENTIFICADOR",
	"OPERADOR",
	"CARACTER DE PUNTUACION",
	"CONSTANTE NUMERICA",
	"CADENA DE CARACTERES",
	"NO IDENTIFICADO"
};

#define CANT_RESERVADAS 36
const char *palabras_reservadas[CANT_RESERVADAS] = {
	"do", "while", "if", "double", "int", "struct", "for",
	"break", "else", "long", "switch",
	"case", "char", "return", "float", "void",
	"sizeof", "const", "short", "default", "unsigned",
	"continue", "signed", "goto", "volatile",
	"static","auto", "enum", "register", "extern", "union",
	"printf","scanf","malloc","memcpy",
	"typedef" //aunque no lo implementamos
};
int esReservada(char *lexema) {
	int i;
	for (i = 0; i < CANT_RESERVADAS; i++) if (strcmp(lexema, palabras_reservadas[i]) == 0) return 1;
	return 0;
}

#define CANT_RESERVADAS_PREPROC 11
const char *palabras_reservadas_preprocesador[CANT_RESERVADAS_PREPROC] = { //revisar despues de un #
	"include", "elif", "endif",
	"pragma", "defined", "ifdef", "ifndef",
	"undef", "line", "error",
	"define" //aunque no lo implementamos
};
int esReservadaPreproc(char *lexema) {
	int i;
	for (i = 0; i < CANT_RESERVADAS_PREPROC; i++) if (strcmp(lexema, palabras_reservadas_preprocesador[i]) == 0) return 1;
	return 0;
}

#define CANT_OPERADORES 13
const char *operadores[CANT_OPERADORES] = { //no usamos la | sola
	"<",">","=","+","-","/",
	"*","!","&","%","^","&&","||"
};//tambien /*,*/,//
int esOperador(char *lexema) {
	int i;
	for (i = 0; i < CANT_OPERADORES; i++) if (strcmp(lexema, operadores[i]) == 0) return 1;
	return 0;
}

#define CANT_TIPOS_DE_IDENTIFICADORES 8
const char *tipos_de_identificadores[CANT_TIPOS_DE_IDENTIFICADORES] = {
	"double", "int", "struct", "void",
	"char", "long", "float", "short"
};
int esTipoDeIdentificador(char *lexema) {
	int i;
	for (i = 0; i < CANT_TIPOS_DE_IDENTIFICADORES; i++)  if (strcmp(lexema, tipos_de_identificadores[i]) == 0) return 1;
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

int dentro_de_cadena = 0;
int dentro_de_comentario_simple = 0;
int dentro_de_comentario_multiple = 0;
int tipo = -1;

void analizarPalabra(struct palabra *elemento, struct idReconocido **headIdentificadores) {

	//CADENAS DE CHARS (dentro del "string")
	if (dentro_de_cadena && elemento->lexema[0] != '"') { elemento->token = tokenType[CADENA_DE_CARACTERES]; }

	//CADENAS DE CHARS (dentro o final del comentario multiple)
	else if (dentro_de_comentario_multiple) {
		if (strcmp(elemento->lexema, "*/") != 0) elemento->token = tokenType[CADENA_DE_CARACTERES];
		else { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_comentario_multiple = 0; } }

	//CADENAS DE CHARS (dentro o final del comentario simple)
	else if (dentro_de_comentario_simple){
		if (elemento->lexema[0] != '\n') elemento->token = tokenType[CADENA_DE_CARACTERES];
		else dentro_de_comentario_simple = 0; /*elemento->token = NULL;*/ }

	//CARACTERES DE PUNTUACION (inicio de comentario multiple)
	else if (strcmp(elemento->lexema,"/*") == 0) { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_comentario_multiple = 1; }

	//CADENAS DE CHARS (para librerias)
	else if (elemento->ant && elemento->ant->lexema[0] == '<' && elemento->ant->ant && strcmp(elemento->ant->ant->lexema, "include") == 0 && elemento->sig && elemento->sig->lexema[0] == '>') { elemento->token = tokenType[CADENA_DE_CARACTERES]; }

	//CARACTERES DE PUNTUACION (inicio o fin de un "string")
	else if (elemento->lexema[0] == '"' && elemento->lexema[1] == '\0') { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_cadena = !dentro_de_cadena; }

	//CARACTERES DE PUNTUACION (inicio de comentario simple)
	else if (strcmp(elemento->lexema, "//") == 0) { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_comentario_simple = 1; }

	//CADENA DE CHARS (un char solo)
	else if (elemento->ant && elemento->sig && elemento->ant->lexema[0] == '\'' && elemento->sig->lexema[0] == '\'') { elemento->token = tokenType[CADENA_DE_CARACTERES]; }

	//IDENTIFICADORES (identificados anteriormente)
	else if ((tipo = esIdReconocido(*headIdentificadores, elemento->lexema))) {
		switch (tipo) {
		case ID_FUNC: elemento->token = tokenType[IDENTIFICADOR_FUNCION]; break;
		case ID_VAR: elemento->token = tokenType[IDENTIFICADOR]; break;
		case ID_MAL: elemento->token = tokenType[NO_IDENTIFICABLE]; break; } tipo = -1; }

	//PALABRAS RESERVADAS
	else if (esReservada(elemento->lexema)) { elemento->token = tokenType[PALABRA_RESERVADA]; }

	//PALABRAS RESERVADAS (del preprocesador)
	else if (elemento->ant && elemento->ant->lexema[0] == '#' && esReservadaPreproc(elemento->lexema)) { elemento->token = tokenType[PALABRA_RESERVADA]; }

	//OPERADORES
	else if (esOperador(elemento->lexema)) { elemento->token = tokenType[OPERADOR]; } //tiene que ir antes de ispunct

	//CARACTERES DE PUNTUACION
	else if (ispunct(elemento->lexema[0]) && elemento->lexema[0] != '_' && elemento->lexema[1] == '\0') { elemento->token = tokenType[CAR_PUNTUACION]; }

	//IDENTIFICADORES (del tipo: int a,b,c;) (este debe ir antes de constantes numericas)
	else if (elemento->ant && elemento->ant->ant && (strcmp(elemento->ant->lexema, ",") == 0) && (esTipoDeIdentificador(elemento->ant->ant->lexema) || (esIdReconocido(*headIdentificadores, elemento->ant->ant->lexema) == ID_VAR))) {
		if (isalpha(elemento->lexema[0]) || elemento->lexema[0]=='_') { elemento->token = tokenType[IDENTIFICADOR]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_VAR); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); }	}

	//CONSTANTES NUMERICAS
	else if (isdigit(elemento->lexema[0]) && elemento->ant && !esTipoDeIdentificador(elemento->ant->lexema)) { elemento->token = tokenType[NUMEROS]; }

	//IDENTIFICADORES (de funcion)
	else if ( elemento->ant && elemento->sig && elemento->sig->lexema[0] == '(' && ( esTipoDeIdentificador(elemento->ant->lexema) || (elemento->ant->ant && esTipoDeIdentificador(elemento->ant->ant->lexema) && elemento->ant->lexema[0] == '*') ) ) {
		if (isalpha(elemento->lexema[0]) || elemento->lexema[0] == '_') { elemento->token = tokenType[IDENTIFICADOR_FUNCION]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_FUNC); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }


	//IDENTIFICADORES (de variables)
	else if ( elemento->ant && ( esTipoDeIdentificador(elemento->ant->lexema) || (elemento->ant->ant && esTipoDeIdentificador(elemento->ant->ant->lexema) && elemento->ant->lexema[0] == '*') ) ) {
		if (isalpha(elemento->lexema[0]) || elemento->lexema[0] == '_') { elemento->token = tokenType[IDENTIFICADOR]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_VAR); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }


	else { elemento->token = tokenType[NO_IDENTIFICABLE]; }
}

void separarLexemas(struct palabra **head, char *nombreArchivo)
{

	int i = 0;
	char casoEspecial;
	int j = 0;
	int linea = 1;
	char caracterEspecial[3] = "\0";
	char unCaracter[3] = "\0";
	char buffer[LEXEMA_SIZE] = "\0";
	FILE *archivo = fopen(nombreArchivo, "r");
	if (archivo == NULL)
	{
		printf("No se pudo abrir el archivo\n");
		return 0;
	}
	unCaracter[0] = getc(archivo); //leo el primer caracter y despues me fijo que es
	do
	{

		if (isalpha(unCaracter[0]) || isdigit(unCaracter[0]) || unCaracter[0] == '_' || unCaracter[0] == '.')
		{
			do
			{
				buffer[i] = unCaracter[0];
				i++;
				unCaracter[0] = getc(archivo);
			} while ((isalpha(unCaracter[0]) || isdigit(unCaracter[0]) || unCaracter[0] == '_' || unCaracter[0] == '.') && unCaracter[0] != EOF);
			i = 0;
			agregarPalabraAlFinal(&(*head), buffer, linea);
		}
		//strcpy (buffer, caracter);
		if (isspace(unCaracter[0]) == 0)
		{
			if (unCaracter[0] == '&' || unCaracter[0] == '/')
			{
				casoEspecial = getc(archivo);
				if (casoEspecial == '&' && unCaracter[0] == '&')
				{
					buffer[0] = unCaracter[0];
					buffer[1] = casoEspecial;
					agregarPalabraAlFinal(&(*head), buffer, linea);
					unCaracter[0] = getc(archivo);
					if (isspace(unCaracter[0])) { unCaracter[0] = getc(archivo); } // si entra aca despues se tiene que avanzar un caracter
					for (j = 0; j<LEXEMA_SIZE; j++) { buffer[j] = '\0'; } //limpias el buffer
				}
				else if (isdigit(casoEspecial) || isspace(casoEspecial))//((casoEspecial != '/' || casoEspecial != '*')&& unCaracter[0] == '/')
				{
					agregarPalabraAlFinal(&(*head), unCaracter, linea); //agrego el dividido solo
					ungetc(casoEspecial, archivo); // devolves igual lo que venga despues del dividido para que se trabaje
					unCaracter[0] = getc(archivo); // avanzas
					if (isspace(unCaracter[0])) { unCaracter[0] = getc(archivo); }
				}
				else { ungetc(casoEspecial, archivo); } // en otro caso devolves el caracter al stream.
			}
			if (unCaracter[0] == '%' || unCaracter[0] == '\\' || unCaracter[0] == '/' || unCaracter[0] == '|')
			{
				unCaracter[1] = getc(archivo);
				agregarPalabraAlFinal(&(*head), unCaracter, linea);
				if (strcmp(unCaracter, "//") == 0)
				{
					caracterEspecial[0] = getc(archivo);
					while (caracterEspecial[0] != '\n' && caracterEspecial[0] != EOF)
					{
						for (j = 0; j<LEXEMA_SIZE; j++) { buffer[j] = '\0'; }
						if ((isalpha(caracterEspecial[0]) || isdigit(caracterEspecial[0]) || caracterEspecial[0] == '_' || caracterEspecial[0] == '.' || isspace(caracterEspecial[0]) == 0) && caracterEspecial[0] != EOF) {
							do
							{
								buffer[i] = caracterEspecial[0];
								i++;
								caracterEspecial[0] = getc(archivo);
							} while ((isalpha(caracterEspecial[0]) || isdigit(caracterEspecial[0]) || caracterEspecial[0] == '_' || caracterEspecial[0] == '.' || isspace(caracterEspecial[0]) == 0) && caracterEspecial[0] != EOF);
						}
						if (strcmp(buffer, "")) { agregarPalabraAlFinal(&(*head), buffer, linea); }
						//if (caracterEspecial[0] == '\n') { linea++; }
						if (i == 0) { caracterEspecial[0] = getc(archivo); }
						i = 0;
						//if (isspace(buffer[0]) == 0) agregarPalabraAlFinal(&(*head), buffer, linea);
						//if (caracterEspecial[0] == '\n') { agregarPalabraAlFinal(&(*head), caracterEspecial,linea); }

					}
					if (caracterEspecial[0] == '\n') {
						agregarPalabraAlFinal(&(*head), caracterEspecial, linea);
						linea++;
					}
					//agregarPalabraAlFinal(&(*head),caracterEspecial);
				}
				else if (strcmp(unCaracter, "/*") == 0)
				{
					caracterEspecial[0] = getc(archivo);
					while (caracterEspecial[0] != '*' && caracterEspecial[0] != EOF)
					{
						for (j = 0; j<LEXEMA_SIZE; j++) { buffer[j] = '\0'; }
						if (((isalpha(caracterEspecial[0]) || isdigit(caracterEspecial[0]) || caracterEspecial[0] == '_' || caracterEspecial[0] == '.' || isspace(caracterEspecial[0]) == 0) && caracterEspecial[0] != EOF && caracterEspecial[0] != '*')) {
							do
							{
								buffer[i] = caracterEspecial[0];
								i++;
								caracterEspecial[0] = getc(archivo);
							} while ((isalpha(caracterEspecial[0]) || isdigit(caracterEspecial[0]) || caracterEspecial[0] == '_' || caracterEspecial[0] == '.' || isspace(caracterEspecial[0]) == 0) && caracterEspecial[0] != EOF && caracterEspecial[0] != '*');
						}
						if (strcmp(buffer, "")) { agregarPalabraAlFinal(&(*head), buffer, linea); }
						if (caracterEspecial[0] == '\n') { linea++; }
						if (i == 0 || isspace(caracterEspecial[0])) { caracterEspecial[0] = getc(archivo); }
						i = 0;
						//if (buffer[0] != "") agregarPalabraAlFinal(&(*head), buffer, linea);
						//if (caracterEspecial[0] == '\n') { agregarPalabraAlFinal(&(*head), caracterEspecial,linea); }

					}
					caracterEspecial[1] = getc(archivo);
					if (strcmp(caracterEspecial, "*/") == 0) {

						agregarPalabraAlFinal(&(*head), caracterEspecial, linea);
						//linea++;
					}
				}
				caracterEspecial[1] = '\0';
				unCaracter[1] = '\0';
			}
			else { agregarPalabraAlFinal(&(*head), unCaracter, linea); }
			unCaracter[1] = '\0';
		}
		if (unCaracter[0] == '\n') { linea++; }
		for (j = 0; j<LEXEMA_SIZE; j++)buffer[j] = '\0';

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

void guardarEnArchivo(struct palabra *head, char *nombreArchivo) {
	FILE *archivo;
	if (archivo = fopen(nombreArchivo, "w")) {
		fprintf(archivo, "LINEA,LEXEMA,TOKEN\n");
		while (head) {
			if (head->lexema[0] != '\n' && head->token != NULL) fprintf(archivo, "%d,%s,%s\n", head->nroLinea, head->lexema, head->token);
			//pongo la condicion para el tema del \n de los comentarios, reviso el token por las dudas que haya otro error en otra cosa

			head = head->sig;
		}
		fclose(archivo);
		printf("La informacion fue guardada correctamente al archivo");
	}
	else printf("\nError al crear el archivo de guardado\n");
}

void mostrarPorPantalla(struct palabra *head) {//@Cambiar LEXEMA_SIZE manualmente
	printf("LINEA\t\t\t    LEXEMA\tTOKEN\n\n");
	while (head) {
		if (head->lexema[0] != '\n' && head->token != NULL) printf("%3d %30s\t%s\n", head->nroLinea, head->lexema, head->token);
		//pongo la condicion para el tema del \n de los comentarios, y reviso el token por las dudas que haya otro error en otra cosa

		head = head->sig;
	}
	printf("\n");
}

void mostrarNoReconocidos(struct palabra *head) { //tiene lexema size de 30
	printf("\n\n\n\t\t-----Elementos no reconocidos:-----\n\n");
	while (head) {
		if (head->token == tokenType[NO_IDENTIFICABLE]) printf("%30s\t%s\n", head->lexema, head->token);

		head = head->sig;
	}
	printf("\n\n\n\n\n");
}

void borrarPalabras(struct palabra **head) {
	struct palabra* actual = *head;
	struct palabra* siguiente;

	while (actual) {
		siguiente = actual->sig;
		free(actual);
		actual = siguiente;
	}
	*head = NULL;
}

void borrarIdentificados(struct idReconocido **headIdentificadores) {
	struct idReconocido* actual = *headIdentificadores;
	struct idReconocido* siguiente;

	while (actual) {
		siguiente = actual->sig;
		free(actual);
		actual = siguiente;
	}
	*headIdentificadores = NULL;
}

int main(int argc, char *argv[]) {

	struct palabra *head=NULL;
	struct idReconocido *headIdentificadores=NULL;

	if (argc == 2 || argc == 3) {
		separarLexemas(&head, argv[1]);

		analizadorLexico(head, &headIdentificadores);

		if (argc == 3) { guardarEnArchivo(head, argv[2]); return 0; }

		//mostrarNoReconocidos(head);

		mostrarPorPantalla(head);
	}
	else printf("Incorrectos parametros para la ejecucion del programa\n usar: (exe dir) o (exe dir dir)\n");

	borrarPalabras(&head);
	borrarIdentificados(&headIdentificadores);
    return 0;
}
