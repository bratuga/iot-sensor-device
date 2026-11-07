# Verificação de dependências — 2026-11-07

Manifestos locais consultados em leitura: PlatformIO espressif32@6.9.0 -> framework-arduinoespressif32 ~3.20017.0 (Arduino 2.0.17); espressif32@8.0.0 -> ~4.30312.0 (Arduino 3.3.12). Cache disponível: framework-arduinoespressif32@3.20017.241212+sha.dcc1105b e framework-arduinoespressif32 4.30312.261016. Isto caracteriza a resolução declarada e a dependência local, não prova qual binário está instalado nos equipamentos.

## CVE-2026-42854
Advisory primário: https://github.com/espressif/arduino-esp32/security/advisories/GHSA-8cmm-3887-r32j (afetados <=3.3.7; correção >=3.3.8).

No cache Arduino 2.0.17 libraries/WebServer/src/Parsing.cpp:164-167 extrai boundary sem limite; :226-230 chama _parseForm; :435-437 aloca char fastBoundary[4 + boundary.length() + 1] na stack. O ramo de ficheiro :422-437 faz esta alocação mesmo quando não existe callback de upload; canUpload só condiciona a chamada :431-432. WebServer.cpp:302-307 executa _parseRequest antes de _handleRequest. Portanto autenticação na callback/handler não protege o parser. Arduino 3.3.12 Parsing.cpp:308 rejeita boundary >70 antes da alocação.

## Outras verificações
GHSA-w887-pg2w-mfj2 (https://github.com/espressif/arduino-esp32/security/advisories/GHSA-w887-pg2w-mfj2) refere WebServer::authenticate/password. Os firmwares usam autenticação própria; não demonstrado esse caminho.
GHSA-9vfw-wx65-c872 (https://github.com/espressif/arduino-esp32/security/advisories/GHSA-9vfw-wx65-c872) refere HTTPUpdateServer e exemplos OTA; Sensor verifica sessão própria e X-CSRF-Token, VMC cookie SameSite=Strict; não atribuir automaticamente a CVE por dependência.
ESP-IDF 4.4 chegou ao fim de suporte em julho de 2024: https://www.espressif.com/sites/default/files/advisory_downloads/AR2024-008%20End-of-Life%20Advisory%20for%20ESP-IDF%20v4.4%20Release%20Branch%20EN.pdf . Arduino 2.0.17 baseia-se nessa linha: https://github.com/espressif/arduino-esp32/releases/tag/2.0.17 . Inventário do Moxa (SO, Python, OpenSSL, Mosquitto) não está fixado no repositório: avaliação CVE do sistema instalado exige versões/backports reais. Não foram contactados equipamentos nem executados exploits.
