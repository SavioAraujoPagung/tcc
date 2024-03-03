#include "Sensor.h"
    
Sensor::Sensor(){
    debug=0;
}   

uint8_t Sensor::getDebug(){
  return debug;
}

void Sensor::adc_desativa(){
    clt_bit(ADCSRA,ADEN);

}

void Sensor::adc_muda_porta(uint8_t porta){
    //adc_desativa();
    porta &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF0)|porta; // clears the bottom 3 bits before ORing
}

void Sensor::adc_liga_gnd(){
    ADMUX = (ADMUX & 0xF0)|0b00001111;
    _delay_us(2);
}

void Sensor::adc_ativa(){
    set_bit(ADCSRA,ADEN);

}

uint16_t Sensor::adc_read(){
    uint16_t temp;
    set_bit(ADCSRA,ADSC);
    while(tst_bit(ADCSRA,ADSC));
    temp = ADC;
    clt_bit(ADCSRA,ADSC);
    return (temp);
}

//a calibração é primeiro no PRETO, depois no BRANCO
void Sensor::calibraSensor(){
    uint16_t temp;
    //calibra e grava no EEPROM os valores do branco e do preto
    //pisca o led pra indicar que vai começar
    uint32_t tempo = millis();
    uint32_t leitura[8] = {0,0,0,0,0,0,0,0};
    uint32_t quantidade[8] = {0,0,0,0,0,0,0,0};
    bool valorLed = HIGH;

    for(uint8_t i=0; i < 4;i++){
        digitalWrite(PINO_LED,valorLed);
        valorLed = !valorLed;
        tempo = millis();
        while(millis() - tempo < 500){
          for(uint8_t j=INICIO_SENSOR_LINHA; j <= FIM_SENSOR_LINHA; j++){
              adc_liga_gnd();
              adc_muda_porta(j);
              adc_read();
              leitura[j] += adc_read();
              quantidade[j]++;
          }
          delay(1);
        }
    }
    for(uint8_t j=INICIO_SENSOR_LINHA; j <= FIM_SENSOR_LINHA; j++){
      maximo[j] = leitura[j] / quantidade[j];
    }
    for(uint8_t j=0; j < 8;j++){
        leitura[j] = quantidade[j] = 0;
    }
    
    //espera um tempo pra posicionar  o carrinho para o branco
    tempo = millis();
    while(millis() - tempo < 4000);
      //pisca o led pra indicar que terminou
    for(uint8_t i=0; i < 4;i++){
        digitalWrite(PINO_LED,valorLed);
        valorLed = !valorLed;
        tempo = millis();
        while(millis() - tempo < 500){
          for(uint8_t j=INICIO_SENSOR_LINHA; j <= FIM_SENSOR_LINHA; j++){
              adc_liga_gnd();
              adc_muda_porta(j);
              adc_read();
              leitura[j] += adc_read();
              quantidade[j]++;
          }
          delay(1);
        }
    }
    
    
    for(uint8_t j=INICIO_SENSOR_LINHA; j <= FIM_SENSOR_LINHA; j++){
          minimo[j] = leitura[j] / quantidade[j];
    }
    //salva na eeprom
    for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
        EEPROM.write(j*2,minimo[j]>> 8 & 0b11);
        EEPROM.write(j*2+1,(uint8_t)(minimo[j] & 0b11111111));
    }
    for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
        EEPROM.write(16+j*2,maximo[j]>> 8 & 0b11);
        EEPROM.write(16+j*2+1,(uint8_t)(maximo[j] & 0b11111111));
    }
    
    

}

void Sensor::leCalibracaoSensorLinha(){
  uint16_t temp;
  for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
    temp = EEPROM.read(j*2);
    temp = temp << 8;
    temp += EEPROM.read(j*2+1);
    minimo[j] = temp;
    temp = EEPROM.read(16+j*2);
    temp = temp << 8;
    temp += EEPROM.read(16+j*2+1);
    maximo[j] = temp;
    calibracao[j] = abs(maximo[j]-minimo[j]);
  }
}

void Sensor::verificaLinha(){

    int32_t conta;
    uint8_t saida=0;
    uint16_t temp;
    if(debug==1){
    Serial.println(" ");
        Serial.print("Minimo         : ");
        for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
            temp = EEPROM.read(j*2);
            temp = temp << 8;
            temp += EEPROM.read(j*2+1);
    if(temp < 1000) Serial.print(" ");
    if(temp < 100) Serial.print("0");
    if(temp < 10) Serial.print("0");
            Serial.print(temp);
            Serial.print(" ");
        }

        Serial.println(" ");
        Serial.print("Maximo         : ");
        for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
            temp = EEPROM.read(16+j*2);
            temp = temp << 8;
            temp += EEPROM.read(16+j*2+1);
    if(temp < 1000) Serial.print(" ");
    if(temp < 100) Serial.print("0");
    if(temp < 10) Serial.print("0");
            Serial.print(temp);
            Serial.print(" ");
        }
        Serial.println(" ");
        Serial.print("Calibração     : ");
        for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
    if(temp < 1000) Serial.print(" ");
    if(temp < 100) Serial.print("0");
    if(temp < 10) Serial.print("0");
            Serial.print(calibracao[j]);
            Serial.print(" ");
        }

        Serial.println(" ");
        Serial.print("Sensor de linha: ");
    }
    //adc_ativa();
    for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
      adc_liga_gnd();
      adc_read();
      adc_muda_porta(j);
      adc_read();

      valorBruto[j]=adc_read();
      conta =  valorBruto[j] - minimo[j];
      conta =  constrain(conta,0,1023);
      conta *= 100;
      conta = conta/calibracao[j];
      //atenção! deixa penas uma das linhas descomentada
      //para fundo branco e linha preta
      //intensidadeLeds[j] = constrain(conta,0,100);
      //para fundo preto e linha branca
      intensidadeLeds[j] = 100 - constrain(conta,0,100); //sensor com lógica invertida

    }
    adc_liga_gnd();
    //adc_desativa();
    if(debug==1) {
      //Serial.print(" ");
      for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
        if(intensidadeLeds[j] < 1000) Serial.print(" ");
        if(intensidadeLeds[j] < 100) Serial.print(" ");
        if(intensidadeLeds[j] < 10) Serial.print(" ");
        Serial.print(intensidadeLeds[j]);
        Serial.print(" ");
      }
      Serial.println(" ");
      Serial.print("Valor bruto    : ");
      for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
        if(valorBruto[j] < 1000) Serial.print(" ");
        if(valorBruto[j] < 100) Serial.print(" ");
        if(valorBruto[j] < 10) Serial.print(" ");
        Serial.print(valorBruto[j]);
        Serial.print(" ");
      }
      Serial.println(" ");  
      delay(250);
    }

    //return saida;
}

void Sensor::iniciaSensor(){
      
  //*************************
  //jamais mexer nesse código
  //limitar a tensão em 10.5Volts para 3 baterias
      //453 do retorno do analogRead com Referencia interna de 1.1V
      
      //limitar a tensão em 7.5 Volts para 2 baterias
      //323 do retorno do analogRead com Referencia interna de 1.1V
      
      //resistencias de 100k no vcc e 4,87K no GND
      //tensão maxima de 23,7V
      //*
  //este código trava a execução caso a bateria esteja fraca
  analogReference(INTERNAL);
  delay(20);
  int32_t bateria=0;
  analogRead(A7);analogRead(A7);analogRead(A7);
  analogRead(A7);analogRead(A7);analogRead(A7);
  Serial.println(analogRead(A7));
  Serial.print("Carga da bateria em: ");
  for(uint16_t i=0; i < 100; i++)
    bateria+=analogRead(A7);
  bateria/=100;
  //bateria-=323; //valores maximo(372) e minimo da bateria(323) 2S
  bateria-=301; //valores maximo(372) e minimo da bateria(323) 2S
  //bateria -=150;
  bateria = constrain(bateria,0,50);
  bateria*=2; // regra de três pra dar em %
  //if(adc_read() <= 323){
  Serial.print(bateria);
  Serial.println("%");
  if(bateria <= 5){
    Serial.println("Bateria fraca, troque a bateria.");
    while(1){
      digitalWrite(PINO_LED,HIGH);
      delay(300);
      digitalWrite(PINO_LED,LOW);
      delay(300);
    }
  }
  //*************************
  
  
  
      //inicialização do conversor ADC
      // AREF = 1.1V
      //inicia todos os parametros necessários
      //ADMUX = ( 1 << REFS1 ) | ( 1 << REFS0 ) | ( 0 << ADLAR );
      //avcc
      ADMUX = ( 0 << REFS1 ) | ( 1 << REFS0 ) | ( 0 << ADLAR );
      ADCSRA = (0<<ADEN)|(0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
      //desativa free running do adc
      clt_bit(ADCSRA,ADATE);
      adc_liga_gnd();
      adc_ativa();
      for(uint8_t j=0; j < 8; j++){ //assim garanto que vou trocar esses valores
          minimo[j]=2000;
          maximo[j]=0;
      }
      uint16_t temp;
      
  /*
  //configuração do pwm dos motores
  TCCR2A = 0b10100011;
  TCCR2B = 0b00000001;
  */
      pinMode(BOTAO,INPUT_PULLUP);
      pinMode(13,OUTPUT);
      
      saida=0;
      debug=0;

      //começa verificar se o botão está apertado
      if(digitalRead(BOTAO)==PRESSIONADO){
          
          uint32_t tempo=millis();
          uint8_t apertoBotao = 0;
          digitalWrite(PINO_LED,HIGH);
          delay(1000);
          digitalWrite(PINO_LED,LOW);
          while(millis() - tempo < 3000){
              if(digitalRead(BOTAO)==PRESSIONADO){
                  apertoBotao++;
                  digitalWrite(PINO_LED,HIGH);
                  delay(300);
                  digitalWrite(PINO_LED,LOW);
              }
          }
          
          if(apertoBotao == 1){
      Serial.println("Iniciando calibração...");
              calibraSensor();
          }else if(apertoBotao == 2){
      Serial.println("Modo debug ativado");
              debug=1;
          }else if(apertoBotao == 3){
              debug=2;
          }
          
          
      }          
      leCalibracaoSensorLinha();
  Serial.println("Estado da calibração:");
  Serial.print("Minimo    : ");
  for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
    temp = EEPROM.read(j*2);
    temp = temp << 8;
    temp += EEPROM.read(j*2+1);
    if(temp < 100) Serial.print("0");
    if(temp < 10) Serial.print("0");
    Serial.print(temp);
    Serial.print(" ");
  }

  Serial.println(" ");
  Serial.print("Máximo    : ");
  for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
    temp = EEPROM.read(16+j*2);
    temp = temp << 8;
    temp += EEPROM.read(16+j*2+1);
    if(temp < 100) Serial.print("0");
    if(temp < 10) Serial.print("0");
    Serial.print(temp);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Calibração: ");
  for(int8_t j=FIM_SENSOR_LINHA; j >= INICIO_SENSOR_LINHA; j--){
    if(temp < 100) Serial.print("0");
    if(temp < 10) Serial.print("0");
    Serial.print(calibracao[j]);
    Serial.print(" ");
  }
  Serial.print("\n");
      delay(1000);
      
}

bool Sensor::atualizaSensor(){
  verificaLinha();
}

uint8_t Sensor::obtemIntensidade(uint8_t i){
    if( i > (FIM_SENSOR_LINHA - INICIO_SENSOR_LINHA)) return 0; //erro
    return intensidadeLeds[i + INICIO_SENSOR_LINHA];
}

static float Sensor::calculaErro(int16_t a, int16_t b) {
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

static bool Sensor::naLinha(int16_t sensor){
  if(sensor > FORA){
    return true;
  }
  return false;
}

static bool Sensor::naLinha(uint8_t e, uint8_t d){
  if(d > FORA || e > FORA) return true;
  else return false;
}
