#include <SoftwareSerial.h>

//macro de funcoes
#define set_bit(y,bit) (y|=(1<<bit))//coloca em 1 o bit x da variável Y
#define clt_bit(y,bit) (y&=~(1<<bit))//coloca em 0 o bit x da variável Y
#define cpl_bit(y,bit) (y^=(1<<bit))// troca o estado logico do bit x da variável Y
#define tst_bit(y,bit) (y&(1<<bit))// retorna 0 ou 1 conforme leitura do bit
#define PINO_LED 13

uint16_t setorInicial=0;
uint16_t pulsosLimite=0;
uint16_t pulsosMarcacao=0;

SoftwareSerial softwareSerial(9, 8); // RX, TX

class Utils{

  /*
    todo:
    Envia os dados para o relatório
    esses valores podem mudar dependendo 
    dos atributos da "struct Calibração"  
  */
  public: static void enviaRelatorio(Configuracao setores[]){
    uint8_t primeiraVez=1;
    softwareSerial.print(F("["));
    for(uint8_t i=0; i < QUANTIDADE_SETORES; i++){
      if(!primeiraVez){
        softwareSerial.print(F(","));
      }else{
        primeiraVez=0;
      }
      softwareSerial.print(F("{\"idSetor\":"));
      softwareSerial.print(setores[i].id);
      softwareSerial.print(F(",\"erroAcumulado\":"));
      softwareSerial.print(setores[i].erroAcumulado);
      softwareSerial.print(F(",\"tempoSetor\":"));
      softwareSerial.print(setores[i].tempo);
      softwareSerial.print(F("}"));
    }
    softwareSerial.print(F("]"));
  }

  public: static void imprimeCalibracao(Configuracao setores[]){
    Serial.println(F("Variaveis globais:"));
    Serial.print(F("SetorInicial: "));
    Serial.println(setorInicial);
    Serial.print(F("PulsosLimite: "));
    Serial.println(pulsosLimite);
    Serial.print(F("PulsosMarcacao: "));
    Serial.println(pulsosMarcacao);
    Serial.println(F("Setores:"));
    for(uint16_t setor=0; setor < QUANTIDADE_SETORES; setor++){
      imprimeSetor(setores[setor]);
      delay(20);
      
    }
    Serial.println();
  }

  public: static void limpaRelatorio(Configuracao setores[]){
    for(uint8_t i=0; i < QUANTIDADE_SETORES; i++){
      setores[i].tempo = 0;
      setores[i].erroAcumulado = 0;
    }
    //gravaCalibracao();
  }

  public: static void imprimeSetor(Configuracao setor){
    String p,d,i;
    //as funçoes de tratamento de string em arduino nao aceitam float.
    //entao precisamos converter em string antes.
    p = setor.p;
    d = setor.d;
    //textoTemp[0]=0;
    //i = calibracao[setor].i;
    //sprintf(textoTemp,"Setor: %d, p: %s, d: %s, ladoPista: %d, velocidade: %d distanciaSetor: %d\n",setor,p.c_str(),d.c_str(),calibracao[setor].ladoPista,calibracao[setor].velocidade,calibracao[setor].giroRodaSetor);
    // sprintf(textoTemp,"Setor: %d, idSetor: %d, p: %s, d: %s, velocidade: %d\n",setor,setor.id,p.c_str(),d.c_str(),setor.velocidade);
    sprintf("Setor: %d, idSetor: %d, p: %s, d: %s, velocidade: %d\n",setor.id,p.c_str(),d.c_str(),setor.velocidade);
    //Serial.print(textoTemp);
  }
};



