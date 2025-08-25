# Test-Strategie und Qualitätssicherung
## IoT Device Emulator

**Datum:** 25. August 2025  
**Version:** 1.0  
**QA Manager:** [Name]  
**Status:** In Bearbeitung  

---

## 1. Test-Strategie Übersicht

### 1.1 Test-Pyramide

```
                    ┌─────────────────┐
                    │   Manual Tests  │ 5%
                    │   (Exploratory) │
                ┌───┴─────────────────┴───┐
                │   End-to-End Tests     │ 15%
                │   (System Integration) │
            ┌───┴─────────────────────────┴───┐
            │     Integration Tests          │ 30%
            │   (Component Integration)      │
        ┌───┴─────────────────────────────────┴───┐
        │            Unit Tests                  │ 50%
        │        (Component Level)               │
        └─────────────────────────────────────────┘
```

### 1.2 Test-Kategorien und Prioritäten

| Test-Typ | Abdeckung | Automatisierung | Ausführung | Verantwortlich |
|----------|-----------|-----------------|------------|----------------|
| **Unit Tests** | 50% | 100% | Bei jedem Build | Entwickler |
| **Integration Tests** | 30% | 95% | CI/CD Pipeline | QA + Dev |
| **System Tests** | 15% | 80% | Nightly | QA Team |
| **Manual Tests** | 5% | 0% | Release Candidate | QA Team |

### 1.3 Test-Umgebungen

**Development Environment:**
```yaml
Purpose: Entwickler-Tests während Entwicklung
Configuration:
  - Local Development Setup
  - Mock External Services
  - Debug Mode aktiviert
  - Schnelle Feedback-Zyklen
```

**Continuous Integration Environment:**
```yaml
Purpose: Automatisierte Tests bei Code-Commits
Configuration:
  - GitHub Actions Runner
  - Docker-based Test Environment
  - Parallel Test Execution
  - Artifact Collection
```

**Staging Environment:**
```yaml
Purpose: Integration und System-Tests
Configuration:
  - Production-ähnliche Konfiguration
  - Echte MQTT Broker
  - OPC UA Test Server
  - Performance Monitoring
```

**Production-like Environment:**
```yaml
Purpose: Acceptance und Performance Tests
Configuration:
  - Identisch zu Production
  - Load Balancer
  - Monitoring Stack
  - Security Policies
```

## 2. Unit Testing

### 2.1 Unit Test Framework Setup

**C++ Unit Tests (Google Test):**
```cpp
// tests/unit/test_device_manager.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core/device_manager.hpp"
#include "mocks/mock_device.hpp"

class DeviceManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        device_manager_ = std::make_unique<DeviceManager>();
        mock_device_ = std::make_shared<MockDevice>();
    }
    
    void TearDown() override {
        device_manager_.reset();
    }
    
    std::unique_ptr<DeviceManager> device_manager_;
    std::shared_ptr<MockDevice> mock_device_;
};

TEST_F(DeviceManagerTest, RegisterDevice_ValidDevice_ReturnsTrue) {
    // Arrange
    const std::string device_name = "test_device";
    EXPECT_CALL(*mock_device_, getDeviceType())
        .WillOnce(::testing::Return("TemperatureSensor"));
    
    // Act
    bool result = device_manager_->registerDevice(device_name, mock_device_);
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(1, device_manager_->getDeviceCount());
}

TEST_F(DeviceManagerTest, RegisterDevice_DuplicateName_ReturnsFalse) {
    // Arrange
    const std::string device_name = "test_device";
    device_manager_->registerDevice(device_name, mock_device_);
    
    // Act
    bool result = device_manager_->registerDevice(device_name, mock_device_);
    
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(1, device_manager_->getDeviceCount());
}

TEST_F(DeviceManagerTest, StartDevice_ExistingDevice_CallsDeviceStart) {
    // Arrange
    const std::string device_name = "test_device";
    device_manager_->registerDevice(device_name, mock_device_);
    
    EXPECT_CALL(*mock_device_, start())
        .WillOnce(::testing::Return(true));
    
    // Act
    bool result = device_manager_->startDevice(device_name);
    
    // Assert
    EXPECT_TRUE(result);
}

// Performance Test
TEST_F(DeviceManagerTest, RegisterMultipleDevices_Performance) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Register 1000 devices
    for (int i = 0; i < 1000; ++i) {
        auto device = std::make_shared<MockDevice>();
        device_manager_->registerDevice("device_" + std::to_string(i), device);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    );
    
    // Should register 1000 devices in less than 100ms
    EXPECT_LT(duration.count(), 100);
}
```

**Mock Objects:**
```cpp
// tests/mocks/mock_device.hpp
#include <gmock/gmock.h>
#include "core/device_base.hpp"

class MockDevice : public DeviceBase {
public:
    MOCK_METHOD(bool, start, (), (override));
    MOCK_METHOD(bool, stop, (), (override));
    MOCK_METHOD(DeviceStatus, getStatus, (), (const, override));
    MOCK_METHOD(bool, configure, (const DeviceConfig& config), (override));
    MOCK_METHOD(std::vector<std::string>, getScenarios, (), (const, override));
    MOCK_METHOD(bool, simulate, (const std::string& scenario), (override));
    MOCK_METHOD(bool, publishData, (), (override));
    MOCK_METHOD(std::string, getDeviceType, (), (const, override));
};

class MockProtocolClient : public ProtocolClient {
public:
    MOCK_METHOD(bool, connect, (), (override));
    MOCK_METHOD(bool, disconnect, (), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
    MOCK_METHOD(bool, publish, (const std::string& topic, const std::string& data), (override));
    MOCK_METHOD(bool, subscribe, (const std::string& topic, MessageCallback callback), (override));
};
```

**Python Unit Tests (pytest):**
```python
# iot-emulator/monitor/tests/unit/test_device_api.py
import pytest
import json
from unittest.mock import Mock, patch, MagicMock
from monitor.api.devices import DeviceAPI
from monitor.backend_bridge import BackendBridge

class TestDeviceAPI:
    
    @pytest.fixture
    def mock_backend(self):
        return Mock(spec=BackendBridge)
    
    @pytest.fixture
    def device_api(self, mock_backend):
        return DeviceAPI(backend_bridge=mock_backend)
    
    def test_get_devices_success(self, device_api, mock_backend):
        # Arrange
        expected_devices = [
            {"name": "temp1", "type": "TemperatureSensor", "status": "active"},
            {"name": "valve1", "type": "PressureValve", "status": "inactive"}
        ]
        mock_backend.get_devices.return_value = expected_devices
        
        # Act
        result = device_api.get_devices()
        
        # Assert
        assert result == expected_devices
        mock_backend.get_devices.assert_called_once()
    
    def test_start_device_success(self, device_api, mock_backend):
        # Arrange
        device_name = "temp1"
        mock_backend.start_device.return_value = True
        
        # Act
        result = device_api.start_device(device_name)
        
        # Assert
        assert result is True
        mock_backend.start_device.assert_called_once_with(device_name)
    
    def test_start_device_not_found(self, device_api, mock_backend):
        # Arrange
        device_name = "nonexistent"
        mock_backend.start_device.side_effect = DeviceNotFoundError(device_name)
        
        # Act & Assert
        with pytest.raises(DeviceNotFoundError):
            device_api.start_device(device_name)
    
    @patch('monitor.api.devices.logger')
    def test_error_logging(self, mock_logger, device_api, mock_backend):
        # Arrange
        mock_backend.get_devices.side_effect = Exception("Backend error")
        
        # Act
        with pytest.raises(Exception):
            device_api.get_devices()
        
        # Assert
        mock_logger.error.assert_called_once()

# Property-based Testing
from hypothesis import given, strategies as st

class TestDeviceValidation:
    
    @given(st.text(min_size=1, max_size=50))
    def test_device_name_validation(self, device_name):
        # Test that any non-empty string up to 50 chars is valid device name
        result = validate_device_name(device_name)
        assert isinstance(result, bool)
    
    @given(st.integers(min_value=1, max_value=65535))
    def test_port_validation(self, port):
        # Test that valid port numbers are accepted
        result = validate_port(port)
        assert result is True
```

### 2.2 Test Coverage Requirements

**Coverage Targets:**
```yaml
Overall Coverage: ≥ 90%

Component-specific Targets:
  Core Engine: ≥ 95%
  Protocol Layer: ≥ 90%
  Device Implementations: ≥ 90%
  API Layer: ≥ 85%
  Web Interface: ≥ 80%
  CLI Tools: ≥ 75%

Critical Path Coverage: 100%
  - Device Lifecycle Management
  - Error Handling Paths
  - Security-related Code
  - Data Persistence
```

**Coverage Measurement:**
```cmake
# CMakeLists.txt - Coverage Configuration
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${TARGET} PRIVATE --coverage -O0)
        target_link_libraries(${TARGET} PRIVATE --coverage)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${TARGET} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${TARGET} PRIVATE -fprofile-instr-generate)
    endif()
endif()

# Coverage Target
add_custom_target(coverage
    COMMAND ${CMAKE_CTEST_COMMAND} --parallel ${PROCESSOR_COUNT}
    COMMAND lcov --capture --directory . --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
    COMMAND lcov --list coverage.info
    COMMAND genhtml coverage.info --output-directory coverage_html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report"
)
```

## 3. Integration Testing

### 3.1 Component Integration Tests

**Device-Protocol Integration:**
```cpp
// tests/integration/test_device_protocol_integration.cpp
class DeviceProtocolIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup real MQTT broker for testing
        mqtt_broker_ = std::make_unique<TestMqttBroker>(1883);
        mqtt_broker_->start();
        
        // Create real MQTT client
        mqtt_client_ = std::make_unique<MqttClient>();
        mqtt_client_->setBrokerAddress("localhost:1883");
        
        // Create temperature sensor with real MQTT client
        temperature_sensor_ = std::make_unique<TemperatureSensor>(mqtt_client_.get());
    }
    
    void TearDown() override {
        temperature_sensor_.reset();
        mqtt_client_.reset();
        mqtt_broker_->stop();
        mqtt_broker_.reset();
    }
    
private:
    std::unique_ptr<TestMqttBroker> mqtt_broker_;
    std::unique_ptr<MqttClient> mqtt_client_;
    std::unique_ptr<TemperatureSensor> temperature_sensor_;
};

TEST_F(DeviceProtocolIntegrationTest, PublishTemperatureData_ValidData_MessageReceived) {
    // Arrange
    std::string received_message;
    std::promise<void> message_received_promise;
    auto message_received_future = message_received_promise.get_future();
    
    // Subscribe to temperature topic
    mqtt_client_->subscribe("sensors/temperature", [&](const std::string& message) {
        received_message = message;
        message_received_promise.set_value();
    });
    
    // Configure temperature sensor
    DeviceConfig config;
    config.parameters["temperature"] = {{"min", 20.0}, {"max", 30.0}};
    temperature_sensor_->configure(config);
    
    // Act
    temperature_sensor_->start();
    temperature_sensor_->publishData();
    
    // Wait for message with timeout
    auto status = message_received_future.wait_for(std::chrono::seconds(5));
    
    // Assert
    ASSERT_EQ(std::future_status::ready, status);
    EXPECT_FALSE(received_message.empty());
    
    // Verify JSON structure
    nlohmann::json json_data = nlohmann::json::parse(received_message);
    EXPECT_TRUE(json_data.contains("timestamp"));
    EXPECT_TRUE(json_data.contains("device"));
    EXPECT_TRUE(json_data.contains("data"));
    EXPECT_TRUE(json_data["data"].contains("temperature"));
}
```

**API Integration Tests:**
```cpp
// tests/integration/test_api_integration.cpp
class APIIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start REST server in background thread
        device_manager_ = std::make_unique<DeviceManager>();
        rest_server_ = std::make_unique<RestServer>(device_manager_.get());
        
        server_thread_ = std::thread([this]() {
            rest_server_->start("localhost", 8081);
        });
        
        // Wait for server to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Setup HTTP client
        http_client_ = std::make_unique<HttpClient>("http://localhost:8081");
    }
    
    void TearDown() override {
        rest_server_->stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
    
private:
    std::unique_ptr<DeviceManager> device_manager_;
    std::unique_ptr<RestServer> rest_server_;
    std::unique_ptr<HttpClient> http_client_;
    std::thread server_thread_;
};

TEST_F(APIIntegrationTest, GetDevices_EmptyManager_ReturnsEmptyList) {
    // Act
    auto response = http_client_->get("/devices");
    
    // Assert
    EXPECT_EQ(200, response.status_code);
    
    nlohmann::json json_response = nlohmann::json::parse(response.body);
    EXPECT_TRUE(json_response.is_array());
    EXPECT_EQ(0, json_response.size());
}

TEST_F(APIIntegrationTest, StartDevice_ExistingDevice_ReturnsSuccess) {
    // Arrange - Register a device
    auto device = std::make_shared<TemperatureSensor>();
    device_manager_->registerDevice("temp1", device);
    
    // Act
    auto response = http_client_->post("/devices/temp1/start");
    
    // Assert
    EXPECT_EQ(200, response.status_code);
    
    nlohmann::json json_response = nlohmann::json::parse(response.body);
    EXPECT_EQ("success", json_response["status"]);
}
```

### 3.2 End-to-End Integration Tests

**Python-C++ Integration:**
```python
# tests/integration/test_python_cpp_integration.py
import pytest
import requests
import subprocess
import time
import os
from pathlib import Path

class TestPythonCppIntegration:
    
    @pytest.fixture(scope="class")
    def cpp_server(self):
        # Start C++ REST server
        server_path = Path(__file__).parent.parent.parent / "build" / "core" / "rest_server"
        if os.name == 'nt':
            server_path = server_path.with_suffix('.exe')
        
        process = subprocess.Popen([str(server_path)], 
                                 stdout=subprocess.PIPE, 
                                 stderr=subprocess.PIPE)
        
        # Wait for server to start
        time.sleep(2)
        
        yield process
        
        # Cleanup
        process.terminate()
        process.wait(timeout=5)
    
    @pytest.fixture
    def flask_app(self, cpp_server):
        from monitor.app import create_app
        app = create_app(testing=True)
        app.config['CPP_BACKEND_URL'] = 'http://localhost:8081'
        return app.test_client()
    
    def test_full_device_lifecycle_via_web_interface(self, flask_app):
        # Test complete device lifecycle through web interface
        
        # 1. Get initial device list (should be empty)
        response = flask_app.get('/api/devices')
        assert response.status_code == 200
        devices = response.get_json()
        assert len(devices) == 0
        
        # 2. Load configuration (this should create devices)
        response = flask_app.post('/api/config/reload')
        assert response.status_code == 200
        
        # 3. Get device list again (should have devices now)
        response = flask_app.get('/api/devices')
        assert response.status_code == 200
        devices = response.get_json()
        assert len(devices) > 0
        
        # 4. Start a device
        device_name = devices[0]['name']
        response = flask_app.post(f'/api/devices/{device_name}/start')
        assert response.status_code == 200
        
        # 5. Check device status
        response = flask_app.get(f'/api/devices/{device_name}/status')
        assert response.status_code == 200
        status = response.get_json()
        assert status['state'] == 'active'
        
        # 6. Run a scenario
        response = flask_app.get(f'/api/devices/{device_name}/scenarios')
        assert response.status_code == 200
        scenarios = response.get_json()
        
        if scenarios:
            scenario_name = scenarios[0]
            response = flask_app.post(f'/api/devices/{device_name}/scenarios/{scenario_name}')
            assert response.status_code == 200
        
        # 7. Stop device
        response = flask_app.post(f'/api/devices/{device_name}/stop')
        assert response.status_code == 200
    
    def test_concurrent_device_operations(self, flask_app):
        import threading
        import concurrent.futures
        
        # Load devices
        flask_app.post('/api/config/reload')
        
        # Get device list
        response = flask_app.get('/api/devices')
        devices = response.get_json()
        
        def start_device(device_name):
            response = flask_app.post(f'/api/devices/{device_name}/start')
            return response.status_code == 200
        
        # Start all devices concurrently
        with concurrent.futures.ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(start_device, device['name']) 
                      for device in devices[:10]]  # Limit to 10 devices
            
            results = [future.result() for future in concurrent.futures.as_completed(futures)]
        
        # All operations should succeed
        assert all(results)
```

## 4. System Testing

### 4.1 Performance Testing

**Load Testing with JMeter:**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- LoadTest.jmx -->
<jmeterTestPlan version="1.2">
  <hashTree>
    <TestPlan guiclass="TestPlanGui" testclass="TestPlan" testname="IoT Emulator Load Test">
      <stringProp name="TestPlan.arguments"></stringProp>
      <boolProp name="TestPlan.functional_mode">false</boolProp>
      <boolProp name="TestPlan.serialize_threadgroups">false</boolProp>
      <elementProp name="TestPlan.user_defined_variables" elementType="Arguments">
        <collectionProp name="Arguments.arguments"/>
      </elementProp>
      <stringProp name="TestPlan.user_define_classpath"></stringProp>
    </TestPlan>
    <hashTree>
      <!-- Thread Group for Normal Load -->
      <ThreadGroup guiclass="ThreadGroupGui" testclass="ThreadGroup" testname="Normal Load">
        <stringProp name="ThreadGroup.on_sample_error">continue</stringProp>
        <elementProp name="ThreadGroup.main_controller" elementType="LoopController">
          <boolProp name="LoopController.continue_forever">false</boolProp>
          <stringProp name="LoopController.loops">100</stringProp>
        </elementProp>
        <stringProp name="ThreadGroup.num_threads">10</stringProp>
        <stringProp name="ThreadGroup.ramp_time">60</stringProp>
        <longProp name="ThreadGroup.start_time">1377677040000</longProp>
        <longProp name="ThreadGroup.end_time">1377677040000</longProp>
        <boolProp name="ThreadGroup.scheduler">false</boolProp>
        <stringProp name="ThreadGroup.duration"></stringProp>
        <stringProp name="ThreadGroup.delay"></stringProp>
      </ThreadGroup>
      <hashTree>
        <!-- HTTP Requests -->
        <HTTPSamplerProxy guiclass="HttpTestSampleGui" testclass="HTTPSamplerProxy" testname="Get Devices">
          <elementProp name="HTTPsampler.Arguments" elementType="Arguments">
            <collectionProp name="Arguments.arguments"/>
          </elementProp>
          <stringProp name="HTTPSampler.domain">localhost</stringProp>
          <stringProp name="HTTPSampler.port">8081</stringProp>
          <stringProp name="HTTPSampler.path">/devices</stringProp>
          <stringProp name="HTTPSampler.method">GET</stringProp>
        </HTTPSamplerProxy>
        
        <!-- Response Time Assertions -->
        <ResponseAssertion guiclass="AssertionGui" testclass="ResponseAssertion" testname="Response Time Assertion">
          <collectionProp name="Asserion.test_strings">
            <stringProp name="49586">200</stringProp>
          </collectionProp>
          <stringProp name="Assertion.test_field">Assertion.response_code</stringProp>
          <boolProp name="Assertion.assume_success">false</boolProp>
          <intProp name="Assertion.test_type">1</intProp>
        </ResponseAssertion>
        
        <DurationAssertion guiclass="DurationAssertionGui" testclass="DurationAssertion" testname="Duration Assertion">
          <stringProp name="DurationAssertion.duration">200</stringProp>
        </DurationAssertion>
      </hashTree>
    </hashTree>
  </hashTree>
</jmeterTestPlan>
```

**Stress Testing Script:**
```python
# tests/performance/stress_test.py
import asyncio
import aiohttp
import time
import statistics
from dataclasses import dataclass
from typing import List

@dataclass
class TestResult:
    response_time: float
    status_code: int
    success: bool

class StressTestRunner:
    def __init__(self, base_url: str):
        self.base_url = base_url
        self.results: List[TestResult] = []
    
    async def make_request(self, session: aiohttp.ClientSession, endpoint: str) -> TestResult:
        start_time = time.time()
        try:
            async with session.get(f"{self.base_url}{endpoint}") as response:
                await response.read()  # Ensure we read the full response
                end_time = time.time()
                
                return TestResult(
                    response_time=end_time - start_time,
                    status_code=response.status,
                    success=response.status == 200
                )
        except Exception as e:
            end_time = time.time()
            return TestResult(
                response_time=end_time - start_time,
                status_code=0,
                success=False
            )
    
    async def run_concurrent_requests(self, endpoint: str, num_requests: int, concurrency: int):
        connector = aiohttp.TCPConnector(limit=concurrency)
        async with aiohttp.ClientSession(connector=connector) as session:
            semaphore = asyncio.Semaphore(concurrency)
            
            async def limited_request():
                async with semaphore:
                    return await self.make_request(session, endpoint)
            
            tasks = [limited_request() for _ in range(num_requests)]
            self.results = await asyncio.gather(*tasks)
    
    def analyze_results(self):
        successful_results = [r for r in self.results if r.success]
        response_times = [r.response_time for r in successful_results]
        
        if not response_times:
            return {
                "error": "No successful requests",
                "total_requests": len(self.results),
                "success_rate": 0
            }
        
        return {
            "total_requests": len(self.results),
            "successful_requests": len(successful_results),
            "success_rate": len(successful_results) / len(self.results),
            "avg_response_time": statistics.mean(response_times),
            "median_response_time": statistics.median(response_times),
            "p95_response_time": self.percentile(response_times, 95),
            "p99_response_time": self.percentile(response_times, 99),
            "min_response_time": min(response_times),
            "max_response_time": max(response_times)
        }
    
    @staticmethod
    def percentile(data: List[float], percentile: int) -> float:
        size = len(data)
        return sorted(data)[int(size * percentile / 100)]

# Test Execution
async def main():
    runner = StressTestRunner("http://localhost:8081")
    
    # Test scenarios
    test_scenarios = [
        {"endpoint": "/devices", "requests": 1000, "concurrency": 10},
        {"endpoint": "/devices", "requests": 2000, "concurrency": 50},
        {"endpoint": "/devices", "requests": 5000, "concurrency": 100},
    ]
    
    for scenario in test_scenarios:
        print(f"Running test: {scenario['requests']} requests, {scenario['concurrency']} concurrency")
        
        start_time = time.time()
        await runner.run_concurrent_requests(
            scenario["endpoint"], 
            scenario["requests"], 
            scenario["concurrency"]
        )
        end_time = time.time()
        
        results = runner.analyze_results()
        results["total_time"] = end_time - start_time
        results["requests_per_second"] = scenario["requests"] / results["total_time"]
        
        print(f"Results: {results}")
        print("-" * 50)

if __name__ == "__main__":
    asyncio.run(main())
```

### 4.2 Security Testing

**Security Test Automation:**
```python
# tests/security/security_tests.py
import pytest
import requests
import json
from security_scanner import SecurityScanner

class TestAPISecurity:
    
    @pytest.fixture
    def security_scanner(self):
        return SecurityScanner("http://localhost:8081")
    
    def test_sql_injection_protection(self, security_scanner):
        """Test SQL injection protection on all endpoints"""
        sql_payloads = [
            "' OR '1'='1",
            "'; DROP TABLE devices; --",
            "1' UNION SELECT * FROM users --"
        ]
        
        endpoints = ["/devices", "/devices/test", "/scenarios"]
        
        for endpoint in endpoints:
            for payload in sql_payloads:
                response = security_scanner.test_sql_injection(endpoint, payload)
                assert response.status_code != 500, f"SQL injection vulnerability at {endpoint}"
                assert "error" not in response.text.lower()
    
    def test_xss_protection(self, security_scanner):
        """Test Cross-Site Scripting protection"""
        xss_payloads = [
            "<script>alert('xss')</script>",
            "javascript:alert('xss')",
            "<img src=x onerror=alert('xss')>"
        ]
        
        for payload in xss_payloads:
            response = security_scanner.test_xss("/devices", payload)
            assert payload not in response.text, "XSS vulnerability detected"
    
    def test_rate_limiting(self):
        """Test API rate limiting"""
        url = "http://localhost:8081/devices"
        
        # Make rapid requests
        responses = []
        for i in range(150):  # Exceed rate limit
            response = requests.get(url)
            responses.append(response)
        
        # Check if rate limiting is active
        too_many_requests = [r for r in responses if r.status_code == 429]
        assert len(too_many_requests) > 0, "Rate limiting not implemented"
    
    def test_input_validation(self):
        """Test input validation and sanitization"""
        invalid_inputs = [
            {"device_name": "a" * 1000},  # Very long string
            {"device_name": ""},  # Empty string
            {"device_name": None},  # Null value
            {"device_name": {"nested": "object"}},  # Object instead of string
        ]
        
        for invalid_input in invalid_inputs:
            response = requests.post(
                "http://localhost:8081/devices/start",
                json=invalid_input
            )
            assert response.status_code in [400, 422], "Input validation failed"
    
    def test_authentication_required(self):
        """Test that protected endpoints require authentication"""
        protected_endpoints = [
            "/admin/config",
            "/admin/logs",
            "/admin/shutdown"
        ]
        
        for endpoint in protected_endpoints:
            response = requests.get(f"http://localhost:8081{endpoint}")
            assert response.status_code in [401, 403], f"Authentication not required for {endpoint}"
    
    def test_cors_configuration(self):
        """Test CORS configuration"""
        headers = {
            "Origin": "http://malicious-site.com",
            "Access-Control-Request-Method": "POST",
            "Access-Control-Request-Headers": "Content-Type"
        }
        
        response = requests.options("http://localhost:8081/devices", headers=headers)
        
        # Check CORS headers
        cors_headers = response.headers.get("Access-Control-Allow-Origin")
        assert cors_headers != "*", "CORS allows all origins (security risk)"
```

## 5. Test Automation und CI/CD Integration

### 5.1 Automated Test Pipeline

**GitHub Actions Test Workflow:**
```yaml
# .github/workflows/test.yml
name: Test Pipeline

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  unit-tests:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup C++ Environment
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake ninja-build gcc-9 g++-9
        
    - name: Setup Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
        
    - name: Install Dependencies
      run: |
        python -m pip install --upgrade pip
        pip install -r iot-emulator/monitor/requirements.txt
        pip install pytest pytest-cov pytest-xdist
        
    - name: Build C++ Tests
      run: |
        cmake -S iot-emulator -B build -G Ninja -DBUILD_TESTS=ON
        cmake --build build
        
    - name: Run C++ Unit Tests
      run: |
        cd build
        ctest --parallel 4 --output-on-failure
        
    - name: Run Python Unit Tests
      run: |
        cd iot-emulator/monitor
        pytest tests/unit/ -v --cov=. --cov-report=xml --cov-report=html
        
    - name: Upload Coverage Reports
      uses: codecov/codecov-action@v3
      with:
        files: ./iot-emulator/monitor/coverage.xml
        flags: unittests
        name: codecov-umbrella

  integration-tests:
    runs-on: ubuntu-latest
    needs: unit-tests
    
    services:
      mqtt:
        image: eclipse-mosquitto:2.0
        ports:
          - 1883:1883
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Build Application
      run: |
        cmake -S iot-emulator -B build -G Ninja
        cmake --build build
        
    - name: Start Application
      run: |
        ./build/core/rest_server &
        SERVER_PID=$!
        echo "SERVER_PID=$SERVER_PID" >> $GITHUB_ENV
        sleep 5  # Wait for server to start
        
    - name: Run Integration Tests
      run: |
        cd iot-emulator/monitor
        pytest tests/integration/ -v --tb=short
        
    - name: Cleanup
      if: always()
      run: |
        if [ ! -z "$SERVER_PID" ]; then
          kill $SERVER_PID || true
        fi

  performance-tests:
    runs-on: ubuntu-latest
    needs: integration-tests
    if: github.ref == 'refs/heads/main'
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup JMeter
      run: |
        wget https://archive.apache.org/dist/jmeter/binaries/apache-jmeter-5.5.tgz
        tar -xzf apache-jmeter-5.5.tgz
        
    - name: Build and Start Application
      run: |
        cmake -S iot-emulator -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
        cmake --build build
        ./build/core/rest_server &
        sleep 10
        
    - name: Run Performance Tests
      run: |
        apache-jmeter-5.5/bin/jmeter -n -t tests/performance/LoadTest.jmx -l results.jtl
        
    - name: Analyze Results
      run: |
        python tests/performance/analyze_results.py results.jtl
        
    - name: Upload Performance Report
      uses: actions/upload-artifact@v3
      with:
        name: performance-report
        path: performance_report.html

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
      
    - name: Run OWASP ZAP Security Scan
      uses: zaproxy/action-full-scan@v0.4.0
      with:
        target: 'http://localhost:8081'
        fail_action: true
```

### 5.2 Test Reporting und Metriken

**Test Dashboard (HTML Report):**
```python
# tests/reporting/test_dashboard.py
import jinja2
import json
from datetime import datetime
from pathlib import Path

class TestDashboard:
    def __init__(self):
        self.template_env = jinja2.Environment(
            loader=jinja2.FileSystemLoader('tests/reporting/templates')
        )
    
    def generate_dashboard(self, test_results):
        """Generate HTML test dashboard"""
        template = self.template_env.get_template('dashboard.html')
        
        # Calculate metrics
        metrics = self.calculate_metrics(test_results)
        
        # Generate HTML
        html_content = template.render(
            timestamp=datetime.now(),
            metrics=metrics,
            test_results=test_results
        )
        
        # Save to file
        output_path = Path('test_reports/dashboard.html')
        output_path.parent.mkdir(exist_ok=True)
        output_path.write_text(html_content)
        
        return output_path
    
    def calculate_metrics(self, test_results):
        """Calculate test metrics and trends"""
        total_tests = sum(r['total'] for r in test_results.values())
        passed_tests = sum(r['passed'] for r in test_results.values())
        failed_tests = sum(r['failed'] for r in test_results.values())
        
        return {
            'total_tests': total_tests,
            'passed_tests': passed_tests,
            'failed_tests': failed_tests,
            'pass_rate': (passed_tests / total_tests) * 100 if total_tests > 0 else 0,
            'coverage': test_results.get('coverage', {}).get('percentage', 0),
            'performance': test_results.get('performance', {}),
            'security': test_results.get('security', {})
        }

# Test Metrics Collection
class TestMetricsCollector:
    def __init__(self):
        self.metrics = {
            'unit_tests': {},
            'integration_tests': {},
            'performance_tests': {},
            'security_tests': {},
            'coverage': {}
        }
    
    def collect_unit_test_metrics(self, test_output):
        """Parse unit test output and extract metrics"""
        # Parse Google Test output
        lines = test_output.split('\n')
        for line in lines:
            if '[  PASSED  ]' in line:
                self.metrics['unit_tests']['passed'] = self.extract_number(line)
            elif '[  FAILED  ]' in line:
                self.metrics['unit_tests']['failed'] = self.extract_number(line)
    
    def collect_coverage_metrics(self, coverage_file):
        """Parse coverage report and extract metrics"""
        with open(coverage_file, 'r') as f:
            coverage_data = json.load(f)
        
        self.metrics['coverage'] = {
            'percentage': coverage_data.get('totals', {}).get('percent_covered', 0),
            'lines_covered': coverage_data.get('totals', {}).get('covered_lines', 0),
            'lines_total': coverage_data.get('totals', {}).get('num_statements', 0)
        }
    
    def save_metrics(self, output_file):
        """Save collected metrics to JSON file"""
        with open(output_file, 'w') as f:
            json.dump(self.metrics, f, indent=2)
```

## 6. Qualitätskontrolle und Standards

### 6.1 Code Quality Gates

**Pre-commit Quality Checks:**
```bash
#!/bin/bash
# .git/hooks/pre-commit

set -e

echo "Running pre-commit quality checks..."

# 1. Code Formatting
echo "Checking code formatting..."
clang-format --dry-run --Werror $(find . -name "*.cpp" -o -name "*.hpp" -o -name "*.h")
black --check iot-emulator/monitor/

# 2. Static Analysis
echo "Running static analysis..."
clang-tidy $(find . -name "*.cpp") -- -std=c++17
cd iot-emulator/monitor && python -m flake8 .

# 3. Security Checks
echo "Running security checks..."
cd iot-emulator/monitor && bandit -r . -x tests/

# 4. Unit Tests
echo "Running unit tests..."
cd build && ctest --parallel 4 --timeout 30

# 5. Coverage Check
echo "Checking test coverage..."
lcov --capture --directory . --output-file coverage.info
coverage_percentage=$(lcov --summary coverage.info | grep "lines" | awk '{print $2}' | sed 's/%//')
if (( $(echo "$coverage_percentage < 90" | bc -l) )); then
    echo "Error: Test coverage ($coverage_percentage%) is below required 90%"
    exit 1
fi

echo "All quality checks passed!"
```

### 6.2 Definition of Done

**Sprint Level Definition of Done:**
```yaml
Code Development:
  ✓ Code written and peer reviewed
  ✓ Unit tests written with ≥90% coverage
  ✓ Integration tests passing
  ✓ Static analysis checks passing
  ✓ Code formatted according to style guide
  ✓ No compiler warnings
  ✓ Memory leaks checked (Valgrind)
  ✓ Performance benchmarks met

Documentation:
  ✓ API documentation updated
  ✓ Code comments added for complex logic
  ✓ User documentation updated
  ✓ CHANGELOG.md updated

Quality Assurance:
  ✓ Manual testing completed
  ✓ Cross-platform testing (Windows/Linux)
  ✓ Security scan passed
  ✓ Accessibility checks (Web UI)
  ✓ Performance regression test passed

Deployment:
  ✓ CI/CD pipeline passing
  ✓ Staging deployment successful
  ✓ Database migrations (if any) tested
  ✓ Configuration changes documented
  ✓ Rollback plan prepared
```

**Release Level Definition of Done:**
```yaml
Testing:
  ✓ All automated tests passing
  ✓ End-to-end testing completed
  ✓ Load testing passed
  ✓ Security penetration testing passed
  ✓ User acceptance testing completed
  ✓ Cross-browser testing (Web UI)
  ✓ Backwards compatibility verified

Documentation:
  ✓ Release notes prepared
  ✓ User manual updated
  ✓ API documentation complete
  ✓ Installation guide updated
  ✓ Troubleshooting guide updated

Operations:
  ✓ Production deployment tested
  ✓ Monitoring and alerting configured
  ✓ Backup and recovery tested
  ✓ Support team trained
  ✓ Go-live checklist completed
```

---

**Nächste Review:** 01.09.2025  
**Verantwortlich:** QA Manager  
**Genehmigt von:** Lead Developer, Projektleiter  
