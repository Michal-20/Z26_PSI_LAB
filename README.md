## Informacja co do plików znajdujących się w repozytorium oraz instrukcja obsługi

**Wykonali:**
- **Jakub Marszałek**
- **Michał Kaczyński**
- **Tomasz Zieliński**

**Numer zespołu: Z34**

### Zawartość repozytorium
2 katalogi jeden z plikami klienta (dockerfile + kod w C) oraz drugi z plikami serwera (dockerfile + kod w C). Dodatkowo istnieje plik docker-compose.yaml, który ma za zadanie usprawnić uruchomienie kodu.

### Instrukcja
Aby uruchomić całe zadania należy skopiować potrzebne pliki: folder Client i Server oraz plik docker-compose.yaml. Następnie z tego samego poziomu uruchomić polecenie:
```bash
docker compose up --build
```