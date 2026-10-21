# Relés e servidor knxd — firmware 0.2.0

A aplicação conserva BME280, duas sondas DS18B20 identificadas por ROM, OLED,
configuração web e OTA. Acrescenta quatro saídas independentes, opcionais.
A configuração anterior (schema 1) migra com as quatro saídas desativadas.

## Configurar pela web

Para cada relé: nome, ativação, GPIO, nível ativo baixo/alto, grupo de comando
e grupo de feedback DPT 1.001. Não são necessários recompilações para mudar
estes parâmetros. Guardar reinicia. GPIO -1 significa não atribuído.
O firmware rejeita conflitos entre saídas, sensores, OLED e grupos de medição.
O perfil disponível é ESP32 clássico; mudar para ESP32-S3 exige um perfil e
compilação adequados, não apenas escolher outros pinos.

Comando KNX GroupValueWrite 0/1: aplica saída e agenda feedback GroupValueWrite.
Comando na página web: aplica a mesma função e agenda o mesmo feedback.
Mesmo um comando repetido gera feedback. GroupValueRead no grupo de feedback
recebe GroupValueResponse. Respostas KNX não são tratadas como comandos.
O feedback representa o estado comandado, não confirmação física dos contactos.

A fila respeita o limite global de telegramas. Falhas de ACK não apagam feedback
pendente. Comandos rápidos podem ser agregados para publicar o estado mais recente.
Após reconexão publica os quatro estados ativos. A interface distingue feedback
pendente e enviado (ACK do túnel, sem garantia de atuação física no barramento).

## Arranque e falhas

Todas as saídas arrancam desligadas, sem restaurar ON de flash. Perda de Wi-Fi ou
KNX mantém o último estado; a web continua a comandar quando acessível.
Durante OTA não são aceites comandos; os relés mantêm o estado até ao reinício,
que volta a desligá-los. Não usar esta política para funções críticas.
O software pré-carrega OFF antes de configurar o GPIO como saída. Para evitar
impulsos durante reset, antes do firmware arrancar, são necessários níveis
elétricos adequados no módulo (pull-up/pull-down ou circuito de enable).
Os GPIO ESP32 são de 3,3 V. Alimentação de módulo a 5 V não comprova compatibilidade
das entradas. Confirmar modelo/foto e nível ativo antes de ligar. GPIO 4/15 e
pinos ocupados pela placa precisam de confirmação específica do hardware.

## Topologia

ESP32 (um túnel por dispositivo) → knxd no Raspberry Pi → um túnel no router KNX → TP.
O Home Assistant pode manter a sua ligação atual independente ao router.
Configurar no ESP o IPv4 completo do Raspberry e a porta do servidor, não o router.
O endereço mencionado terminado em .249 ainda precisa de confirmação: não foi
gravado como valor predefinido. A porta predefinida permanece 3671 e é editável.
Não há descoberta automática nem configuração remota do knxd neste firmware.

O número de clientes depende do pool de endereços individuais e dos recursos
configurados no knxd; não é ilimitado. Reservar endereços únicos suficientes para
5–10 dispositivos e verificar atribuição no diagnóstico. Não reutilizar endereços
de equipamentos TP nem presumir que cada cliente recebe sempre o mesmo endereço.

## Ensaio no equipamento

1. Com as cargas desligadas, configurar um relé e verificar OFF ao arrancar.
2. Enviar 1 e 0 ao grupo de comando no ETS: confirmar saída e feedback separado.
3. Repetir pela web e observar feedback no ETS/HA.
4. Ler o grupo de feedback: confirmar GroupValueResponse.
5. Desligar o acesso ao knxd, mudar pela web e restaurar: confirmar estado atual.
6. Testar comandos repetidos/rápidos e reinício/OTA; repetir para as quatro saídas.
7. Confirmar que BME280, duas sondas, OLED e respetivas publicações continuam ativos.

As fotos da placa e do módulo de relés, GPIO físicos, endereços de grupo e IPv4
completo ainda são necessários para o ensaio físico. Não foi gravado firmware
nem alterada a instalação KNX durante este desenvolvimento.
