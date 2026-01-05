# Flash-Anleitung: LILYGO T-Display-S3 (Ubuntu/Linux)

Diese Anleitung beschreibt zwei Wege, um die **Flock You** Firmware auf ein **LILYGO T-Display-S3 (ESP32‑S3)** zu flashen: (A) **vorgebaute Firmware aus GitHub Actions** (kein lokaler Build) und (B) **lokal kompilieren + upload via PlatformIO**. [file:1]

> Empfehlung: Verwende **Variante A**, wenn du einfach nur flashen willst (z.B. auf einem frisch ausgepackten Board). [file:1]

---

## 📋 Voraussetzungen

### Hardware

- LILYGO T-Display-S3 (ESP32‑S3)
- USB‑C **Datenkabel** (viele Kabel sind nur „charge-only“)

### Ubuntu Pakete

```bash
sudo apt update
sudo apt install -y python3 python3-pip unzip git
```

### Serielle Berechtigung (einmalig)

```bash
sudo usermod -a -G dialout $USER
# danach ab- und wieder anmelden (oder reboot)
```

---

## 🅰️ Variante A (empfohlen): Vorgebaute Firmware aus GitHub Actions flashen

Diese Variante funktioniert ohne lokalen Build, indem das CI-Artefakt genutzt wird (enthält u.a. ein **merged** Image). [file:1]

### A1) Firmware-Artefakt herunterladen

1. Öffne die Actions-Übersicht des Repositories und wähle den neuesten erfolgreichen Run auf `main`.
2. Lade das Artifact **`firmware-lilygo-tdisplay-s3`** herunter.
3. Entpacke die ZIP-Datei:

```bash
mkdir -p ~/Downloads/flock-you-fw && cd ~/Downloads/flock-you-fw
unzip ~/Downloads/firmware-lilygo-tdisplay-s3.zip
ls -lah
```

### A2) Prüfen, ob das „Full Flash“-Paket enthalten ist

Wenn die CI korrekt konfiguriert ist, sollten mindestens diese Dateien vorhanden sein:

- `firmware_merged.bin` (ein einziges Image für `0x0`)
- `flash_args.txt` (Offsets für manuelles Flashen)
- `bootloader.bin`, `partitions.bin`, `firmware.bin`

Wenn du **nur** `firmware.bin` und `firmware.elf` siehst, ist das Artefakt ein „App-only“-Paket; dann siehe Abschnitt **A5** (Update-Flash) oder nutze Variante B. [file:1]

### A3) USB-Port finden

Board anstecken und Port prüfen:

```bash
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || true
```

Typisch ist `/dev/ttyACM0` oder `/dev/ttyUSB0`.

### A4) Flashen mit `firmware_merged.bin` (immer‑funktioniert‑Weg)

1. `esptool` installieren/aktualisieren:

```bash
python3 -m pip install -U esptool
```

2. Board ggf. in den Bootloader bringen:

- **BOOT gedrückt halten**
- kurz **RESET** drücken
- **BOOT loslassen**

3. Flashen:

```bash
# Beispiel: Port anpassen
PORT=/dev/ttyACM0

esptool.py --chip esp32s3 --port "$PORT" --baud 921600 write_flash 0x0 firmware_merged.bin
```

Nach erfolgreichem Flash erscheint typischerweise am Ende „Hard resetting via RTS pin…“.

### A5) Optional: Manuelles Flashen mit Offsets (wenn du kein merged Image nutzen willst)

Die Offsets stehen in `flash_args.txt`.

Beispiel (Dateien müssen im Ordner vorhanden sein):

```bash
PORT=/dev/ttyACM0

esptool.py --chip esp32s3 --port "$PORT" --baud 921600 write_flash \
  0x0 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

Falls zusätzlich `boot_app0.bin` vorhanden ist, wird es üblicherweise bei `0xe000` geflasht.

### A6) Sonderfall: „App-only“ Update-Flash (nur `firmware.bin` vorhanden)

Wenn das Artefakt nur `firmware.bin` enthält, kann man **nur die App** flashen (typisch Offset `0x10000`). [file:1]

> Das klappt in der Regel nur zuverlässig, wenn Bootloader/Partitionen auf dem Board bereits passend vorhanden sind.

```bash
PORT=/dev/ttyACM0
python3 -m pip install -U esptool
esptool.py --chip esp32s3 --port "$PORT" --baud 921600 write_flash 0x10000 firmware.bin
```

---

## 🅱️ Variante B: Lokal kompilieren + Upload mit PlatformIO

Diese Variante ist der „Fallback“, wenn du kein passendes CI-Artefakt hast oder aktiv entwickeln willst. [file:1]

### B1) PlatformIO installieren

```bash
python3 -m pip install -U platformio
pio --version
```

### B2) Repository klonen und flashen

```bash
git clone https://github.com/philibertschlutzki/flock-you.git
cd flock-you

# Build
pio run

# Upload (Flash)
pio run -t upload
```

---

## ✅ Erfolgskontrolle (Serial Monitor)

Serielle Ausgabe (115200 baud) ansehen:

```bash
# falls Variante B genutzt wird:
cd flock-you
pio device monitor
```

Alternativ ohne PlatformIO z.B. mit `picocom`:

```bash
sudo apt install -y picocom
picocom -b 115200 /dev/ttyACM0
```

---

## ❌ Häufige Fehler & Fixes

### „Permission denied“ auf `/dev/ttyACM0`

- Prüfen, ob der User in `dialout` ist und neu angemeldet wurde.

### „Failed to connect“ / „Timed out waiting for packet header“

- Bootloader erzwingen (BOOT halten → RESET tippen → BOOT loslassen) und erneut flashen.
- Anderes USB‑Kabel testen (Datenkabel!).

### Port „busy“

- Alle Serial-Monitore schließen (auch VSCode/PlatformIO Monitor), dann erneut flashen.

---

## 🔄 Firmware aktualisieren

- Variante A: neues Artifact herunterladen und erneut flashen.
- Variante B:

```bash
cd flock-you
git pull
pio run -t upload
```
