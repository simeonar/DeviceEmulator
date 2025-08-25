# Systemarchitektur Übersicht
## IoT Device Emulator

**Datum:** 25. August 2025  
**Version:** 1.0  
**Architekt:** Lead Developer  
**Status:** In Entwicklung  

---

## 1. Architektur-Übersicht

### 1.1 High-Level Systemarchitektur

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Browser   │    │   CLI Client    │    │  External Apps  │
│                 │    │                 │    │                 │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          │              HTTP/REST API                  │
          │                      │                      │
      ┌───▼──────────────────────▼──────────────────────▼───┐
      │              Python Flask Server                   │
      │          (Web Interface & REST API)                │
      │                                                    │
      │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
      │  │   Monitor   │  │     API     │  │   Bridge    │ │
      │  │   Server    │  │  Endpoints  │  │   Layer     │ │
      │  └─────────────┘  └─────────────┘  └─────────────┘ │
      └────────────────────────┬───────────────────────────┘
                               │ HTTP/REST
                               │
      ┌────────────────────────▼───────────────────────────┐
      │               C++ Core Engine                      │
      │                                                    │
      │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
      │  │   Device    │  │   Device    │  │  Protocol   │ │
      │  │   Manager   │  │   Runner    │  │   Layer     │ │
      │  └─────────────┘  └─────────────┘  └─────────────┘ │
      │                                                    │
      │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
      │  │Temperature  │  │  Pressure   │  │   Custom    │ │
      │  │   Sensor    │  │    Valve    │  │   Devices   │ │
      │  └─────────────┘  └─────────────┘  └─────────────┘ │
      └────────────────┬───────────────┬───────────────────┘
                       │               │
                       │               │
          ┌────────────▼──┐       ┌────▼────────┐
          │  MQTT Broker  │       │ OPC UA      │
          │  (Mosquitto)  │       │ Clients     │
          └───────────────┘       └─────────────┘
```

### 1.2 Technologie-Stack

| Layer | Technologie | Version | Zweck |
|-------|-------------|---------|-------|
| **Frontend** | HTML/CSS/JavaScript | ES6+ | Web User Interface |
| **API Layer** | Python Flask | 2.3+ | REST API & Web Server |
| **Core Engine** | C++17 | ISO/IEC 14882:2017 | Device Simulation Engine |
| **Build System** | CMake | 3.20+ | Cross-Platform Build |
| **Dependency Mgmt** | vcpkg | Latest | C++ Package Management |
| **Messaging** | MQTT (Mosquitto) | 3.1.1 | Device Communication |
| **Industrial Protocol** | OPC UA (Open62541) | 1.3+ | Industrial Communication |
| **Configuration** | YAML | 1.2 | Device Configuration |
| **Testing** | Google Test | 1.12+ | Unit Testing Framework |
| **Containerization** | Docker | 20.10+ | Service Deployment |

### 1.3 Architekturprinzipien

#### **Separation of Concerns**
- **C++ Core:** Performance-kritische Device-Simulation
- **Python Layer:** Web-Interface und API
- **Protocol Layer:** Abstrahierte Kommunikation

#### **Modulare Architektur**
- **Plugin-basierte Devices:** Neue Geräte ohne Core-Änderungen
- **Protocol Adapters:** Austauschbare Kommunikationsprotokolle
- **Configurable Components:** YAML-basierte Konfiguration

#### **Event-Driven Design**
- **Asynchrone Kommunikation:** Message-basiert zwischen Komponenten
- **Real-time Updates:** WebSocket für Live-Daten
- **Scenario-driven Simulation:** Event-basierte Gerätesteuerung

## 2. C++ Core Engine Architektur

### 2.1 Komponentendiagramm

```cpp
┌─────────────────────────────────────────────────────────────┐
│                     C++ Core Engine                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────┐    ┌─────────────────┐                │
│  │  DeviceManager  │    │   RestServer    │                │
│  │                 │    │                 │                │
│  │ - registerDevice│◄───┤ - handleRequest │                │
│  │ - startDevice   │    │ - jsonResponse  │                │
│  │ - stopDevice    │    │ - corsEnabled   │                │
│  │ - getStatus     │    └─────────────────┘                │
│  └─────────┬───────┘                                       │
│            │                                               │
│            │ manages                                       │
│            ▼                                               │
│  ┌─────────────────┐                                       │
│  │  DeviceRunner   │                                       │
│  │                 │                                       │
│  │ - start()       │                                       │
│  │ - stop()        │                                       │
│  │ - isRunning()   │                                       │
│  │ - getDevice()   │                                       │
│  └─────────┬───────┘                                       │
│            │                                               │
│            │ wraps                                         │
│            ▼                                               │
│  ┌─────────────────┐    ┌─────────────────┐                │
│  │   DeviceBase    │    │ ProtocolClient  │                │
│  │   (abstract)    │    │   (interface)   │                │
│  │                 │    │                 │                │
│  │ + start()       │    │ + connect()     │                │
│  │ + stop()        │    │ + disconnect()  │                │
│  │ + getStatus()   │    │ + publish()     │                │
│  │ + getScenarios()│    │ + subscribe()   │                │
│  │ + simulate()    │    └─────────────────┘                │
│  └─────────┬───────┘                                       │
│            │                                               │
│            │ implements                                    │
│            ▼                                               │
│  ┌─────────────────┐    ┌─────────────────┐                │
│  │TemperatureSensor│    │  PressureValve  │                │
│  │                 │    │                 │                │
│  │ - mqttClient    │    │ - opcuaServer   │                │
│  │ - temperature   │    │ - pressure      │                │
│  │ - publishData() │    │ - publishData() │                │
│  └─────────────────┘    └─────────────────┘                │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Klassendesign Details

#### **DeviceManager**
```cpp
class DeviceManager {
public:
    // Device Lifecycle Management
    bool registerDevice(const std::string& name, 
                       std::shared_ptr<DeviceBase> device);
    bool startDevice(const std::string& name);
    bool stopDevice(const std::string& name);
    
    // Status & Information
    DeviceStatus getDeviceStatus(const std::string& name);
    std::vector<std::string> getDeviceList();
    std::vector<std::string> getScenarios(const std::string& name);
    
    // Scenario Management
    bool runScenario(const std::string& deviceName, 
                    const std::string& scenario);
                    
    // Configuration
    bool loadFromConfig(const std::string& configPath);

private:
    std::map<std::string, std::shared_ptr<DeviceRunner>> devices_;
    std::mutex devices_mutex_;
    ConfigLoader config_loader_;
};
```

#### **DeviceRunner**
```cpp
class DeviceRunner {
public:
    explicit DeviceRunner(std::shared_ptr<DeviceBase> device);
    ~DeviceRunner();
    
    // Thread Management
    bool start();
    bool stop();
    bool isRunning() const;
    
    // Device Access
    std::shared_ptr<DeviceBase> getDevice() const;
    DeviceStatus getStatus() const;

private:
    std::shared_ptr<DeviceBase> device_;
    std::unique_ptr<std::thread> worker_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> stop_requested_;
    
    void runLoop();
};
```

#### **DeviceBase (Abstract)**
```cpp
class DeviceBase {
public:
    virtual ~DeviceBase() = default;
    
    // Core Device Interface
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual DeviceStatus getStatus() const = 0;
    
    // Configuration
    virtual bool configure(const DeviceConfig& config) = 0;
    virtual DeviceConfig getConfig() const = 0;
    
    // Scenario Support
    virtual std::vector<std::string> getScenarios() const = 0;
    virtual bool simulate(const std::string& scenario) = 0;
    
    // Data Publishing
    virtual bool publishData() = 0;
    virtual void setPublishInterval(std::chrono::milliseconds interval);

protected:
    DeviceConfig config_;
    std::chrono::milliseconds publish_interval_{1000};
    std::atomic<DeviceStatus> status_{DeviceStatus::Inactive};
};
```

### 2.3 Threading-Modell

```
Main Thread:
├── DeviceManager (Thread-safe operations)
├── RestServer (HTTP Request handling)
└── Configuration Management

Device Threads (per device):
├── DeviceRunner::runLoop()
│   ├── Device::start()
│   ├── Periodic data publishing
│   ├── Scenario execution
│   └── Device::stop()
└── Protocol Communication
    ├── MQTT Client (async)
    └── OPC UA Server (async)
```

**Thread-Safety Mechanismen:**
- `std::mutex` für DeviceManager operations
- `std::atomic` für status flags
- Thread-safe Protocol clients
- RAII-basierte Resource Management

## 3. Python Flask API Layer

### 3.1 API-Architektur

```python
Flask Application Structure:

app.py                    # Main Flask Application
├── routes/
│   ├── devices.py       # Device Management Endpoints
│   ├── scenarios.py     # Scenario Management Endpoints
│   ├── monitoring.py    # Real-time Data Endpoints
│   └── config.py        # Configuration Endpoints
├── backend_bridge.py    # C++ Backend Communication
├── websocket_handler.py # Real-time WebSocket Handler
└── static/             # Web Interface Assets
    ├── css/
    ├── js/
    └── templates/
```

### 3.2 REST API Spezifikation

#### **Device Management Endpoints**

```yaml
/api/devices:
  GET:
    summary: List all available devices
    responses:
      200:
        schema:
          type: array
          items:
            $ref: '#/definitions/Device'

/api/devices/{device_name}:
  GET:
    summary: Get device details
    responses:
      200:
        schema:
          $ref: '#/definitions/DeviceDetail'
  
  POST:
    summary: Start device
    responses:
      200:
        description: Device started successfully
  
  DELETE:
    summary: Stop device
    responses:
      200:
        description: Device stopped successfully

/api/devices/{device_name}/status:
  GET:
    summary: Get device status
    responses:
      200:
        schema:
          $ref: '#/definitions/DeviceStatus'

/api/devices/{device_name}/scenarios:
  GET:
    summary: List device scenarios
    responses:
      200:
        schema:
          type: array
          items:
            type: string

/api/devices/{device_name}/scenarios/{scenario_name}:
  POST:
    summary: Execute scenario
    responses:
      200:
        description: Scenario executed successfully
```

#### **Data Models**

```yaml
definitions:
  Device:
    type: object
    properties:
      name:
        type: string
      class:
        type: string
      protocol:
        type: string
      status:
        $ref: '#/definitions/DeviceStatus'

  DeviceStatus:
    type: object
    properties:
      state:
        type: string
        enum: [inactive, starting, active, stopping, error]
      uptime:
        type: integer
      last_update:
        type: string
        format: date-time
      data:
        type: object

  DeviceDetail:
    allOf:
      - $ref: '#/definitions/Device'
      - type: object
        properties:
          config:
            type: object
          scenarios:
            type: array
            items:
              type: string
          history:
            type: array
            items:
              $ref: '#/definitions/DataPoint'
```

### 3.3 Backend-Bridge Implementation

```python
class BackendBridge:
    """Communication bridge to C++ REST server"""
    
    def __init__(self, backend_url="http://localhost:8081"):
        self.backend_url = backend_url
        self.session = requests.Session()
        
    def get_devices(self) -> List[Dict]:
        """Get all devices from C++ backend"""
        response = self.session.get(f"{self.backend_url}/devices")
        response.raise_for_status()
        return response.json()
        
    def start_device(self, device_name: str) -> bool:
        """Start a specific device"""
        response = self.session.post(
            f"{self.backend_url}/devices/{device_name}/start"
        )
        return response.status_code == 200
        
    def get_device_status(self, device_name: str) -> Dict:
        """Get device status and data"""
        response = self.session.get(
            f"{self.backend_url}/devices/{device_name}/status"
        )
        response.raise_for_status()
        return response.json()
```

## 4. Protocol Layer Architektur

### 4.1 Protocol Abstraction

```cpp
class ProtocolClient {
public:
    virtual ~ProtocolClient() = default;
    
    // Connection Management
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() const = 0;
    
    // Data Operations
    virtual bool publish(const std::string& topic, 
                        const std::string& data) = 0;
    virtual bool subscribe(const std::string& topic, 
                          MessageCallback callback) = 0;
    
    // Configuration
    virtual bool configure(const ProtocolConfig& config) = 0;
};
```

### 4.2 MQTT Implementation

```cpp
class MqttClient : public ProtocolClient {
private:
    mqtt::async_client client_;
    std::string broker_address_;
    int qos_level_;
    
public:
    // Connection Management
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    
    // MQTT Specific Operations
    bool publish(const std::string& topic, 
                const std::string& payload) override;
    bool subscribe(const std::string& topic, 
                  MessageCallback callback) override;
    
    // Configuration
    void setBrokerAddress(const std::string& address);
    void setQoSLevel(int qos);
    void setClientId(const std::string& client_id);
};
```

### 4.3 OPC UA Implementation

```cpp
class OpcUaServer : public ProtocolClient {
private:
    UA_Server* server_;
    UA_ServerConfig* config_;
    std::thread server_thread_;
    
public:
    // Server Management
    bool start(uint16_t port) override;
    bool stop() override;
    bool isRunning() const override;
    
    // Node Management
    bool addVariable(const std::string& name, 
                    UA_Variant* value);
    bool updateVariable(const std::string& name, 
                       UA_Variant* value);
    
    // OPC UA Specific
    bool addMethod(const std::string& name, 
                  UA_MethodCallback callback);
};
```

## 5. Datenfluss-Architektur

### 5.1 Datenfluss-Diagramm

```
Configuration Flow:
YAML File → ConfigLoader → DeviceManager → DeviceRunner → Device

Command Flow:
Web UI/CLI → Flask API → Backend Bridge → C++ REST Server → DeviceManager

Data Flow:
Device → Protocol Client → External Systems (MQTT/OPC UA)
Device → DeviceManager → REST API → Flask → WebSocket → Web UI

Event Flow:
Scenario Trigger → DeviceManager → Device::simulate() → State Change → Data Update
```

### 5.2 Real-time Data Pipeline

```python
# WebSocket Data Pipeline
class WebSocketHandler:
    def __init__(self, backend_bridge):
        self.backend_bridge = backend_bridge
        self.connected_clients = set()
        
    async def handle_client(self, websocket, path):
        self.connected_clients.add(websocket)
        try:
            while True:
                # Fetch latest data from C++ backend
                devices_data = self.backend_bridge.get_all_device_data()
                
                # Broadcast to all connected clients
                message = json.dumps({
                    'type': 'device_update',
                    'data': devices_data,
                    'timestamp': datetime.utcnow().isoformat()
                })
                
                await asyncio.gather(
                    *[client.send(message) for client in self.connected_clients],
                    return_exceptions=True
                )
                
                await asyncio.sleep(0.5)  # 2Hz update rate
                
        except websockets.exceptions.ConnectionClosed:
            self.connected_clients.remove(websocket)
```

## 6. Konfigurationsmanagement

### 6.1 YAML-Konfigurationsstruktur

```yaml
# devices.yaml
devices:
  temperature_sensor:
    class: TemperatureSensor
    protocol: mqtt
    config:
      mqtt:
        host: 127.0.0.1
        port: 1883
        topic: sensors/temperature
        qos: 1
      parameters:
        temperature:
          min: 20.0
          max: 30.0
          step: 0.5
          unit: "°C"
        humidity:
          min: 40.0
          max: 60.0
          step: 1.0
          unit: "%"
      initial_state: inactive
      publish_interval: 1000
      scenarios:
        - overheat
        - disconnect
        - sensor_drift

  pressure_valve:
    class: PressureValve
    protocol: opcua
    config:
      opcua:
        port: 4840
        endpoint: "opc.tcp://localhost:4840/valve"
      parameters:
        pressure:
          min: 0.0
          max: 10.0
          step: 0.1
          unit: "bar"
        valve_position:
          min: 0
          max: 100
          step: 1
          unit: "%"
      initial_state: inactive
      scenarios:
        - leak
        - jam
        - overpressure
```

### 6.2 Configuration Loader

```cpp
class ConfigLoader {
public:
    struct DeviceConfig {
        std::string name;
        std::string device_class;
        std::string protocol;
        std::map<std::string, std::any> config_params;
        std::vector<std::string> scenarios;
        DeviceStatus initial_state;
    };
    
    bool loadFromFile(const std::string& config_path);
    std::vector<DeviceConfig> getDeviceConfigs() const;
    bool validateConfig(const DeviceConfig& config) const;
    
private:
    YAML::Node config_root_;
    std::vector<DeviceConfig> device_configs_;
    
    DeviceConfig parseDeviceConfig(const YAML::Node& device_node);
    bool validateMqttConfig(const YAML::Node& mqtt_config);
    bool validateOpcUaConfig(const YAML::Node& opcua_config);
};
```

## 7. Performance und Skalierbarkeit

### 7.1 Performance-Ziele

| Metrik | Zielwert | Messmethode |
|--------|----------|-------------|
| Device Startup Time | <100ms | Chronometer |
| HTTP Response Time | <200ms | Flask Metrics |
| Memory Usage per Device | <10MB | Profiling |
| CPU Usage (50 devices) | <50% | System Monitor |
| Concurrent Devices | 100+ | Load Testing |
| Data Throughput | 1000 msg/s | MQTT Benchmarks |

### 7.2 Skalierungsstrategien

#### **Vertikale Skalierung**
- Multi-threading für Device Management
- Asynchrone Protocol Clients
- Memory Pool für Device Objects
- Connection Pooling für HTTP Requests

#### **Horizontale Skalierung**
- Device Distribution across Processes
- Load Balancer für Flask API
- MQTT Broker Clustering
- Database Sharding für Historical Data

### 7.3 Resource Management

```cpp
class ResourceManager {
public:
    // Memory Management
    static std::shared_ptr<DeviceBase> createDevice(
        const std::string& device_type,
        const DeviceConfig& config
    );
    
    // Connection Pooling
    std::shared_ptr<ProtocolClient> getProtocolClient(
        const std::string& protocol_type
    );
    
    // Thread Pool Management
    void submitTask(std::function<void()> task);
    
private:
    std::map<std::string, std::unique_ptr<DeviceFactory>> device_factories_;
    std::map<std::string, std::shared_ptr<ProtocolClient>> protocol_clients_;
    ThreadPool thread_pool_;
};
```

## 8. Sicherheitsarchitektur

### 8.1 Security Layer

```
┌─────────────────────────────────────────────────────────────┐
│                    Security Layer                          │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │    HTTPS    │  │  API Keys   │  │  Input      │         │
│  │ Encryption  │  │ Validation  │  │ Validation  │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │  MQTT TLS   │  │ OPC UA      │  │   Config    │         │
│  │ Encryption  │  │ Security    │  │   Signing   │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
└─────────────────────────────────────────────────────────────┘
```

### 8.2 Security Measures

**API Security:**
- JWT-basierte Authentifizierung
- Rate Limiting (100 requests/min)
- Input Validation und Sanitization
- CORS-Policy für Web-Interface

**Protocol Security:**
- MQTT: TLS 1.3 Encryption
- OPC UA: Security Policy Basic256Sha256
- REST: HTTPS Only in Production

**Configuration Security:**
- Encrypted Configuration Files
- Environment-based Secrets
- Principle of Least Privilege

---

**Nächste Review:** 01.09.2025  
**Verantwortlich:** Lead Developer  
**Genehmigt von:** CTO, Projektleiter  
