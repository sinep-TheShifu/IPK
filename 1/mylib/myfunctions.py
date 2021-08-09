import sys
import argparse
import socket
import re  
import os 
import select
import pathlib 

# Parsovanie argumentov
def parse_arguments():
    parser = argparse.ArgumentParser(description='Simple distributed file system.')

    parser.add_argument("-n", required=True, help='IP of nameserver')
    parser.add_argument('-f', required=True, help='SURL of file at distributed server')

    args = parser.parse_args()
    return args

# Rezolucia domenoveho mena na IP adresu a port
def get_fs_info(args):
    ns = get_ip_and_port(args.n)    # IP a port name serveru
    fs = get_domain_name(args.f)         # Domenove meno file serveru

    # UDP request na name server, vracia odpoved
    response = nsp_request(ns[0], int(ns[1]), fs)

    # Kontrola odpovede
    response = repr(response).strip()   # Bytes issue
    response = response[2:-1].strip()
    match = re.search("\AERR", response)
    if match:
        print("Name server response:", response)
        sys.exit(1)

    # Funkcia vrati IP, port
    response = response[2:]     # Odstranienie b'OK

    return get_ip_and_port(response)

# Vrati cestu k suboru/suborom
def get_files(surl, file_server):
    surl = surl.strip()
    domain_name = get_domain_name(surl)
    start = 7 + len(domain_name)  
    files = [] 

    # Hromadne stahovanie -> ziskam index a zneho citam
    if surl.endswith("/*"):
        files.append("index")
        download_files(files, file_server, domain_name, False)    # Ziskam subor index
        files = files + parse_index()    # Ziskam pole suborov na stiahnutie
        download_files(files, file_server, domain_name, True)
    # Ziskam cestu
    else:
        surl = surl[start:]
        files.append(surl)        # Ziskam cestu k suboru
        download_files(files, file_server, domain_name, False)

# Stiahnutie suborov z file serveru
def download_files(files, file_server, domain_name, bulk):
    serverName = file_server[0]             # IP 
    serverPort = int(file_server[1])        # Port

    for file2get in files:
        #print("Searching for file:", file2get)
        message = bytes("GET " + file2get + " FSP/1.0\r\n" +        # Sprava
                        "Hostname: " + domain_name + "\r\n" +
                        "Agent: xandra05\r\n\r\n", encoding="utf-8")
        reply = b''

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    # Socket
        
        try:
            sock.connect((serverName, serverPort))       # Pripojenie na server
        except socket.error as e:
            sock.close()
            print("File server response:", e)
            sys.exit(1)
        
        try:
            #Posielanie dat
            sock.sendall(message)

            # Odpoved
            while True: 
                data = sock.recv(2048)
                if not data:
                    break
                reply += data  

        # Spracovanie vynimky
        except socket.error as e:
            sock.close()    # Zavriem socket
            print("Unexpected error with file server. FS response:", e)
            sys.exit(1)

        finally:
            sock.close()
        
        # Hlavicka
        header = reply.split(b'\r\n\r\n')[0]
        file_ok = control_reply(header, file2get, bulk)   # Kontrola odpovede

        # Telo suboru
        content = reply[len(header)+4:]

        # Zapis odpovede do suboru pri Success odpovedi
        if file_ok == True:
            write_file(file2get, content, bulk)

# Zapise data do suboru
def write_file(file_name, content, bulk):
    # Nevytvara sa adresarova struktura
    if bulk == False:
        # print("Creating file:", file_name)
        file_name = file_name.split("/")
        file_name = file_name[len(file_name) - 1]
        f = open(file_name, 'wb')
        f.truncate(0)
        f.write(content)
        f.close()
    # Vytvara sa adresarova struktura
    else:
        path = file_name.split("/")
        del path[-1]    # Odstrani nazov suboru z cesty
        #file_name = file_name[len(file_name) - 1]
        
        path_act = str(pathlib.Path().absolute())    # Aktualna cesta
        for pth in path:
            path_act = path_act + "/" + pth
            if os.path.isdir(path_act) == False:   # Kontrola ci uz adresar neexistuje         
                os.mkdir(path_act)
                #print("Making directory", path_act)
        
        f = open(file_name, 'wb')
        f.truncate(0)
        f.write(content)
        f.close()

# UDP request na name server, vracia odpoved serveru
def nsp_request(serverName, serverPort, domainName):
    # Vytvorenie UDP dotazu na name server
    message = bytes('WHEREIS ' + domainName, encoding='utf-8') # WHEREIS server.name

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(message, (serverName, serverPort))
    sock.setblocking(0)
    ready = select.select([sock], [], [], 30)   # Nastavenie timeoutu na 10s
    if ready[0]:    # Server odpoveda
        receivedMessage, serverAddress = sock.recvfrom(2048)
    else:           # Server neodpoveda
        print("Name server did not respond.")
        sock.close
        sys.exit(1)

    sock.close()
    return receivedMessage

# Vrati pole suborov 
def parse_index():
    try:
        files = []
        f = open("index", "r")
        file_content = f.read().split("\n")
        for line in file_content:
            if line != '':
                files.append(line)
    finally:
        f.close()

    return files

# Kontrola hlavicky odpovede z file servera
def control_reply(reply, subor, bulk):
    # Odpoved zo severu nieje Success
    if repr(reply).find("Success") == -1:
        print("File:", subor, "was not downloaded. Server response:", reply)
        
        # Ak nieje zadane hromadne stahovanie, tak sa proces ukonci 
        if bulk == False: 
            sys.exit(1)
        # Hromadne stahovanie bolo zadane, subor sa preskoci
        return False    
    return True # Odpoved zo servera je success

# Vrati v poli ip adresu a port
def get_ip_and_port(ip_port):
    ip_port = ip_port.strip()                                       # Odstrani biele znaky zo zaciatku a konca
    match = re.match(r'^[0-9]+(?:\.[0-9]+){3}:[0-9]+$', ip_port)    # Regex pre IP:port

    # Chybne zadany argument pre name server
    if match == None:
        print(ip_port, "is not a valid argument.1")
        sys.exit(1)

    ip_port = ip_port.split(":")    # Rozdeli string podla dvojbodky
    control_ip(ip_port[0])          # Kontrola validity IP adresy
    control_port(ip_port[1])        # Kontrola validity portu

    return ip_port

# Vrati domenove meno serveru zo SURL
def get_domain_name(surl):
    surl = surl.strip()                      # Odstrani biele znaky na zaciatku a konci
    protocol = re.search("\Afsp://", surl)   # Kontrola protokolu

    # Protokol je zle definovany -> chyba
    if protocol == None:
        print(surl, 'is not valid argument.')
        sys.exit(1)
    
    surl = surl[6:]         # Odstranenie definicie protokolu
    surl = surl.split("/")  # Rozdelenie podla lomitka
    domain_name = surl[0]   # Domenove meno sa nachadza na prvej pozicii

    return domain_name

# Kontrola validnej IP adresy
def control_ip(ip_addr):
    try:
        socket.inet_aton(ip_addr)
    except socket.error:
        print(ip_addr, 'is not a valid IP address.')
        sys.exit(1)

# Kontrola validneho cisla portu
def control_port(port):
    port = int(port)        # Pretypovanie zo sting na int
    if port < 0 or port > 65535:
        print(port, 'is not valid port number')
        sys.exit(1)

