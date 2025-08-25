# Technische Spezifikation
## IoT Device Emulator

Datum: 25. August 2025
Version: 1.0
Autor: Technical Lead
Genehmigt: Projektleiter

---

## 1. Zweck und Geltungsbereich
Dieses Dokument beschreibt die technischen Spezifikationen der IoT Device Emulator Plattform: APIs, Datenmodelle, Protokolle, Performance-Ziele und Qualitätsanforderungen.

---

## 2. Schnittstellen (APIs)

### 2.1 REST API v1
- Base URL: /api/v1
- Authentifizierung: JWT (Bearer)
- Content-Type: application/json

Endpunkte (Auszug):
- GET /devices: Liste Geräte (Filter: type, status, protocol)
- POST /devices: Gerät anlegen
- GET /devices/{id}: Gerätedetails
- PUT /devices/{id}: Aktualisieren
- DELETE /devices/{id}: Löschen
- POST /devices/{id}/simulate: Simulation starten
- DELETE /devices/{id}/simulate: Simulation stoppen
- GET /simulations: Aktive Simulationen
- POST /simulations: Batch anlegen
- GET /monitoring/metrics: Systemmetriken
- GET /monitoring/health: Healthcheck

Response-Codes: 200, 201, 202, 400, 401, 403, 404, 409, 422, 429, 500

### 2.2 WebSocket
- Endpoint: /ws
- Protokoll: JSON Frames
- Topics: device/{id}/metrics, system/events

Beispiel-Message:
{
  "topic": "device/temp_001/metrics",
  "ts": 1692950400,
  "values": {"temperature": 23.7, "humidity": 65.2}
}

---

## 3. Datenmodelle

### 3.1 Device
{
  "id": "string",
  "name": "Temperature_Sensor_001",
  "type": "temperature_sensor",
  "protocol": "mqtt|opcua|http|coap",
  "configuration": { /* protokollspezifisch */ },
  "status": "active|inactive",
  "created_at": "ISO-8601",
  "updated_at": "ISO-8601",
  "created_by": "user_id"
}

Validierung:
- name: 3..64 Zeichen, unique
- type: enum
- protocol: enum

### 3.2 Simulation
{
  "id": "string",
  "name": "Production_Simulation_Test",
  "device_ids": ["..."],
  "status": "running|stopped|failed",
  "configuration": {
    "duration": 60..86400,
    "data_frequency": 100..5000,
    "scaling_factor": 0.1..10.0
  },
  "metrics": {
    "messages_sent": 0..1e9,
    "errors": 0..1e6,
    "avg_latency_ms": 0..5000
  },
  "start_time": "ISO-8601",
  "end_time": "ISO-8601|null"
}

### 3.3 User
{
  "id": "string",
  "username": "string",
  "email": "email",
  "role": "admin|user|viewer",
  "permissions": ["..."],
  "last_login": "ISO-8601"
}

---

## 4. Protokolle

### 4.1 MQTT
- Versionen: 3.1.1, 5.0
- QoS: 0,1,2 (Default 1)
- Topics: sensors/{device}/{metric}
- Auth: Username/Password, TLS client cert optional

### 4.2 OPC UA
- SecurityPolicy: Basic256Sha256
- MessageSecurityMode: SignAndEncrypt
- NodeId-Schema pro Device-Typ definiert

### 4.3 HTTP/CoAP
- REST/JSON bzw. CBOR
- Retry mit Exponential Backoff

---

## 5. Qualitätsanforderungen (NFR)
- Performance: API p95 < 500ms; 10k msgs/s, Engine <10ms Datengenerierung
- Zuverlässigkeit: 99.9% Verfügbarkeit, RPO ≤ 1h, RTO ≤ 4h
- Sicherheit: OWASP ASVS L2, GDPR, ISO 27001, IEC 62443
- Skalierbarkeit: Linear bis 1000+ Devices; Auto-Scaling
- Beobachtbarkeit: Traces, strukturierte Logs, Metriken; SLO/SLA definiert
- Wartbarkeit: Clean Architecture, 80%+ Unit Test Coverage

---

## 6. Fehler- und Ausnahmebehandlung
- Einheitliche Fehlerstruktur: { code, message, details, trace_id }
- Idempotenz-Tokens für POST /simulations
- Rate Limiting: 100 req/min/User, 1000 req/min/IP
- Circuit Breaker bei Downstream-Failure

---

## 7. Sicherheitsspezifikation
- AuthN: OAuth2/JWT (RS256), Refresh Tokens
- AuthZ: RBAC mit fein granularen Permissions
- Secrets: Vault, niemals im Repo
- Transport: TLS 1.3 überall, HSTS, HTTPS only
- Input Validation: serverseitig + schema-basiert

---

## 8. Deployment- und Betriebsanforderungen
- Container Images: distroless, CVE-Scans (Trivy)
- Kubernetes: Liveness/Readiness, PodDisruptionBudget, HPA
- Konfiguration: 12-Factor, ENV/ConfigMaps/Secrets
- Observability: Prometheus, Grafana, Loki, Tempo/Jaeger
- CI/CD: GitHub Actions, Quality Gates, Canary/Blue-Green

---

## 9. Migrations- und Versionierung
- API Versionierung: /api/v1, Deprecation Policy 6 Monate
- Datenmigrationen: Versionierte Migrationsskripte
- Backward Compatibility für Minor Releases

---

## 10. Abnahmekriterien
- Alle NFRs erfüllt (Messprotokolle vorhanden)
- Tests grün: Unit ≥ 80%, Integration ≥ 70%, E2E ≥ 60%
- Security Scan ohne Critical/High Findings
- Load Test: 1k Devices, 10k msgs/s stabil ≥ 60 Min

---

Nächste Review: 01. September 2025
