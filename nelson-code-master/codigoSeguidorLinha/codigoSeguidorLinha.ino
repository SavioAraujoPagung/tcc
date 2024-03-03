f#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial softwareSerial(9, 8); // RX, TX
#define MAXSIZEBUFFER 400
char leituraSerial[MAXSIZEBUFFER];

#include "codigoAuxiliar.h"
#include "calibracao.h"

//variáveis globais existentes:
//uint8_t sensor0, sensor1, sensor2, sensor3; (leitura, valores entre 0 e 100)
//int16_t potenciaMotorEsquerdo, potenciaMotorDireito; (gravação, valores entre -255 a 255);
#define FORA 10
#define LINHA 95
#define CONSIDERADO_LINHA 30

#define ENDERECOTENTATIVA 100
int16_t tentativa=0;


float erro;
float erroAnterior;
float Kp = 3;
float Kd = 50;
int16_t velocidade = 100;
uint16_t contadorFora = 0;
#define TEMPOMAXIMOFORA 200

#define TEMPOLEDACESSO_IGNORARNOVOSETOR 50

#define QUANTIDADELEITURAMARCADOR 4

#define APERTADO LOW

int tempoLed=0;

Sensor sensorLinha;
Motor motorDireito(PINO4 ,PINO3 ,pwm2 );
Motor motorEsquerdo(PINO2 ,PINO1 ,pwm1 );
uint32_t tt;

#define BOTAO2 12
#define PINOMARCADORSETOR 18
#define PINOMARCADORSETOR2 17
#define PINOMARCADORFIMPISTA 16

#define PINOENCODERESQUERDO 19

uint8_t sensorEsquerdo, sensorDireito;
int16_t potenciaMotorEsquerdo, potenciaMotorDireito;
uint16_t setorAtual=0;
//uint16_t ladoPista=0;
uint16_t contadorMarcadorSetor=0;
uint16_t contadorMarcadorFimPista=0;
uint8_t marcadorDireito=0;
uint8_t marcadorEsquerdo=0;

#define QUANTIDADETOTALMARCADORFIMPISTA 2
uint16_t quantidadeMarcadorFimPista=0;

bool naLinha(){
  if(sensorDireito > FORA || sensorEsquerdo > FORA) return true;
  else return false;
  
}
bool naLinha(int16_t sensor){
  if(sensor > FORA){
    return true;
  }
  return false;
  
}
float calculaErro(int16_t a, int16_t b) {
  if (a >= FORA && b >= FORA) { //os dois estão bastante na linha branca
    return a-b;
  }
  else if (a < FORA && b >= FORA) { //o A saiu da linha
    return (200 - b )* -1;
  }
  else if (a >= FORA && b < FORA) { // o B saiu da linha
    return (200 - a);
  }
  return 0;
}
uint32_t tempoMilis;
//coloque o código nessa função
//atenção, não pode ter um while travando a saida da função. Ela tem que ficar em loop
//ou seja, essa função é sempre chamada
//automaticamente essas variáveis são atualizadas por outros códigos
void setup() {
	Serial.begin(38400);
	softwareSerial.begin(38400);
	motorDireito.potencia(0);
	motorEsquerdo.potencia(0);
	
	pinMode(BOTAO2, INPUT_PULLUP);
	pinMode(BOTAO, INPUT_PULLUP);
	pinMode(PINOENCODERESQUERDO, INPUT_PULLUP);

	sensorLinha.iniciaSensor();
	inicioCalibracao();
	carregaCalibracao();
	imprimeCalibracao();
	//gravaCalibracao();
	//pinMode(8,INPUT_PULLUP);
	//pinMode(9,INPUT_PULLUP);
	aguardaInicio();	
	//setorAtual=setorInicial;
	//ladoPista = calibracao[setorAtual].ladoPista;
	//vai começar, adiciona um na tentativa
	EEPROM.get(ENDERECOTENTATIVA,tentativa);
	tentativa++;
	EEPROM.put(ENDERECOTENTATIVA,tentativa);
	tempoMilis=millis();
	setorAtual = 0;
}

void aguardaInicio(){
	uint8_t sensor0=0, sensor1=0;
	uint8_t marcador_curva1=0, marcador_curva2=0, marcador_final=0;
	int i=0,x=0;
	while(digitalRead(BOTAO) != APERTADO && digitalRead(BOTAO2) != APERTADO ){
		sensorLinha.atualizaSensor();
		sensor0 = sensorLinha.obtemIntensidade(0);
		sensor1 = sensorLinha.obtemIntensidade(1);
		marcador_curva1 = (digitalRead(PINOMARCADORSETOR)==APERTADO);
		marcador_curva2 = (digitalRead(PINOMARCADORSETOR2)==APERTADO);
		marcador_final = (digitalRead(PINOMARCADORFIMPISTA)==APERTADO);
		//sensor2 = sensorLinha.obtemIntensidade(2);
		//sensor3 = sensorLinha.obtemIntensidade(3);
        //sensor4 = sensorLinha.obtemIntensidade(4);
        //sensor5 = sensorLinha.obtemIntensidade(5);
		if(sensor0 > CONSIDERADO_LINHA || sensor1 > CONSIDERADO_LINHA
		|| marcador_curva1 == true || marcador_curva2 == true || marcador_final == true){
			digitalWrite(PINO_LED,HIGH);
		//}else if(digitalRead(8) == APERTADO || digitalRead(9) == APERTADO/*|| digitalRead(5)== APERTADO*/){
		//	digitalWrite(PINO_LED,HIGH);
		}
		else{
			digitalWrite(PINO_LED,LOW);
		}
		//verifica se tem algo para calibrar
		if(softwareSerial.available()>0){
			uint32_t tempo = millis();
			i=0;
			while(i < MAXSIZEBUFFER){
				x = softwareSerial.read();
				if(x!=-1){
					leituraSerial[i] = (char)x;
					i++;
				}
				if((millis() - tempo) > 2000) {
					leituraSerial[i]=0;
					break;
				}

			}
		}else if(Serial.available()>0){
			uint32_t tempo = millis();
			i=0;
			while(i < MAXSIZEBUFFER){
				x = Serial.read();
				if(x!=-1){
					leituraSerial[i] = (char)x;
					i++;
				}
				if((millis() - tempo) > 2000) {
					leituraSerial[i]=0;
					break;
				}
			}
		} 
		if(i!=0){
			if(i == MAXSIZEBUFFER){
				leituraSerial[MAXSIZEBUFFER-1]=0;
			}
			if(leituraSerial[0] == 'r'){//solicita relatorio
        		Serial.println(F("Enviando relatório..."));
				enviaRelatorio();
			}else if(leituraSerial[0] == 'c'){//limpa relatorio
				limpaRelatorio();
			}else{
				Serial.println(F("entrei aqui pra calibrar"));
				Serial.println(leituraSerial);
				calibraVeiculo();
				gravaCalibracao();
				imprimeCalibracao();
			}
			i=0;
		}
		/*
		Serial.print("\nerro: ");
		erro = calculaErro(sensor0,sensor1);
		Serial.println(erro);
		Serial.print("MD: ");
		Serial.print(velocidade - ((int)((erro*sqrt(abs(erro))*Kp))));
		Serial.print(" ME: ");
		Serial.print(velocidade + ((int)((erro*sqrt(abs(erro))*Kp))));
		delay(100);
		*/
	}
	Serial.println(F("saiu do aguardaInicio"));
	limpaRelatorio();
	delay(1000);
	/*
	if(digitalRead(BOTAO2) == APERTADO){
		calibracao[0].velocidade = calibracao[1].velocidade;
		calibracao[0].p = calibracao[1].p;
		calibracao[0].d = calibracao[1].d;
	}
	*/
	
}

uint32_t tempoInicial, tempoFinal;
uint16_t contadorLoop=0;
char leituraVelocidade=0;
uint32_t inicioParcial, fimParcial;
uint32_t tempoParcial;
void loop(){
	tt = micros();
	//verifica se tem algo na serial para calibrar
		/*if(Serial.available()>0){
			leituraSerial = softwareSerial.readStringUntil(']');
			calibraVeiculo();
		}*/
		
	//leitura dos sensores analógicos e marcadores analogicos
	sensorLinha.atualizaSensor();
	sensorDireito = sensorLinha.obtemIntensidade(1);
	sensorEsquerdo = sensorLinha.obtemIntensidade(0);
	
	//já digo que os marcadores nao estão na linha, assim a lógica abaixo simplifica
	marcadorEsquerdo = marcadorDireito = 0;
	

	////////////////////////////////////
	//só entra em leitura de setor quando o led apaga, isso significa que ele ignora qualquer leitura muito proxima uma da outra
	if(tempoLed==0 || contadorMarcadorSetor!=0){
		if(digitalRead(PINOMARCADORSETOR)==APERTADO || digitalRead(PINOMARCADORSETOR2)==APERTADO){
				contadorMarcadorSetor++;
				
		}else{
			contadorMarcadorSetor=0;
		}
	}
	
	//encoder
	/*if(encoderValorNovo!=encoderValorAntigo && encoderValorNovo == 0){
		giroRodaTotal++;
		giroRodaSetor[setorAtual]++;
	}
	encoderValorAntigo = encoderValorNovo;
	encoderValorNovo = digitalRead(PINOENCODERESQUERDO);
	*/


	marcadorDireito = (digitalRead(PINOMARCADORFIMPISTA) == APERTADO);
	//marcador de fim de pista
	if(marcadorDireito){
		contadorMarcadorFimPista++;
	}else{
		contadorMarcadorFimPista=0;
	}


	//Muda de setor
	if(contadorMarcadorSetor == QUANTIDADELEITURAMARCADOR){
		tempoLed=TEMPOLEDACESSO_IGNORARNOVOSETOR;
		//calibracao[setorAtual].tempoSetor = (millis() - tempoMilis);
		
		
		//esse código também tem no final da pista( marcaçaõ de fim de pista)
		tempoParcial = millis() - tempoMilis;
		if(tempoParcial > 64000) { //pra evitar overflow (o tempo aqui já é gigante.. entao pode ser ignorado
			tempoParcial = 64000;
		}
		calibracao[setorAtual].tempoSetor = tempoParcial;
		tempoMilis = millis();
		setorAtual++;
		
		softwareSerial.print(F("Setor: "));
		softwareSerial.println(setorAtual);
	}

	if(contadorMarcadorFimPista == QUANTIDADELEITURAMARCADOR){
		//guardo o tempo da volta
		if(quantidadeMarcadorFimPista == 0){
			tempoMilis = millis(); //começa o primeiro setor aqui
			tempoInicial = millis();
			Serial.print("Comecou: ");
			Serial.print(tempoInicial);			
		}
		quantidadeMarcadorFimPista++;
		
		if(quantidadeMarcadorFimPista == QUANTIDADETOTALMARCADORFIMPISTA){
			tempoFinal = millis();
			quantidadeMarcadorFimPista++; //para entrar só uma vez nesse if
		}
	}
	//acabou a corrida trava o carrinho
	if(quantidadeMarcadorFimPista >= QUANTIDADETOTALMARCADORFIMPISTA){
	
		//nao teve mudança de setor mas teve fim da corrida, então preciso salvar o ultimo setor por aqui
		tempoParcial = millis() - tempoMilis;
		if(tempoParcial > 64000) { //pra evitar overflow (o tempo aqui já é gigante.. entao pode ser ignorado
			tempoParcial = 64000;
		}
		calibracao[setorAtual].tempoSetor = tempoParcial;
		tempoMilis = millis();
		setorAtual++;
	
		potenciaMotorDireito = potenciaMotorEsquerdo = 0;
		//delay pra garantir que o carrinho vai passar do fim
		motorDireito.potencia(50);
		motorEsquerdo.potencia(50);
		delay(100);
		motorDireito.potencia(potenciaMotorDireito);
		motorEsquerdo.potencia(potenciaMotorEsquerdo);
		/*Serial.print(F("Total de pulsos: "));
		Serial.println(giroRodaTotal);*/
		delay(50);
		Serial.print(F("Tempo da volta: "));
		Serial.println(tempoFinal - tempoInicial);
		Serial.print(F("Parcial por setor: "));
		for(int16_t i; i < QUANTIDADE_SETORES; i++){
			Serial.print(F("setor "));
			Serial.print(i);
			Serial.print(F(": "));
			//Serial.println(giroRodaSetor[i]);
			delay(50);
		}
		
		
		
		setorAtual=QUANTIDADE_SETORES-1;
		gravaCalibracao();
		//vou enviar só quando reiniciar e eu pedir
		//enviaRelatorio();
		while(1){
			digitalWrite(PINO_LED,HIGH);
			delay(200);
			digitalWrite(PINO_LED,LOW);
			delay(200);
		}
	}
	
	//acabou SETOR trava o carrinho
	if(setorAtual >= QUANTIDADE_SETORES){
		potenciaMotorDireito = potenciaMotorEsquerdo = 0;
		motorDireito.potencia(potenciaMotorDireito);
		motorEsquerdo.potencia(potenciaMotorEsquerdo);
		setorAtual=QUANTIDADE_SETORES-1;
		gravaCalibracao();
		while(1){
			digitalWrite(PINO_LED,HIGH);
			delay(1000);
			digitalWrite(PINO_LED,LOW);
			delay(1000);
		}
	}
	
	
	//fim da logida de mudar o setor ou fim de pista
	//mantem o led aceso enquanto o tempo for > 0
	if(tempoLed > 0){
		digitalWrite(13,1);
		tempoLed--;
	
	}else{
		digitalWrite(13,0);
	}
	
	//////////////calculo do PID
	//carrega valores da calibração
	Kp = calibracao[setorAtual].p;
	Kd = calibracao[setorAtual].d;
	velocidade = calibracao[setorAtual].velocidade;
	
	//analiso se preciso usar o freio
	//uso se tiver algum valor diferente de zero na giroRodaSetor e eu tiver atingido esse valor
	/*if(calibracao[setorAtual].giroRodaSetor != 0 && giroRodaSetor[setorAtual] >= calibracao[setorAtual].giroRodaSetor){
		velocidade = -255;
	}
	*/
	erroAnterior = erro;

	erro = calculaErro(sensorDireito,sensorEsquerdo);
	if(calibracao[setorAtual].erroAcumulado < 64000){ //limite pra evitar overflow. Valor já é gigante, ta muito ruim se chegar nisso
		calibracao[setorAtual].erroAcumulado += abs(erro);
	}
  	if(!naLinha()){
		erro = erroAnterior;
	}
    if(!naLinha()){
		if(contadorFora > TEMPOMAXIMOFORA){
			potenciaMotorDireito = potenciaMotorEsquerdo = 0;
			motorDireito.potencia(potenciaMotorDireito);
			motorEsquerdo.potencia(potenciaMotorEsquerdo);
			return;
		}
		contadorFora++;
    }else{
		contadorFora=0;
	}
	if(setorAtual==QUANTIDADE_SETORES){
		velocidade = 0;
	}
	
	//potenciaMotorDireito = velocidade - ((int)((erro*sqrt(abs(erro))*Kp) + Kd*(erro - erroAnterior)));
	//potenciaMotorEsquerdo = velocidade + ((int)((erro*sqrt(abs(erro))*Kp) + Kd*(erro - erroAnterior)));
	potenciaMotorDireito = velocidade - ((int)((erro*Kp) + Kd*(erro - erroAnterior)));
	potenciaMotorEsquerdo = velocidade + ((int)((erro*Kp) + Kd*(erro - erroAnterior)));
	
	
	motorDireito.potencia(potenciaMotorDireito);
	motorEsquerdo.potencia(potenciaMotorEsquerdo);
	//Fim do PID
	
	/*//////parada por limite de pulsos:
	if(giroRodaTotal > pulsosLimite){
		potenciaMotorDireito = potenciaMotorEsquerdo = 0;
		motorDireito.potencia(potenciaMotorDireito);
		motorEsquerdo.potencia(potenciaMotorEsquerdo);
		while(1);
	}*/
	
	
	
	//////////////envio da telemetria
	/*Serial.print(tentativa);
	Serial.print(",");
	Serial.print(setorAtual);
	Serial.print(",");
	Serial.print(Kp);
	Serial.print(",");
	Serial.print(Kd);
	Serial.print(",");
	Serial.print(velocidade);
	Serial.print(",");
	Serial.print(potenciaMotorDireito);
	Serial.print(",");
	Serial.print(potenciaMotorEsquerdo);
	Serial.print(",");
	Serial.print(erro);
	Serial.print(",");
	Serial.print(erroAcumulado);
	Serial.println();
	*/
	/*
	Serial.write(tentativa);
	Serial.write(setorAtual);
	Serial.write((uint32_t)Kp);
	Serial.write((uint32_t)Kd);
	Serial.write(velocidade);
	Serial.write(potenciaMotorDireito);
	Serial.write(potenciaMotorEsquerdo);
	Serial.write((uint32_t)erro);
	Serial.write((uint32_t)erroAcumulado);
	Serial.println();
	*/
	/*
	if(repeticaoTelemetria == 20){
		repeticaoTelemetria = 0;
		Serial.print(millis());
		Serial.print(" ");
		Serial.print(setorAtual);
		Serial.print(" ");
		Serial.print(potenciaMotorDireito);
		Serial.print(" ");
		Serial.print(potenciaMotorEsquerdo);
		Serial.print(" ");
		Serial.print(erro);
		Serial.print(" ");
		Serial.print(erroAcumulado);
		Serial.println();
	}else{
		repeticaoTelemetria++;
	}
	*/
	
	
	///////////// fim da telemetria
	/*if(micros()-tt > 1000){
		Serial.println("tururu...");
	}*/
    while(micros()-tt < 1000 ); //500hz//1khz//2Khz
	/*contadorLoop++;
	if(millis()-tempoMilis >= 1000){
		Serial.println(contadorLoop);
		contadorLoop=0;
		tempoMilis=millis();
	}
	*/
}
