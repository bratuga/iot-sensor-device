# Aquecimento por divisões e coletor KNX

## Configurações 0.3.2

As configurações têm quatro separadores: **Funcionalidades** (escolha do modo,
divisões e associações, sensores, relés e entradas), **KNX** (apenas os canais
selecionados, grupos de receção e envio), **Hardware** (GPIO, I²C, ROM, OLED)
e **Configuração** (rede, nome, diagnóstico, firmware, backup e terminal).
Guardar as configurações gerais reinicia o ESP; o mapa do coletor tem um botão
de gravação separado. Os campos ocultos ao trocar de separador mantêm o que foi
digitado até guardar ou sair.

Em **Funcionalidades → Controlo de piso radiante**, a bomba tem «Tempo de
serviço» (900000 ms = 15 minutos) e «Pausa de serviço» (300000 ms = 5 minutos).
Ambos aceitam 1 minuto a 24 horas. Configurações anteriores recebem os valores
predefinidos ao serem carregadas, sem apagar o mapa KNX.

O modo **Termóstato de divisão** é exclusivo do controlo do coletor. Exige
BME280 ativo, KNX ligado e publicação da temperatura BME no canal 1 DPT 9.001.
Configure também um grupo DPT 9.001 para o setpoint, e dois grupos DPT 1.001
distintos para o pedido e respetivo feedback. O mosaico principal mostra a
temperatura BME e permite ligar/desligar e ajustar o setpoint em passos de
0,5 °C; só envia o novo valor KNX 5 segundos após o último toque. O setpoint
e o estado ligado/desligado são persistidos na NVS. O pedido de calor liga
abaixo de setpoint − margem, desliga acima de setpoint + margem e conserva o
estado entre esses limiares. Falha ou expiração da leitura local desliga o
pedido; o mosaico só considera a ação concluída depois do feedback KNX.
Não há comando de bomba, válvulas ou divisões enquanto este modo está ativo.
O mapa do coletor continua guardado para regressar mais tarde. Desliga primeiro
o termóstato e aguarda feedback OFF antes de regressar ao coletor.

O terminal em Configuração mostra os 48 telegramas mais recentes guardados em
RAM, atualizados pela página sem voltar a ler grupos no barramento. Permite
GroupValueRead e escrita DPT 1.001/9.xxx apenas a uma sessão de administração.
Uma resposta «na fila» ainda não confirma a entrega no barramento.
O backup exporta a configuração e o mapa sem passwords; no restauro conservam-se
as passwords que estão atualmente na configuração. O restauro exige que o aquecimento
esteja parado e reinicia o dispositivo.

## Interface e configuração

O painel público apresenta quatro indicadores: temperatura BME, humidade, pressão
e hora local do navegador. As duas sondas DS18B20 aparecem apenas na entrada e
no retorno do coletor. O título reflete o nome configurado e a identificação do
dispositivo aparece no rodapé. Os mosaicos das divisões mostram o ambiente, o
setpoint, o pedido, ligar/desligar e o modo automático. O anel fica vermelho com
pedido de calor, cinzento desligado e verde em espera. O coletor tem oito posições, luzes de estado para a bomba e
cada cabeça, temperatura de ida (DS18B20 1) e retorno (DS18B20 2). Configure as
ROM das sondas nessa ordem. Sem uma leitura válida aparece «Sem leitura».
O coletor usa as oito colunas disponíveis sem largura mínima nem deslocamento
horizontal; rotor, indicadores e contagem de circuitos mostram exclusivamente o
feedback KNX, sem temporizadores simulados. Os seus ícones não comandam a bomba
nem uma válvula isolada: usam-se os comandos seguros do painel/divisões.
Os relés e a secção de saídas aparecem apenas quando há relés configurados;
o diagnóstico fica abaixo e pode ser ocultado nas configurações.
Verde significa feedback KNX ligado; apagado, desligado; amarelo, divergência
entre comando e feedback; cinzento, estado desconhecido. Não é medição de caudal
nem confirmação física da abertura da válvula.

As configurações estão numa vista separada, aberta pelo ícone de definições.
O login integrado usa `admin` e a palavra-passe de administração conservada no
JSON de configuração na partição SPIFFS. Durante estes testes, uma compilação local que inclua
`LOCAL_AP_PASSWORD` em `secrets.h` também aceita essa palavra-passe como alternativa;
não altera a configuração persistida. `SETUP` na consola USB mostra ambas as
palavras-passe atuais para recuperar o acesso sem apagar as configurações. Guarda apenas
um token de sessão na aba e expira após 30 minutos sem utilização. O painel e
os seus comandos são públicos nesta rede de testes; as alterações de configuração
e a atualização OTA exigem sessão de administração. Guardar regressa ao painel.
Em cada divisão pode:

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
- No modo manual, o botão liga/desliga mantém o pedido até nova alteração ou paragem do controlo; em automático o botão fica oculto.

Um comando KNX de ativação atualiza a divisão; no modo manual, altera o pedido manual sem mudar o modo. Uma resposta a uma leitura atualiza a habilitação.
Temperatura inválida ou expirada cancela o pedido automático. O tempo máximo de
validade é configurável, inicialmente cinco minutos. Setpoint permitido: 5–35 °C.

Para arrancar: comandar as válvulas pedidas, receber feedback elétrico ON e
aguardar o tempo de abertura configurado (inicialmente 120 segundos). Só depois
comandar a bomba. Quando outra divisão pede calor enquanto a bomba já trabalha
com um percurso confirmado, abre-se o circuito adicional sem desligar a bomba.
Um circuito só fecha com a bomba ligada se outro percurso continuar confirmado
e tiver cumprido o tempo de abertura. Ao desligar o último circuito, desliga-se
primeiro a bomba e aguarda-se o feedback OFF antes de fechar a válvula.
Perda de KNX, falta de feedback ou erro de envio
bloqueiam o controlo e pedem OFF da bomba; os percursos existentes são mantidos
até receber feedback OFF. Uma rede indisponível não permite garantir a execução
do comando remoto: configure também o comportamento de falha nos atuadores.

Guardar associações, alterar a configuração principal, reiniciar pela consola ou
atualizar por OTA requer aquecimento parado e feedback OFF dos canais configurados.
Usar «Parar aquecimento» e aguardar antes dessas operações.

## Persistência e implementação

A configuração do aquecimento usa `heating/layout` em NVS, separada do schema 2
principal. As temperaturas recebidas e os setpoints enviados são guardados em
`heating/values`, associados ao endereço KNX de cada divisão, e recuperados após
reinício. Os botões +/− alteram o setpoint em 0,5 °C (5–30 °C). O valor aparece
logo no mosaico, mas cada divisão espera cinco segundos sem novos toques antes
de enviar um único DPT 9.001 ao seu grupo KNX. Ao colocar o envio em fila,
persiste o valor em NVS e mostra o estado real do túnel KNX. Não modifica os
pinos, endereço ou driver do BME280.

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

A opção BME local é individual e desligada por defeito. Sem temperatura KNX válida dentro da validade configurada, usa uma amostra local válida. Um novo telegrama válido recupera prioridade imediatamente. O BME mede o ambiente junto ao ESP, não cada divisão. Sem sensor válido não há procura automática. Sem setpoint recebido durante 300 segundos usa 20 °C; a temperatura de recurso de 20 °C é apenas apresentada, não habilita aquecimento.

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
confirmação e abertura dos circuitos pedidos. Ao adicionar um circuito, a bomba
permanece ligada enquanto já houver outro percurso confirmado. Se outro circuito
permanecer confirmado e tiver cumprido o tempo de abertura, o anterior fecha
sem interromper a bomba; caso contrário mantém-se aberto até haver substituto
pronto, ou desliga-se a bomba antes de fechar o último. A bomba funciona em
ciclos configuráveis de 15 minutos de serviço e 5 minutos de pausa por defeito:
durante a pausa as válvulas pedidas ficam abertas, e o tempo começa após feedback
OFF da bomba. Um novo pedido não salta a pausa. A interface apresenta uma barra
vermelha durante a espera de abertura, verde durante o serviço e laranja na pausa.
Cada válvula mostra «A abrir» e o tempo restante após feedback ON.
Perda de ligação exige nova confirmação e reativação manual. Um ON externo da
bomba sem qualquer percurso pronto provoca um comando OFF e uma falha explícita.
Um feedback espontâneo de circuito atualiza o estado observado e pode iniciar o
tempo de abertura; um feedback espontâneo da bomba atualiza o estado observado,
respeitando a regra de percurso confirmado. O feedback do grupo da divisão
recebido de outro dispositivo não altera o pedido interno; apenas o endereço
de comando da divisão altera o seu estado e provoca a publicação do feedback.

As falhas identificam a saída e os grupos KNX (ou perda de ligação/caminho).
O desenho da bomba mostra apenas o estado da bomba; a falha do controlo aparece
no diagnóstico separado. Não há polling periódico: mantêm-se as leituras de
arranque/reconexão/configuração, o botão **Reler valores KNX** e no máximo uma
leitura de feedback por comando sem confirmação. Um setpoint válido guardado
não é substituído por 20 °C após cinco minutos.

### Validação reproduzível

Executar `python scripts/test_host.py`, `node scripts/test_web_ui_minimal.cjs` e
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
  modo manual sem limite, restauro do setpoint e overflow de `millis()`.
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

A compilação ESP32 e o teste DOM completo correm também no workflow
`.github/workflows/build.yml` em cada publicação. O teste sem dependências
cobre o acesso público, o pedido de setpoint de 0,5 °C e o login integrado.
