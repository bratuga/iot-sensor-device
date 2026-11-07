# Security Review: iot-sensor-device

## Scope

Revisão estática de bratuga/iot-sensor-device no main c92b7e570285bf8303ddabccb832fb70d7eabd24. Branch de revisão: review/security-2026-11-07. Sem alterações de firmware.

- Scan mode: repository
- Target kind: git_revision
- Target ID: target_sha256_13a9b9fd98b89e867f13283380b40a2815fb5c5e70962a4c52999e68867b2788
- Revision: c92b7e570285bf8303ddabccb832fb70d7eabd24
- Inventory strategy: repository
- Included paths: .
- Excluded paths: none
- Artifacts reviewed: threatmodel.md, artifacts/dependency-review.md, artifacts/functional-review.md
- Scan context: Código revisto offline; consulta separada a advisories primários. Decisões atuais do utilizador: configuração/OTA/diagnóstico privados; leituras e comandos públicos apenas LAN; credenciais comuns apenas iniciais e persistentes após mudança. Observações de integração e propostas dependem de aprovação.

Limitations and exclusions:
- Revisão estática de uma passagem com baseline independente e investigação focada. Não é garantia de ausência de vulnerabilidades.
- Não foram executados builds, testes, PoCs, uploads OTA ou comandos contra dispositivos. Testes existentes foram lidos como evidência, não executados.
- Advisories e código das dependências foram consultados separadamente; firmware instalado, overrides privados, versões/backports do SO e ACLs não foram verificados.
- Credenciais iniciais comuns e operações públicas LAN são decisões aceites do utilizador, não findings. Não foram expostos segredos de instalação.
- Excluded installed-devices-and-binaries: Equipamentos instalados, binários, rede real e ensaios elétricos não analisados.
- Excluded non-main-branches: Branches diferentes de main excluídas desta revisão.

### Scan Summary

| Field | Value |
| --- | --- |
| Scan outcome | completed |
| Reportable findings | 2 |
| Severity mix | medium: 1, low: 1 |
| Confidence mix | high: 2 |
| Coverage | complete |
| Validation mode | Static source review and primary advisory correlation |

Canonical artifacts: `scan-manifest.json`, `findings.json`, and `coverage.json`. This report is a deterministic projection of those files.

## Threat Model

ESP32 com sensores, relés, termóstato/aquecimento, HTTP80, KNX UDP, histórico SPIFFS, configuração persistente e OTA autenticado. Setup carrega configuração e inicia STA/AP (src/main.cpp:47,1579-1618).

### Assets

- Credenciais/configuração em SPIFFS /config0.bin e /config1.bin; legado NVS sensor/config mantém-se na migração (src/Config.cpp:14,469-513).
- Sessão admin/CSRF em RAM e firmware (src/main.cpp:738-758,1208-1227,1468-1518).
- Relés/KNX aquecimento, NVS heating/layout e heating/values; histórico SPIFFS /history0.bin e /history1.bin (src/main.cpp:127-148,863-864,895-897,1085-1087).

### Trust Boundaries

- Público LAN -\> status/operações com CSRF público; intencional sem admin para operações (src/main.cpp:751-758,1186-1206,1305-1307,1392-1410).
- Login admin -\> X-Admin-Session aleatório -\> configuração/OTA; expiração 30min, bloqueio após5 falhas (src/main.cpp:738-758,1208-1235,1247-1268).
- LOCAL_AP_PASSWORD se definido é credencial admin alternativa independente da password persistida (src/main.cpp:1217-1221).
- Gateway KNX configurado -\> túnel -\> comandos/feedback; datagramas limitados256 (src/main.cpp:637-676).
- OTA -\> flash exige sessão, CSRF, estado não ocupado e heatingSafe; pára túnel (src/main.cpp:1486-1518).

### Attacker Capabilities

- LAN/AP tem leituras/comandos operacionais intencionalmente públicos.
- Sem sessão não deve alterar config/firmware ou aceder diagnóstico KNX.
- Peers KNX têm autoridade barramento; segmentação real desconhecida.
- Acesso físico/build é separado de autoridade Web.

### Security Objectives

- Política nova: admin inicial admin/iot-device alterável e persistente; AP comum; leituras/comandos públicos e configuração/OTA/diagnóstico privados.
- Preservar dual-copy validado e redação de credenciais no GET (src/Config.cpp:24-59,469-513; src/main.cpp:1232).
- Preservar shutdown/feedback em config/update; não inferir segurança física.

### Assumptions

- Build atual gera admin aleatório e AP Sensor-\<MAC\>, senha admin ou override LOCAL_AP_PASSWORD (src/main.cpp:585-599,1585-1590).
- Status público inclui logs telegramas mesmo sem admin/showDiagnostics; difere privacidade pretendida (src/main.cpp:1405-1466).
- Flash encryption, secure boot, overrides privados e hardware não verificados.
- Config SPIFFS e legado NVS podem conter segredos; web omite. AP 192.168.4.1 HTTP80. KNX endpoint vem config.gateway/port/rate. Aquecimento guarda layout/values NVS; histórico valida ambas cópias SPIFFS (src/Config.cpp:14-59,469-513; src/main.cpp:585-625,637-676,127-148,1381-1386).

## Findings

| Finding | Severity | Confidence | Detailed write-up |
| --- | --- | --- | --- |
| [Pedido multipart pode reiniciar o ESP32 antes da autenticação (CVE-2026-42854)](#finding-1) | medium | high | inline below |
| [Estado público expõe o monitor de telegramas KNX](#finding-2) | low | high | inline below |

### Confidence Scale

| Label | Meaning |
| --- | --- |
| high | Direct evidence supports the finding with no material unresolved blocker. |
| medium | Evidence supports a plausible issue, but material runtime or reachability proof remains. |
| low | Evidence is incomplete and the item is retained only for explicit follow-up. |

<a id="finding-1"></a>

### [1] Pedido multipart pode reiniciar o ESP32 antes da autenticação (CVE-2026-42854)

| Field | Value |
| --- | --- |
| Severity | medium |
| Confidence | high |
| Confidence rationale | Manifesto PlatformIO, cache da versão declarada e fluxo parser-before-handler confirmados por leitura; sem reprodução no equipamento. |
| Category | Stack-based buffer overflow em dependência HTTP |
| CWE | CWE-121 |
| Affected lines | platformio.ini:5, src/main.cpp:47 |

#### Summary

Um cliente com acesso HTTP ao ESP32 pode atingir a alocação de stack baseada no boundary multipart em Arduino-ESP32 2.0.17. A dependência fixada por este projeto está no intervalo afetado pelo CVE-2026-42854. O resultado sustentado é corrupção de memória/queda; execução de código é possibilidade do advisory, não foi demonstrada neste firmware.

#### Root Cause

O firmware expõe WebServer; a biblioteca processa Content-Type multipart e cria um buffer de stack proporcional ao boundary antes de chamar o handler de rota. A pinagem mantém a versão vulnerável. O controlo de sessão no callback de aplicação não é alcançado antes do parser vulnerável.

**Dependência afetada** — `platformio.ini:5`

A plataforma 6.9.0 seleciona framework Arduino 2.0.17 (~3.20017.0), cuja implementação multipart verificada tem boundary sem limite.

```
platform = espressif32@6.9.0
```

#### Validation

Confirmada resolução 6.9.0 -\> Arduino 2.0.17 e Parsing.cpp:164-167,226-230,435-437 do pacote local. WebServer.cpp:302-307 chama _parseRequest antes de _handleRequest. A alocação de stack não depende do retorno da callback upload. Evidência e fontes em artifacts/dependency-review.md.

Validation method: Rastreio estático do projeto e dependência local; advisory primário

**Dependência afetada** — `platformio.ini:5`

A plataforma 6.9.0 seleciona framework Arduino 2.0.17 (~3.20017.0), cuja implementação multipart verificada tem boundary sem limite.

```
platform = espressif32@6.9.0
```

Limitations:
- Não foi executado exploit nem confirmado o hash do binário instalado.

#### Dataflow

Header Content-Type -\> boundaryStr -\> _parseForm -\> fastBoundary\[boundary.length()+5\]

#### Reachability

Servidor HTTP iniciado no ESP32; não requer credenciais. O acesso de rede ao equipamento é pré-requisito.

#### Severity

**Medium** — Ataque requer acesso à rede local/AP. Um pedido não autenticado pode interromper o controlador; não foi demonstrado RCE nem impacto físico. A exposição restrita à LAN reduz a gravidade relativamente ao CVSS genérico do fornecedor.

Exposição a redes não confiáveis ou RCE reproduzida elevariam a gravidade; um binário comprovadamente compilado com correção elimina este caminho.

#### Remediation

Atualizar para Arduino-ESP32 corrigido (\>=3.3.8, preferindo versão compatível atual validada) ou aplicar o backport do limite de 70 caracteres antes do parsing. Testar a migração de APIs/driver em bancada e manter proteções físicas.

Tests:
- Teste offline do parser rejeita boundary \>70 antes da callback.
- Revalidar compilação e regressões de KNX, sensores, Wi-Fi e OTA na versão escolhida.

<a id="finding-2"></a>

### [2] Estado público expõe o monitor de telegramas KNX

| Field | Value |
| --- | --- |
| Severity | low |
| Confidence | high |
| Confidence rationale | Produção do log e serialização pública sem guarda confirmadas no mesmo ficheiro. |
| Category | Missing authorization for diagnostic data |
| CWE | CWE-862 |
| Affected lines | src/main.cpp:1463-1466, src/main.cpp:267-278 |

#### Summary

`GET /api/status` devolve os 48 telegramas recentes com endereço de origem/grupo, operação e bytes. Um cliente anónimo pode consultar continuamente o diagnóstico KNX, embora o requisito atual reserve diagnóstico a administradores.

#### Root Cause

`Events::onTelegram` forma o texto origem/grupo/operação/dados e `logTelegram` guarda-o num ring48. A resposta pública de status publica esse ring. A opção de UI não faz autorização no servidor.

**Log no JSON público** — `src/main.cpp:1463-1466`

O handler /api/status copia o ring de telegramas sem chamar auth; showDiagnostics só é um valor de UI.

```
    JsonArray t = d.createNestedArray("telegrams");
    for (unsigned i = 0; i < logCount; ++i)
      t.add(logs[(logNext + MaxLogs - logCount + i) % MaxLogs]);
    sendJson(d);
```

#### Validation

Confirmados eventos em :267-278, ring :187-192, rota sem auth :1405 e saída :1463-1466. `/api/knx/command` usa mutation, portanto escrita arbitrária não faz parte deste achado.

Validation method: Rastreio estático

**Log no JSON público** — `src/main.cpp:1463-1466`

O handler /api/status copia o ring de telegramas sem chamar auth; showDiagnostics só é um valor de UI.

```
    JsonArray t = d.createNestedArray("telegrams");
    for (unsigned i = 0; i < logCount; ++i)
      t.add(logs[(logNext + MaxLogs - logCount + i) % MaxLogs]);
    sendJson(d);
```

Limitations:
- Não observados telegramas reais nem inferida sensibilidade de grupos particulares.

#### Dataflow

Telegrama recebido -\> logTelegram -\> ring48 -\> /api/status -\> cliente anónimo

#### Reachability

Qualquer cliente HTTP LAN/AP; polling acompanha o fluxo sem acesso direto à interface KNX.

#### Severity

**Low** — Divulgação limitada ao tráfego observado pelo túnel, acessível na LAN/AP; não concede escrita arbitrária nem expõe credenciais. Dados operacionais simples já são públicos por decisão do utilizador.

Dados sensíveis adicionais nos grupos ou exposição fora da LAN elevariam impacto.

#### Remediation

Separar telegramas/diagnóstico em endpoint autenticado, mantendo apenas leituras operacionais e estado mínimo na API pública.

Tests:
- Resposta pública não contém telegrams nem diagnóstico técnico.
- Sessão admin válida pode consultar o monitor; sessão expirada recebe401.

## Reviewed Surfaces

| Surface | Risk Area | Outcome | Notes |
| --- | --- | --- | --- |
| API HTTP e dependência WebServer | Autorização e parsing | Reported | Parser multipart e telegramas KNX públicos confirmados. WebMCP get_recent_knx_telegrams utiliza os mesmos dados; não é uma vulnerabilidade adicional. |
| UI, configuração dual-copy, sessões e OTA | not recorded | No issue found | 53/53 ficheiros lidos; DOM usa textContent/value, limites e redação presentes. A autenticação do callback não corrige o parser anterior da dependência. |
| KNX, relés, termóstato e aquecimento | not recorded | No issue found | Biblioteca e testes lidos; limites de arrays/grupos, filas e feedback. Ausência de sonda válida desativa supervisão térmica conforme documentado; segurança física não demonstrada. |

## Open Questions And Follow Up

- Confirmar se o build privado define LOCAL_AP_PASSWORD: src/main.cpp:1217-1221 aceita esse valor como credencial alternativa mesmo após alterar a credencial persistida. Macro não definida no inventário revisto.
- Confirmar o binário instalado e testar sensores/atuadores em bancada; branches feature/\* não auditadas.
