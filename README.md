# IoT Device Emulator

Modularer, erweiterbarer Emulator für IoT-Geräte mit C++-Kern und Python-Monitoring.

## Systemarchitektur

**Hauptkomponenten:**
- **DeviceRunner:** Universeller Hintergrund-Runner für jedes Gerät. Jedes Gerät läuft in einem eigenen Thread, verwaltet vom DeviceManager.
- **DeviceManager:** Zentrale Geräteverwaltung und Lebenszyklus-Manager. Bietet Zugriff auf alle Geräte und deren Runner.
- **REST API (Flask, Python):** Einheitliches Backend für Web-Interface und CLI. Stellt Endpunkte für Gerätestatus, Benutzerdaten, Protokollinformationen und Szenarienmanagement bereit.
- **Web-Interface (monitor):** Visualisiert Geräte-/Benutzerdaten, Protokoll-/Serviceinformationen, ermöglicht Szenarienauswahl und -bearbeitung für jedes Gerät.
- **Konsole (CLI):** Direkte Geräteverwaltung, Szenarienausführung und Datenstreaming über die Kommandozeile. CLI kommuniziert mit dem Backend über die REST API.

**Funktionsweise:**
- Die gesamte Logik und Szenarienausführung wird im C++-Kern (DeviceManager/DeviceRunner) verwaltet.
- Die Python Flask REST API stellt Endpunkte für Geräteliste, Status, Benutzerdaten und Szenarienmanagement bereit.
- Web-UI und CLI nutzen die gleiche REST API, sodass alle Aktionen synchronisiert sind.
- Szenarien für jedes Gerät sind als Dropdown im Web-UI verfügbar, mit Editor für den Inhalt.
- Die CLI kann Szenarien auslösen und Live-Daten empfangen.

**Vorteile:**
- Zentrale Quelle für Gerätestatus und Szenarien
- Einfache Erweiterbarkeit (neue Geräte, Protokolle, Szenarien)
- Einheitliche Nutzererfahrung in Web und CLI
- Modulare, skalierbare und wartbare Architektur

## DeviceRunner: universeller Gerätestarter

DeviceRunner ist ein universelles Modul, um jedes Gerät in einem eigenen Hintergrund-Thread zu starten. Jedes emulierte Gerät wird in einen DeviceRunner eingebettet, der den Lebenszyklus (start/stop) verwaltet und den unabhängigen Betrieb im Hintergrund sicherstellt.

**Vorteile:**
- Paralleler Betrieb mehrerer Geräte, jedes in eigenem Thread
- Flexible Steuerung des Gerätestatus über DeviceManager
- Einfache Erweiterung um neue Gerätetypen ohne Änderung der Runner-Logik

**Funktionsprinzip:**
- Für jedes Gerät wird ein DeviceRunner-Objekt mit shared_ptr<DeviceBase> erstellt.
- Beim Start wird ein Thread gestartet, der device->start() aufruft und das Gerät betreibt (z.B. Datenpublikation, Ereignisemulation).
- Das Stoppen beendet den Thread sauber und gibt Ressourcen frei.
- DeviceManager verwaltet eine Map<Name, shared_ptr<DeviceRunner>> für alle Geräte.

## Gerät hinzufügen und konfigurieren

Alle Geräte werden in der Datei `config/devices.yaml` beschrieben. Beispiel:

```yaml
temperature_sensor:
  class: TemperatureSensor
  protocol: mqtt
  port: 1883
  mqtt:
    host: 127.0.0.1
    port: 1883
    topic: sensors/temperature
  initial_state: inactive
  scenarios:
    - overheat
    - disconnect

pressure_valve:
  class: PressureValve
  protocol: opcua
  port: 4840
  initial_state: inactive
  scenarios:
    - leak
    - jam
```

**Neues Gerät hinzufügen:**
1. Fügen Sie einen Abschnitt mit einem eindeutigen Namen hinzu (z.B. `my_device:`).
2. Geben Sie die Geräteklasse (`class`), das unterstützte Protokoll (`protocol`: mqtt, opcua, rest etc.), Port und Protokollparameter an.
3. Für MQTT: Parameter im Abschnitt `mqtt` angeben (host, port, topic).
4. Für OPC UA: Nur Port und Klassenname angeben.
5. Szenarienliste (`scenarios`) hinzufügen, falls benötigt.

Nach Änderungen an `devices.yaml` starten Sie die Services neu, um die Änderungen zu übernehmen.

## Build und Ausführung

### Automatischer Start aller Services

Empfohlen: PowerShell-Skripte zum Starten und Stoppen des gesamten Systems verwenden:

```pwsh
powershell -ExecutionPolicy Bypass -File ../start_all.ps1
```

Dies startet:
- MQTT-Broker (Docker)
- C++ REST-Server
- Python-Monitor-Server

Zum Stoppen aller Services:

```pwsh
powershell -ExecutionPolicy Bypass -File ../stop_all.ps1
```

### Manueller Start (Alternative)

**C++-Projekt bauen:**
```sh
cmake -S iot-emulator -B build && cmake --build build
```

**Python-Monitor-Server starten:**
```pwsh
.venv\Scripts\python.exe iot-emulator\monitor\app.py
```

**C++ REST-Server starten:**
```pwsh
build\core\Release\rest_server.exe
```

**MQTT-Broker (Docker) starten:**
```sh
docker run -d --name iot-mqtt-broker -p 1883:1883 eclipse-mosquitto
```

**Broker stoppen:**
```sh
docker stop iot-mqtt-broker
docker rm iot-mqtt-broker
```

## MQTT-Publishing prüfen

Um zu prüfen, ob z.B. der TemperatureSensor Daten an den MQTT-Broker sendet:

```sh
mosquitto_sub -h 127.0.0.1 -p 1883 -t sensors/temperature
```

Wenn Sie regelmäßig Werte sehen, funktioniert das Publishing.

## Konsolen-Benutzung

- Die CLI unterstützt zwei Modi:
  - **Single-Command-Modus:** Mit Argument starten, z.B. `iot_emulator_cli.exe test`
  - **Interaktiver Modus (REPL):** Ohne Argument starten, dann Befehle einzeln eingeben (`exit`/`quit` zum Beenden)
- Unterstützte Befehle:
  - `test` — Testausgabe
  - `exit`/`quit` — Beenden
  - Andere Befehle — Unbekannte-Befehl-Meldung

## Externe Abhängigkeiten: Paho MQTT

Dieses Projekt benötigt Paho MQTT C und C++ Libraries für die MQTT-Emulation. Diese sind nicht im Repository enthalten.

1. Paho MQTT C für Windows herunterladen: https://github.com/eclipse/paho.mqtt.c/releases
2. Paho MQTT C++ Quellen herunterladen: https://github.com/eclipse/paho.mqtt.cpp/releases
3. Beide Ordner in `external/paho/` ablegen:
   - `external/paho/eclipse-paho-mqtt-c-win64-*/include` und `lib`
   - `external/paho/paho.mqtt.cpp-*/include`, `lib`, und `build` (nach Build)
4. Bauen Sie paho.mqtt.cpp wie oben beschrieben, bevor Sie das Hauptprojekt bauen.

---

Wenn Sie weitere Hilfe oder eine noch detailliertere Anleitung benötigen, geben Sie bitte Bescheid!
