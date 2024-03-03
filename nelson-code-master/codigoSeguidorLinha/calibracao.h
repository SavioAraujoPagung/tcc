#include <avr/wdt.h>
#define ENDERECO_BASE 200
#define ENDERECO_SETOR_INICIAL 198
#define ENDERECO_PULSOS_LIMITE 196
#define ENDERECO_PULSOS_MARCACAO 194
//isso é opção de correr sem ser no meio da pista,
//o carrinho de vcs não faz isso, então podem ignorar
#define LADO_DIREITO 2
#define LADO_ESQUERDO 3

#define DISTANCIA_PULSO 5.34
//essa estrutura será diferente para cada um carrinho
//é de escolha do competidor

#include "utils.h"

struct Calibracao{
	float p;
	float d;
	//uint16_t ladoPista;
	int16_t velocidade;
	//uint16_t giroRodaSetor;
	uint16_t idSetor;
	
	//parte do relatorio para envio
	int16_t velocidadeMedia;
	uint16_t tempoSetor;
	uint16_t erroAcumulado;
};



//vetor com o tamanho de setores, isso vai variar de acordo com a pista
#define QUANTIDADE_SETORES 10
Calibracao calibracao[QUANTIDADE_SETORES+1];//evitar overflow
Calibracao calibracaoTemp;
uint32_t tempoSetores[QUANTIDADE_SETORES+1];

/*ATENÇÃO!!!!!!
Por algum motivo usar variáveis locais do tipo char nessas funções está dando zica de travar o arduino
Não consegui descobrir o problema, então coloquei elas como globais.
*/
char pTemp[6];
char dTemp[6];
char iTemp[6];
char textoTemp[100];
uint16_t setorInicial=0;
uint16_t pulsosLimite=0;
uint16_t pulsosMarcacao=0;

void imprimeSetor(uint8_t setor);
void imprimeCalibracao();
void gravaCalibracao();


/*inicio da calibração pra teste.
essa função nao deve existir na verdade, pois os valores devem ficar na memoria
EEPROM do arduino, esses valores devem conseguir ser alterados sem ter q gravar
o código, então essa função aqui foi só pra teste */
void inicioCalibracao(){
    //uint16_t k = 0;
	for(uint8_t k = 0; k < QUANTIDADE_SETORES; k++){
		calibracao[k].p=0.7;
		calibracao[k].d=20;
		//calibracao[k].ladoPista=1;
		calibracao[k].velocidade=50;
		//calibracao[k].tempoIgnoraSetor=100;
		//calibracao[k].tempoMudaSetor=100;
		//calibracao[k].tempoForaLinha=200;
	}
}


//devemos editar aqui para colocar mais atributos
void insereValorCalibracao(char* chave, char* valor, uint8_t indice){
	uint8_t temp8;
	float tempFloat;
	if(strncmp(chave,"setor",5)==0){
		temp8 = atoi(valor);
		calibracao[indice].idSetor = temp8;
		return;
	}
	if(strncmp(chave,"v",1)==0){
		temp8 = atoi(valor);
		calibracao[indice].velocidade = temp8;
		return;
	}
	if(strncmp(chave,"kp",2)==0){
		tempFloat = atof(valor);
		calibracao[indice].p = tempFloat;
		return;
	}
	if(strncmp(chave,"kd",2)==0){
		tempFloat = atof(valor);
		calibracao[indice].d = tempFloat;
		return;
	}
	/*if(strncmp(chave,"ki",2)==0){
		tempFloat = atof(valor);
		calibracao[indice].i = tempFloat;
		return;
	}*/
}


char linhaTemp[70];
char temp[30];
char atributo[15];
char valor[8];
uint8_t quantidadeAtributos;
int inicioColchete, fimColchete;
/*essa função aqui faz leitura dos dados por JSON via string, fiz meu próprio parser*/
bool calibraVeiculo(){		

	//apago coisas desnecessárias
	obtemPosicaoVetorString(leituraSerial,leituraSerial,'[',1);
	obtemPosicaoVetorString(leituraSerial,leituraSerial,']',0);
	int quantidadeIndices = retornaTamanhoVetorString(leituraSerial,'}');
	Serial.println(quantidadeIndices);
	if(quantidadeIndices == 0) return false; //algum erro ocorreu
 
	for(int16_t i=0; i < quantidadeIndices-1; i++){//o ultimo indice é vazio
		obtemSubstringVetorString(linhaTemp,leituraSerial,'{','}',i+1);
		removeCaractere(linhaTemp, '"');
		Serial.println(linhaTemp);
	    quantidadeAtributos = retornaTamanhoVetorString(linhaTemp,',');
		if(quantidadeAtributos == 0) return false; //algum erro ocorreu
		for(uint8_t k=0; k < quantidadeAtributos; k++){
			obtemPosicaoVetorString(temp,linhaTemp,',',k);
			obtemPosicaoVetorString(atributo,temp,':',0);
			obtemPosicaoVetorString(valor,temp,':',1);
			insereValorCalibracao(atributo,valor,i);
		}
		linhaTemp[0]=0;
	}
	return true;
}

void gravaSetor(uint16_t setor){
	
	uint16_t posicaoMemoria = ENDERECO_BASE + setor*sizeof(Calibracao);
	EEPROM.put(posicaoMemoria,calibracao[setor]);
}


void gravaCalibracao(){
	EEPROM.put(ENDERECO_SETOR_INICIAL,setorInicial);
	EEPROM.put(ENDERECO_PULSOS_LIMITE,pulsosLimite);
	EEPROM.put(ENDERECO_PULSOS_MARCACAO,pulsosMarcacao);	
//	uint16_t posicaoMemoria;
	for(uint16_t setor=0; setor < QUANTIDADE_SETORES; setor++){
		gravaSetor(setor);
//		posicaoMemoria = ENDERECO_BASE + setor*sizeof(Calibracao);
//		EEPROM.put(posicaoMemoria,calibracao[setor]);
	}
}

void carregaCalibracao(){
	//carrega também o setor inicial atual, isso é uma variável que utilizo se quiser
	//começar a correr no meio da pista, isso serve pra facilitar o teste
	EEPROM.get(ENDERECO_SETOR_INICIAL,setorInicial);
	EEPROM.get(ENDERECO_PULSOS_LIMITE,pulsosLimite);
	EEPROM.get(ENDERECO_PULSOS_MARCACAO,pulsosMarcacao);		
	uint16_t posicaoMemoria;
	for(uint16_t setor=0; setor < QUANTIDADE_SETORES; setor++){
		posicaoMemoria = ENDERECO_BASE + setor*sizeof(Calibracao);
		EEPROM.get(posicaoMemoria,calibracao[setor]);
	}
}



void imprimeSetor(uint8_t setor){
	String p,d;
	//as funçoes de tratamento de string em arduino nao aceitam float.
	//entao precisamos converter em string antes.
	p = calibracao[setor].p;
	d = calibracao[setor].d;
	//textoTemp[0]=0;
	//i = calibracao[setor].i;
	//sprintf(textoTemp,"Setor: %d, p: %s, d: %s, ladoPista: %d, velocidade: %d distanciaSetor: %d\n",setor,p.c_str(),d.c_str(),calibracao[setor].ladoPista,calibracao[setor].velocidade,calibracao[setor].giroRodaSetor);
	sprintf(textoTemp,"Setor: %d, idSetor: %d, p: %s, d: %s, velocidade: %d\n",setor,calibracao[setor].idSetor,p.c_str(),d.c_str(),calibracao[setor].velocidade);
	Serial.print(textoTemp);
}

void imprimeCalibracao(){
	Serial.println(F("Variaveis globais:"));
	Serial.print(F("SetorInicial: "));
	Serial.println(setorInicial);
	Serial.print(F("PulsosLimite: "));
	Serial.println(pulsosLimite);
	Serial.print(F("PulsosMarcacao: "));
	Serial.println(pulsosMarcacao);
	Serial.println(F("Setores:"));
	for(uint16_t setor=0; setor < QUANTIDADE_SETORES; setor++){
		imprimeSetor(setor);
		delay(20);
		
	}
	Serial.println();
}

void limpaRelatorio(){
	for(uint8_t i=0; i < QUANTIDADE_SETORES; i++){
		calibracao[i].tempoSetor = 0;
		calibracao[i].velocidadeMedia = 0;
		calibracao[i].erroAcumulado = 0;
	}
	gravaCalibracao();
}

/*char linhaTemp[100];
char temp[30];
char atributo[20];
char valor[10];

uint8_t quantidadeAtributos;
int inicioColchete, fimColchete;
*/
void enviaRelatorio(){
	uint8_t primeiraVez=1;
	softwareSerial.print(F("["));
  //Serial.print(F("["));
	for(uint8_t i=0; i < QUANTIDADE_SETORES; i++){
		if(!primeiraVez){
			softwareSerial.print(F(","));
      //Serial.print(F(","));
		}else{
			primeiraVez=0;
		}
    /*Serial.print(F("{\"idSetor\":"));
    Serial.print(calibracao[i].idSetor);
    Serial.print(F(",\"erro\":"));
    Serial.print(calibracao[i].erroAcumulado);
    Serial.print(F(",\"vm\":"));
    Serial.print(calibracao[i].velocidadeMedia);
    Serial.print(F("}"));
   */
		softwareSerial.print(F("{\"idSetor\":"));
		softwareSerial.print(calibracao[i].idSetor);
		softwareSerial.print(F(",\"erro\":"));
		softwareSerial.print(calibracao[i].erroAcumulado);
		softwareSerial.print(F(",\"vm\":"));
		softwareSerial.print(calibracao[i].velocidadeMedia);
		softwareSerial.print(F(",\"ts\":"));
		softwareSerial.print(calibracao[i].tempoSetor);
		softwareSerial.print(F("}"));
	}
	softwareSerial.print(F("]"));
 // Serial.print(F("]\n"));
  softwareSerial.write((uint8_t)0);
}
