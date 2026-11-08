# Uniformização Orbit — branch de revisão

Implementação na branch `design/orbit-uniformizacao`. Não foi feito merge nem atualizado qualquer equipamento.

- Navegação Orbit sem a linha curva lateral, paletas do firmware, configurações numa página separada e cinco grupos.
- Dashboards e comandos operacionais públicos na LAN; configuração autenticada.
- Primeiro administrador `admin` / `iot-device`; credenciais persistidas preservadas e password alterável.
- ESP: recuperação `iot-device` / `iot-device`, pesquisa de redes, hostname, DHCP/IP fixo e NTP. O Moxa não tem assistente AP.
- Testes locais não equivalem a validação elétrica, instalação ou ensaio no equipamento.

## Limites e decisões pendentes

Coletor e termóstatos preservados, incluindo debounce dos setpoints, confirmação de feedback e proteções térmicas. OLED configurado mostra acesso AP quando a ligação Wi-Fi falha. Rede/NTP persistem no journal existente; migração mantém defaults DHCP e passwords anteriores. Diagnóstico e telegramas requerem autenticação. Logs em RAM; não foi inventado suporte microSD que o firmware não possui.

Validação local: compilação ESP32, testes JavaScript mínimos e suite DOM de termóstatos/coletor/KNX. GPIO e operação física aguardam ensaio autorizado.

A vulnerabilidade do parser Arduino-ESP32 2.0.17 descrita na branch `review/security-2026-11-07` continua pendente de atualização da plataforma e validação de compatibilidade. A proteção dos handlers não corrige o parser.

Compatibilidade: branch atualizada sobre main 300b189, incluindo o fecho/reabertura das válvulas durante a pausa temporizada da bomba. Testes nativos e testes JS/DOM passaram na CI dessa base.
