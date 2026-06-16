
#include <stdio.h>
#include "pico/stdlib.h"
/**
 * @brief Programa principal
*/
int main(void) {
  int boton1;
  int boton2;
  int boton3;
  // Inicializo el USB
  // Demora para esperar la conexion
  sleep_ms(1000); 
  gpio_init(6);
  gpio_init(7); 
  gpio_init(8);
  gpio_init(20);
  gpio_init(21);
  gpio_init(22);
  gpio_set_dir(6,GPIO_OUT);
  gpio_set_dir(7,GPIO_OUT);
  gpio_set_dir(8,GPIO_OUT);
  gpio_set_dir(20,GPIO_IN);
  gpio_set_dir(21,GPIO_IN);
  gpio_set_dir(22,GPIO_IN);

  // Inicializacion de GPIO con gpio_init()
  /* Habilito el GPIO25 (LED)
  
  gpio_init(PICO_DEFAULT_LED_PIN);
  GPIO25 como salida
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT)
  Configuracion de entrada/salida con gpio_set_dir()
  */
  while (true) {
    gpio_put(20,1);
    sleep_ms(500);
    gpio_put(20,0);
    sleep_ms(500);
    boton1=gpio_get(10);
    if(boton1==1){
        gpio_put(20,1);
        }
    if(boton1==0){
        gpio_put(20,0);

    gpio_put(21,1);
    sleep_ms(500);
    gpio_put(21,0);
    sleep_ms(500);
    boton2=gpio_get(10);
    if(boton2==1){
        gpio_put(21,1);
        }
    if(boton2==0){
        gpio_put(21,0);

    gpio_put(22,1);
    sleep_ms(500);
    gpio_put(22,0);
    sleep_ms(500);
    boton3=gpio_get(10);
    if(boton3==1){
        gpio_put(22,1);
        }
    if(boton3==0){
        gpio_put(22,0);
    }
    


    /* Prendo LED
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    */
    // Resolver logica para GPIO20 -> GPIO6
    // Resolver logica para GPIO21 -> GPIO7
   
 // Resolver logica para GPIO22 -> GPIO8
  }
  return 0;   
           }
          }
        }