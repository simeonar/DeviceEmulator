# Monitoring und Observability
## IoT Device Emulator

**Datum:** 25. August 2025  
**Version:** 1.0  
**SRE Team:** [Name]  
**Status:** In Bearbeitung  

---

## 1. Monitoring-Architektur

### 1.1 Observability Stack Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│    Metrics      │    │      Logs       │    │     Traces      │
│   (Prometheus)  │    │ (ELK/Loki Stack)│    │    (Jaeger)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │    Grafana      │
                    │  (Visualization)│
                    └─────────────────┘
                                 │
                    ┌─────────────────┐
                    │   AlertManager  │
                    │  (Notifications)│
                    └─────────────────┘
```

### 1.2 Prometheus Configuration

**Prometheus Setup:**
```yaml
# prometheus/prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s
  external_labels:
    cluster: 'iot-emulator-prod'
    region: 'eu-west-1'

rule_files:
  - "rules/*.yml"

alerting:
  alertmanagers:
    - static_configs:
        - targets:
          - alertmanager:9093

scrape_configs:
  # IoT Emulator Core instances
  - job_name: 'iot-emulator-core'
    static_configs:
      - targets: ['iot-emulator-core-1:8081', 'iot-emulator-core-2:8081']
    metrics_path: /metrics
    scrape_interval: 10s
    scrape_timeout: 5s
    params:
      format: ['prometheus']

  # IoT Emulator Web instances
  - job_name: 'iot-emulator-web'
    static_configs:
      - targets: ['iot-emulator-web-1:8080', 'iot-emulator-web-2:8080']
    metrics_path: /metrics
    scrape_interval: 15s

  # Kubernetes service discovery
  - job_name: 'kubernetes-pods'
    kubernetes_sd_configs:
      - role: pod
        namespaces:
          names:
            - iot-emulator
    relabel_configs:
      - source_labels: [__meta_kubernetes_pod_annotation_prometheus_io_scrape]
        action: keep
        regex: true
      - source_labels: [__meta_kubernetes_pod_annotation_prometheus_io_path]
        action: replace
        target_label: __metrics_path__
        regex: (.+)
      - source_labels: [__address__, __meta_kubernetes_pod_annotation_prometheus_io_port]
        action: replace
        regex: ([^:]+)(?::\d+)?;(\d+)
        replacement: $1:$2
        target_label: __address__

  # MQTT Broker monitoring
  - job_name: 'mosquitto'
    static_configs:
      - targets: ['mosquitto-1:9001', 'mosquitto-2:9001']
    metrics_path: /metrics

  # Infrastructure monitoring
  - job_name: 'node-exporter'
    static_configs:
      - targets: ['node-exporter:9100']

  - job_name: 'redis'
    static_configs:
      - targets: ['redis-exporter:9121']

  - job_name: 'postgres'
    static_configs:
      - targets: ['postgres-exporter:9187']

  # Load Balancer monitoring
  - job_name: 'nginx'
    static_configs:
      - targets: ['nginx-exporter:9113']

  # Blackbox monitoring (external endpoints)
  - job_name: 'blackbox'
    metrics_path: /probe
    params:
      module: [http_2xx]
    static_configs:
      - targets:
        - https://iot-emulator.company.com/health
        - https://iot-emulator.company.com/api/health
    relabel_configs:
      - source_labels: [__address__]
        target_label: __param_target
      - source_labels: [__param_target]
        target_label: instance
      - target_label: __address__
        replacement: blackbox-exporter:9115
```

**Custom Metrics Collection in C++:**
```cpp
// iot-emulator/core/metrics/prometheus_metrics.hpp
#ifndef IOT_EMULATOR_CORE_METRICS_PROMETHEUS_METRICS_HPP
#define IOT_EMULATOR_CORE_METRICS_PROMETHEUS_METRICS_HPP

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#include <prometheus/exposer.h>
#include <memory>
#include <string>

namespace iot_emulator {
namespace metrics {

class PrometheusMetrics {
public:
    static PrometheusMetrics& getInstance();
    
    void initialize(const std::string& listen_address = "0.0.0.0:8081");
    void shutdown();
    
    // Device metrics
    void incrementDevicesRegistered(const std::string& device_type);
    void incrementDevicesStarted(const std::string& device_type);
    void incrementDevicesStopped(const std::string& device_type);
    void setActiveDevicesCount(const std::string& device_type, double count);
    void incrementDeviceErrors(const std::string& device_type, const std::string& error_type);
    
    // Protocol metrics
    void incrementMqttMessagesPublished(const std::string& topic);
    void incrementMqttMessagesReceived(const std::string& topic);
    void incrementOpcuaRequests(const std::string& method);
    void observeProtocolLatency(const std::string& protocol, double latency_seconds);
    
    // System metrics
    void observeRequestDuration(const std::string& endpoint, const std::string& method, double duration);
    void incrementHttpRequests(const std::string& endpoint, const std::string& method, int status_code);
    void setMemoryUsage(double bytes);
    void setCpuUsage(double percentage);
    
    // Business metrics
    void incrementScenariosExecuted(const std::string& scenario_name);
    void observeScenarioExecutionTime(const std::string& scenario_name, double duration);
    void incrementDataPointsGenerated(const std::string& device_type);

private:
    PrometheusMetrics() = default;
    ~PrometheusMetrics() = default;
    
    std::shared_ptr<prometheus::Registry> registry_;
    std::unique_ptr<prometheus::Exposer> exposer_;
    
    // Device metrics
    prometheus::Family<prometheus::Counter>* devices_registered_total_;
    prometheus::Family<prometheus::Counter>* devices_started_total_;
    prometheus::Family<prometheus::Counter>* devices_stopped_total_;
    prometheus::Family<prometheus::Gauge>* devices_active_count_;
    prometheus::Family<prometheus::Counter>* device_errors_total_;
    
    // Protocol metrics
    prometheus::Family<prometheus::Counter>* mqtt_messages_published_total_;
    prometheus::Family<prometheus::Counter>* mqtt_messages_received_total_;
    prometheus::Family<prometheus::Counter>* opcua_requests_total_;
    prometheus::Family<prometheus::Histogram>* protocol_latency_seconds_;
    
    // HTTP metrics
    prometheus::Family<prometheus::Histogram>* http_request_duration_seconds_;
    prometheus::Family<prometheus::Counter>* http_requests_total_;
    
    // System metrics
    prometheus::Gauge* memory_usage_bytes_;
    prometheus::Gauge* cpu_usage_percentage_;
    
    // Business metrics
    prometheus::Family<prometheus::Counter>* scenarios_executed_total_;
    prometheus::Family<prometheus::Histogram>* scenario_execution_time_seconds_;
    prometheus::Family<prometheus::Counter>* data_points_generated_total_;
    
    void createMetricFamilies();
};

} // namespace metrics
} // namespace iot_emulator

#endif // IOT_EMULATOR_CORE_METRICS_PROMETHEUS_METRICS_HPP
```

**Metrics Implementation:**
```cpp
// iot-emulator/core/metrics/prometheus_metrics.cpp
#include "prometheus_metrics.hpp"
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/text_serializer.h>

namespace iot_emulator {
namespace metrics {

PrometheusMetrics& PrometheusMetrics::getInstance() {
    static PrometheusMetrics instance;
    return instance;
}

void PrometheusMetrics::initialize(const std::string& listen_address) {
    registry_ = std::make_shared<prometheus::Registry>();
    exposer_ = std::make_unique<prometheus::Exposer>(listen_address, "/metrics");
    
    createMetricFamilies();
    exposer_->RegisterCollectable(registry_);
}

void PrometheusMetrics::createMetricFamilies() {
    // Device metrics
    devices_registered_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_devices_registered_total")
        .Help("Total number of devices registered")
        .Register(*registry_);
    
    devices_started_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_devices_started_total")
        .Help("Total number of devices started")
        .Register(*registry_);
    
    devices_stopped_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_devices_stopped_total")
        .Help("Total number of devices stopped")
        .Register(*registry_);
    
    devices_active_count_ = &prometheus::BuildGauge()
        .Name("iot_emulator_devices_active_count")
        .Help("Current number of active devices")
        .Register(*registry_);
    
    device_errors_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_device_errors_total")
        .Help("Total number of device errors")
        .Register(*registry_);
    
    // Protocol metrics
    mqtt_messages_published_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_mqtt_messages_published_total")
        .Help("Total number of MQTT messages published")
        .Register(*registry_);
    
    mqtt_messages_received_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_mqtt_messages_received_total")
        .Help("Total number of MQTT messages received")
        .Register(*registry_);
    
    opcua_requests_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_opcua_requests_total")
        .Help("Total number of OPC UA requests")
        .Register(*registry_);
    
    protocol_latency_seconds_ = &prometheus::BuildHistogram()
        .Name("iot_emulator_protocol_latency_seconds")
        .Help("Protocol operation latency in seconds")
        .Buckets({0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1.0, 5.0, 10.0})
        .Register(*registry_);
    
    // HTTP metrics
    http_request_duration_seconds_ = &prometheus::BuildHistogram()
        .Name("iot_emulator_http_request_duration_seconds")
        .Help("HTTP request duration in seconds")
        .Buckets({0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1.0, 5.0, 10.0})
        .Register(*registry_);
    
    http_requests_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_http_requests_total")
        .Help("Total number of HTTP requests")
        .Register(*registry_);
    
    // System metrics
    memory_usage_bytes_ = &prometheus::BuildGauge()
        .Name("iot_emulator_memory_usage_bytes")
        .Help("Current memory usage in bytes")
        .Register(*registry_);
    
    cpu_usage_percentage_ = &prometheus::BuildGauge()
        .Name("iot_emulator_cpu_usage_percentage")
        .Help("Current CPU usage percentage")
        .Register(*registry_);
    
    // Business metrics
    scenarios_executed_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_scenarios_executed_total")
        .Help("Total number of scenarios executed")
        .Register(*registry_);
    
    scenario_execution_time_seconds_ = &prometheus::BuildHistogram()
        .Name("iot_emulator_scenario_execution_time_seconds")
        .Help("Scenario execution time in seconds")
        .Buckets({1, 5, 10, 30, 60, 300, 600, 1800, 3600})
        .Register(*registry_);
    
    data_points_generated_total_ = &prometheus::BuildCounter()
        .Name("iot_emulator_data_points_generated_total")
        .Help("Total number of data points generated")
        .Register(*registry_);
}

void PrometheusMetrics::incrementDevicesRegistered(const std::string& device_type) {
    devices_registered_total_->Add({{"device_type", device_type}}).Increment();
}

void PrometheusMetrics::incrementDevicesStarted(const std::string& device_type) {
    devices_started_total_->Add({{"device_type", device_type}}).Increment();
}

void PrometheusMetrics::setActiveDevicesCount(const std::string& device_type, double count) {
    devices_active_count_->Add({{"device_type", device_type}}).Set(count);
}

void PrometheusMetrics::incrementDeviceErrors(const std::string& device_type, const std::string& error_type) {
    device_errors_total_->Add({{"device_type", device_type}, {"error_type", error_type}}).Increment();
}

void PrometheusMetrics::incrementMqttMessagesPublished(const std::string& topic) {
    mqtt_messages_published_total_->Add({{"topic", topic}}).Increment();
}

void PrometheusMetrics::observeProtocolLatency(const std::string& protocol, double latency_seconds) {
    protocol_latency_seconds_->Add({{"protocol", protocol}}).Observe(latency_seconds);
}

void PrometheusMetrics::observeRequestDuration(const std::string& endpoint, const std::string& method, double duration) {
    http_request_duration_seconds_->Add({{"endpoint", endpoint}, {"method", method}}).Observe(duration);
}

void PrometheusMetrics::incrementHttpRequests(const std::string& endpoint, const std::string& method, int status_code) {
    http_requests_total_->Add({
        {"endpoint", endpoint}, 
        {"method", method}, 
        {"status_code", std::to_string(status_code)}
    }).Increment();
}

} // namespace metrics
} // namespace iot_emulator
```

### 1.3 Python Metrics Collection

**Flask Metrics Integration:**
```python
# iot-emulator/monitor/metrics/prometheus_metrics.py
from prometheus_client import Counter, Histogram, Gauge, Info, start_http_server
from prometheus_client import CollectorRegistry, multiprocess, generate_latest
from functools import wraps
import time
import psutil
import threading
from typing import Optional, Dict, Any

class PrometheusMetrics:
    """Prometheus metrics collection for Python components."""
    
    def __init__(self, registry: Optional[CollectorRegistry] = None):
        self.registry = registry or CollectorRegistry()
        self._setup_metrics()
        self._system_metrics_thread = None
        self._running = False
    
    def _setup_metrics(self):
        """Initialize all metric collectors."""
        
        # HTTP Request metrics
        self.http_requests_total = Counter(
            'iot_emulator_web_http_requests_total',
            'Total HTTP requests received',
            ['method', 'endpoint', 'status_code'],
            registry=self.registry
        )
        
        self.http_request_duration_seconds = Histogram(
            'iot_emulator_web_http_request_duration_seconds',
            'HTTP request duration in seconds',
            ['method', 'endpoint'],
            buckets=[0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1.0, 5.0, 10.0],
            registry=self.registry
        )
        
        # Backend communication metrics
        self.backend_requests_total = Counter(
            'iot_emulator_web_backend_requests_total',
            'Total requests to backend',
            ['endpoint', 'status_code'],
            registry=self.registry
        )
        
        self.backend_request_duration_seconds = Histogram(
            'iot_emulator_web_backend_request_duration_seconds',
            'Backend request duration in seconds',
            ['endpoint'],
            buckets=[0.01, 0.05, 0.1, 0.5, 1.0, 5.0, 10.0, 30.0],
            registry=self.registry
        )
        
        # WebSocket metrics
        self.websocket_connections_current = Gauge(
            'iot_emulator_web_websocket_connections_current',
            'Current WebSocket connections',
            registry=self.registry
        )
        
        self.websocket_messages_total = Counter(
            'iot_emulator_web_websocket_messages_total',
            'Total WebSocket messages',
            ['direction'],  # sent/received
            registry=self.registry
        )
        
        # Application metrics
        self.active_sessions = Gauge(
            'iot_emulator_web_active_sessions',
            'Number of active user sessions',
            registry=self.registry
        )
        
        self.cache_operations_total = Counter(
            'iot_emulator_web_cache_operations_total',
            'Total cache operations',
            ['operation', 'result'],  # get/set/delete, hit/miss/success/error
            registry=self.registry
        )
        
        # System metrics
        self.system_memory_usage_bytes = Gauge(
            'iot_emulator_web_system_memory_usage_bytes',
            'System memory usage in bytes',
            registry=self.registry
        )
        
        self.system_cpu_usage_percent = Gauge(
            'iot_emulator_web_system_cpu_usage_percent',
            'System CPU usage percentage',
            registry=self.registry
        )
        
        # Application info
        self.app_info = Info(
            'iot_emulator_web_app',
            'Application information',
            registry=self.registry
        )
    
    def start_system_metrics_collection(self, interval: int = 30):
        """Start collecting system metrics in background thread."""
        if self._system_metrics_thread and self._system_metrics_thread.is_alive():
            return
        
        self._running = True
        self._system_metrics_thread = threading.Thread(
            target=self._collect_system_metrics,
            args=(interval,)
        )
        self._system_metrics_thread.daemon = True
        self._system_metrics_thread.start()
    
    def stop_system_metrics_collection(self):
        """Stop system metrics collection."""
        self._running = False
        if self._system_metrics_thread:
            self._system_metrics_thread.join(timeout=5)
    
    def _collect_system_metrics(self, interval: int):
        """Collect system metrics periodically."""
        while self._running:
            try:
                # Memory usage
                memory = psutil.virtual_memory()
                self.system_memory_usage_bytes.set(memory.used)
                
                # CPU usage
                cpu_percent = psutil.cpu_percent(interval=1)
                self.system_cpu_usage_percent.set(cpu_percent)
                
                time.sleep(interval)
            except Exception as e:
                print(f"Error collecting system metrics: {e}")
                time.sleep(interval)
    
    def set_app_info(self, version: str, environment: str, build_date: str):
        """Set application information."""
        self.app_info.info({
            'version': version,
            'environment': environment,
            'build_date': build_date
        })

# Global metrics instance
metrics = PrometheusMetrics()

def monitor_requests(metrics_instance: PrometheusMetrics = metrics):
    """Decorator to monitor HTTP requests."""
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            start_time = time.time()
            
            # Extract request info (Flask specific)
            from flask import request
            method = request.method
            endpoint = request.endpoint or 'unknown'
            
            try:
                response = func(*args, **kwargs)
                status_code = getattr(response, 'status_code', 200)
                
                # Record metrics
                duration = time.time() - start_time
                metrics_instance.http_requests_total.labels(
                    method=method,
                    endpoint=endpoint,
                    status_code=status_code
                ).inc()
                
                metrics_instance.http_request_duration_seconds.labels(
                    method=method,
                    endpoint=endpoint
                ).observe(duration)
                
                return response
                
            except Exception as e:
                # Record error
                duration = time.time() - start_time
                metrics_instance.http_requests_total.labels(
                    method=method,
                    endpoint=endpoint,
                    status_code=500
                ).inc()
                
                metrics_instance.http_request_duration_seconds.labels(
                    method=method,
                    endpoint=endpoint
                ).observe(duration)
                
                raise e
        
        return wrapper
    return decorator

def monitor_backend_requests(metrics_instance: PrometheusMetrics = metrics):
    """Decorator to monitor backend API requests."""
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            start_time = time.time()
            
            # Extract endpoint from function name or args
            endpoint = func.__name__
            
            try:
                result = func(*args, **kwargs)
                status_code = 200  # Assume success if no exception
                
                duration = time.time() - start_time
                metrics_instance.backend_requests_total.labels(
                    endpoint=endpoint,
                    status_code=status_code
                ).inc()
                
                metrics_instance.backend_request_duration_seconds.labels(
                    endpoint=endpoint
                ).observe(duration)
                
                return result
                
            except Exception as e:
                duration = time.time() - start_time
                status_code = getattr(e, 'status_code', 500)
                
                metrics_instance.backend_requests_total.labels(
                    endpoint=endpoint,
                    status_code=status_code
                ).inc()
                
                metrics_instance.backend_request_duration_seconds.labels(
                    endpoint=endpoint
                ).observe(duration)
                
                raise e
        
        return wrapper
    return decorator

# Flask integration
from flask import Flask, Response

def setup_metrics_endpoint(app: Flask, metrics_instance: PrometheusMetrics = metrics):
    """Setup /metrics endpoint for Prometheus scraping."""
    
    @app.route('/metrics')
    def metrics_endpoint():
        """Prometheus metrics endpoint."""
        data = generate_latest(metrics_instance.registry)
        return Response(data, mimetype='text/plain')
    
    # Setup application info
    import os
    metrics_instance.set_app_info(
        version=os.getenv('APP_VERSION', '1.0.0'),
        environment=os.getenv('ENVIRONMENT', 'development'),
        build_date=os.getenv('BUILD_DATE', 'unknown')
    )
    
    # Start system metrics collection
    metrics_instance.start_system_metrics_collection()
    
    return metrics_instance
```

## 2. Alerting und Incident Management

### 2.1 AlertManager Configuration

**AlertManager Setup:**
```yaml
# alertmanager/alertmanager.yml
global:
  smtp_smarthost: 'smtp.company.com:587'
  smtp_from: 'alerts@company.com'
  smtp_auth_username: 'alerts@company.com'
  smtp_auth_password: '${SMTP_PASSWORD}'

templates:
  - '/etc/alertmanager/templates/*.tmpl'

route:
  group_by: ['alertname', 'severity', 'service']
  group_wait: 10s
  group_interval: 10s
  repeat_interval: 4h
  receiver: 'default-receiver'
  
  routes:
    # Critical alerts go to PagerDuty immediately
    - match:
        severity: critical
      receiver: 'pagerduty-critical'
      group_wait: 0s
      repeat_interval: 5m
    
    # Warning alerts go to Slack
    - match:
        severity: warning
      receiver: 'slack-warnings'
      group_wait: 30s
      repeat_interval: 2h
    
    # Infrastructure alerts
    - match:
        category: infrastructure
      receiver: 'infrastructure-team'
    
    # Application alerts
    - match:
        category: application
      receiver: 'dev-team'

receivers:
  - name: 'default-receiver'
    email_configs:
      - to: 'ops-team@company.com'
        subject: 'IoT Emulator Alert: {{ .GroupLabels.alertname }}'
        body: |
          {{ range .Alerts }}
          Alert: {{ .Annotations.summary }}
          Description: {{ .Annotations.description }}
          Severity: {{ .Labels.severity }}
          Instance: {{ .Labels.instance }}
          {{ end }}

  - name: 'pagerduty-critical'
    pagerduty_configs:
      - routing_key: '${PAGERDUTY_ROUTING_KEY}'
        description: 'IoT Emulator Critical Alert'
        details:
          summary: '{{ .GroupLabels.alertname }}'
          urgency: 'high'

  - name: 'slack-warnings'
    slack_configs:
      - api_url: '${SLACK_WEBHOOK_URL}'
        channel: '#iot-emulator-alerts'
        title: 'IoT Emulator Warning'
        text: |
          {{ range .Alerts }}
          *Alert:* {{ .Annotations.summary }}
          *Severity:* {{ .Labels.severity }}
          *Instance:* {{ .Labels.instance }}
          *Description:* {{ .Annotations.description }}
          {{ end }}
        
  - name: 'infrastructure-team'
    email_configs:
      - to: 'infrastructure@company.com'
        subject: 'Infrastructure Alert: {{ .GroupLabels.alertname }}'
    
  - name: 'dev-team'
    email_configs:
      - to: 'dev-team@company.com'
        subject: 'Application Alert: {{ .GroupLabels.alertname }}'

inhibit_rules:
  - source_match:
      severity: 'critical'
    target_match:
      severity: 'warning'
    equal: ['alertname', 'instance']
```

### 2.2 Prometheus Alert Rules

**Core Application Alerts:**
```yaml
# prometheus/rules/iot-emulator-alerts.yml
groups:
  - name: iot-emulator-core
    rules:
      # High-level service availability
      - alert: IoTEmulatorDown
        expr: up{job="iot-emulator-core"} == 0
        for: 1m
        labels:
          severity: critical
          category: application
          service: iot-emulator
        annotations:
          summary: "IoT Emulator instance is down"
          description: "IoT Emulator instance {{ $labels.instance }} has been down for more than 1 minute."
          runbook_url: "https://docs.company.com/runbooks/iot-emulator-down"
      
      # High error rate
      - alert: HighErrorRate
        expr: |
          (
            rate(iot_emulator_http_requests_total{status_code=~"5.."}[5m]) /
            rate(iot_emulator_http_requests_total[5m])
          ) * 100 > 5
        for: 5m
        labels:
          severity: warning
          category: application
          service: iot-emulator
        annotations:
          summary: "High error rate detected"
          description: "Error rate is {{ $value }}% for instance {{ $labels.instance }}"
      
      # High response time
      - alert: HighResponseTime
        expr: |
          histogram_quantile(0.95, 
            rate(iot_emulator_http_request_duration_seconds_bucket[5m])
          ) > 2.0
        for: 5m
        labels:
          severity: warning
          category: application
          service: iot-emulator
        annotations:
          summary: "High response time detected"
          description: "95th percentile response time is {{ $value }}s for {{ $labels.instance }}"
      
      # Memory usage
      - alert: HighMemoryUsage
        expr: |
          (iot_emulator_memory_usage_bytes / (1024 * 1024 * 1024)) > 1.5
        for: 10m
        labels:
          severity: warning
          category: infrastructure
          service: iot-emulator
        annotations:
          summary: "High memory usage"
          description: "Memory usage is {{ $value }}GB for {{ $labels.instance }}"
      
      # CPU usage
      - alert: HighCPUUsage
        expr: iot_emulator_cpu_usage_percentage > 80
        for: 15m
        labels:
          severity: warning
          category: infrastructure
          service: iot-emulator
        annotations:
          summary: "High CPU usage"
          description: "CPU usage is {{ $value }}% for {{ $labels.instance }}"

  - name: iot-emulator-devices
    rules:
      # Device failures
      - alert: HighDeviceFailureRate
        expr: |
          (
            rate(iot_emulator_device_errors_total[5m]) /
            rate(iot_emulator_devices_started_total[5m])
          ) * 100 > 10
        for: 5m
        labels:
          severity: warning
          category: application
          service: iot-emulator
        annotations:
          summary: "High device failure rate"
          description: "Device failure rate is {{ $value }}% for device type {{ $labels.device_type }}"
      
      # No active devices
      - alert: NoActiveDevices
        expr: sum(iot_emulator_devices_active_count) == 0
        for: 5m
        labels:
          severity: critical
          category: application
          service: iot-emulator
        annotations:
          summary: "No active devices"
          description: "No devices are currently active in the emulator"
      
      # MQTT connection issues
      - alert: MQTTConnectionIssues
        expr: |
          rate(iot_emulator_mqtt_messages_published_total[5m]) == 0 and
          sum(iot_emulator_devices_active_count) > 0
        for: 2m
        labels:
          severity: warning
          category: application
          service: iot-emulator
        annotations:
          summary: "MQTT publishing stopped"
          description: "No MQTT messages published despite having active devices"

  - name: iot-emulator-infrastructure
    rules:
      # Database connection
      - alert: DatabaseConnectionDown
        expr: up{job="postgres"} == 0
        for: 1m
        labels:
          severity: critical
          category: infrastructure
          service: database
        annotations:
          summary: "Database connection down"
          description: "PostgreSQL database is not responding"
      
      # Redis connection
      - alert: RedisConnectionDown
        expr: up{job="redis"} == 0
        for: 1m
        labels:
          severity: warning
          category: infrastructure
          service: cache
        annotations:
          summary: "Redis connection down"
          description: "Redis cache is not responding"
      
      # MQTT Broker issues
      - alert: MQTTBrokerDown
        expr: up{job="mosquitto"} == 0
        for: 1m
        labels:
          severity: critical
          category: infrastructure
          service: mqtt
        annotations:
          summary: "MQTT broker down"
          description: "MQTT broker is not responding"
      
      # Disk space
      - alert: LowDiskSpace
        expr: |
          (
            node_filesystem_avail_bytes{mountpoint="/"} /
            node_filesystem_size_bytes{mountpoint="/"}
          ) * 100 < 10
        for: 5m
        labels:
          severity: critical
          category: infrastructure
          service: system
        annotations:
          summary: "Low disk space"
          description: "Disk usage is above 90% on {{ $labels.instance }}"

  - name: iot-emulator-business
    rules:
      # Scenario execution failures
      - alert: ScenarioExecutionFailures
        expr: |
          rate(iot_emulator_scenarios_executed_total[10m]) == 0 and
          sum(iot_emulator_devices_active_count) > 0
        for: 5m
        labels:
          severity: warning
          category: application
          service: iot-emulator
        annotations:
          summary: "No scenarios being executed"
          description: "No device scenarios have been executed despite having active devices"
      
      # Data generation stopped
      - alert: DataGenerationStopped
        expr: rate(iot_emulator_data_points_generated_total[5m]) == 0
        for: 3m
        labels:
          severity: critical
          category: application
          service: iot-emulator
        annotations:
          summary: "Data generation stopped"
          description: "No data points are being generated by the emulator"
```

### 2.3 Grafana Dashboard Configuration

**Main Dashboard JSON:**
```json
{
  "dashboard": {
    "id": null,
    "title": "IoT Device Emulator - Overview",
    "tags": ["iot-emulator", "overview"],
    "timezone": "browser",
    "panels": [
      {
        "id": 1,
        "title": "Service Status",
        "type": "stat",
        "targets": [
          {
            "expr": "up{job=\"iot-emulator-core\"}",
            "legendFormat": "Core Service",
            "refId": "A"
          },
          {
            "expr": "up{job=\"iot-emulator-web\"}",
            "legendFormat": "Web Service",
            "refId": "B"
          }
        ],
        "fieldConfig": {
          "defaults": {
            "color": {
              "mode": "thresholds"
            },
            "thresholds": {
              "steps": [
                {"color": "red", "value": null},
                {"color": "green", "value": 1}
              ]
            },
            "mappings": [
              {"options": {"0": {"text": "DOWN"}}, "type": "value"},
              {"options": {"1": {"text": "UP"}}, "type": "value"}
            ]
          }
        },
        "gridPos": {"h": 8, "w": 12, "x": 0, "y": 0}
      },
      {
        "id": 2,
        "title": "Active Devices",
        "type": "stat",
        "targets": [
          {
            "expr": "sum(iot_emulator_devices_active_count)",
            "legendFormat": "Total Active Devices",
            "refId": "A"
          }
        ],
        "fieldConfig": {
          "defaults": {
            "color": {"mode": "palette-classic"},
            "unit": "short"
          }
        },
        "gridPos": {"h": 8, "w": 12, "x": 12, "y": 0}
      },
      {
        "id": 3,
        "title": "Request Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(iot_emulator_http_requests_total[5m])",
            "legendFormat": "{{method}} {{endpoint}}",
            "refId": "A"
          }
        ],
        "yAxes": [
          {"label": "Requests/sec", "min": 0},
          {"show": false}
        ],
        "gridPos": {"h": 8, "w": 24, "x": 0, "y": 8}
      },
      {
        "id": 4,
        "title": "Response Time",
        "type": "graph",
        "targets": [
          {
            "expr": "histogram_quantile(0.50, rate(iot_emulator_http_request_duration_seconds_bucket[5m]))",
            "legendFormat": "50th percentile",
            "refId": "A"
          },
          {
            "expr": "histogram_quantile(0.95, rate(iot_emulator_http_request_duration_seconds_bucket[5m]))",
            "legendFormat": "95th percentile",
            "refId": "B"
          },
          {
            "expr": "histogram_quantile(0.99, rate(iot_emulator_http_request_duration_seconds_bucket[5m]))",
            "legendFormat": "99th percentile",
            "refId": "C"
          }
        ],
        "yAxes": [
          {"label": "Seconds", "min": 0},
          {"show": false}
        ],
        "gridPos": {"h": 8, "w": 12, "x": 0, "y": 16}
      },
      {
        "id": 5,
        "title": "Error Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(iot_emulator_http_requests_total{status_code=~\"5..\"}[5m])",
            "legendFormat": "5xx Errors",
            "refId": "A"
          },
          {
            "expr": "rate(iot_emulator_http_requests_total{status_code=~\"4..\"}[5m])",
            "legendFormat": "4xx Errors",
            "refId": "B"
          }
        ],
        "yAxes": [
          {"label": "Errors/sec", "min": 0},
          {"show": false}
        ],
        "gridPos": {"h": 8, "w": 12, "x": 12, "y": 16}
      },
      {
        "id": 6,
        "title": "Device Types Distribution",
        "type": "piechart",
        "targets": [
          {
            "expr": "iot_emulator_devices_active_count",
            "legendFormat": "{{device_type}}",
            "refId": "A"
          }
        ],
        "gridPos": {"h": 8, "w": 12, "x": 0, "y": 24}
      },
      {
        "id": 7,
        "title": "MQTT Message Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(iot_emulator_mqtt_messages_published_total[5m])",
            "legendFormat": "Published - {{topic}}",
            "refId": "A"
          },
          {
            "expr": "rate(iot_emulator_mqtt_messages_received_total[5m])",
            "legendFormat": "Received - {{topic}}",
            "refId": "B"
          }
        ],
        "yAxes": [
          {"label": "Messages/sec", "min": 0},
          {"show": false}
        ],
        "gridPos": {"h": 8, "w": 12, "x": 12, "y": 24}
      },
      {
        "id": 8,
        "title": "System Resources",
        "type": "graph",
        "targets": [
          {
            "expr": "iot_emulator_memory_usage_bytes / (1024 * 1024 * 1024)",
            "legendFormat": "Memory Usage (GB)",
            "refId": "A"
          },
          {
            "expr": "iot_emulator_cpu_usage_percentage",
            "legendFormat": "CPU Usage (%)",
            "refId": "B"
          }
        ],
        "yAxes": [
          {"label": "GB / %", "min": 0},
          {"show": false}
        ],
        "gridPos": {"h": 8, "w": 24, "x": 0, "y": 32}
      }
    ],
    "time": {"from": "now-1h", "to": "now"},
    "refresh": "10s",
    "schemaVersion": 30,
    "version": 1
  }
}
```

## 3. Logging und Distributed Tracing

### 3.1 Structured Logging Configuration

**C++ Logging (spdlog):**
```cpp
// iot-emulator/core/logging/logger.hpp
#ifndef IOT_EMULATOR_CORE_LOGGING_LOGGER_HPP
#define IOT_EMULATOR_CORE_LOGGING_LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/json.h>
#include <memory>
#include <string>

namespace iot_emulator {
namespace logging {

class Logger {
public:
    enum class Level {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        CRITICAL = 5
    };

    static void initialize(const std::string& log_file_path,
                          Level level = Level::INFO,
                          bool console_output = true,
                          bool json_format = false);
    
    static std::shared_ptr<spdlog::logger> get(const std::string& name = "default");
    
    static void shutdown();
    
    // Structured logging helpers
    template<typename... Args>
    static void info_structured(const std::string& message, Args&&... args);
    
    template<typename... Args>
    static void error_structured(const std::string& message, Args&&... args);
    
    template<typename... Args>
    static void warn_structured(const std::string& message, Args&&... args);

private:
    static std::shared_ptr<spdlog::logger> default_logger_;
    static bool initialized_;
};

// Structured logging macros
#define LOG_INFO_STRUCTURED(msg, ...) \
    iot_emulator::logging::Logger::info_structured(msg, __VA_ARGS__)

#define LOG_ERROR_STRUCTURED(msg, ...) \
    iot_emulator::logging::Logger::error_structured(msg, __VA_ARGS__)

#define LOG_WARN_STRUCTURED(msg, ...) \
    iot_emulator::logging::Logger::warn_structured(msg, __VA_ARGS__)

// Request/Response logging
#define LOG_REQUEST(request_id, method, endpoint, user_id) \
    LOG_INFO_STRUCTURED("HTTP request", \
        "request_id", request_id, \
        "method", method, \
        "endpoint", endpoint, \
        "user_id", user_id, \
        "timestamp", std::chrono::system_clock::now())

#define LOG_RESPONSE(request_id, status_code, duration_ms) \
    LOG_INFO_STRUCTURED("HTTP response", \
        "request_id", request_id, \
        "status_code", status_code, \
        "duration_ms", duration_ms, \
        "timestamp", std::chrono::system_clock::now())

// Device operation logging
#define LOG_DEVICE_OPERATION(device_name, operation, result, details) \
    LOG_INFO_STRUCTURED("Device operation", \
        "device_name", device_name, \
        "operation", operation, \
        "result", result, \
        "details", details, \
        "timestamp", std::chrono::system_clock::now())

} // namespace logging
} // namespace iot_emulator

#endif // IOT_EMULATOR_CORE_LOGGING_LOGGER_HPP
```

**Implementation:**
```cpp
// iot-emulator/core/logging/logger.cpp
#include "logger.hpp"
#include <spdlog/async.h>
#include <spdlog/pattern_formatter.h>

namespace iot_emulator {
namespace logging {

std::shared_ptr<spdlog::logger> Logger::default_logger_;
bool Logger::initialized_ = false;

void Logger::initialize(const std::string& log_file_path,
                       Level level,
                       bool console_output,
                       bool json_format) {
    if (initialized_) {
        return;
    }
    
    std::vector<spdlog::sink_ptr> sinks;
    
    // Console sink
    if (console_output) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        if (json_format) {
            console_sink->set_pattern(R"({"timestamp":"%Y-%m-%dT%H:%M:%S.%f","level":"%l","logger":"%n","message":"%v"})");
        } else {
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%n] [%l] %v");
        }
        sinks.push_back(console_sink);
    }
    
    // File sink with rotation
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        log_file_path, 100 * 1024 * 1024, 10);
    
    if (json_format) {
        file_sink->set_pattern(R"({"timestamp":"%Y-%m-%dT%H:%M:%S.%f","level":"%l","logger":"%n","message":"%v","thread_id":%t})");
    } else {
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%n] [%l] [%t] %v");
    }
    sinks.push_back(file_sink);
    
    // Create async logger
    spdlog::init_thread_pool(8192, 1);
    default_logger_ = std::make_shared<spdlog::async_logger>(
        "iot_emulator",
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );
    
    default_logger_->set_level(static_cast<spdlog::level::level_enum>(level));
    default_logger_->flush_on(spdlog::level::warn);
    
    spdlog::register_logger(default_logger_);
    spdlog::set_default_logger(default_logger_);
    
    initialized_ = true;
}

std::shared_ptr<spdlog::logger> Logger::get(const std::string& name) {
    if (name == "default" || !initialized_) {
        return default_logger_;
    }
    
    auto logger = spdlog::get(name);
    if (!logger) {
        logger = default_logger_->clone(name);
        spdlog::register_logger(logger);
    }
    
    return logger;
}

template<typename... Args>
void Logger::info_structured(const std::string& message, Args&&... args) {
    if (default_logger_) {
        default_logger_->info(fmt::format("{{\"message\":\"{}\",{}}}", 
            message, fmt::format("\"{}\":{}", args...)));
    }
}

template<typename... Args>
void Logger::error_structured(const std::string& message, Args&&... args) {
    if (default_logger_) {
        default_logger_->error(fmt::format("{{\"message\":\"{}\",{}}}", 
            message, fmt::format("\"{}\":{}", args...)));
    }
}

} // namespace logging
} // namespace iot_emulator
```

### 3.2 Distributed Tracing (Jaeger)

**OpenTelemetry C++ Integration:**
```cpp
// iot-emulator/core/tracing/tracer.hpp
#ifndef IOT_EMULATOR_CORE_TRACING_TRACER_HPP
#define IOT_EMULATOR_CORE_TRACING_TRACER_HPP

#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/exporters/jaeger/jaeger_exporter.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <memory>
#include <string>

namespace iot_emulator {
namespace tracing {

class Tracer {
public:
    static void initialize(const std::string& service_name,
                          const std::string& jaeger_endpoint);
    
    static std::shared_ptr<opentelemetry::trace::Tracer> get();
    
    static void shutdown();
    
    // Helper methods for common operations
    static std::unique_ptr<opentelemetry::trace::Span> startDeviceOperation(
        const std::string& device_name,
        const std::string& operation);
    
    static std::unique_ptr<opentelemetry::trace::Span> startHttpRequest(
        const std::string& method,
        const std::string& endpoint);
    
    static std::unique_ptr<opentelemetry::trace::Span> startProtocolOperation(
        const std::string& protocol,
        const std::string& operation);

private:
    static std::shared_ptr<opentelemetry::trace::Tracer> tracer_;
    static bool initialized_;
};

// RAII span wrapper
class ScopedSpan {
public:
    ScopedSpan(std::unique_ptr<opentelemetry::trace::Span> span)
        : span_(std::move(span)) {}
    
    ~ScopedSpan() {
        if (span_) {
            span_->End();
        }
    }
    
    void setStatus(opentelemetry::trace::StatusCode code,
                   const std::string& description = "") {
        if (span_) {
            span_->SetStatus(code, description);
        }
    }
    
    void addEvent(const std::string& name,
                  const std::map<std::string, std::string>& attributes = {}) {
        if (span_) {
            opentelemetry::common::KeyValueIterable attrs(attributes);
            span_->AddEvent(name, attrs);
        }
    }
    
    void setAttribute(const std::string& key, const std::string& value) {
        if (span_) {
            span_->SetAttribute(key, value);
        }
    }

private:
    std::unique_ptr<opentelemetry::trace::Span> span_;
};

// Tracing macros
#define TRACE_DEVICE_OPERATION(device_name, operation) \
    auto trace_span = iot_emulator::tracing::ScopedSpan( \
        iot_emulator::tracing::Tracer::startDeviceOperation(device_name, operation))

#define TRACE_HTTP_REQUEST(method, endpoint) \
    auto trace_span = iot_emulator::tracing::ScopedSpan( \
        iot_emulator::tracing::Tracer::startHttpRequest(method, endpoint))

#define TRACE_PROTOCOL_OPERATION(protocol, operation) \
    auto trace_span = iot_emulator::tracing::ScopedSpan( \
        iot_emulator::tracing::Tracer::startProtocolOperation(protocol, operation))

} // namespace tracing
} // namespace iot_emulator

#endif // IOT_EMULATOR_CORE_TRACING_TRACER_HPP
```

**Python Tracing Integration:**
```python
# iot-emulator/monitor/tracing/tracer.py
from opentelemetry import trace
from opentelemetry.exporter.jaeger.thrift import JaegerExporter
from opentelemetry.sdk.trace import TracerProvider
from opentelemetry.sdk.trace.export import BatchSpanProcessor
from opentelemetry.instrumentation.flask import FlaskInstrumentor
from opentelemetry.instrumentation.requests import RequestsInstrumentor
from functools import wraps
import time
from typing import Optional, Dict, Any

class TracingManager:
    """Centralized tracing management for Python components."""
    
    def __init__(self):
        self.tracer_provider: Optional[TracerProvider] = None
        self.tracer: Optional[trace.Tracer] = None
        self.initialized = False
    
    def initialize(self, service_name: str, jaeger_endpoint: str):
        """Initialize OpenTelemetry tracing."""
        if self.initialized:
            return
        
        # Set up tracer provider
        self.tracer_provider = TracerProvider()
        trace.set_tracer_provider(self.tracer_provider)
        
        # Create Jaeger exporter
        jaeger_exporter = JaegerExporter(
            agent_host_name="jaeger",
            agent_port=14268,
            collector_endpoint=jaeger_endpoint,
        )
        
        # Create span processor
        span_processor = BatchSpanProcessor(jaeger_exporter)
        self.tracer_provider.add_span_processor(span_processor)
        
        # Get tracer
        self.tracer = trace.get_tracer(service_name)
        
        # Auto-instrument Flask and requests
        FlaskInstrumentor().instrument()
        RequestsInstrumentor().instrument()
        
        self.initialized = True
    
    def get_tracer(self) -> trace.Tracer:
        """Get the configured tracer."""
        if not self.initialized:
            raise RuntimeError("Tracing not initialized")
        return self.tracer
    
    def shutdown(self):
        """Shutdown tracing."""
        if self.tracer_provider:
            self.tracer_provider.shutdown()

# Global tracing manager
tracing_manager = TracingManager()

def trace_function(operation_name: Optional[str] = None):
    """Decorator to automatically trace function execution."""
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            if not tracing_manager.initialized:
                return func(*args, **kwargs)
            
            tracer = tracing_manager.get_tracer()
            span_name = operation_name or f"{func.__module__}.{func.__name__}"
            
            with tracer.start_as_current_span(span_name) as span:
                try:
                    # Add function metadata
                    span.set_attribute("function.name", func.__name__)
                    span.set_attribute("function.module", func.__module__)
                    
                    # Execute function
                    start_time = time.time()
                    result = func(*args, **kwargs)
                    end_time = time.time()
                    
                    # Add execution metadata
                    span.set_attribute("function.duration", end_time - start_time)
                    span.set_attribute("function.result.type", type(result).__name__)
                    
                    return result
                    
                except Exception as e:
                    # Record exception
                    span.record_exception(e)
                    span.set_status(trace.Status(trace.StatusCode.ERROR, str(e)))
                    raise
        
        return wrapper
    return decorator

def trace_backend_request(endpoint: str):
    """Decorator to trace backend API requests."""
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            if not tracing_manager.initialized:
                return func(*args, **kwargs)
            
            tracer = tracing_manager.get_tracer()
            
            with tracer.start_as_current_span(f"backend.{endpoint}") as span:
                try:
                    span.set_attribute("backend.endpoint", endpoint)
                    span.set_attribute("backend.operation", func.__name__)
                    
                    start_time = time.time()
                    result = func(*args, **kwargs)
                    end_time = time.time()
                    
                    span.set_attribute("backend.duration", end_time - start_time)
                    span.set_attribute("backend.success", True)
                    
                    return result
                    
                except Exception as e:
                    span.record_exception(e)
                    span.set_status(trace.Status(trace.StatusCode.ERROR, str(e)))
                    span.set_attribute("backend.success", False)
                    raise
        
        return wrapper
    return decorator

class SpanContext:
    """Context manager for manual span creation."""
    
    def __init__(self, operation_name: str, attributes: Optional[Dict[str, Any]] = None):
        self.operation_name = operation_name
        self.attributes = attributes or {}
        self.span = None
        self.tracer = None
    
    def __enter__(self):
        if tracing_manager.initialized:
            self.tracer = tracing_manager.get_tracer()
            self.span = self.tracer.start_span(self.operation_name)
            
            # Set attributes
            for key, value in self.attributes.items():
                self.span.set_attribute(key, value)
        
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.span:
            if exc_type:
                self.span.record_exception(exc_val)
                self.span.set_status(trace.Status(trace.StatusCode.ERROR, str(exc_val)))
            
            self.span.end()
    
    def set_attribute(self, key: str, value: Any):
        """Set span attribute."""
        if self.span:
            self.span.set_attribute(key, value)
    
    def add_event(self, name: str, attributes: Optional[Dict[str, Any]] = None):
        """Add span event."""
        if self.span:
            self.span.add_event(name, attributes or {})

# Usage examples
@trace_function("device.management.get_devices")
def get_devices_from_backend():
    """Example of automatic function tracing."""
    pass

@trace_backend_request("devices")
def fetch_device_list():
    """Example of backend request tracing."""
    pass

# Manual span usage
def complex_operation():
    with SpanContext("complex.operation", {"user_id": "123"}) as span:
        span.add_event("operation.started")
        
        # Do some work
        time.sleep(0.1)
        
        span.set_attribute("work.completed", True)
        span.add_event("operation.completed")
```

---

**Nächste Review:** 01.09.2025  
**Verantwortlich:** SRE Team Lead  
**Genehmigt von:** Operations Manager, Security Team  
