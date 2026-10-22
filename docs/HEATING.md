# Aquecimento: sete circuitos e uma bomba

## Estado e âmbito

Análise de 21/10/2026 sobre o PR #1, commit
`bcd771f9012829051f463c87e4c0b55141cf2eee` (ainda aberto nesta análise).
`include/HeatingCore.h` implementa um núcleo C++14 sem dependências de hardware,
com testes nativos em `test/heating_tests.cpp`. É uma primeira implementação
manual **isolada**, sem instância no loop, endpoints Web, persistência, emissões
KNX ou acesso a GPIO. Não é um controlador instalado nem firmware pronto para
comissionar. Não foram atribuídos endereços ou ligações físicas.

O código atual concentra aquisição BME280/DS18B20, OLED, Web, OTA e encaminhamento
KNX em `src/main.cpp`. `Config.cpp` valida quatro relés e preserva os sensores;
`RelayCore.h` gere estado comandado e publicação com revisões/ACK. O túnel portátil
está em `lib/knx-tunneling`. `setRelay()` é usado pela Web e telegramas de comando,
mas não conhece bomba, válvulas nem zonas. Os quatro relés mantêm estado durante
perda de rede e OTA; essa política não constitui uma sequência de aquecimento.

Esta alteração preserva esses caminhos e o schema 2. A ligação ao runtime deve
ser um passo explícito após comissionamento. Não basta aumentar os arrays de 4
para 8: é necessário garantir exclusividade de comando, capacidade elétrica,
feedback, falhas do transporte e sequência hidráulica.

## Topologias comparadas e decisão proposta

| Opção | Capacidade e benefícios | Condições e limitações |
|---|---|---|
| Atuadores KNX existentes | Oito canais lógicos independentes; aproveita canais já cablados; sensores podem continuar distribuídos | Confirmar modelos, canais livres, tipos de válvula, capacidade para bomba/contactor, feedback real e watchdog autónomo; perda do ESP/túnel não pode deixar bomba ligada sem caminho aberto |
| Oito saídas locais | Coordenação local sem depender do transporte KNX para aplicar saídas | Pelo menos oito canais independentes, revisão de placa, drivers, alimentação, isolamento e eventual contactor; quatro GPIO adicionais não são automaticamente utilizáveis; ESP permanece ponto único de falha |
| Híbrida | Mapeia cada canal para KNX ou local; reutiliza parte dos quatro relés | Mesmas garantias entre transportes; nunca fechar válvulas locais só porque se enviou OFF à bomba remota; maior complexidade de validação |

Recomendação condicional: reutilizar atuadores KNX **se** tiverem canais adequados
e comportamento autónomo seguro comprovado. Caso contrário, dimensionar oito
saídas locais ou solução híbrida. Não se conhece ainda a capacidade dos atuadores
existentes. O software utiliza oito identidades lógicas, independentemente da
escolha. Um canal não pode servir simultaneamente duas identidades. Dois circuitos
no mesmo quarto partilham zona térmica, mas conservam comandos individuais.

## Mapeamento lógico proposto

| Circuito | Índice no código | Zona | Identificação |
|---|---:|---|---|
| C1 | 0 | Z1 | Quarto A, circuito 1 |
| C2 | 1 | Z1 | Quarto A, circuito 2 |
| C3 | 2 | Z2 | Quarto B, circuito 1 |
| C4 | 3 | Z2 | Quarto B, circuito 2 |
| C5 | 4 | Z3 | Divisão por identificar |
| C6 | 5 | Z4 | Divisão por identificar |
| C7 | 6 | Z5 | Divisão por identificar |
| P1 | 7 | Sistema | Bomba |

A/B e Z3–Z5 são identificadores provisórios, não nomes de divisões confirmados.
`ZoneForCircuit` contém esta proposta. Backend começa `Unassigned`, canal local
-1 e grupos 0 (não atribuído, nunca endereço de destino). `validBindings` rejeita
canais repetidos, capacidade insuficiente, grupos de comando/feedback coincidentes
e colisões com uma lista de grupos reservados. O integrador deve passar **todos**
os grupos existentes e validar também GPIO/polaridade com `Config.cpp`.

## Objetos KNX propostos (endereços por atribuir no ETS)

Direções relativas ao coordenador ESP. Os nomes abaixo são contratos lógicos,
não endereços nem números de objetos de um produto ETS existente.

| Nome simbólico | DPT | Direção / semântica |
|---|---|---|
| Cn.Manual.Request | 1.001 | Recebe Write: pedido manual abrir/fechar |
| Cn.Requested | 1.001 | Publica e responde Read: pedido aceite |
| Cn.Output.Commanded | 1.001 | Publica e responde Read: saída pretendida pelo sequenciador |
| Cn.Actuator.Command | 1.001 | Envia ao atuador remoto; apenas backend KNX |
| Cn.Actuator.Physical | 1.001 | Recebe prova de posição aberta/fechada, se efetivamente disponível |
| Cn.Physical.Valid | 1.001 | Publica validade/frescura separada; 0 significa desconhecido/expirado |
| P1.Manual.Permit | 1.001 | Recebe permissão manual; não força bomba sem circuito |
| P1.Output.Commanded | 1.001 | Publica intenção de saída |
| P1.Actuator.Command | 1.001 | Envia pedido ao atuador da bomba |
| P1.Actuator.Physical | 1.001 | Recebe prova de funcionamento/paragem, conforme hardware validado |
| P1.Physical.Valid | 1.001 | Publica validade separada |
| System.Arm.Request | 1.001 | Write 1 solicita rearme; 0 solicita paragem segura |
| System.Armed / System.Fault | 1.001 | Estados publicados e legíveis |
| Zn.Temperature | 9.001 | Futuro: recebe temperatura em °C |
| Zn.Setpoint.Request / Zn.Setpoint | 9.001 | Futuro: pedido e valor aceite em °C, grupos distintos |
| Zn.Auto.Enable / Zn.Demand / Zn.Temperature.Valid | 1.001 | Futuro: autorização, procura calculada e validade |

Todos os grupos têm atribuição explícita; comando nunca partilha grupo com estado.
Pedidos Read/Response não são comandos. ACK KNXnet/IP confirma transporte, não
posição de válvula, contacto nem caudal. Um objeto de estado elétrico do atuador
pode igualmente NÃO provar abertura hidráulica. Não o promover a `Physical`
sem validar a sua semântica. Estado desconhecido não é publicado como OFF medido.
A fase e o motivo textual de falha ficam no diagnóstico Web; não foi inventado um
DPT normalizado para o enum interno.

Referências de tipos: [KNX Association, HVAC concept](https://support.knx.org/hc/en-us/articles/4502397598226-HVAC-concept)
e [MDT, manual KNX Modbus Gateway](https://www.mdt.fr/fileadmin/user_upload/user_upload/download/SCN-MBGRTU-01_MDT_TM_V10_EN.pdf).
A utilização destes tipos não comprova compatibilidade dos equipamentos instalados.

## Controlo manual e sequência

1. Arranque em `Unconfigured`. Tempos a zero impedem comissionamento acidental.
   Com bindings validados, `commission()` emite apenas intenção bomba OFF;
   comandos de válvulas permanecem inválidos até existir prova de paragem.
2. Confirmada paragem, fecha saídas e permanece desarmado. `arm()` exige saúde
   de transporte/interlock e confirmação fresca da bomba OFF. Nunca recupera ON
   da flash nem pedidos anteriores. Após falha, rearme é sempre explícito.
3. Web/KNX entregam pedidos ao mesmo árbitro. Um pedido contém sete booleanos e
   permissão da bomba. Permissão ON sem circuitos não liga a bomba. É permitido
   abrir circuitos com bomba inibida para operação manual.
4. Em `Opening`, abre os circuitos pedidos; espera tempo mínimo de abertura e
   confirmação física fresca de **todos os circuitos pedidos**. Só então pede ON
   à bomba. Não utiliza `delay()` nem bloqueia aquisição/OLED.
5. Em `Running`, supervisiona caminho aberto e confirmação da bomba. Qualquer
   alteração no conjunto de circuitos para primeiro a bomba. É conservador e
   evita transições entre circuitos sem ensaio hidráulico.
6. Em `Stopping`, mantém os circuitos abertos anteriores até confirmar paragem
   da bomba; depois altera/fecha válvulas. Novo pedido durante paragem não salta
   esta barreira. Abre o novo conjunto e volta a aplicar a sequência completa.
7. Interlock, transporte indisponível, timeout, feedback expirado, manutenção ou
   expiração do comando manual retiram permissão, apagam pedidos e bloqueiam.
   Pede OFF da bomba; conserva caminhos anteriores até prova de paragem. Se OFF
   não puder ser confirmado, não presume que a bomba parou para fechar válvulas.

**Prova de paragem** deve significar que já é seguro fechar o caminho hidráulico.
Um contacto OFF pode exigir tempo adicional de desaceleração no adaptador físico.
A implementação não define esse tempo, pós-circulação térmica ou bypass: dependem
da bomba/gerador. Se a instalação exige pós-circulação, essa política deve ser
integrada antes de ativar saídas. Proteções elétricas/hidráulicas e watchdog no
atuador têm de cobrir falha do ESP, energia, rede e contactos colados; software
num ESP sem comunicação não consegue garantir OFF num atuador remoto.

Não há fallback implícito de confirmação física para temporização. Uma instalação
sem feedback adequado precisará de uma política alternativa explicitamente
validada (por exemplo interlock autónomo do atuador e abertura temporizada). O
núcleo atual ficará bloqueado, deliberadamente, em vez de declarar confirmação.

## Prioridades e concorrência

Ordem: interlock/falha/manutenção > desarmado/paragem segura > manual > automático
futuro. Web e KNX têm a mesma prioridade. Processamento serial no loop; vence o
último pedido aceite. `manual()` exige revisão esperada: um pedido Web baseado em
estado antigo recebe Conflict (HTTP 409 na futura integração), sem alteração.
O adaptador KNX aplica o bit recebido ao snapshot corrente e submete a revisão
corrente no mesmo ciclo. Telegramas duplicados não são toggles: pedidos absolutos
0/1 são idempotentes nas saídas. Novo pedido aceite renova a lease manual.

Safety trip não depende de revisão de cliente. O motivo inicial de falha fica
retido até rearme. Libertar um interlock não rearma. Cada comando de saída tem
revisão própria; feedback de revisão antiga, comandado ou ACK é recusado. Os
objetos KNX não transportam esta revisão: o adaptador não pode simplesmente
carimbar qualquer telegrama atrasado com a revisão atual. Deve invalidar caches,
resincronizar após reconexão e estabelecer frescura/causalidade por leitura
controlada, sequência do transporte e/ou entrada física. Esta garantia continua
pendente da implementação do adaptador.

`openMs`, `transitionMs`, `feedbackMs` e `manualLeaseMs` têm de ser configurados
segundo equipamento e operação pretendida. São obrigatórios, positivos e inferiores
a 2^31 ms; transitionMs > openMs. Relógio monotónico de 32 bits, tick frequente,
sem saltos superiores a meia volta. Os valores curtos nos testes são só de teste.
Feedback deve ser renovado com periodicidade inferior a feedbackMs. Não se grava
estado ON persistente. Lease não é um watchdog físico e exige loop em execução.

## Integração futura ao runtime

- Introduzir serviço único proprietário dos oito canais e adapters Local/KNX.
  Consumir intenções válidas, descartar ON obsoleto, tratar falha/fila cheia como
  indisponibilidade e priorizar OFF de bomba. Nunca reproduzir fila ON após reconexão.
- Antes de ativar aquecimento, bloquear `/api/relay` e grupos diretos de qualquer
  canal reservado; `setRelay()` não pode contornar o sequenciador. Os restantes
  relés independentes podem conservar comportamento atual.
- Adaptador local traduz canal para GPIO/polaridade validados; adaptador remoto
  usa `TunnelClient::writeBool`. Entradas físicas são separadas de `onSent`.
- Expor pedido, intenção, feedback físico/validade, revisão, fase, origem e falha.
  Reutilizar autenticação/CSRF para Web. Guardar configuração apenas desarmado,
  validar a configuração completa e migrar com aquecimento desativado.
- Executar tick mesmo durante preparação de OTA/reinício. Recusar manutenção até
  paragem comprovada; não reutilizar política atual de manter relés ON na OTA.
- Testar adaptadores com transporte falso, filas saturadas, feedback atrasado e
  reconexão antes de qualquer ensaio isolado de hardware. Não ligar o serviço no
  loop antes dessas garantias estarem implementadas.

## Preparação para temperatura e setpoints

`ZoneInput` aceita origem BME280, DS18B20 ou KNX, valor/validade, timestamp,
setpoint/validade, histerese e idade máxima. `demand()` é uma função pura com banda
simétrica e OFF em dados inválidos/expirados. Ainda não comanda o controlador;
manual continua a ser a única origem de pedidos. Não há setpoints, gamas de
conforto ou tempos térmicos presumidos.

Na fase automática, mapear sensor por identidade estável (ROM DS18B20 ou grupo
KNX, não ordem de deteção), selecionar temperatura ambiente adequada e definir
limites de configuração. As duas sondas DS18B20 locais não são sete sensores.
Cada quarto pode partilhar temperatura/setpoint entre os seus dois circuitos;
manter enable/override individual para cada circuito. Zonas restantes são
independentes. Comandos manuais devem sobrepor-se à procura automática apenas
até lease/libertação explícita, sempre sujeitos às mesmas proteções. Perda de
sensor desativa procura dessa zona e gera diagnóstico; não invalida medições
ou procura das restantes zonas. Não ativar controlo automático neste patch.

## Verificação e dados necessários

`python scripts/test_host.py` compila C++14 com warnings como erros e UBSan,
executa sensores/relés, o novo controlador e regressão KNX. O workflow existente
já chama esse comando. Nenhum teste acede à rede ou ao equipamento.

Confirmar antes da integração física: nomes das três divisões, modelos/canais
livres e aplicação ETS dos atuadores, tipo/tempo/posição sem energia das válvulas,
controlo e prova de paragem da bomba, bypass/caudal mínimo, exigência de
pós-circulação, grupos KNX, GPIO e proteção elétrica se houver saídas locais.
Estas lacunas não impedem o desenvolvimento e testes do núcleo entregue.

### Resultado desta execução

- Testes nativos: passaram sensores/relés, controlador manual e KNX, com UBSan.
- Controlador: 256 combinações de circuitos/permissão, mudanças de circuitos,
  prioridade de falhas, lease, timeout, feedback antigo/não físico e wrap do relógio.
- KNX existente: 169 verificações em 11 cenários e 50 000 datagramas malformados.
- Compilação ESP32 não concluída neste ambiente: ficou na obtenção
  das bibliotecas do projeto. Não é declarado sucesso de compilação ou de ensaio físico.
- BME280, DS18B20, OLED, configuração e runtime existentes não foram alterados.
  Compatibilidade de execução no equipamento continua por verificar.

## Confirmação do utilizador após a análise

O utilizador confirmou que possui atuadores KNX e pretende criar divisões e
associar-lhes os canais dos coletores. A integração KNX passa a ser a opção
preferencial, sujeita à confirmação dos modelos, canais e feedback disponíveis.
A interface futura deverá permitir nomes de divisões e associação de um ou vários
canais por divisão. O mapeamento `ZoneForCircuit` deste núcleo continua a ser uma
proposta fixa para testes; criação dinâmica de divisões, edição das associações,
persistência e interface Web ainda não estão implementadas. Esta confirmação não
atribui grupos KNX, GPIO nem ligações físicas.

## Interface do coletor e associação de divisões

O painel Web mostra as oito posições físicas observadas na fotografia do coletor,
na ordem da esquerda para a direita, e reserva uma delas para ficar sem circuito.
Os sete circuitos lógicos podem ser associados a posições distintas e a uma das
cinco divisões. O mapa é guardado numa chave NVS separada (`heating/layout`),
sem alterar o schema 2 nem as configurações existentes do BME280, Wi-Fi ou KNX.

O painel principal contém a representação dos caudalímetros, retorno, bomba e
circuitos agrupados por divisão. A vista **Configurações** contém a edição dos
nomes e associações, sensores, KNX, relés e OTA. As associações são validadas
antes da gravação: posições 1–8, sem repetição entre os sete circuitos;
`Por atribuir` é permitido. A oitava posição pode continuar livre.

Os indicadores de bomba e circuitos mostram **estado desconhecido** e os botões
de aquecimento estão inativos nesta fase. A fotografia mostra tampas brancas nas
válvulas, sem atuadores elétricos visíveis. Uma posição mapeada não comprova que
tenha atuador, feedback ou alimentação. O novo endpoint de configuração nunca
envia telegramas nem modifica GPIO. O controlador portátil de `HeatingCore.h`
continua desacoplado do runtime até serem definidos e verificados os canais do
atuador KNX e a política de confirmação de abertura/paragem.

Para a fase de controlo físico: identificar a posição não usada; relacionar as
outras sete com os circuitos reais; fornecer os canais/grupos KNX de comando e
estado da bomba e de cada válvula, o tipo NC/NO e os tempos reais; confirmar como
a bomba recebe alimentação e se existe confirmação física ou interlock. Só então
ligar os botões do painel ao sequenciador com a bomba OFF antes de fechar válvulas.
