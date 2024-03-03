#include "Configuracao.h"
#include <EEPROM.h>
#include <stdio.h>

#define ENDERECO_BASE 200
#define ENDERECO_SETOR_INICIAL 198
#define ENDERECO_PULSOS_LIMITE 196
#define ENDERECO_PULSOS_MARCACAO 194

class EEPROMDatabase
{
  private: static void gravarConfiguracao(Configuracao setor);

  public: static void gravaConfiguracaoSetores(Configuracao setores[]);
  
  public: static void recuperaConfiguracaoSetores(Configuracao setores[]);
};
