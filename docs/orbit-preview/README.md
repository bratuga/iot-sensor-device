# Orbit · proposta para revisão

Proposta de 07.11.2026. Esta branch contém apenas documentação e um protótipo navegável com dados simulados. Não altera firmware, serviços, configurações, KNX, MQTT, equipamentos ou `main`.

Abrir `index.html` num navegador. Não necessita de servidor, bibliotecas externas ou acesso aos equipamentos. O seletor inicial permite comparar os quatro equipamentos. Para rever os formulários protegidos, usar as credenciais de demonstração `admin` / `iot-device`.

O login no protótipo serve apenas para visualizar o percurso. Não é uma implementação de autenticação para produção. As ações de guardar, atualizar, restaurar e enviar telegramas são simuladas. O nome na rede, a escolha de IP fixo e os comandos do display têm resposta local no protótipo.

## Fontes inspecionadas

| Repositório | Base da branch | Data do commit |
| --- | --- | --- |
| bratuga/iot-vmc-controler | `65cf2b1e3a5381a0d7bba2352eb26d769d3895c1` | 06.11.2026 |
| bratuga/iot-sensor-device | `c92b7e570285bf8303ddabccb832fb70d7eabd24` | 07.11.2026 |
| bratuga/iot-lusol | `6e6d390a1c26b6e2b74d3ca65db1f856967f3eb1` | 01.11.2026 |
| bratuga/iot-moxa-router | `0857d2d7252ea5d3a3a50c1b1d9616bf3846cfa9` | 07.11.2026 |
| bratuga/design | `16baa5150894320e4db27fcc83f7b51692cc183e` | referência visual |

O protótipo usa as cores dos projetos e a navegação inspirada em Orbit. O desenho da VMC e o respetivo termóstato foram extraídos de `web/index.html`; o coletor e os termóstatos de divisão foram extraídos de `include/WebUi.h`, incluindo os estilos e as funções de desenho, com dados simulados. As ações originais de rede foram substituídas por ações locais. Não foram alterados os componentes originais nos repositórios.

## Decisões confirmadas pelo proprietário

- Dashboard, sensores, gráficos, histórico e comandos operacionais ficam públicos **na rede local**.
- Administração e configurações ficam em página própria, protegida. A proteção deverá existir nos endpoints, além da navegação.
- AP dos ESP: SSID `iot-device`, palavra-passe `iot-device`, incluindo recuperação após perda de Wi-Fi. Estes valores partilhados são uma opção explícita do proprietário.
- Credenciais iniciais de administração: `admin` / `iot-device`. A palavra-passe pode ser alterada e deve sobreviver aos reinícios. Não a repor a cada arranque.
- AP de recuperação Wi-Fi **não se aplica ao Moxa**. A sua configuração de rede fica na administração.
- A curto avança a página no StamPLC. A mantido durante 3 segundos alterna entre normal e configuração.
- B/C alteram Modo, Setpoint, Bypass e percentagens de insuflação/extração. Alterar percentagens passa para Manual.
- Noite mantém a lógica, o horário e os valores já configurados. O código atual implementa Noite como condição do automático, não como valor independente do enum `Mode`. Não introduzir um novo algoritmo só para apresentar quatro opções na interface.
- Manter os cartões VMC, aquecimento e termóstatos. Pode mudar a sua posição e a organização da página.
- A implementação funcional aguarda aprovação dos mockups. Dúvidas que afetem o funcionamento devem ser apresentadas antes de alterar o comportamento.

## Organização Web proposta

Navegação comum: Painel, Sensores, Histórico e Configuração. O protótipo inclui uma entrada Recuperação para rever o assistente dos ESP; na implementação, este percurso será aberto no estado de configuração/recuperação do dispositivo. Não é uma forma de contornar o login na rede normal. Em ecrãs estreitos, o menu torna-se horizontal e quebra em linhas.

| Área protegida | Conteúdo |
| --- | --- |
| Técnico | Rede, nome/hostname, IP DHCP ou fixo, gateway/DNS, NTP/fuso, backup/restauro, auditoria, atualização e credenciais |
| Diagnóstico | Logs da mesma origem dos logs série; no Moxa, logs dos serviços; destino SD quando existe; versão e estado do equipamento |
| Hardware | Placa, sensores, saídas, pinout e parâmetros fixos identificados no código; leitura apenas para elementos não configuráveis |
| KNX | Servidor, teste de ligação, objetos de grupo, teste de envio e receção com operação/DPT/valor explícitos |
| Configurações | Parâmetros específicos da VMC, sensores/aquecimento, LusoSol ou gateway |

Usar os mesmos componentes para campos, seletores, switches, botões de guardar/testar, estados pendente/erro/sucesso e confirmações de ações destrutivas. Cabeçalhos curtos. Sem bibliotecas remotas no firmware. Rodapé discreto com versão e data reais da compilação; neste protótipo, o rodapé identifica o commit e a **data da fonte**, pois não foi produzido um firmware novo.

O LusoSol permanece MQTT-only: o painel KNX explica a ausência desta função, sem sugerir que existe uma ligação KNX direta. No Moxa, SolaX continua apenas de leitura. No LusoSol, a saída física continua bloqueada como no código de referência.

## Rede, hora e identidade

O assistente deve procurar redes, permitir uma rede oculta, configurar credenciais, nome do dispositivo e DHCP/IP fixo. O nome é um identificador DNS válido e único, usado também no hostname/mDNS; evitar um nome visual diferente do nome de rede. A unicidade e os erros de configuração precisam de validação na implementação.

Quando há conectividade e falta uma hora válida, tentar NTP. Preservar RTC quando disponível e indicar hora não sincronizada sem inventar timestamps. A disponibilidade de NTP não pode bloquear controlo local. Manter fuso horário e horário de verão de Lisboa configuráveis. A uniformização não deve alterar o fuso/horário guardado silenciosamente.

Os ESP têm hoje comportamentos e endereços AP diferentes. O protótipo do StamPLC mostra `192.168.99.1`, já presente no respetivo código; uniformizar o IP do AP não foi solicitado. Reconexão STA, entrada/saída de AP, captive portal, persistência e concorrência entre equipamentos com o mesmo SSID devem ser tratados explicitamente na implementação, sem suspender proteções ou controlo por esperas de rede.

## Display StamPLC

Resolução 240 × 135, sem barras superior/inferior permanentes. Páginas normais, pela ordem pedida:

1. Estado
2. Modo: Auto / Manual / Boost / Noite
3. Setpoint
4. Bypass
5. Ar novo
6. Insuflação
7. Extração
8. Rejeição
9. Percentagem de insuflação
10. Percentagem de extração
11. Temperatura interna do controlador / tensão

Temperatura e humidade usam valores grandes. Fundo azul para ar novo, verde para insuflação, laranja para extração, coral para rejeição, mantendo sempre o nome textual. A versão fica em Diagnóstico/Sobre, para respeitar a remoção das barras no modo normal; no Web fica no rodapé.

No protótipo, a rotação automática é opcional para facilitar a revisão (7 s por página, valor demonstrativo). A implementação deve conservar a temporização existente até decisão em contrário. A interação não deve disparar comandos ao terminar uma pressão longa. B/C não alteram páginas de simples leitura. O passo de percentagens do protótipo é 20%, conforme o ajuste local atual; os limites efetivos e níveis físicos têm de continuar a vir do controlador. Setpoint usa o passo atual de 0,5 °C.

Alarmes prioritários continuam visíveis mesmo sem barras permanentes. O desenho propõe um ecrã de alarme temporário; prioridade, reconhecimento e proteções existentes não são removidos. As estimativas de bypass não se tornam feedback físico só por mudar o desenho.

## Revisão e validação

Verificado em navegador: renderização dos quatro percursos de equipamento, componentes originais da VMC/coletor, login de demonstração, campos DHCP/IP fixo, atualização do nome de rede, envio KNX simulado, alteração de percentagem com passagem a Manual, e A durante 3 s para entrar/sair da configuração. Inspecionado em larguras de desktop e telemóvel. JavaScript verificado sintaticamente.

Ainda não validado: compilação deste desenho em firmware, recursos RAM/flash, tipografia real do display, interação física dos botões, recuperação após falha de rede/reboot, RTC/NTP real, backend de autenticação, persistência, equipamentos, cablagem ou atuação KNX. As alterações serão implementadas só depois da aprovação visual e validadas por equipamento.

A revisão de código e vulnerabilidades é separada, na branch `review/security-2026-11-07`, para poder ser aprovada independentemente do desenho.
