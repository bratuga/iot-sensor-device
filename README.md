# MySmartHouse · Sensor KNX

Firmware reutilizável para ESP32 clássico, com BME280, duas sondas DS18B20,
OLED SSD1306 opcional, configuração web e atualização OTA. Comunica por Wi-Fi
com um servidor KNXnet/IP tunneling UDP, como o knxd. O Home Assistant lê os
valores através do KNX; não há dependência de MQTT nem de HA no firmware.

## Estado

Primeira implementação do dispositivo de sensores (0.1.0). O módulo de tunneling
foi previamente confirmado pelo proprietário em dois projetos num ESP32.
A aplicação de sensores/web/OTA é nova e requer ensaio na placa e instalação.
O perfil de compilação atual é ESP32 clássico com flash de 4 MB; a revisão exata
do WIFI KIT 32 e o respetivo pinout ainda têm de ser confirmados. Não usar esta
imagem num ESP32-S3.

## Começar

1. Clonar este repositório e abrir a pasta no VS Code com PlatformIO.
2. Compilar com `pio run -e esp32`.
3. Confirmar a placa e flash antes de carregar por USB:
   `pio run -e esp32 -t upload`.
4. Abrir a consola com `pio device monitor -b 115200`. No primeiro arranque é
   mostrada uma password aleatória para o utilizador web `admin` e para o AP.
   O comando USB `SETUP` volta a mostrar a password e abre o AP.
5. Ligar à rede `Sensor-…` e abrir `http://192.168.4.1`.
6. Guardar SSID/password Wi-Fi. Configurar pinos, sensores, servidor KNX e grupos
   pela web. Cada gravação reinicia o dispositivo.

Os pinos, sensores, OLED e ligação KNX começam desativados. Não são assumidos
pinos de outra revisão Heltec. A password de administração deve ter 12–63
caracteres; os campos de password vazios mantêm os valores anteriores.

## Funcionalidades

- Temperatura/humidade e pressão opcional do BME280.
- Duas sondas DS18B20 identificadas por ROM com CRC, sem depender da ordem de deteção.
- Leitura DS18B20 assíncrona, validação de valores e expiração de leituras antigas.
- Endereço de grupo, nome, correção, período, variação mínima e intervalo mínimo
  independentes para cada medição.
- Envio periódico, por variação ou ambos; primeira publicação após conexão;
  confirmação do túnel antes de atualizar o último valor enviado.
- Resposta a GroupValueRead com o valor válido atual, mesmo quando os dois modos
  de publicação espontânea estiverem desligados.
- Reconexão Wi-Fi/KNX com espera para evitar acumular sessões abandonadas.
- Monitor de telegramas na web, OLED e consola; comandos USB de diagnóstico.
- Configuração persistente em NVS; web autenticada, proteção CSRF e OTA autenticada.
- Exportação JSON sem passwords para consulta; importação automática não implementada.

## Documentação

- [Instalação, ligações e mapa KNX](docs/SETUP.md)
- [Operação, diagnóstico e recuperação](docs/OPERATIONS.md)
- [Validação e limitações](docs/VALIDATION.md)
- [Reutilização do módulo de tunneling](lib/knx-tunneling/REUSE.md)
- [API do módulo KNX](lib/knx-tunneling/README.md)

## Estrutura e testes

`src/` contém a aplicação e a configuração; `include/` contém a lógica de
publicação, conversão DPT e interface web embutida. `lib/knx-tunneling/` conserva
a biblioteca de comunicação existente, independente dos sensores.

Executar `python scripts/test_host.py` para testes nativos. O workflow GitHub
Actions compila o firmware ESP32 e disponibiliza `firmware.bin` como artefacto
quando a compilação passa. Binários e caches não são versionados no Git.

A administração usa HTTP numa rede local de confiança. O firmware não implementa
TLS, KNX Secure, assinatura de firmware nem rollback automático da aplicação.

## Saídas de relé (0.2.0)

Quatro relés configuráveis pela web, com GPIO, polaridade e grupos KNX de comando/feedback.
Controlo web e KNX partilham o mesmo estado. Ver [relés e knxd](docs/RELAYS.md).

## Aquecimento por divisões e coletor KNX

Até oito circuitos e uma bomba, divisões editáveis e associações de vários circuitos
por divisão. O painel mostra temperaturas de ida/retorno e indicadores de estado.
Configuração KNX, controlo automático/manual e sequência de bomba/válvulas:
[docs/HEATING.md](docs/HEATING.md). O controlo arranca parado e requer configuração
dos grupos e ativação explícita no painel.
