# Reutilizar o módulo KNX tunneling

## Finalidade e estado

Este módulo C++14 implementa um cliente KNXnet/IP tunneling UDP independente
da aplicação. Está em `lib/knx-tunneling` neste repositório; pode ser copiado
integralmente para outro projeto ou, mais tarde, para a raiz de um repositório próprio.

Em 21/10/2026, o proprietário confirmou que esta versão já foi utilizada em dois
projetos num ESP32 clássico, com envio e receção KNX a funcionar. Trata-se de
validação física reportada pelo proprietário, não de um ensaio executado nesta
sessão. Essa confirmação não abrange ESP32-S3.

## O que transportar

| Ficheiros | Função |
| --- | --- |
| `include/knx/TunnelClient.h` e `src/TunnelClient.cpp` | API e implementação do módulo |
| `library.json` | Metadados para utilização como biblioteca PlatformIO |
| `README.md`, `REUSE.md`, `VALIDATION.md` | Integração, reutilização e registo de testes |
| `tests/` e `scripts/run_tests.py` | Testes reproduzíveis; não integram o firmware em execução |
| `.gitignore` | Exclusão de ficheiros gerados |

Não transportar `build/`, `.pio/`, caches, binários, credenciais ou perfis privados.
Os ficheiros de configuração para compilar, como `platformio.ini` e
`library.json`, são úteis; os resultados gerados pela compilação são descartáveis.

## Integrar noutro projeto

### PlatformIO

Copiar esta pasta para `lib/knx-tunneling` no projeto de destino, preservando a
estrutura interna. Incluir `<knx/TunnelClient.h>` no código da aplicação e usar
C++14 ou superior. O exemplo em `tests/esp32_compile` documenta a configuração
usada no teste de compilação; não é uma aplicação de rede pronta a instalar.

### Outro sistema de compilação

Compilar `src/TunnelClient.cpp` juntamente com a aplicação e acrescentar
`include/` aos diretórios de cabeçalhos, com C++14 ou superior.
A API é C++; uma aplicação exclusivamente em C precisa de um adaptador C/C++.

### Responsabilidades da aplicação

1. Disponibilizar rede e um socket UDP ligado a uma porta local.
2. Implementar `knxip::Transport::send`: enviar ou copiar integralmente o
   datagrama antes de retornar; indicar sucesso ou falha.
3. Implementar `knxip::Listener` para receber telegramas, eventos e confirmações.
4. Criar `TunnelClient`, manter o transporte e o listener vivos e preencher
   `Config` com o endereço do servidor e os limites de envio.
5. Chamar `start(config, endpointLocal, nowMs)` e verificar o retorno.
   O endpoint deve conter o IP e a porta reais do socket.
6. Entregar cada datagrama UDP completo a `onDatagram`, incluindo IP e porta
   de origem, e chamar `poll(nowMs)` frequentemente, tipicamente a cada 1–10 ms.
7. Executar tudo na mesma tarefa ou serializar o acesso. Os callbacks devem ser
   curtos e não chamar funções do ciclo de vida do cliente de forma reentrante.

O relógio é monotónico em milissegundos. O IP é representado pelo inteiro dos
quatro octetos: 192.0.2.20 corresponde a `0xC0000214`.
Wi-Fi, OLED, sensores, configuração web, OTA e persistência ficam na aplicação.

## Enviar e receber valores

- `writeBool(group, value, token)`: valor booleano compacto.
- `write(group, bytes, size, false, token)`: envio de valor multibyte.
- `read(group, token)`: pedido de leitura.
- `respond(...)`: resposta a um pedido de leitura.
- `Listener::onTelegram`: telegrama recebido, com grupo, origem, operação e dados.
- `Listener::onSent`: confirmação do túnel; não comprova o estado de um atuador.

Verificar sempre `Queued`, `NotConnected`, `QueueFull` ou `InvalidArgument`.
A fila contém até 16 mensagens copiadas. O payload normal tem até 14 bytes.
A aplicação define os grupos e os DPT e faz a conversão dos valores dos sensores;
o módulo não converte automaticamente temperatura ou humidade.
Validar os componentes do endereço de grupo antes de usar `groupAddress`:
principal 0–31, intermédio 0–7 e subgrupo 0–255.

## Ligação, recuperação e vários dispositivos

Usar normalmente `AddressMode::Assigned` e aceitar o endereço individual
atribuído pelo servidor. O destino de dados pode ter uma porta diferente da
porta de controlo: o transporte deve respeitar o endpoint passado pelo módulo.

Começar com um limite conservador, por exemplo 5 telegramas por segundo.
O limite inclui retransmissões e aplica-se a cada instância, não à soma dos
dispositivos. Dimensionar a carga agregada e as ligações disponíveis no servidor.

Na perda de rede, chamar `networkLost` uma vez e suspender `poll`.
Retomar com o mesmo IP/socket; se mudarem, respeitar a limpeza ou expiração da
sessão anterior antes de criar um novo cliente. Para terminar normalmente,
chamar `stop` e continuar a receber e a executar `poll` até `Stopped`.
As mensagens pendentes são descartadas após perda da ligação; a aplicação
decide se deve publicar valores atualizados.

## Testar depois de integrar

A partir da pasta do módulo:

```sh
python scripts/run_tests.py --sanitize
```

O teste exige Python 3 e um compilador C++14; aceita também
`--compiler /caminho/para/compilador`. Usa transporte simulado e não contacta o KNX.
Consultar `VALIDATION.md` para distinguir testes locais e confirmação física.
Uma nova aplicação ou plataforma deve validar a sua própria integração,
sobretudo reconexão, atribuição de grupos e codificação dos valores.

## Separar para um repositório próprio

1. Copiar o conteúdo desta pasta para a raiz do novo repositório.
2. Manter código, testes, metadados e documentação juntos.
3. Executar o teste a partir da nova raiz e confirmar os caminhos relativos.
4. Criar uma versão identificável e fixar essa versão nos projetos consumidores.
5. Decidir explicitamente a licença antes de distribuir a biblioteca a terceiros.

A estrutura atual não depende do MyVMC nem de caminhos do computador original.
Por agora o módulo permanece neste repositório; não é criado outro repositório.
Não inclui KNX Secure, tunneling TCP, routing ou uma interface elétrica TP.
