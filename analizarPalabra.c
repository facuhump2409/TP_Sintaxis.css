void analizarPalabra(struct palabra *elemento, struct idReconocido **headIdentificadores) { 

	//CADENAS DE CHARS
	if (dentro_de_cadena && elemento->lexema[0] != '"') { elemento->token = tokenType[CADENA_DE_CARACTERES]; }

	//CADENAS DE CHARS
	else if (dentro_de_comentario_multiple) {
		if (strcmp(elemento->lexema, "*/") != 0) elemento->token = tokenType[CADENA_DE_CARACTERES];
		else { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_comentario_multiple = 0; } }
	
	//CADENAS DE CHARS
	else if (dentro_de_comentario_simple){
		if (elemento->lexema[0] != '\n') elemento->token = tokenType[CADENA_DE_CARACTERES];
		else dentro_de_comentario_simple = 0; /*elemento->token = NULL;*/ }

	//CARACTERES DE PUNTUACION
	else if (strcmp(elemento->lexema,"/*") == 0) { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_comentario_multiple = 1; }
	
	//CADENAS DE CHARS
	else if (elemento->ant && elemento->ant->lexema[0] == '<' && elemento->ant->ant && strcmp(elemento->ant->ant->lexema, "include") == 0 && elemento->sig && elemento->sig->lexema[0] == '>') { elemento->token = tokenType[CADENA_DE_CARACTERES]; }

	//CARACTERES DE PUNTUACION
	else if (elemento->lexema[0] == '"' && elemento->lexema[1] == '\0') { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_cadena = !dentro_de_cadena; }

	//CARACTERES DE PUNTUACION
	else if (strcmp(elemento->lexema, "//") == 0) { elemento->token = tokenType[CAR_PUNTUACION]; dentro_de_comentario_simple = 1; }

	//CARACTERES DE PUNTUACION
	else if (elemento->ant && elemento->sig && elemento->ant->lexema[0] == '\'' && elemento->sig->lexema[0] == '\'') { elemento->token = tokenType[CADENA_DE_CARACTERES]; }

	//IDENTIFICADORES
	else if ((tipo = esIdReconocido(*headIdentificadores, elemento->lexema))) {
		switch (tipo) {
		case ID_FUNC: elemento->token = tokenType[IDENTIFICADOR_FUNCION]; break;
		case ID_VAR: elemento->token = tokenType[IDENTIFICADOR]; break;
		case ID_MAL: elemento->token = tokenType[NO_IDENTIFICABLE]; break; } tipo = -1; }

	//PALABRAS RESERVADAS
	else if (esReservada(elemento->lexema)) { elemento->token = tokenType[PALABRA_RESERVADA]; }

	//PALABRAS RESERVADAS
	else if (elemento->ant && elemento->ant->lexema[0] == '#' && esReservadaPreproc(elemento->lexema)) { elemento->token = tokenType[PALABRA_RESERVADA]; }

	//OPERADORES
	else if (esOperador(elemento->lexema)) { elemento->token = tokenType[OPERADOR]; } //tiene que ir antes de ispunct

	//CARACTERES DE PUNTUACION
	else if (ispunct(elemento->lexema[0]) && elemento->lexema[0] != '_' && elemento->lexema[1] == '\0') { elemento->token = tokenType[CAR_PUNTUACION]; }

	//CONSTANTES NUMERICAS
	else if (isdigit(elemento->lexema[0]) && elemento->ant && !esTipoDeIdentificador(elemento->ant->lexema)) { elemento->token = tokenType[NUMEROS]; }

	//IDENTIFICADORES
	else if (elemento->ant && ( esTipoDeIdentificador(elemento->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->lexema)==ID_VAR ) && elemento->sig && elemento->sig->lexema[0] == '(') {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR_FUNCION]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_FUNC); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }
	
	//IDENTIFICADORES
	else if (elemento->ant && (esTipoDeIdentificador(elemento->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->lexema) == ID_VAR) && elemento->sig && elemento->sig->lexema[0] == '(' && elemento->ant->ant && elemento->ant->ant->lexema[0] == '*') {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR_FUNCION]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_FUNC); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }

	//IDENTIFICADORES
	else if (elemento->ant && (esTipoDeIdentificador(elemento->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->lexema) == ID_VAR)) {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_VAR); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); } }
	
	//IDENTIFICADORES
	else if (elemento->ant && elemento->ant->ant && (esTipoDeIdentificador(elemento->ant->ant->lexema) || esIdReconocido(*headIdentificadores, elemento->ant->ant->lexema) == ID_VAR) && elemento->ant->lexema[0] == '*') {
		if (isalpha(elemento->lexema[0])) { elemento->token = tokenType[IDENTIFICADOR]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_VAR); }
		else { elemento->token = tokenType[NO_IDENTIFICABLE]; agregarIdentificadorAlFinal(&(*headIdentificadores), elemento->lexema, ID_MAL); }	}
	

	else { elemento->token = tokenType[NO_IDENTIFICABLE]; }
}
