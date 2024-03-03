#include <stdint.h>

class Serializacao {
    private: static int16_t retornaPosicaoCharString(char* texto, char caractere, uint8_t ocorrencia);

    //pega elemento de uma string como se fosse um vetor separado por um caractere especifico. Como:
    //234;234;2232;2;34;556;4;3;5;4;32;
    //posicao começa de 0
    public: static int16_t obtemPosicaoVetorString(char *substring, char* texto, char caractere, uint8_t posicao);

    //tem q ter os caracteres, se nao dá merda
    public: static int16_t obtemSubstringVetorString(char *substring, char* texto, char caractereInicio, char caractereFim, uint8_t posicao);

    public: static uint8_t retornaTamanhoVetorString(char* texto, char caractere);
    
    public: static void removeCaractere(char * texto, char caractere);
};

