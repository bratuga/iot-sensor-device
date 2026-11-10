# Controlos Orbit — 10-11-2026

Referência visual: iot-vmc-controler main 754e3b0. Botão de tema claro/escuro persistido no navegador; botão circular com pressão de 1,5 s, cancelamento e estado confirmado. Cartões operacionais e termóstatos preservados.

O comando geral para todas as saídas controladas, incluindo relés independentes. Wi-Fi, sensores e comunicações continuam ativos. O pedido OFF e o plano de retoma são guardados em NVS; o arranque mantém OFF até novo comando Ligar. A primeira atualização, sem chave de comando geral, conserva a política de arranque anterior. Falhas de gravação param as saídas na sessão atual mas são apresentadas como erro, sem alegar persistência.

CPU: frequência real em MHz, porque este perfil Arduino ESP32 não disponibiliza contadores de utilização. Memória: percentagem ocupada do heap interno de 8 bits. Não representa toda a RAM física.

## Validação

Compilação ESP32 local e testes do componente de interface (pressão curta/longa, cancelamento, teclado, estado indisponível, falha e tema). Inspeção visual com APIs simuladas, computador e 390 px, claro/escuro. Os resultados de CI pertencem ao commit respetivo. Não foi instalado firmware nem validada a atuação elétrica/KNX em equipamento.

## Sensores/aquecimento

O bloqueio geral é separado da intenção já guardada do coletor/termóstato e dos modos/setpoints. Parar não apaga essa intenção. Alterações explícitas posteriores às configurações continuam a prevalecer. A retoma dos relés exige o mesmo GPIO, polaridade e grupos KNX; relés reconfigurados permanecem desligados.

O coletor confirma bomba OFF antes de fechar válvulas. Enquanto houver confirmação pendente, o círculo mostra A parar; após 90 segundos mostra Sem confirmação, nunca um OFF fictício. A retoma exige confirmação OFF e respeita a pausa térmica existente. Um ON externo durante OFF provoca novo comando OFF, respeitando a mesma sequência. Este dispositivo não pode impedir outro controlador KNX de insistir em comandos contrários.

O termóstato usa um token diferente por comando; ACKs antigos não confirmam a nova paragem. A indicação OFF geral exige comando, ACK e feedback OFF. Escritas de diagnóstico, porta série e entradas locais para grupos de atuação controlados são bloqueadas durante OFF; leitura e feedback continuam.

`test/power_tests.cpp` cobre persistência do plano, mudança de mapeamento dos relés, sequência bomba/válvulas e ON externo. A suite nativa existente cobre controlo, proteção térmica e KNX. `scripts/test_orbit_controls.cjs` verifica o componente extraído do firmware; os testes UI existentes mantêm os contratos dos cartões.
