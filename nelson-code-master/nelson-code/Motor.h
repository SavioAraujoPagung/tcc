#include <avr/wdt.h>

//******** classe dos motores
class Motor {
  private :
    uint8_t  pino1 ; uint8_t  pino2 ; uint8_t  pwm ;
    
  public :
    Motor (uint8_t  pino1 , uint8_t  pino2 , uint8_t  pwm );
  
    void potencia(int16_t v);
};
