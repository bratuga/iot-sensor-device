# Orbit no firmware do sensor KNX

A versão `0.3.20-orbit` integra na `main` o painel Orbit, o controlo do coletor
e as alterações validadas na branch de integração. Mantém o sequenciamento KNX,
as proteções térmicas e a gravação da histerese a uma casa decimal.

- Painel de coletor ou termóstato, Sensores, Histórico e Ajuda são acessíveis
  na rede local. Sensores mostra as cinco leituras locais, incluindo ida e retorno.
- A página `/config.html` tem cinco separadores: Técnico, Diagnóstico, Hardware,
  KNX e Configurações. Os endpoints de administração exigem sessão, e os
  comandos protegidos exigem também o token da página.
- Técnico inclui pesquisa de Wi-Fi, DHCP ou IPv4 fixo, NTP e fuso, backup,
  restauro, OTA, palavra-passe e reinício. Os detalhes do diagnóstico e os
  telegramas KNX exigem autenticação. Os logs são mantidos em RAM.
- Mantém as opções de recuperação Wi-Fi e as credenciais já gravadas.
- No painel, o interruptor indica o estado do controlo e o botão Atualizar
  relê o KNX. A pressão é apresentada em kPa. Temperatura e humidade mostram
  tendência quando há 30 minutos de leituras válidas desde o arranque.
- O histórico mantém 48 horas, em gráficos separados e sincronizados de
  temperatura e humidade, com cerca de 2 horas visíveis de cada vez.
- A intenção de controlo é guardada em NVS. Ao reiniciar, o coletor só retoma
  depois de confirmar OFF da bomba e das válvulas. Uma falha de segurança
  cancela a retoma. A confirmação ON da bomba é registada nos logs.

## Uniformização do painel

No telemóvel, a barra inferior apresenta Painel, Sensores, Histórico e
Configuração. O comando geral de 76 px e o tema ficam no topo. A resposta ao
comando aparece junto ao círculo, incluindo recusas, paragem pendente e
ausência de confirmação. A ajuda continua pública no rodapé, junto da versão
e da data de compilação fornecidas pelo dispositivo.

Na configuração, os separadores têm ligações diretas em
`/config.html#tecnico`, `#diagnostico`, `#hardware`, `#knx` e
`#configuracoes`. As setas, Home e End mudam de separador; o formulário
mantém as edições ao alternar. As mesmas operações de administração continuam
a exigir sessão. NTP mantém o servidor e o fuso configuráveis e sincroniza
no arranque; o OTA mantém o carregamento local do ficheiro de firmware.

O OLED SSD1306 opcional (128 × 32 ou 128 × 64) conserva a leitura em foco e
as mensagens de AP e KNX. O novo layout refere-se ao painel web.

## Validação

A compilação `pio run -e esp32` passou: 59,3% da partição de aplicação
e 19,9% da RAM estática. Os testes abaixo passaram nesta integração.

`python3 scripts/test_host.py` verifica o controlo, as proteções, os sensores,
a persistência e o túnel KNX. Os testes existentes da interface são
`node scripts/test_web_ui_minimal.cjs` e `node scripts/test_web_ui.cjs`
(o segundo usa jsdom).

O teste `scripts/test_orbit_browser.cjs` usa o HTML embebido no firmware,
com a API simulada, em Chromium. Percorre os ecrãs a 320, 390, 768 e 1440 px,
verifica navegação, ausência de overflow horizontal da página, ausência de
IDs duplicados, autenticação e os cinco separadores. Também testa termóstato,
histerese, comandos KNX, GPIO/OLED, diagnóstico, rede, backup/restauro, OTA
e administração. Com `playwright-core` no `NODE_PATH`, definir
`ORBIT_CHROMIUM` para o executável do Chromium e executar
`node scripts/test_orbit_browser.cjs`. `ORBIT_SCREENSHOTS` é opcional.

A simulação Web confirma pedidos e respostas da interface, sem enviar
telegramas, gravar na flash ou instalar firmware num ESP32 físico. Para validar a instalação física, testar num equipamento o arranque, a recuperação AP,
as alterações de rede, a persistência, OTA e o feedback dos atuadores KNX.
Esta integração não atualiza a plataforma Arduino-ESP32 nem resolve a
pendência registada em `review/security-2026-11-07`.

## Instalar e reverter

Guardar um backup da configuração antes de atualizar. Numa cópia do repositório:

```powershell
git switch main
git pull
pio run -e esp32
```

A atualização OTA usa `.pio/build/esp32/firmware.bin`. Para reverter, recompilar
e reinstalar a versão anterior do firmware a partir do respetivo commit.
