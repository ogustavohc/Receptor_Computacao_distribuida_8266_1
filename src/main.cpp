#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "espnow.h"

#define TAG "Sender"
#define LED_BRANCO 14
#define LED_VERMELHO 4
#define LED_VERDE 5

typedef struct __attribute__((packed)) {
    uint8_t n_piscadas;
    uint8_t cor_led;
    uint8_t velocidade_piscadas;
} dados_tarefa_t;

typedef struct __attribute__((packed)){
    uint8_t esp;
    bool concluida;
} dados_resposta_t;

typedef enum {
    TAREFA_RECEBIDA,
    TAREFA_CONCLUIDA,
} state_machine_tarefas;

state_machine_tarefas st_tarefas;
dados_tarefa_t resposta;

uint8_t transmissor_mac[] = {0xB8, 0x1F, 0x3F, 0xDA, 0x2A, 0xB8};

int contador = 0;
int led_piscante = 14;

void OnDataSent(uint8_t *mac_addr , uint8_t sendStatus) {
    if (sendStatus == 0) {
        Serial.println("Resposta enviada com sucesso");
    } else {
        Serial.println("Falha no envio");
    }
}

void OnDataRecv(unsigned char *mac_addr, unsigned char *incomingData, uint8_t len){

  
    if (len != sizeof(dados_tarefa_t)){
      Serial.println("Tamanho incorreto de mensagem");
      return;
    } 
    Serial.println("Mensagem chegou!");
    memcpy(&resposta, incomingData, sizeof(dados_tarefa_t));
    st_tarefas = TAREFA_RECEBIDA;
  
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0){
    Serial.println("Erro ao iniciar esp now");
    return;
  }
  Serial.println("Esp now ligado");

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_add_peer(transmissor_mac, ESP_NOW_ROLE_COMBO, 0, NULL, 0);

  pinMode(LED_BRANCO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
}

void loop() {
  switch(st_tarefas){
            case TAREFA_RECEBIDA:
                if (contador > resposta.n_piscadas){
                    st_tarefas = TAREFA_CONCLUIDA;
                    dados_resposta_t resposta = {.esp = 3, .concluida = true};
                    esp_now_send(transmissor_mac, (uint8_t*)&resposta, sizeof(resposta));
                    contador = 0;
                    break;
                } else {
                    if (resposta.cor_led == 1){
                        led_piscante = LED_VERMELHO;
                    }
                    else if (resposta.cor_led == 2){
                        led_piscante = LED_VERDE;
                    }
                    else {
                        led_piscante = LED_BRANCO;
                    }
                    digitalWrite(led_piscante, 1);
                    delay(resposta.velocidade_piscadas);
                    digitalWrite(led_piscante, 0);
                    delay(resposta.velocidade_piscadas);
                    contador++;
                    break;
                }
            case TAREFA_CONCLUIDA:
              //Serial.println("CONCLUIDO");
              break;
        }
  yield();
  //Serial.println("Rotina...");
}

