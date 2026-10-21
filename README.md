# iot-sensor-device

Base para dispositivos ESP32 com integração KNX.

O módulo reutilizável de KNXnet/IP tunneling está em [lib/knx-tunneling](lib/knx-tunneling).

- [Guia de reutilização e futura separação](lib/knx-tunneling/REUSE.md)
- [API e integração técnica](lib/knx-tunneling/README.md)
- [Validação e testes](lib/knx-tunneling/VALIDATION.md)

O módulo permanece neste repositório por agora. A biblioteca de comunicação
não inclui a aplicação completa de sensores, configuração web ou OTA.
