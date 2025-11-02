// Inclusão de bibliotecas:
#include "esp_camera.h"  // Importa os recursos disponíveis na biblioteca esp_camera.h, permitindo a utilização de 
// funções já prontas para configuração e utilização do ESP32 Cam.
#include <WiFi.h> // Importa as funções da biblioteca WiFi.h, permitindo a utilização de recursos para comunicação Wifi com o ESP32 Cam.
#include <HTTPClient.h> // Importa as funções da biblioteca HTTPClient.h, permitindo a utilização de recursos para estabelecer requisições HTTP com o ESP32 Cam.
#include <WiFiClientSecure.h> // Importa as funções da biblioteca WiFiClientSecure.h, permitindo a utilização de recursos para estabelecer requisições HTTPS com o servidor flask.
#include "driver/rtc_io.h" // Importa funções de entrada e saída que permitem gerenciar recursos, como para realização do modo deep sleep.
#define rede_wifi "Nome_da_rede" // Define o nome da rede Wifi com a qual o ESP32 Cam irá se conectar.
#define senha_da_rede_wifi "Senha_da_rede" // Define a senha da rede Wifi com a qual o ESP32 Cam irá se conectar.

// Abaixo se realiza configurações para comunicação com o servidor flask e a plataforma ThingSpeak:

// Define o caminho URL do servidor flask para armazenamento das imagens de cada planta:
const char* flask_url = "https://Nome_do_servidor_render/upload/rosa_deserto"; // Ou "https://Nome_do_servidor_render/upload/cacto_estrela".

// Define as configurações do ThingSpeak:
const char* chave_api_de_escrita = "Chave_de_escrita"; // Define a chave para escrita no canal ou chave.
const char* servidor_ThingSpeak = "https://api.thingspeak.com/update"; // Define o servidor da plataforma.

String imageUrl = ""; // Define uma variável global, correspondente ao caminho URL da imagem atual no servidor flask.

int tentativas = 0; // Define o número de tentativas de requisições HTTP.

// Define o tempo de espera para a utilização do modo de deep sleep, capaz de hibernar a placa durante o tempo informado,
// desabilitando a câmera, recursos de Wifi, entre outros:
#define tempo_de_espera  3600000000 // Define um tempo de espera de 60 minutos = 3600000000 us entre as capturas, para a duração de cada hibernação.
// #define tempo_de_espera 60000000 // Define um tempo de espera de 1 minuto entre as capturas, para a duração de cada hibernação.

// Definição dos pinos do ESP32 Cam:
#define PWDN_GPIO_NUM     32
#define FLASH_LED_PIN      4
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Função que realiza as configurações para utilização do ESP32 Cam:
void configura_ESP32Cam() {
  camera_config_t config; // Define o objeto config como sendo de uma classe de configurações do ESP32 Cam.
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG; // Define que a imagem capturada possuirá o formato JPEG.
  config.frame_size = FRAMESIZE_SVGA; // Define a resolução da imagem a ser exibida no ThingSpeak em 800 x 600 pixels.
  config.jpeg_quality = 20; // Define um nível de compressão relativamente baixo, permitindo uma melhor resolução da foto,
  // às custas de um maior tamanho de arquivo.
  config.fb_count = 1; // Define que apenas um buffer será utilzado para armazenar a imagem.

  // Abaixo se inicia a câmera do ESP32 Cam, com as configurações acima:
  esp_camera_init(&config); // Obtém o retorno da inicialização e o salva em uma variável.
}

void setup() { // Função sem retorno, a qual é executada uma única vez para a realização de configurações iniciais
// (correspondentes ao estabelecimento da conexão com a rede Wifi):
  WiFi.begin(rede_wifi, senha_da_rede_wifi); // Inicia a conexão do ESP32 com os parâmetros passados.
  // Laço que aguarda o estabelecimento da comunicação com a rede.
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
  }
  configura_ESP32Cam(); // Chama e executa a função de configuração do ESP32 Cam.
  pinMode(FLASH_LED_PIN, OUTPUT); // Define o pino do LED de flash como saída.
  digitalWrite(FLASH_LED_PIN, LOW); // Desliga o LED do flash do ESP32 Cam.
  captura_imagem(); // Captura uma nova imagem e a transmite para o servidor e então para o ThingSpeak.
  tentativas = 0; // Reseta o número de tentativas.
  updateThingSpeak(imageUrl); // Fornece o caminho URL da nova imagem obtida e carregada no servidor flask de argumento para a função
  // que atualiza a imagem exibida na plataforma do ThingSpeak.
  tentativas = 0; // Reseta o número de tentativas.
  // Desconecta da rede Wifi, liberando recursos:
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  esp_camera_deinit(); // Desativa a câmera e outros periféricos.
  esp_deep_sleep(tempo_de_espera); // Coloca o ESP32 Cam no modo de deep sleep durante 1 hora, e depois o reinicia.
}

void captura_imagem() {  // Laço de execução constante e sem retorno, no qual se executa o programa principal
// (visando capturar uma imagem a cada hora, repassar ela para o servidor flask e então para o ThingSpeak):

  digitalWrite(FLASH_LED_PIN, HIGH); // Liga o LED do flash do ESP32 Cam antes de realizar a captura da imagem.
  delay(150); // Aguarda um breve período de tempo para garantir que o LED do flash se estabilize.

  camera_fb_t *fb = esp_camera_fb_get(); // Realiza a captura de uma imagem, conforme configurado, e retorna o
  // ponteiro para a posição da imagem, o qual é salvo em uma variável.

  digitalWrite(FLASH_LED_PIN, LOW); // Desliga o LED do flash do ESP32 Cam após realizar a captura da imagem.

  // Abaixo se realiza o upload da imagem obtida para o servidor flask:
  WiFiClientSecure client; // Define um cliente HTTPS.
  client.setInsecure(); // Ignora a verificação SSL para funcionar com a comunicação com o Render.
  HTTPClient Flask; // Define um objeto da classe HTTP, nomeado de Flask, correspondente ao servidor flask que armazenará
  // as imagens.
  // realiza a configuração das propiedades de: caminho do servidor, tipo de conteúdo e tempo de requisição máximo:
  Flask.begin(client, flask_url);
  Flask.addHeader("Content-Type", "image/jpeg");
  Flask.setTimeout(30000); // Configura um tempo de retorno de no máximo 30 segundos para processar a requisição.

  // Realiza a requisição de postagem da imagem, apontada pelo ponteiro e de comprimento len, está em andamento:
  int httpResponseCode = Flask.POST(fb->buf, fb->len); // Realiza a postagem da imagem para o servidor flask e retorna a resposta da requisição
  // armazenado-a em uma variável.
  
  if (httpResponseCode == 200) { // Caso a requisição tenha ocorrido devidamente:
    imageUrl = Flask.getString(); // Obtém o caminho URL da imagem carregada no servidor e salva o caminho em uma variável.
    // Remove o início do caminho do URL, caso este começe com "https://", a fim de evitar problemas de compatibilidade com o ThingSpeak:
    if (imageUrl.startsWith("https://")) {
      imageUrl = imageUrl.substring(8); // Atualiza o caminho desconsiderando os primeiros 8 caracteres (correspondentes a "https://").
    }
    if (imageUrl.startsWith("http://")) {
      imageUrl = imageUrl.substring(7); // Atualiza o caminho desconsiderando os primeiros 7 caracteres (correspondentes a "http://").
    }
  }
  else{ // Caso a requisição tenha falhado:
    if(tentativas < 10){ // Caso não tenham ocorrido dez tentativas de envio:
      Flask.end(); // Finaliza a requisição HTTP atual.
      delay(5000); // Aguarda 5 segundos antes de nova tentativa (poderia ser substituído por um temporizador caso houvessem outras tarefas).
      tentativas = tentativas + 1; // Incrementa o número de tentativas.
      captura_imagem(); // Realiza uma nova captura e uma nova transmissão.
    }
  } 
  Flask.end(); // Finaliza a requisição HTTP.
  esp_camera_fb_return(fb); // Libera o espaço de memória correspondente ao buffer utilizado para a imagem.
}

// Função que repassa a última imagem coletada para o respectivo field no ThingSpeak (ao informar o caminho URL da imagem no servidor flask):
void updateThingSpeak(String imageUrl) {
  
  HTTPClient ThingSpeak; // Cria o objeto ThingSpeak com classe de cliente HTTP.
  
  String url = String(servidor_ThingSpeak) + "?api_key=" + chave_api_de_escrita + "&field1=" + imageUrl; // Define o endereço URL
  // do field associado a imagem coletada pelo ESP32 Cam, e repassa o caminho da imagem para o field.
  
  ThingSpeak.begin(url); // Configura o objeto ThingSpeak com o URL do serviço a se acessar.
  ThingSpeak.setTimeout(30000); // Configura um tempo de retorno de no máximo 30 segundos para processar a requisição.
  
  int codigo_HTTP = ThingSpeak.GET(); // Realiza a requisição de escrita no field do ThingSpeak (informado pelo URL).

  if ((codigo_HTTP != 200) && (tentativas < 10)) { // Caso a solicitação tenha falhado e tenham ocorrido menos de dez tentativas de envio:
    ThingSpeak.end(); // Finaliza a requisição HTTP atual.
    delay(5000); // Aguarda 5 segundos antes de nova tentativa (poderia ser substituído por um temporizador caso houvessem outras tarefas).
    tentativas = tentativas + 1; // Incrementa o número de tentativas.
    updateThingSpeak(imageUrl); // Realiza uma nova requisição para exibir a imagem na plataforma ThingSpeak.
  }

  ThingSpeak.end(); // Finaliza a requisição HTTP.
}

void loop(){ // Função sem retorno de repetição continua:
  // Nada ocorre nesta função devido a utilização do modo de deep sleep.
}
