# Build und Deployment Handbuch
## IoT Device Emulator

**Datum:** 25. August 2025  
**Version:** 1.0  
**DevOps Engineer:** [Name]  
**Status:** In Bearbeitung  

---

## 1. Entwicklungsumgebung Setup

### 1.1 System-Voraussetzungen

**Mindestanforderungen:**
- **OS:** Windows 10 (x64) / Ubuntu 20.04+ / CentOS 8+
- **CPU:** 4 Cores, 2.5 GHz
- **RAM:** 8 GB (empfohlen: 16 GB)
- **Disk:** 50 GB freier Speicher
- **Network:** Breitband-Internetverbindung

**Software-Abhängigkeiten:**
```yaml
Build Tools:
  - CMake: ≥ 3.20
  - C++ Compiler: GCC 9+ / Clang 10+ / MSVC 2019+
  - Python: ≥ 3.8
  - Node.js: ≥ 16.0 (für Web-Interface)
  - Git: ≥ 2.30

Development Tools:
  - VS Code / Visual Studio / CLion
  - Docker Desktop: ≥ 20.10
  - PowerShell: ≥ 7.0 (Windows)
```

### 1.2 Entwicklungsumgebung Initialisierung

**Schritt 1: Repository Clone**
```powershell
# Repository clonen
git clone https://github.com/simeonar/DeviceEmulator.git
cd DeviceEmulator

# Submodules initialisieren
git submodule update --init --recursive
```

**Schritt 2: Abhängigkeiten Setup**
```powershell
# vcpkg Bootstrap (Windows)
.\vcpkg\bootstrap-vcpkg.bat

# vcpkg Bootstrap (Linux)
./vcpkg/bootstrap-vcpkg.sh

# Python Virtual Environment
python -m venv .venv
.venv\Scripts\activate  # Windows
# source .venv/bin/activate  # Linux

# Python Dependencies
pip install -r iot-emulator/monitor/requirements.txt
```

**Schritt 3: External Dependencies**
```powershell
# Paho MQTT Libraries (Manual Download Required)
# 1. Download Paho MQTT C: https://github.com/eclipse/paho.mqtt.c/releases
# 2. Download Paho MQTT C++: https://github.com/eclipse/paho.mqtt.cpp/releases
# 3. Extract to external/paho/

# Directory Structure:
external/paho/
├── eclipse-paho-mqtt-c-win64-1.3.14/
│   ├── include/
│   └── lib/
└── paho.mqtt.cpp-1.3.0/
    ├── include/
    ├── lib/
    └── build/
```

**Schritt 4: Build Configuration**
```powershell
# CMake Configuration
cmake -S iot-emulator -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake

# Verify Configuration
cmake --build build --config Release --dry-run
```

### 1.3 IDE-Konfiguration

**VS Code Setup (.vscode/settings.json):**
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    ],
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "python.defaultInterpreterPath": ".venv/Scripts/python.exe",
    "python.testing.unittestEnabled": true,
    "python.testing.pytestEnabled": true,
    "files.associations": {
        "*.yaml": "yaml",
        "*.yml": "yaml"
    },
    "cmake.sourceDirectory": "${workspaceFolder}/iot-emulator"
}
```

**CLion Configuration:**
```yaml
CMake Configuration:
  - CMAKE_TOOLCHAIN_FILE: vcpkg/scripts/buildsystems/vcpkg.cmake
  - CMAKE_BUILD_TYPE: Debug/Release
  - Build Directory: build/
  
Python Interpreter:
  - Path: .venv/Scripts/python.exe
  - Package Manager: pip
```

## 2. Build-System

### 2.1 CMake Build-Struktur

**Root CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(IoTDeviceEmulator VERSION 1.0.0 LANGUAGES CXX)

# C++ Standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg Integration
find_package(PkgConfig REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# External Dependencies
set(PAHO_MQTT_C_PATH "${CMAKE_SOURCE_DIR}/../external/paho/eclipse-paho-mqtt-c-win64-1.3.14")
set(PAHO_MQTT_CPP_PATH "${CMAKE_SOURCE_DIR}/../external/paho/paho.mqtt.cpp-1.3.0")

# Include Directories
include_directories(${PAHO_MQTT_C_PATH}/include)
include_directories(${PAHO_MQTT_CPP_PATH}/include)

# Subdirectories
add_subdirectory(core)
add_subdirectory(devices)
add_subdirectory(protocols)
add_subdirectory(cli)
add_subdirectory(tests)

# Installation
install(TARGETS rest_server iot_emulator_cli
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
```

**Komponenten-spezifische CMakeFiles:**
```cmake
# core/CMakeLists.txt
add_library(core
    device_base.cpp
    device_manager.cpp
    device_runner.cpp
    config_loader.cpp
    rest_server.cpp
)

target_link_libraries(core
    yaml-cpp
    nlohmann_json::nlohmann_json
    ${PAHO_MQTT_C_PATH}/lib/paho-mqtt3c.lib
    ${PAHO_MQTT_CPP_PATH}/lib/paho-mqttpp3.lib
)

# Executable for REST Server
add_executable(rest_server rest_server_main.cpp)
target_link_libraries(rest_server core)
```

### 2.2 Build-Targets und Konfiguration

**Standard Build-Targets:**
```powershell
# Debug Build
cmake --build build --config Debug

# Release Build  
cmake --build build --config Release

# Specific Targets
cmake --build build --target rest_server
cmake --build build --target iot_emulator_cli
cmake --build build --target tests

# Clean Build
cmake --build build --target clean
```

**Build-Konfigurationen:**
```yaml
Debug Configuration:
  - Optimierung: -O0
  - Debug Symbols: Ja
  - Assertions: Aktiviert
  - Logging Level: DEBUG
  - Address Sanitizer: Optional

Release Configuration:
  - Optimierung: -O3
  - Debug Symbols: Nein
  - Assertions: Deaktiviert
  - Logging Level: INFO
  - Link-Time Optimization: Aktiviert
```

### 2.3 Cross-Platform Build

**Windows Build:**
```powershell
# Visual Studio Generator
cmake -G "Visual Studio 16 2019" -A x64 -S iot-emulator -B build

# Ninja Generator (Faster)
cmake -G "Ninja" -S iot-emulator -B build

# MSBuild
cmake --build build --config Release -- /m:4
```

**Linux Build:**
```bash
# GCC Build
export CC=gcc-9
export CXX=g++-9
cmake -S iot-emulator -B build -DCMAKE_BUILD_TYPE=Release

# Clang Build
export CC=clang-10
export CXX=clang++-10
cmake -S iot-emulator -B build -DCMAKE_BUILD_TYPE=Release

# Parallel Build
cmake --build build -- -j$(nproc)
```

## 3. Automatisierung und CI/CD

### 3.1 GitHub Actions Workflow

**.github/workflows/ci.yml:**
```yaml
name: CI/CD Pipeline

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-windows:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
        
    - name: Setup Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
        
    - name: Setup vcpkg
      run: |
        .\vcpkg\bootstrap-vcpkg.bat
        echo "VCPKG_ROOT=$env:GITHUB_WORKSPACE\vcpkg" >> $env:GITHUB_ENV
        
    - name: Cache vcpkg
      uses: actions/cache@v3
      with:
        path: vcpkg/installed
        key: vcpkg-${{ runner.os }}-${{ hashFiles('vcpkg.json') }}
        
    - name: Download Paho MQTT
      run: |
        # Download and extract Paho libraries
        # (Implementation details)
        
    - name: Configure CMake
      run: |
        cmake -S iot-emulator -B build `
        -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
        
    - name: Build
      run: cmake --build build --config Release
      
    - name: Test
      run: |
        cd build
        ctest --output-on-failure --parallel 4
        
    - name: Python Tests
      run: |
        python -m venv .venv
        .venv\Scripts\activate
        pip install -r iot-emulator\monitor\requirements.txt
        cd iot-emulator\monitor
        python -m pytest tests/
        
    - name: Package
      run: |
        cmake --build build --config Release --target package
        
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: windows-build
        path: build/IoTDeviceEmulator-*.zip

  build-linux:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
        
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake ninja-build gcc-9 g++-9
        
    - name: Setup vcpkg
      run: |
        ./vcpkg/bootstrap-vcpkg.sh
        echo "VCPKG_ROOT=$GITHUB_WORKSPACE/vcpkg" >> $GITHUB_ENV
        
    - name: Build and Test
      env:
        CC: gcc-9
        CXX: g++-9
      run: |
        cmake -S iot-emulator -B build -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DCMAKE_BUILD_TYPE=Release
        
        cmake --build build
        cd build && ctest --parallel 4

  security-scan:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Run CodeQL Analysis
      uses: github/codeql-action/init@v2
      with:
        languages: cpp, python
        
    - name: Build for Analysis
      run: |
        cmake -S iot-emulator -B build
        cmake --build build
        
    - name: Perform CodeQL Analysis
      uses: github/codeql-action/analyze@v2
```

### 3.2 Automated Testing Pipeline

**Unit Testing Strategy:**
```cmake
# tests/CMakeLists.txt
find_package(GTest REQUIRED)

# Core Tests
add_executable(core_tests
    test_device_manager.cpp
    test_device_runner.cpp
    test_config_loader.cpp
)

target_link_libraries(core_tests
    core
    GTest::gtest_main
    GTest::gmock_main
)

# Register Tests
gtest_discover_tests(core_tests)

# Coverage Target
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(core_tests PRIVATE --coverage)
    target_link_options(core_tests PRIVATE --coverage)
endif()
```

**Python Testing:**
```python
# iot-emulator/monitor/tests/test_api.py
import pytest
import requests
from unittest.mock import Mock, patch

@pytest.fixture
def app():
    from monitor.app import create_app
    app = create_app(testing=True)
    return app

def test_device_list_endpoint(client):
    response = client.get('/api/devices')
    assert response.status_code == 200
    assert 'devices' in response.json

def test_device_start_endpoint(client):
    response = client.post('/api/devices/test_device/start')
    assert response.status_code == 200

# Integration Tests
@pytest.mark.integration
def test_full_device_lifecycle():
    # Test complete device lifecycle
    pass
```

### 3.3 Quality Gates

**Code Quality Checks:**
```yaml
Quality Gates Configuration:
  
  Static Analysis:
    Tool: SonarQube
    Rules:
      - C++ Core Guidelines
      - MISRA C++ (subset)
      - Custom Security Rules
    Thresholds:
      - Critical Issues: 0
      - Major Issues: ≤ 5
      - Coverage: ≥ 90%
      - Duplicated Code: ≤ 3%
      
  Security Scanning:
    Tools:
      - CodeQL (GitHub)
      - Bandit (Python)
      - Clang Static Analyzer
    Policy: Zero high-severity vulnerabilities
    
  Performance Testing:
    Tool: Custom benchmarks
    Criteria:
      - Memory leaks: 0
      - Performance regression: ≤ 5%
      - Load test: Must pass
```

**Automated Quality Checks:**
```powershell
# Pre-commit Hooks
# .git/hooks/pre-commit
#!/bin/sh
set -e

echo "Running pre-commit checks..."

# Format Check
clang-format --dry-run --Werror $(find . -name "*.cpp" -o -name "*.hpp")

# Static Analysis
clang-tidy $(find . -name "*.cpp") -- -std=c++17

# Python Linting
cd iot-emulator/monitor
python -m flake8 .
python -m mypy .

# Unit Tests
cd ../../build
ctest --parallel 4

echo "All checks passed!"
```

## 4. Deployment-Strategien

### 4.1 Standalone Deployment

**Windows Standalone Package:**
```powershell
# Package Creation Script
$PackageName = "IoTDeviceEmulator-v$Version-win64"
$PackageDir = "package/$PackageName"

# Create Package Structure
New-Item -ItemType Directory -Path $PackageDir -Force
New-Item -ItemType Directory -Path "$PackageDir/bin" -Force
New-Item -ItemType Directory -Path "$PackageDir/config" -Force
New-Item -ItemType Directory -Path "$PackageDir/logs" -Force

# Copy Binaries
Copy-Item "build/core/Release/rest_server.exe" "$PackageDir/bin/"
Copy-Item "build/cli/Release/iot_emulator_cli.exe" "$PackageDir/bin/"

# Copy Dependencies
Copy-Item "external/paho/*/lib/*.dll" "$PackageDir/bin/"
Copy-Item "vcpkg/installed/x64-windows/bin/*.dll" "$PackageDir/bin/"

# Copy Configuration
Copy-Item "iot-emulator/config/*" "$PackageDir/config/"

# Copy Python Components
Copy-Item "iot-emulator/monitor" "$PackageDir/monitor" -Recurse

# Create Launcher Scripts
@"
@echo off
echo Starting IoT Device Emulator...
start /B bin\rest_server.exe
cd monitor
python app.py
"@ | Out-File "$PackageDir/start_emulator.bat" -Encoding ASCII

# Create Package
Compress-Archive -Path $PackageDir -DestinationPath "$PackageName.zip"
```

**Linux Package (AppImage/Snap):**
```bash
#!/bin/bash
# build_appimage.sh

# Install linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage

# Create AppDir structure
mkdir -p IoTDeviceEmulator.AppDir/usr/bin
mkdir -p IoTDeviceEmulator.AppDir/usr/lib
mkdir -p IoTDeviceEmulator.AppDir/usr/share/applications

# Copy binaries
cp build/core/rest_server IoTDeviceEmulator.AppDir/usr/bin/
cp build/cli/iot_emulator_cli IoTDeviceEmulator.AppDir/usr/bin/

# Copy libraries
cp -r iot-emulator/monitor IoTDeviceEmulator.AppDir/usr/share/

# Create desktop file
cat > IoTDeviceEmulator.AppDir/usr/share/applications/iot-device-emulator.desktop << EOF
[Desktop Entry]
Type=Application
Name=IoT Device Emulator
Exec=rest_server
Icon=iot-device-emulator
Categories=Development;
EOF

# Build AppImage
./linuxdeploy-x86_64.AppImage --appdir IoTDeviceEmulator.AppDir --output appimage
```

### 4.2 Docker Deployment

**Multi-stage Dockerfile:**
```dockerfile
# Build Stage
FROM ubuntu:20.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    ninja-build \
    gcc-9 \
    g++-9 \
    python3 \
    python3-pip \
    git \
    curl \
    pkg-config

# Set working directory
WORKDIR /build

# Copy source
COPY . .

# Build vcpkg
RUN ./vcpkg/bootstrap-vcpkg.sh

# Download Paho MQTT (automated)
RUN ./scripts/download_paho.sh

# Build application
RUN cmake -S iot-emulator -B build -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Runtime Stage
FROM ubuntu:20.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -m -s /bin/bash appuser

# Copy built application
COPY --from=builder /build/build/core/rest_server /usr/local/bin/
COPY --from=builder /build/build/cli/iot_emulator_cli /usr/local/bin/
COPY --from=builder /build/iot-emulator/monitor /app/monitor
COPY --from=builder /build/iot-emulator/config /app/config

# Install Python dependencies
COPY iot-emulator/monitor/requirements.txt /app/
RUN pip3 install -r /app/requirements.txt

# Set permissions
RUN chown -R appuser:appuser /app
USER appuser

# Expose ports
EXPOSE 8080 8081

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
  CMD curl -f http://localhost:8081/health || exit 1

# Start script
COPY docker/start.sh /app/
CMD ["/app/start.sh"]
```

**Docker Compose:**
```yaml
# docker-compose.yml
version: '3.8'

services:
  iot-emulator:
    build:
      context: .
      dockerfile: Dockerfile
    ports:
      - "8080:8080"  # Web Interface
      - "8081:8081"  # REST API
    volumes:
      - ./config:/app/config:ro
      - ./logs:/app/logs
    environment:
      - LOG_LEVEL=INFO
      - MQTT_BROKER_HOST=mqtt-broker
    depends_on:
      - mqtt-broker
    restart: unless-stopped
    
  mqtt-broker:
    image: eclipse-mosquitto:2.0
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./docker/mosquitto.conf:/mosquitto/config/mosquitto.conf:ro
    restart: unless-stopped
    
  monitoring:
    image: prom/prometheus:latest
    ports:
      - "9090:9090"
    volumes:
      - ./docker/prometheus.yml:/etc/prometheus/prometheus.yml:ro
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
      - '--storage.tsdb.path=/prometheus'
    restart: unless-stopped

volumes:
  prometheus_data:
```

### 4.3 Kubernetes Deployment

**Kubernetes Manifests:**
```yaml
# k8s/deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: iot-device-emulator
  labels:
    app: iot-device-emulator
spec:
  replicas: 2
  selector:
    matchLabels:
      app: iot-device-emulator
  template:
    metadata:
      labels:
        app: iot-device-emulator
    spec:
      containers:
      - name: emulator
        image: iot-device-emulator:latest
        ports:
        - containerPort: 8080
        - containerPort: 8081
        env:
        - name: LOG_LEVEL
          value: "INFO"
        - name: MQTT_BROKER_HOST
          value: "mqtt-broker-service"
        resources:
          limits:
            memory: "1Gi"
            cpu: "500m"
          requests:
            memory: "512Mi"
            cpu: "250m"
        livenessProbe:
          httpGet:
            path: /health
            port: 8081
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /ready
            port: 8081
          initialDelaySeconds: 5
          periodSeconds: 5
        volumeMounts:
        - name: config-volume
          mountPath: /app/config
          readOnly: true
      volumes:
      - name: config-volume
        configMap:
          name: emulator-config

---
apiVersion: v1
kind: Service
metadata:
  name: iot-device-emulator-service
spec:
  selector:
    app: iot-device-emulator
  ports:
  - name: web
    port: 8080
    targetPort: 8080
  - name: api
    port: 8081
    targetPort: 8081
  type: LoadBalancer

---
apiVersion: v1
kind: ConfigMap
metadata:
  name: emulator-config
data:
  devices.yaml: |
    devices:
      temperature_sensor:
        class: TemperatureSensor
        protocol: mqtt
        # ... configuration
```

## 5. Environment Management

### 5.1 Environment-spezifische Konfiguration

**Development Environment:**
```yaml
# config/dev/app.yaml
app:
  name: IoT Device Emulator (Development)
  log_level: DEBUG
  debug_mode: true
  
rest_server:
  host: localhost
  port: 8081
  cors_enabled: true
  
mqtt:
  broker_host: localhost
  broker_port: 1883
  
performance:
  max_devices: 10
  publish_interval: 1000ms
```

**Production Environment:**
```yaml
# config/prod/app.yaml
app:
  name: IoT Device Emulator
  log_level: INFO
  debug_mode: false
  
rest_server:
  host: 0.0.0.0
  port: 8081
  cors_enabled: false
  
mqtt:
  broker_host: production-mqtt-broker
  broker_port: 8883
  tls_enabled: true
  
performance:
  max_devices: 100
  publish_interval: 500ms
  
monitoring:
  prometheus_enabled: true
  metrics_port: 9090
```

### 5.2 Configuration Management

**Environment Configuration Loader:**
```cpp
class EnvironmentConfig {
public:
    static EnvironmentConfig& getInstance() {
        static EnvironmentConfig instance;
        return instance;
    }
    
    bool loadEnvironment(const std::string& env_name) {
        std::string config_path = "config/" + env_name + "/";
        
        // Load environment-specific configurations
        loadAppConfig(config_path + "app.yaml");
        loadDeviceConfig(config_path + "devices.yaml");
        loadSecurityConfig(config_path + "security.yaml");
        
        return validateConfiguration();
    }
    
    template<typename T>
    T get(const std::string& key) const {
        return config_tree_.get<T>(key);
    }

private:
    YAML::Node config_tree_;
};
```

**Environment Switching:**
```powershell
# Environment Management Scripts

# Development
.\scripts\set-environment.ps1 -Environment dev

# Staging  
.\scripts\set-environment.ps1 -Environment staging

# Production
.\scripts\set-environment.ps1 -Environment prod

# Custom Environment
.\scripts\set-environment.ps1 -Environment custom -ConfigPath "c:\custom\config"
```

## 6. Monitoring und Logging

### 6.1 Application Monitoring

**Prometheus Metrics Integration:**
```cpp
#include <prometheus/registry.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>

class MetricsCollector {
public:
    MetricsCollector() {
        // Create metrics
        device_count_gauge_ = prometheus::BuildGauge()
            .Name("iot_emulator_devices_total")
            .Help("Total number of registered devices")
            .Register(registry_);
            
        messages_sent_counter_ = prometheus::BuildCounter()
            .Name("iot_emulator_messages_sent_total")
            .Help("Total messages sent by devices")
            .Register(registry_);
            
        response_time_histogram_ = prometheus::BuildHistogram()
            .Name("iot_emulator_response_time_seconds")
            .Help("Response time for API requests")
            .Register(registry_);
    }
    
    void recordDeviceCount(int count) {
        device_count_gauge_.Set(count);
    }
    
    void recordMessageSent() {
        messages_sent_counter_.Increment();
    }
    
    void recordResponseTime(double seconds) {
        response_time_histogram_.Observe(seconds);
    }

private:
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Gauge& device_count_gauge_;
    prometheus::Counter& messages_sent_counter_;
    prometheus::Histogram& response_time_histogram_;
};
```

### 6.2 Structured Logging

**Logging Configuration:**
```cpp
#include <spdlog/spdlog.h>
#include <spdlog/sinks/json_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

class LoggingManager {
public:
    static void initialize(const std::string& log_level) {
        // JSON structured logging
        auto json_sink = std::make_shared<spdlog::sinks::json_sink_mt>(
            "logs/application.json"
        );
        
        // Rotating file sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/application.log", 
            1024 * 1024 * 10,  // 10MB
            5                   // 5 files
        );
        
        // Console sink for development
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        
        // Create logger with multiple sinks
        auto logger = std::make_shared<spdlog::logger>(
            "multi_sink",
            spdlog::sinks_init_list{json_sink, file_sink, console_sink}
        );
        
        // Set log level
        spdlog::level::level_enum level = spdlog::level::from_str(log_level);
        logger->set_level(level);
        
        // Set as default logger
        spdlog::set_default_logger(logger);
        
        // Set pattern for structured logs
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] [%s:%#] %v");
    }
};

// Usage in application
SPDLOG_INFO("Device started", 
    spdlog::source_location{__FILE__, __LINE__, __FUNCTION__},
    fmt::arg("device_name", "temperature_sensor_1"),
    fmt::arg("device_type", "TemperatureSensor"),
    fmt::arg("protocol", "mqtt"));
```

### 6.3 Health Checks

**Health Check Implementation:**
```cpp
class HealthChecker {
public:
    struct HealthStatus {
        bool overall_healthy;
        std::map<std::string, bool> component_health;
        std::string message;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
    };
    
    HealthStatus checkHealth() {
        HealthStatus status;
        status.timestamp = std::chrono::system_clock::now();
        
        // Check individual components
        status.component_health["device_manager"] = checkDeviceManager();
        status.component_health["mqtt_broker"] = checkMqttConnection();
        status.component_health["opcua_server"] = checkOpcUaServer();
        status.component_health["rest_api"] = checkRestApi();
        status.component_health["storage"] = checkStorage();
        
        // Overall health
        status.overall_healthy = std::all_of(
            status.component_health.begin(),
            status.component_health.end(),
            [](const auto& pair) { return pair.second; }
        );
        
        status.message = status.overall_healthy ? "All systems operational" 
                                                : "Some components degraded";
        
        return status;
    }

private:
    bool checkDeviceManager() {
        // Check if DeviceManager is responsive
        return device_manager_.isHealthy();
    }
    
    bool checkMqttConnection() {
        // Check MQTT broker connectivity
        return mqtt_client_.isConnected();
    }
    
    // ... other health checks
};
```

## 7. Performance Optimization

### 7.1 Build Optimization

**Compiler Optimization Flags:**
```cmake
# Release Build Optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /O2 /GL /DNDEBUG)
        target_link_options(${TARGET} PRIVATE /LTCG)
    else()
        target_compile_options(${TARGET} PRIVATE -O3 -DNDEBUG -march=native)
        target_link_options(${TARGET} PRIVATE -flto)
    endif()
endif()

# Profile-Guided Optimization (PGO)
option(ENABLE_PGO "Enable Profile-Guided Optimization" OFF)
if(ENABLE_PGO)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /GL)
        target_link_options(${TARGET} PRIVATE /LTCG:PGI)
    else()
        target_compile_options(${TARGET} PRIVATE -fprofile-generate)
        target_link_options(${TARGET} PRIVATE -fprofile-generate)
    endif()
endif()
```

**Memory Optimization:**
```cpp
// Custom Allocators for Performance
template<typename T>
class DeviceAllocator {
public:
    using value_type = T;
    
    T* allocate(std::size_t n) {
        return static_cast<T*>(memory_pool_.allocate(n * sizeof(T)));
    }
    
    void deallocate(T* p, std::size_t n) {
        memory_pool_.deallocate(p, n * sizeof(T));
    }

private:
    MemoryPool memory_pool_;
};

// Usage
using DeviceVector = std::vector<Device, DeviceAllocator<Device>>;
```

### 7.2 Runtime Optimization

**Thread Pool Configuration:**
```cpp
class OptimizedThreadPool {
public:
    OptimizedThreadPool(size_t num_threads = std::thread::hardware_concurrency()) 
        : stop_(false) {
        
        // Create worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { workerThread(); });
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        auto result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            tasks_.emplace([task] { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
    
    void workerThread() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                
                if (stop_ && tasks_.empty()) return;
                
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            task();
        }
    }
};
```

---

**Nächste Review:** 01.09.2025  
**Verantwortlich:** DevOps Engineer  
**Genehmigt von:** Lead Developer, Projektleiter  
