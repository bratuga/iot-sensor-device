# Contexto de retoma — MySmartHouse / iot-sensor-device

Atualizado em 2026-11-07. Este ficheiro resume decisões e pontos de retoma; o código e os testes em `main` são a referência para o comportamento exato. É o firmware do sensor/coletor KNX. O controlador de ventilação MyVMC tem repositório próprio (`bratuga/iot-vmc-controler`).

## Objetivo e instalação

O ESP32 integra sensores e relés locais no KNX e pode assumir o controlo de um piso radiante ou funcionar como termóstato de uma divisão. A instalação de referência tem cinco divisões, sete circuitos e uma bomba de circulação. Dois quartos têm dois circuitos cada. O software admite até oito circuitos associados às divisões. Usa KNXnet/IP tunneling via knxd num Raspberry Pi. O Home Assistant lê o barramento KNX.

O protótipo é um WiFi Kit 32. O BME280 foi validado no endereço I²C 0x77 (119 decimal), com SDA=23 e SCL=22. O OLED é opcional. As duas sondas DS18B20 representam ida (água quente) e retorno; confirmar o mapeamento das ROM no equipamento. Os quatro relés locais são independentes do coletor, cujas saídas são comandadas pelo KNX.

## Mapa do código

| Área | Ficheiros |
| --- | --- |
| Arranque, sensores, rede, APIs HTTP, integração KNX e OTA | `src/main.cpp` |
| Configuração, validação e persistência | `src/Config.cpp`, `include/Config.h`, `include/ConfigJournal.h` |
| Interface web embebida no firmware | `include/WebUi.h` |
| Pedidos por divisão, sequenciamento KNX, leituras e estados guardados | `include/HeatingCore.h`, `include/HeatingRuntime.h`, `include/HeatingReadPlan.h`, `include/HeatingSnapshot.h` |
| Proteções térmicas e modo termóstato | `include/PumpThermalGuard.h`, `include/ThermostatCore.h` |
| Histórico, sensores e relés | `include/History.h`, `include/SensorCore.h`, `include/Bme280Direct.h`, `include/RelayCore.h` |
| Cliente de túnel e verificações | `lib/knx-tunneling/`, `test/`, `scripts/` |

Documentação: [instalação](SETUP.md), [aquecimento](HEATING.md), [operações](OPERATIONS.md), [relés](RELAYS.md) e [validação](VALIDATION.md).

## Decisões a preservar

- Bomba e válvulas: endereços KNX separados de comando e feedback (DPT 1.001). O ACK do túnel confirma transporte; a atuação só é dada por concluída após o feedback de estado. O feedback KNX não substitui uma confirmação física independente.
- Por divisão: associar um ou mais circuitos e configurar os grupos de pedido/ativação, estado, temperatura e setpoint. Temperaturas em DPT 9.001. Painel com modo automático/manual, estado e setpoint; a associação de circuitos fica nas configurações e a visualização física no coletor.
- Leitura KNX no arranque ou a pedido. Depois, usar telegramas recebidos em vez de reler continuamente todo o barramento. Estados guardados devem ser distinguíveis de feedback recente.
- Abrir circuito e aguardar confirmação e tempo de abertura antes de arrancar a bomba. Nas paragens de segurança, confirmar OFF da bomba antes de fechar as válvulas. O controlo não deve retomar se faltar confirmação OFF das saídas utilizadas.
- Proteção da ida: com pedido de calor, permitir circulação apesar de a água estar fria. Contar cinco minutos consecutivos abaixo do mínimo apenas com feedback KNX de bomba ligada e leitura válida. Ao atingir o mínimo, limpar a contagem. Se continuar abaixo do mínimo, parar, aguardar a pausa configurada e repetir com nova janela. A proteção do retorno permanece independente.
- Configuração web separa Funcionalidades, KNX, Hardware e Configuração. Os campos de canais inativos não devem impedir guardar por causa de endereços antigos. Guardar configuração geral e guardar mapa de aquecimento são operações distintas; confirmar o comportamento nas respetivas APIs antes de as alterar.
- Preservar persistência, atualização OTA e autenticação. Não guardar credenciais ou valores privados de instalação neste ficheiro.

## Último ponto confirmado e próxima verificação

A versão em `main` verificada para este ponto de retoma é `0.3.17-sensor-knx-fields`. Corrige a duplicação dos seletores ao reabrir as configurações e acrescenta **Configurar envio KNX dos sensores**, que abre o separador KNX. Se surgir um erro como «grupo KNX '0' inválido», confirmar que o campo está visível e introduzir o endereço de grupo completo definido no ETS. Cada canal ativo precisa do seu endereço. Temperatura BME: DPT 9.001; humidade: DPT 9.007; pressão em Pa: DPT 9.006.

Os testes de interface passaram após a correção dos campos. A lógica de circulação e repetição de tentativas passou nos testes nativos da versão anterior. Ainda não há confirmação, nesta conversa, de compilação e instalação OTA de `0.3.17` no equipamento nem da publicação BME observada no barramento. Próximo passo: instalar essa versão, reabrir as configurações, ativar os canais pretendidos, preencher os grupos, guardar e verificar que persistem após reinício e que os telegramas chegam ao KNX.

## Fluxo de trabalho

Publicar alterações no `main` de `bratuga/iot-sensor-device`. Na cópia local, o operador faz:

```powershell
git pull
pio run -e esp32
```

Para OTA, usar `.pio/build/esp32/firmware.bin` e confirmar a versão no painel após reiniciar. Compilar não instala o firmware. Monitor série: 115200 baud. Para verificações no repositório: `python3 scripts/test_host.py`, `node scripts/test_web_ui_minimal.cjs` e `node scripts/test_web_ui.cjs` (com jsdom). Distinguir testes de software, compilação para ESP32 e validação no equipamento.
