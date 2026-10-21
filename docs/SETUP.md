# Preparar o primeiro dispositivo

## Informação ainda necessária

- Fotografia ou revisão exata do WIFI KIT 32, capacidade de flash e controlador OLED.
- GPIO efetivamente ligados ao BME280 e aos DS18B20.
- IP/porta do servidor knxd e grupos/DPT reservados no projeto KNX.
- Função e nome de cada sonda de temperatura.

O firmware não depende destas informações para compilar; depende delas para
uma instalação correta. O teste `lib/knx-tunneling/tests/esp32_compile` é apenas
um teste de compilação. Não contém o programa antigo com Wi-Fi/OLED/consola.

## Ligações

Confirmar a tensão admissível de cada módulo antes de ligar. O barramento do
ESP32 usa lógica de 3,3 V. BME280 e OLED podem partilhar SDA/SCL se os endereços
I2C forem diferentes; usar massa comum. O firmware permite BME280 em 0x76/0x77
(decimal 118/119) e OLED SSD1306 em 0x3C/0x3D (decimal 60/61).

Os DS18B20 usam alimentação externa de três fios, partilham um GPIO OneWire e
normalmente necessitam de um pull-up de 4,7 kΩ entre dados e 3,3 V. O modo
parasita não é suportado por esta aplicação. Considerar cablagem e pull-up
adequados ao comprimento real do barramento.

Os GPIO disponíveis neste perfil são 4, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23,
25, 26, 27, 32 e 33. Isto não significa que todos estejam livres em cada placa.
GPIO15 é um pino de strapping: confirmar o circuito da placa antes de usar.
GPIO16/17 podem estar ocupados em módulos com PSRAM. Os pinos de flash, USB
serial e os de entrada apenas são excluídos. Use -1 para desativar OneWire ou
reset do OLED. O reset OLED não é um comando de alimentação/Vext.

OLED: apenas SSD1306 I2C, largura 128 e altura 32/64. Placas que necessitem de
Vext ou outro controlador precisam de um perfil adicional após identificação.

## Configuração inicial em duas fases

1. Aceder via AP ou rede Wi-Fi; definir GPIO e ativar BME/OLED conforme a placa.
   Definir o GPIO OneWire e guardar/reiniciar.
2. Na página de estado, copiar cada ROM detetado para Sonda 1/Sonda 2 e atribuir
   o nome da sua função. A deteção é feita no arranque e mostra até oito sondas;
   apenas as duas selecionadas são lidas. Reiniciar para voltar a detetar.
3. Configurar os grupos e ativar apenas os canais desejados. Ativar a ligação
   KNX depois de confirmar o servidor e os endereços.

O intervalo de amostragem é comum (2–60 s, por omissão 5 s). Os intervalos de
publicação são independentes. `periodMs` aceita 2 s–24 h; `minGapMs` aceita
1 s–1 h e não pode exceder `periodMs`. O limite global do dispositivo é de
1–20 telegramas/s, por omissão 5. É necessário dimensionar a soma dos dispositivos.

## Mapa KNX sugerido — reservar antes de ativar

| Medição | Grupo ilustrativo | DPT | Unidade transmitida |
| --- | --- | --- | --- |
| Temperatura BME280 | 2/0/1 | 9.001 | °C |
| Humidade BME280 | 2/0/2 | 9.007 | % |
| Pressão BME280 (opcional) | 2/0/3 | 9.006 | Pa |
| DS18B20 1 | 2/0/4 | 9.001 | °C |
| DS18B20 2 | 2/0/5 | 9.001 | °C |

Estes grupos não são enviados por omissão. Substituir pelos grupos livres da
instalação. Não reutilizar os grupos de teste do protótipo sem confirmação.

A biblioteca de tunneling fornecida limita os grupos a 0x0000–0x7FFF. Para
preservar essa implementação já testada, esta aplicação aceita principal 0–15,
intermédio 0–7 e subgrupo 0–255, excluindo 0/0/0. O espaço KNX completo tem
principal 0–31; estender a biblioteca para 16–31 exige alteração e testes próprios.
Não configurar dois canais ativos com o mesmo grupo.

Pressão é transmitida em Pa, não hPa. DPT9 tem precisão variável com a magnitude;
a conversão arredonda ao valor representável mais próximo. Os sensores não
publicam NaN, valores desconectados ou leituras expiradas. Uma falha de sensor
não publica zero como se fosse uma medição válida.

Configurar o Home Assistant para ler estes grupos KNX e respetivos tipos de
dados. HA não precisa de contactar diretamente o ESP32. Cada dispositivo tem
um túnel; confirmar a capacidade e os endereços individuais disponíveis no knxd.
