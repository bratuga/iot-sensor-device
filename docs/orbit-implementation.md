# Orbit no firmware do sensor KNX

A branch `integration/orbit-firmware` integra `design/orbit-uniformizacao`
sobre a versão `0.3.19-room-hysteresis` da `main`. O binário apresenta a
versão `0.3.19-orbit.1`. A integração mantém o controlo da bomba, o
sequenciamento KNX, as proteções térmicas e a gravação da histerese a uma casa
decimal.

- Painel de coletor ou termóstato, Sensores, Histórico e Ajuda são acessíveis
  na rede local. Sensores mostra as cinco leituras locais, incluindo ida e retorno.
- A página `/config.html` tem cinco separadores: Técnico, Diagnóstico, Hardware,
  KNX e Configurações. Os endpoints de administração exigem sessão, e os
  comandos protegidos exigem também o token da página.
- Técnico inclui pesquisa de Wi-Fi, DHCP ou IPv4 fixo, NTP e fuso, backup,
  restauro, OTA, palavra-passe e reinício. Os detalhes do diagnóstico e os
  telegramas KNX exigem autenticação. Os logs são mantidos em RAM.
- As credenciais e a recuperação Wi-Fi seguem a `main`: o AP identifica o
  equipamento, as credenciais já gravadas são mantidas e uma instalação nova
  gera uma palavra-passe própria. As credenciais mostradas no terminal série
  não são copiadas para os logs Web.

## Validação

A compilação `pio run -e esp32` passou: 58,8% da partição de aplicação
e 19,7% da RAM estática. Os testes abaixo passaram nesta integração.

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
telegramas, gravar na flash ou instalar firmware num ESP32 físico. Antes de
promover para `main`, validar num equipamento o arranque, a recuperação AP,
as alterações de rede, a persistência, OTA e o feedback dos atuadores KNX.
Esta integração não atualiza a plataforma Arduino-ESP32 nem resolve a
pendência registada em `review/security-2026-11-07`.

## Instalar e reverter

Guardar um backup da configuração antes de atualizar. Numa cópia do repositório:

```powershell
git fetch origin
git switch --track origin/integration/orbit-firmware
pio run -e esp32
```

A atualização OTA usa `.pio/build/esp32/firmware.bin`. Para voltar à versão
estável, mudar para `main`, compilar e reinstalar o binário dessa branch.
A `main` ignora as novas opções de IP fixo e NTP; após o regresso poderá ser
necessário procurar o ESP32 pelo endereço DHCP.
