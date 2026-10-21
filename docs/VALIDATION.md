# Validação da aplicação de sensores

## Escopo

O módulo KNX existente tem confirmação de utilização física pelo proprietário
em dois projetos ESP32. Essa confirmação não é apresentada como um ensaio da
nova aplicação de sensores, configuração web e OTA.

## Verificações locais — 21/10/2026

- Testes nativos da conversão DPT9: vetores positivos/negativos, alteração de
  expoente, arredondamento, pressão e rejeição de NaN/infinito/valores fora da gama.
- Validação de grupos compatíveis com o módulo e ROM DS18B20 com CRC.
- Publicação inicial, limiar de variação, período, intervalo mínimo, ACK positivo,
  falha, reconexão e contagem de tempo com overflow.
- Regressão do tunneling: 169 verificações, 11 grupos de cenários e 50 000
  datagramas malformados/aleatórios. UBSan ativo; testes passaram em Linux.
  As proteções de páginas de memória específicas de Windows não foram executadas.
- Sintaxe do JavaScript da página web verificada com `node --check`.

- Compilação completa `pio run -e esp32`: **PASSOU** com PlatformIO 6.1.18,
  espressif32 6.9.0, Arduino ESP32 2.0.17 e toolchain Xtensa 8.4.0.
- RAM estática: 49 828 / 327 680 bytes (15,2%); flash da aplicação:
  938 017 / 1 966 080 bytes (47,7%). RAM dinâmica em execução não foi medida.
- Foram emitidos dois avisos de pré-processador na dependência OneWire 2.3.8,
  relativos a diretivas `#undef`; não houve erros de compilação.
- As versões diretas e transitivas das bibliotecas estão fixadas em `platformio.ini`.

O binário foi gerado localmente para verificar a compilação; não foi gravado
num dispositivo nem incluído no histórico Git. O workflow preparado no GitHub
reproduz os testes e a compilação, mas o seu resultado é independente deste
ensaio local.

## Verificações na placa antes de uso contínuo

1. Confirmar revisão, flash, pinout e modelo OLED; primeiro carregamento por USB.
2. Confirmar acesso AP, gravação de configuração, reinício e recuperação Wi-Fi.
3. Verificar BME280 e identificar cada sonda por ROM; desligar uma sonda e
   confirmar que não é publicado um valor falso nem trocada pela outra.
4. Confirmar DPT e unidade de cada grupo no ETS/monitor KNX e Home Assistant.
5. Observar publicação periódica, por variação, respostas de leitura e limite
   agregado de telegramas.
6. Interromper Wi-Fi/knxd e verificar reconexão sem acumular túneis.
7. Ensaiar OTA com uma imagem correta e confirmar a recuperação por USB.
8. Observar funcionamento prolongado com sensores e rede reais.

Não foram feitos envios para a instalação KNX, gravações numa placa ou uploads
OTA reais durante o desenvolvimento remoto. GPIO continuam desativados por
omissão até confirmação da placa. Não é um produto KNX certificado.
