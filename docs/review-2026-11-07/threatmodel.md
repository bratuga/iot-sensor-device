# Threat Model

ESP32 com sensores, relés, termóstato/aquecimento, HTTP80, KNX UDP, histórico SPIFFS, configuração persistente e OTA autenticado. Setup carrega configuração e inicia STA/AP (src/main.cpp:47,1579-1618).

## Assets

- Credenciais/configuração em SPIFFS /config0.bin e /config1.bin; legado NVS sensor/config mantém-se na migração (src/Config.cpp:14,469-513).
- Sessão admin/CSRF em RAM e firmware (src/main.cpp:738-758,1208-1227,1468-1518).
- Relés/KNX aquecimento, NVS heating/layout e heating/values; histórico SPIFFS /history0.bin e /history1.bin (src/main.cpp:127-148,863-864,895-897,1085-1087).

## Trust Boundaries

- Público LAN -> status/operações com CSRF público; intencional sem admin para operações (src/main.cpp:751-758,1186-1206,1305-1307,1392-1410).
- Login admin -> X-Admin-Session aleatório -> configuração/OTA; expiração 30min, bloqueio após5 falhas (src/main.cpp:738-758,1208-1235,1247-1268).
- LOCAL_AP_PASSWORD se definido é credencial admin alternativa independente da password persistida (src/main.cpp:1217-1221).
- Gateway KNX configurado -> túnel -> comandos/feedback; datagramas limitados256 (src/main.cpp:637-676).
- OTA -> flash exige sessão, CSRF, estado não ocupado e heatingSafe; pára túnel (src/main.cpp:1486-1518).

## Attacker Capabilities

- LAN/AP tem leituras/comandos operacionais intencionalmente públicos.
- Sem sessão não deve alterar config/firmware ou aceder diagnóstico KNX.
- Peers KNX têm autoridade barramento; segmentação real desconhecida.
- Acesso físico/build é separado de autoridade Web.

## Security Objectives

- Política nova: admin inicial admin/iot-device alterável e persistente; AP comum; leituras/comandos públicos e configuração/OTA/diagnóstico privados.
- Preservar dual-copy validado e redação de credenciais no GET (src/Config.cpp:24-59,469-513; src/main.cpp:1232).
- Preservar shutdown/feedback em config/update; não inferir segurança física.

## Assumptions

- Build atual gera admin aleatório e AP Sensor-<MAC>, senha admin ou override LOCAL_AP_PASSWORD (src/main.cpp:585-599,1585-1590).
- Status público inclui logs telegramas mesmo sem admin/showDiagnostics; difere privacidade pretendida (src/main.cpp:1405-1466).
- Flash encryption, secure boot, overrides privados e hardware não verificados.
- Config SPIFFS e legado NVS podem conter segredos; web omite. AP 192.168.4.1 HTTP80. KNX endpoint vem config.gateway/port/rate. Aquecimento guarda layout/values NVS; histórico valida ambas cópias SPIFFS (src/Config.cpp:14-59,469-513; src/main.cpp:585-625,637-676,127-148,1381-1386).

---

## Saved Model Context

- Source: scan
- Scan: 5c1a23ae-cf1b-4803-aee6-bfe875376fd7
- Target: iot-sensor-device
- Revision: c92b7e570285bf8303ddabccb832fb70d7eabd24
- Result status: completed
- Model origin: not recorded
- Model scope: not recorded
- Scan scope: .
- Scan scope description: Revisão estática de bratuga/iot-sensor-device no main c92b7e570285bf8303ddabccb832fb70d7eabd24. Branch de revisão: review/security-2026-11-07. Sem alterações de firmware.
