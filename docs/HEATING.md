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
