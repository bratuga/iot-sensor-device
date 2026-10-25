# Operação e recuperação

## Acesso web

Utilizador `admin`; password gerada no primeiro arranque e conservada em NVS.
O comando `SETUP` na consola USB mostra a password do AP e a password Web
guardada no JSON da NVS, e ativa o AP protegido. Durante os testes, compilações
locais com `LOCAL_AP_PASSWORD` também aceitam essa password no login Web como
alternativa, sem alterar a configuração guardada. Compilações obtidas do CI sem
`secrets.h` continuam a usar apenas a password Web guardada. Não existe password padrão igual em todos
os dispositivos. Depois de ligado à LAN, consultar o IP na consola ou tentar
`http://<nome-do-dispositivo>.local` num cliente com mDNS.

Se a rede não estiver configurada, o AP começa imediatamente. Se não houver
ligação Wi-Fi após 60 s desde o arranque, o AP é aberto para recuperação. Mantém-se
ativo até ao próximo arranque; reiniciar após corrigir a rede para o fechar quando
o dispositivo voltar a conseguir ligar-se à LAN. O comando `SETUP` exige acesso
físico à consola; não está exposto pela API web.

Passwords não são devolvidas por `/api/config` nem incluídas na exportação.
São guardadas na NVS; esta versão não ativa flash encryption nem secure boot.
Não expor a interface HTTP à Internet. Todos os pedidos de alteração exigem
autenticação e o token CSRF obtido após autenticação.

## Diagnóstico USB (115200 baud, terminar com newline)

| Comando | Efeito |
| --- | --- |
| `HELP` | Lista comandos |
| `STATUS` | Mostra IP e estado KNX |
| `SETUP` | Abre AP e mostra as passwords do AP e do Web ao operador local |
| `READ 2/0/1` | Envia GroupValueRead |
| `WRITE 2/1/1 1` | Envia booleano DPT1 |
| `FLOAT 2/0/1 21.5` | Envia valor DPT9 |
| `REBOOT` | Desliga o túnel e reinicia |

READ/WRITE/FLOAT atuam no barramento real. Usar só grupos de teste autorizados.
Os comandos não são executados automaticamente. Um retorno `Queued` confirma
apenas a entrada na fila; os contadores permitem observar confirmações do túnel.
Cada telegrama de grupo recebido aparece na consola. Os últimos 12 são guardados
em RAM e mostrados na web; o OLED alterna medições com o último telegrama.
Não existe histórico persistente de medições nesta versão.

## Reconexão

Ao perder o Wi-Fi ou mudar de IP, a aplicação termina o socket e espera 130 s
antes de tentar um novo túnel, para não acumular sessões antigas. O intervalo
deve ser revisto se o servidor usar outro timeout. As filas antigas não são
reproduzidas: após reconexão publicam-se novas leituras válidas.

Os sensores são lidos mesmo sem KNX. O envio é considerado confirmado apenas
quando chega o ACK do túnel. Isto não comprova entrega física no TP nem ação de
um atuador. Os valores permanecem elegíveis para nova tentativa após falha,
respeitando o intervalo mínimo.

## Atualizações OTA

Primeira instalação por USB. Depois usar a secção Atualizar firmware da web,
com `firmware.bin` do perfil `esp32` e a alimentação estável. A imagem vai para
a partição OTA inativa; é validada pela biblioteca Update antes de mudar a
partição de arranque. Há duas partições de aplicação de 1,875 MiB em flash de 4 MiB.

Durante o upload, publicações e leituras são suspensas. Se o upload falhar,
a aplicação continua e restabelece o KNX após o período de recuperação. Uma
imagem que arranque mas tenha um erro lógico não provoca rollback automático.
Recuperar por USB nesse caso. Esta versão não verifica assinaturas de firmware;
um administrador autenticado é responsável por escolher a imagem correta.

Não guardar ficheiros compilados, caches ou passwords no repositório. O workflow
Actions guarda a imagem como artefacto de compilação separado do histórico Git.

## Referências de implementação

- https://docs.platformio.org/en/latest/platforms/espressif32.html
- https://github.com/espressif/arduino-esp32/tree/2.0.17/libraries/Update
- https://github.com/milesburton/Arduino-Temperature-Control-Library
- https://github.com/adafruit/Adafruit_BME280_Library
- https://github.com/adafruit/Adafruit_SSD1306
