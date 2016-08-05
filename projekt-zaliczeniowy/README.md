Temat: 2. Wersja rozproszona narzędzia arp-scan(1)

Opis: 
Narzędzie pozwala na sprawdzenie, które komputery w sieci są włączone poprzez żądania ARP. Zdalny klient (my-arp-scan-client) wysyła nieprzetworzonymi gniazdami IP do serwerów (my-arp-scan) zlecenia sprawdzenia komputerów w sieciach lokalnych. Serwery odpytują komputery w sieci lokalnej wysyłając żądania ARP. Zebrane wyniki zostają odesłane do klienta. 

Zawartość plików źródłowych: 
my-arp-scan.c - kod źródłowy serwera
my-arp-scan-client.c - kod źródłowy klienta

Kompilacja:
gcc -Wall my-arp-scan.c -o my-arp-scan -lnet -lpcap -lm
gcc -Wall my-arp-scan-client.c -o my-arp-scan-client

Sposób uruchomienia: 
my-arp-scan.c INTERFEJS
my-arp-scan-client.c LICZBA_SERWERÓW [LISTA_ADRESÓW_IP_SERWERÓW...]