# Revisão funcional e de integração — 7 de novembro de 2026

Âmbito: código main dos quatro repositórios, sem mudar firmware nem contactar equipamentos. Estas observações não constituem validação elétrica ou ensaio de segurança física.

| Prioridade | Observação | Evidência | Próximo passo para aprovação |
|---|---|---|---|
| Alta | Convenção de sinal Moxa/Lusol incompatível para ligação direta: Moxa preserva positivo=exportação; Lusol assume positivo=importação. | Moxa `moxa-meter-web/MQTT-CONTRATO.md:13`; Lusol `include/SurplusControl.h:5,24` e `src/main.cpp:132` | Definir explicitamente a conversão na fronteira MQTT e validar com carga conhecida antes de habilitar saída física. |
| Alta | Lusol só aceita número simples e timeout 12s; contrato Moxa exige JSON `valid`, qualidade e idade; medidor principal documenta pausa15s mais duração/transações. Uma subscrição direta do tópico numérico não verifica qualidade conjunta e pode causar cortes entre amostras normais. | Lusol `src/main.cpp:30,94-104,242-243`; Moxa `moxa-meter-web/MQTT-CONTRATO.md:47-57,67` | Definir adapter/contrato e timeout seguro com dados observados. Não ajustar automaticamente o controlo. |
| Média | VMC inicia AP em falha no arranque, mas `NetworkService::loop()` não o ativa após perda Wi-Fi durante funcionamento; só tenta STA. | VMC `firmware/src/services/NetworkService.cpp:118,122-149` | Implementar recuperação uniforme após aprovação, mantendo saída/controlos seguros. |
| Média | Lusol usa tempo desde boot para fallback Wi-Fi; depois de entrar `WIFI_AP`, os ramos de reconexão/MQTT exigem `WIFI_STA`. Não recupera automaticamente a rede quando esta regressa. | Lusol `src/main.cpp:209-226` | Estado AP+STA com tentativas controladas, e teste de perda/reposição de rede. |
| Média | Moxa installer apaga auth.json e instala `--no-auth`; retirar apenas a flag faria o serviço tentar abrir um ficheiro ausente. | Moxa `moxa-meter-web/install.py:84-88,126`; `server.py:29-31` | Provisionar credencial inicial e migração antes de ativar auth; preservar gateway/serviços e acesso Ethernet. |
| Baixa | VMC GET público `/api/v1/control` devolve o conjunto completo de parâmetros técnicos, apesar de GET `/api/v1/settings` exigir admin. | VMC `firmware/src/main.cpp:201-206,331,412`; `firmware/src/ConfigStore.cpp:45-78` | Definir projeção pública mínima para dashboard e guardar configuração técnica na API privada. Não contém password. |

As credenciais comuns solicitadas, controlo público apenas na LAN, mudança B/C para Manual e sem AP no Moxa são decisões aceites do utilizador. Os defaults comuns serão iniciais: não substituir palavras-passe alteradas em reinícios. Noite mantém a lógica existente. Nada foi alterado nesta revisão.
