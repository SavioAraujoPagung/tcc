#include "Configuracao.h"

void Configuracao::insereValorCalibracao (char* chave, char* valor){
    uint8_t temp8;
    float tempFloat;
    if(strncmp(chave,"setor",5)==0){
      temp8 = atoi(valor);
      this->id = temp8;
      return;
    }
    if(strncmp(chave,"velocide",8)==0){
      temp8 = atoi(valor);
      this->velocidade = temp8;
      return;
    }
    if(strncmp(chave,"kp",2)==0){
      tempFloat = atof(valor);
      this->p = tempFloat;
      return;
    }
    if(strncmp(chave,"kd",2)==0){
      tempFloat = atof(valor);
      this->d = tempFloat;
      return;
    }
    if(strncmp(chave,"ki",2)==0){
      tempFloat = atof(valor);
      this->i = tempFloat;
      return;
    }
}

/*essa função aqui faz leitura dos dados por JSON via string, fiz meu próprio parser*/
static bool Configuracao::calibraVeiculo(Configuracao setores[]){
  char linhaTemp[100];
  char temp[30];
  char atributo[20];
  char valor[10];
  uint8_t quantidadeAtributos;
  //apago coisas desnecessárias
  Serializacao::obtemPosicaoVetorString("leituraSerial","leituraSerial",'[',1);
  Serializacao::obtemPosicaoVetorString("leituraSerial","leituraSerial",']',0);
  int quantidadeIndices = Serializacao::retornaTamanhoVetorString("leituraSerial",'}');

  if(quantidadeIndices == 0) return false; //algum erro ocorreu
  
  for(uint8_t i=0; i < quantidadeIndices-1; i++){//o ultimo indice é vazio
    Serializacao::obtemSubstringVetorString(linhaTemp,"leituraSerial",'{','}',i+1);
    Serializacao::removeCaractere(linhaTemp, '"');

    quantidadeAtributos = Serializacao::retornaTamanhoVetorString(linhaTemp,',');
    if(quantidadeAtributos == 0) return false; //algum erro ocorreu

    for(uint8_t k=0; k < quantidadeAtributos; k++){
      Serializacao::obtemPosicaoVetorString(temp,linhaTemp,',',k);
      Serializacao::obtemPosicaoVetorString(atributo,temp,':',0);
      Serializacao::obtemPosicaoVetorString(valor,temp,':',1);
      setores[i].insereValorCalibracao(atributo, valor);
    }

    linhaTemp[0]=0;
  }

  return true;
}

static void Configuracao::inicioCalibracao(Configuracao setores[]) {
  for(uint8_t setor = 0; setor < QUANTIDADE_SETORES; setor++){
    setores[setor].p=0.7;
    setores[setor].d=20;
    //calibracao[k].ladoPista=1;
    setores[setor].velocidade=50;
    //calibracao[k].tempoIgnoraSetor=100;
    //calibracao[k].tempoMudaSetor=100;
    //calibracao[k].tempoForaLinha=200;
  }
}