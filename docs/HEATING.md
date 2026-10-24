# Aquecimento por divisões e coletor KNX

## Interface e configuração

O painel apresenta mosaicos das divisões configuradas, com botão de pedido
vermelho (ligado) ou cinzento (desligado), e o coletor com oito posições, luzes de estado para a bomba e
cada cabeça, temperatura de ida (DS18B20 1) e retorno (DS18B20 2). Configure as
ROM das sondas nessa ordem. Sem uma leitura válida aparece «Sem leitura».
Verde significa feedback KNX ligado; apagado, desligado; amarelo, divergência
entre comando e feedback; cinzento, estado desconhecido. Não é medição de caudal
nem confirmação física da abertura da válvula.

As configurações estão numa vista separada. Em cada divisão pode:

- editar o nome e os quatro grupos KNX;
- associar um ou vários circuitos livres;
- retirar circuitos, que ficam disponíveis para outra divisão;
- remover a divisão, libertando os seus circuitos.

O limite é oito circuitos, independentemente da distribuição por divisões.
Esta versão reserva espaço para até 16 divisões, incluindo divisões vazias.
Cada circuito pertence no máximo a uma divisão. As posições físicas são únicas;
«Por atribuir» não representa uma ligação física. Os circuitos sem divisão não
recebem pedidos de calor. Os mapas antigos de sete circuitos e cinco divisões
são carregados sem perder os nomes e associações; o oitavo fica livre.

## Grupos KNX

Cada circuito associado e a bomba precisam de comando e feedback DPT 1.001.
ON deve significar abertura/ativação no atuador KNX; configure a polaridade
NC/NO no próprio atuador. A bomba é um canal separado, além dos oito circuitos.
Cada divisão tem pedido/ativação e respetivo feedback DPT 1.001, temperatura e
setpoint DPT 9.001. Os grupos são validados, sem duplicação nem colisão com os
sensores e os quatro relés independentes. Não existem endereços predefinidos.
O feedback da divisão representa a habilitação do pedido, não uma válvula aberta.
Os objetos de estado e temperatura devem responder a GroupValueRead.

## Funcionamento

Depois de configurar, usar «Ativar controlo». O arranque e a recuperação de uma
falha nunca restauram pedidos anteriores. A bomba recebe OFF primeiro; o controlo
só pode ser ativado depois do feedback OFF. Escolher em cada divisão:

- Automático: temperatura e setpoint KNX, histerese total de 0,5 °C.
- Desligado: sem pedido de calor.
- Botão de ligar/desligar: pedido manual temporário de até 30 minutos, sujeito à mesma sequência da bomba.

Um comando KNX de ativação coloca a divisão em automático. Uma resposta a uma
leitura atualiza a habilitação sem substituir o modo manual escolhido na Web.
Temperatura inválida ou expirada cancela o pedido automático. O tempo máximo de
validade é configurável, inicialmente cinco minutos. Setpoint permitido: 5–35 °C.

Para arrancar: comandar as válvulas pedidas, receber feedback elétrico ON e
aguardar o tempo de abertura configurado (inicialmente 120 segundos). Só depois
comandar a bomba. Mudanças de circuitos param primeiro a bomba e aguardam feedback
OFF antes de mudar as válvulas. Perda de KNX, falta de feedback ou erro de envio
bloqueiam o controlo e pedem OFF da bomba; os percursos existentes são mantidos
até receber feedback OFF. Uma rede indisponível não permite garantir a execução
do comando remoto: configure também o comportamento de falha nos atuadores.

Guardar associações, alterar a configuração principal, reiniciar pela consola ou
atualizar por OTA requer aquecimento parado e feedback OFF dos canais configurados.
Usar «Parar aquecimento» e aguardar antes dessas operações.

## Persistência e implementação

A configuração do aquecimento usa `heating/layout` em NVS, separada do schema 2
principal. Não modifica os pinos, endereço ou driver do BME280 nem as credenciais.
A autenticação Web continua desativada conforme autorizado para os testes locais;
as mutações mantêm o token de sessão.

`HeatingRuntime.h` contém o sequenciador usado pelo firmware; `HeatingCore.h`
conserva o protótipo isolado anterior e o cálculo de histerese. Os testes de host
cobrem 255 combinações de pedidos, espera de abertura, ordem de paragem, falta de
feedback, reconexão, temporizadores e validação do mapa. Executar:

```sh
python scripts/test_host.py
pio run -e esp32
```

A validação em host não substitui o ensaio com os atuadores e as sondas reais.

A verificação DOM da interface está em `scripts/test_web_ui.cjs` (Node e jsdom
30.1.3). Usa `NODE_PATH` para indicar uma instalação externa de jsdom, sem adicionar
dependências ao firmware. Verifica adicionar/remover divisões, associar os oito
circuitos, preservar edições durante atualização de estados e cores dos mosaicos.

## Valores recebidos e diagnóstico após reiniciar

Os setpoints válidos e as últimas temperaturas recebidas são guardados em
`heating/values`, associados aos grupos KNX (não à posição da divisão na lista).
A configuração de grupos continua em `heating/layout`. Ao reiniciar, o setpoint
é recuperado e a última temperatura aparece identificada como leitura guardada.
Essa temperatura não é considerada atual para o termóstato: uma nova leitura KNX
é necessária. Estados ON de bomba/válvulas e pedidos manuais não são restaurados.

Alterações de setpoint e a primeira temperatura são gravadas no próximo ciclo;
as alterações seguintes de temperatura são agrupadas em intervalos de até 60 s
para reduzir escritas na flash. Reinício pelo firmware, guardar associações e OTA
forçam a gravação pendente. Um corte abrupto de alimentação pode perder a última
fração desse intervalo. Falhas de gravação são registadas na consola e expostas
em `valuesSaved` na API. Valores não recebidos ainda por esta versão não podem ser
recuperados retroativamente de reinícios anteriores.

O feedback do atuador é aceite antes ou depois do ACK do túnel; a confirmação do
estado requer ambos. O ACK não substitui o feedback. O painel mostra o grupo da
bomba que aguarda OFF, apresenta temperatura e setpoint independentemente e permite
«Reler valores KNX». A leitura da bomba é priorizada no arranque/reconexão.

## Tráfego KNX por eventos

Não existe varrimento contínuo dos grupos. Arranque, reconexão, guardar o mapa e
«Reler valores KNX» iniciam uma única passagem limitada, espaçada em 500 ms;
circuitos sem associação são ignorados. A passagem termina mesmo que um objeto
não responda. Depois, os telegramas Write/Response recebidos atualizam a memória.
Após um comando pode ser feita uma única leitura do feedback desse atuador se
ainda não chegou o estado esperado. Não se repete a leitura de todos os valores.

O estado elétrico confirmado é conservado durante a ligação, até novo feedback,
novo comando ou perda de KNX. Não expira artificialmente para exigir polling.
As temperaturas mantêm a validade configurada; o sensor de origem deve publicar
as suas medições. Uma temperatura expirada cancela o pedido automático sem
desencadear varrimentos do barramento.


## 0.2.2 — controlo por divisão e alternativa BME

Cada divisão permite escolher termostato KNX externo (segue ON/OFF) ou automático ESP (temperatura/setpoint com margem ± configurável, padrão 0,3 °C). O ON/OFF habilita a divisão no automático. O comando e o feedback da divisão respondem a GroupValueRead; mudanças pelo KNX ou painel publicam o feedback. O último pedido é guardado, mas as saídas arrancam desarmadas.

A opção BME local é individual e desligada por defeito. Após 300 segundos desde o arranque, sem temperatura KNX válida dentro da validade configurada, usa uma amostra local válida. Um novo telegrama válido recupera prioridade imediatamente. O BME mede o ambiente junto ao ESP, não cada divisão. Sem sensor válido não há procura automática. Sem setpoint recebido durante 300 segundos usa 20 °C; a temperatura de recurso de 20 °C é apenas apresentada, não habilita aquecimento.

No arranque envia OFF à bomba e, após confirmação, aos circuitos configurados (incluindo sem divisão). Só o atuador publica o próprio feedback. Ausência de confirmação é indicada após 30 segundos; não se fabrica um OFF confirmado. Ativar controlo preserva o pedido das divisões. Leituras gerais ocorrem apenas no arranque/reconexão ou botão Reler valores KNX.

## 0.2.3 — apenas circuitos utilizados e diagnóstico de feedback

Um circuito é utilizado quando está associado a uma divisão (`zone >= 0`). Um
circuito sem divisão pode manter os endereços KNX gravados, mas não recebe
comandos, não é lido e não participa na confirmação nem nos timeouts. A bomba
participa sempre quando o controlo do aquecimento está habilitado. Esta regra
substitui o comportamento de arranque descrito para 0.2.2.

No arranque/reconexão, envia-se OFF à bomba; só após confirmação se envia OFF aos
circuitos utilizados. O botão **Ativar controlo** fica disponível quando essas
saídas confirmam OFF. Um comando só fica confirmado com ACK de transporte e
feedback do atuador correspondente. O ACK sozinho não confirma a saída.

Após pedido de calor, o tempo de abertura começa no primeiro feedback ON da
válvula; telegramas ON repetidos não reiniciam esse tempo. A bomba liga após a
confirmação e abertura dos circuitos pedidos. Na paragem ou mudança de divisão,
espera-se o feedback OFF da bomba antes de fechar os circuitos. A bomba só volta
a ligar quando todas as transições dos circuitos utilizados estão confirmadas. Perda de ligação
exige nova confirmação e reativação manual. Um ON externo da bomba quando está
pedido OFF provoca um comando OFF e uma falha explícita.

As falhas identificam a saída e os grupos KNX (ou perda de ligação/caminho).
O desenho da bomba mostra apenas o estado da bomba; a falha do controlo aparece
no diagnóstico separado. Não há polling periódico: mantêm-se as leituras de
arranque/reconexão/configuração, o botão **Reler valores KNX** e no máximo uma
leitura de feedback por comando sem confirmação. Um setpoint válido guardado
não é substituído por 20 °C após cinco minutos.

### Validação reproduzível

Executar `python scripts/test_host.py` e
`NODE_PATH=<pasta com node_modules> node scripts/test_web_ui.cjs` (jsdom instalado).

- Simulação com o mesmo `Runtime` e seletor de comandos usados no ESP: todas as
  6.561 combinações de circuitos sem utilização, utilizados sem pedido e com pedido.
- Arranque, ciclo de aquecimento, paragem e mudança de divisão; verificação de
  que a bomba nunca liga sem caminho confirmado e tempo de abertura cumprido,
  e de que não se fecham válvulas enquanto a bomba simulada continua ON.
- 32 sequências determinísticas de mudanças rápidas de pedido e atrasos variáveis,
  com ordem de telegramas preservada por saída do atuador.
- ACK antes/depois do feedback, atrasos de 27 s, feedback repetido, ausente ou
  contrário, ACK ausente, falha de transporte, fila cheia, reconexão e ON externo.
- Temperaturas/setpoints, histerese, validade, alternativa BME, modo externo,
  modo manual de 30 minutos, restauro do setpoint e overflow de `millis()`.
- Regressões KNX (50.000 datagramas), leituras finitas e painel DOM, incluindo
  diagnóstico separado da bomba e posições sem utilização.

`test/fixtures/knx_v4_feedback.txt` contém os feedbacks do atuador 1.1.20 do
último arranque no registo telegramas_v4.xml, de 18:54:22.551536 UTC até antes do
OFF indevido de 18:54:52.763334 UTC. Colunas: milissegundos desde o primeiro
comando OFF à bomba, índice da saída (0–7 circuitos, 8 bomba), valor booleano.
A simulação reproduz esse prefixo, ativa o controlo aos 22 s e pede WC/C4 aos
28,04 s; depois prolonga o ciclo com um atuador simulado. Não reproduz os
feedbacks OFF causados pelo erro antigo como se fossem eventos independentes.

Isto é validação de software com dados capturados e atuadores simulados; não é
um ensaio físico do ESP, da bomba, do caudal ou dos tempos reais das cabeças.

Validação desta revisão: suites nativas com UndefinedBehaviorSanitizer e avisos
como erros, simulação, regressões KNX e DOM passaram. A compilação
`python -m platformio run -e esp32` passou com as dependências fixadas no projeto:
52.444 bytes RAM (16,0%) e 976.821 bytes flash (49,7%). A biblioteca OneWire
2.3.8 emite dois avisos de pré-processamento; não houve erros de compilação.
