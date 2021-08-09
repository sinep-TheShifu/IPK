--- README ---

Program: SFTP server a klient
Implementácia SFTP serveru a klienta pomocou SFTP protokolu.
Viac informácií o SFTP na stránkach RFC913.


--- SPUSTENIE ---
1. Príkaz *make*
2. ./ipk-simpleftp-server {-i interface} {-p port} -u path
3. ./ipk-simpleftp-client {-p port} -h ip

{} - voliteľný argument




--- PRÍKLAD SPUSTENIA ---

make

./ipk-simpleftp-server -p 8080 -u tmp/db.txt

./ipk-simpleftp-client -p 8080 -h 127.0.0.1




--- PODROBNÉ INFORMÁCIE A ZMENY OPROTI RFC913 ---

=> manual.pdf






