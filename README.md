# 📊 Sistema de Monitoramento de Vinheria com ESP32 e MQTT

## 📝 Descrição do Projeto

Este projeto foi desenvolvido para a **Vinheria Agnello** com o objetivo de monitorar as condições de armazenamento dos vinhos utilizando um **ESP32** conectado a um servidor de IoT por meio do protocolo **MQTT**.

A vinheria pretende expandir suas atividades para o mercado europeu e, por isso, o sistema segue o padrão de tópicos do **FIWARE (IoT Agent MQTT / UltraLight 2.0)**, preparando a solução para interoperar com o ecossistema de negócios da União Europeia.

O sistema realiza o monitoramento de:

- 🌡️ Temperatura
- 💧 Umidade
- 💡 Luminosidade

Cada leitura é enviada ao servidor com **timestamp**, via Wi-Fi, e pode ser visualizada em tempo real pelo aplicativo **MyMQTT**. Pelo mesmo aplicativo também é possível enviar comandos ao dispositivo.

---

## ⚙️ Condições Ideais de Armazenamento

### 🌡️ Temperatura

- Ideal: cerca de **13°C**
- Oscilações acima de 3°C podem causar aromas indesejados
- Calor excessivo reduz rapidamente a vida útil do vinho

### 💧 Umidade

- Ideal: próxima de **70%** (entre 60% e 80%)
- Umidade baixa resseca o vedante e favorece a oxidação
- Umidade alta danifica rótulos e favorece fungos

### 💡 Luminosidade

- Ambiente com iluminação muito suave ou em penumbra
- A luz, principalmente a ultravioleta, altera os compostos orgânicos do vinho

---

## 📡 Comunicação MQTT

O dispositivo é identificado como `wine001` e utiliza a chave de serviço `TEF`, padrão do FIWARE IoT Agent MQTT.

| Função | Tópico | Exemplo de payload |
|--------|--------|--------------------|
| Leituras completas | `/TEF/wine001/attrs` | `t\|24.0\|h\|40.0\|l\|24\|ts\|2026-10-22T00:48:03Z` |
| Temperatura (°C) | `/TEF/wine001/attrs/t` | `24.0` |
| Umidade (%) | `/TEF/wine001/attrs/h` | `40.0` |
| Luminosidade (%) | `/TEF/wine001/attrs/l` | `24` |
| Comandos para o dispositivo | `/TEF/wine001/cmd` | `wine001@on\|` ou `wine001@off\|` |
| Resposta aos comandos | `/TEF/wine001/cmdexe` | `wine001@on\|LED ligado` |

Significado dos campos do payload:

- `t` → temperatura
- `h` → umidade
- `l` → luminosidade
- `ts` → timestamp da leitura (ISO 8601, UTC)

---

## 🧠 Funcionamento do Código

O código foi desenvolvido na Arduino IDE e executa continuamente o seguinte fluxo:

### Principais funcionalidades:

- Conexão do ESP32 ao Wi-Fi
- Sincronização de data e hora via NTP para o timestamp
- Conexão ao broker MQTT
- Leitura de temperatura e umidade (DHT11) e de luminosidade (LDR)
- Envio das leituras ao servidor a cada 5 segundos
- Recebimento de comandos para ligar e desligar o LED
- Confirmação da execução dos comandos
- Reconexão automática ao Wi-Fi e ao broker em caso de queda
- Exibição dos dados no Monitor Serial

### Sensores utilizados:

- DHT11 → temperatura e umidade
- LDR → luminosidade

---

## 🔩 Componentes Utilizados

- 1x ESP32
- 1x Sensor DHT11
- 1x Sensor LDR
- 1x LED
- 1x Resistor 220Ω (na simulação)
- Jumpers
- Cabo USB
- 1x Celular com o aplicativo MyMQTT

---

## 🔌 Montagem do Circuito

### Sensores

- DHT11 (DATA) → D4
- LDR → D34

### LED

- LED1 → D2

### Alimentação

- VCC dos sensores → 3V3
- GND dos sensores → GND

---

## 💻 Dependências

- Arduino IDE
- Suporte à placa **esp32 by Espressif Systems**
- Bibliotecas:
  - `PubSubClient`
  - `DHT sensor library`
  - `Adafruit Unified Sensor`
- Aplicativo MyMQTT

---

## 📁 Estrutura do Repositório

```
firmware/vinheria_agnello.ino    Código do ESP32
wokwi/sketch.ino                 Código da simulação no Wokwi
wokwi/diagram.json               Circuito da simulação no Wokwi
wokwi/libraries.txt              Bibliotecas da simulação no Wokwi
README.md                        Documentação do projeto
```

---

## ▶️ Como Executar o Projeto

### ESP32

1. Instale a Arduino IDE
2. Instale o suporte à placa **esp32 by Espressif Systems**
3. Instale as bibliotecas **PubSubClient**, **DHT sensor library** e **Adafruit Unified Sensor**
4. Abra o arquivo `firmware/vinheria_agnello.ino`
5. Configure `SSID` e `PASSWORD` com os dados da rede Wi-Fi (rede 2,4 GHz)
6. Conecte o ESP32 via USB
7. Selecione:
   - Placa: ESP32 Dev Module
   - Porta correta
8. Faça o upload do código
9. Abra o Monitor Serial em 115200 baud

### Simulação no Wokwi

1. Acesse o projeto: https://wokwi.com/projects/478542509886996481
2. Clique em **Start Simulation**
3. Acompanhe o Serial Monitor com a conexão ao Wi-Fi, ao broker e as leituras
4. Clique no sensor DHT22 e no LDR durante a simulação para alterar os valores lidos

Na simulação, o código utiliza a rede virtual `Wokwi-GUEST` (sem senha) e o sensor DHT22 no lugar do DHT11, pois o Wokwi não possui o DHT11. A leitura é feita com a mesma biblioteca e o restante do código é idêntico.

### MyMQTT

O servidor MQTT utilizado é o disponibilizado pelo professor.

1. Abra o MyMQTT e, na aba **Dashboard**, adicione uma conexão com:
   - Broker: `3.135.234.101`
   - Porta: `1883`
   - Usuário e senha: em branco
2. Conecte ao broker
3. Na aba **Subscribe**, assine `/TEF/wine001/attrs` para visualizar as leituras
4. Assine também `/TEF/wine001/cmdexe` para receber a confirmação dos comandos
5. Na aba **Publish**, envie no tópico `/TEF/wine001/cmd`:
   - `wine001@on|` para ligar o LED
   - `wine001@off|` para desligar o LED

---

## 🧪 Monitor Serial

O sistema também envia informações para o Monitor Serial, exibindo a cada 5 segundos:

- Data e hora da leitura
- Temperatura
- Umidade
- Luminosidade

Exemplo:

```
[2026-10-22T00:47:42Z] T=24.0 C | U=40.0 % | L=24 % (raw=1001)
```

---

## 🔗 Simulação no Wokwi

[https://wokwi.com/projects/478542509886996481](https://wokwi.com/projects/478542509886996481)

---

## 👥 Integrantes

- Gabriel Freitas da Silva Carvalho — RM 570881
- Erick Martins Picolo — RM 572892
- Guilherme Marcon Dantas — RM 572171
- Luiz Felipe Cardoso de Oliveira — RM 569782
