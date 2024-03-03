#ifndef UTILS_H
#define UTILS_H


int16_t retornaPosicaoCharString(char* texto, char caractere, uint8_t ocorrencia){
	int16_t posicao=0;
	int16_t temp=0;
	if(ocorrencia == 0) return -1; //-1 não achou
	while(texto[posicao]!= 0){
		if(texto[posicao] == caractere){
			temp++;
		}
		if(temp == ocorrencia) return posicao;
		posicao++;
	}
	if(texto[posicao] == caractere){
		return posicao;
	}
	return -1;
}


//pega elemento de uma string como se fosse um vetor separado por um caractere especifico. Como:
//234;234;2232;2;34;556;4;3;5;4;32;
//posicao começa de 0
int16_t obtemPosicaoVetorString(char *substring, char* texto, char caractere, uint8_t posicao){
	int16_t inicio, fim;
	int16_t i=0;
	int16_t k=0;
	if(posicao == 0){
		inicio = 0;
	}else{
		inicio = retornaPosicaoCharString(texto,caractere,posicao)+1;
	}
	fim = retornaPosicaoCharString(texto,caractere,posicao+1);
	/*Serial.print("Inicio: ");
	Serial.print(inicio);
	Serial.print(" - Fim: ");
	Serial.println(fim);
	*/
	if(fim == -1 && inicio != -1){ //encontrou 1 apenas, então considero o fim como final da string
		fim = retornaPosicaoCharString(texto,0,posicao+1);
	}
	if(inicio < 0 || fim <= 0) return -1; //erro
	k=inicio;
	while(k<fim){
		substring[i] = texto[k];
		i++;
		k++;
	}
	
	substring[i]=0;//finaliza a string
	//Serial.print(F("Substring lida: "));
	//Serial.println(substring);
}

//tem q ter os caracteres, se nao dá merda
int16_t obtemSubstringVetorString(char *substring, char* texto, char caractereInicio, char caractereFim, int16_t posicao){
	int16_t inicio, fim;
	int16_t i=0;
	int16_t k=0;
	if(posicao == 0){
		inicio = 0;
		if(texto[inicio] == caractereInicio){
			inicio++;
		}
	}else{
		inicio = retornaPosicaoCharString(texto,caractereInicio,posicao)+1;
	}
	fim = retornaPosicaoCharString(texto,caractereFim,posicao);
	if(fim == -1 && inicio != -1){ //encontrou 1 apenas, então considero o fim como final da string
		fim = retornaPosicaoCharString(texto,0,posicao);
	}
	if(inicio < 0 || fim <= 0) return -1; //erro
	k=inicio;
	while(k<fim){
		substring[i] = texto[k];
		i++;
		k++;
	}
	substring[i]=0;//finaliza a string
	//Serial.print(F("Substring lida: "));
	//Serial.println(substring);
}

int16_t retornaTamanhoVetorString(char* texto, char caractere){
	int16_t i=0;
	int16_t tamanho=0;
	while(texto[i]!=0){
		if(texto[i]==caractere){
			tamanho++;
		}
		i++;
	}
	
	return tamanho+1;
}

void removeCaractere(char * texto, char caractere){
	int i=0;
	int j=0;
	if(caractere == 0){ //nao permito remover fim de string
		return;
	}
	while(texto[i] != 0){
		if(texto[i] == caractere){
			j=i;
			while(texto[j] != 0){
				texto[j] = texto[j+1];
				j++;
			}
		}
		i++;
	}
}

#endif UTILS_H