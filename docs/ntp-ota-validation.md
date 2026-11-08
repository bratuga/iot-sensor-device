# NTP e validação OTA

Implementação na branch `feature/ntp-ota-validation`, a partir do main atual em 08.11.2026. Sem instalação nos equipamentos nem merge nesta entrega.

## Hora

Painel com servidor principal, alternativo, fuso horário e estado do relógio/armazenamento. Os valores anteriores são preservados; o campo alternativo novo usa time.cloudflare.com. Europe/Lisbon e UTC têm nomes legíveis, e um fuso personalizado existente é preservado.

«Sincronizar agora» utiliza a configuração já guardada. Campos alterados impedem o pedido até guardar. A API é autenticada; sem Wi-Fi ou com OTA/sincronização em curso, recusa o pedido. A resposta inicial é apenas «pendente». O sucesso exige callback SNTP; relógio já válido não é suficiente. Timeout de 30 s, sem bloquear o loop de controlo. NTP pode acertar a hora do sistema; sem alteração às regras de controlo/horários.

## Firmware ESP

Os novos firmware.bin continuam válidos para USB. Incluem um descritor próprio no offset 288 com projeto, perfil de hardware e versão, segundo o [formato de aplicação ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/api-reference/system/app_image_format.html#adding-a-custom-structure-to-an-application). O build falha se o descritor faltar ou ficar no local errado.

O browser apresenta a versão e rejeita imagens incompatíveis. O dispositivo repete a validação, com buffer de 392 bytes, antes de iniciar a escrita na partição inativa. Só finaliza/ativa após receção completa e conclusão HTTP autenticada. SDK valida a imagem ao finalizar. Uploads abortados, maiores/menores que o tamanho declarado, múltiplos ficheiros e falhas de escrita não ativam firmware.

Não depende do nome do ficheiro. Firmwares antigos sem descritor são recusados após instalar este validador. A primeira instalação desta versão é feita pelo procedimento anterior (ou USB), verificando manualmente o destino. Esta verificação previne trocas acidentais e deteta corrupção; não é assinatura criptográfica, Secure Boot ou política anti-downgrade. Uma versão anterior com identidade válida continua permitida.

## Validação

Testes de fragmentação do cabeçalho, alvo/processador errado, ausência de identidade, truncagem, excesso de bytes, cancelamento, erro de escrita e recusa na finalização. Testes de estado NTP com Wi-Fi indisponível, relógio previamente válido, callback real simulado, timeout e rollover de millis. Testes existentes de controlo e interface mantidos. Ensaios de rede/OTA em hardware permanecem pendentes.

Sensor: preserva main f216204 (arranque seguro do aquecimento, dashboard e histórico). Por decisão explícita do proprietário em 08.11.2026, mantém-se a condição OTA existente em modo termóstato: não foi acrescentada exigência de OFF confirmado. A regulação pode ficar suspensa durante o upload; este comportamento pré-existente não foi alterado.

Verificação local: compilação do firmware e descritor passou; os três binários reais foram cruzados com os validadores web (cada projeto aceita apenas o seu). Painéis NTP revistos no navegador com dados simulados, incluindo apresentação móvel e alterações por guardar. Isto não constitui validação física de NTP/OTA.
