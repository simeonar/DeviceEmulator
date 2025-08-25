# Deployment und Operations Handbuch
## IoT Device Emulator

**Datum:** 25. August 2025  
**Version:** 1.0  
**DevOps Engineer:** [Name]  
**Status:** In Bearbeitung  

---

## 1. Deployment-Strategien

### 1.1 Containerisierte Deployment

**Docker Compose Setup:**
```yaml
# docker/docker-compose.prod.yml
version: '3.8'

services:
  iot-emulator-core:
    build:
      context: ../iot-emulator
      dockerfile: docker/Dockerfile.core
      target: production
    image: iot-emulator/core:${VERSION:-latest}
    container_name: iot-emulator-core
    restart: unless-stopped
    environment:
      - LOG_LEVEL=${LOG_LEVEL:-INFO}
      - MAX_DEVICES=${MAX_DEVICES:-1000}
      - MQTT_BROKER_URL=${MQTT_BROKER_URL:-mqtt://mosquitto:1883}
      - OPC_UA_PORT=${OPC_UA_PORT:-4840}
    ports:
      - "8081:8081"  # REST API
      - "4840:4840"  # OPC UA
    volumes:
      - ./config:/app/config:ro
      - ./logs:/app/logs
      - device_data:/app/data
    depends_on:
      - mosquitto
      - prometheus
    networks:
      - iot-network
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8081/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 40s
    deploy:
      resources:
        limits:
          memory: 2G
          cpus: '1.0'
        reservations:
          memory: 512M
          cpus: '0.25'

  iot-emulator-web:
    build:
      context: ../iot-emulator
      dockerfile: docker/Dockerfile.web
      target: production
    image: iot-emulator/web:${VERSION:-latest}
    container_name: iot-emulator-web
    restart: unless-stopped
    environment:
      - FLASK_ENV=production
      - SECRET_KEY=${SECRET_KEY}
      - BACKEND_URL=http://iot-emulator-core:8081
      - REDIS_URL=redis://redis:6379/0
    ports:
      - "8080:8080"
    volumes:
      - ./logs:/app/logs
    depends_on:
      - iot-emulator-core
      - redis
    networks:
      - iot-network
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 5s
      retries: 3
    deploy:
      resources:
        limits:
          memory: 1G
          cpus: '0.5'
        reservations:
          memory: 256M
          cpus: '0.1'

  mosquitto:
    image: eclipse-mosquitto:2.0
    container_name: mosquitto
    restart: unless-stopped
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mosquitto/mosquitto.conf:/mosquitto/config/mosquitto.conf:ro
      - ./mosquitto/passwd:/mosquitto/config/passwd:ro
      - mosquitto_data:/mosquitto/data
      - mosquitto_logs:/mosquitto/log
    networks:
      - iot-network

  redis:
    image: redis:7-alpine
    container_name: redis
    restart: unless-stopped
    ports:
      - "6379:6379"
    volumes:
      - redis_data:/data
    networks:
      - iot-network
    command: redis-server --appendonly yes --requirepass ${REDIS_PASSWORD}

  prometheus:
    image: prom/prometheus:latest
    container_name: prometheus
    restart: unless-stopped
    ports:
      - "9090:9090"
    volumes:
      - ./prometheus/prometheus.yml:/etc/prometheus/prometheus.yml:ro
      - prometheus_data:/prometheus
    networks:
      - iot-network
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
      - '--storage.tsdb.path=/prometheus'
      - '--web.console.libraries=/etc/prometheus/console_libraries'
      - '--web.console.templates=/etc/prometheus/consoles'
      - '--web.enable-lifecycle'

  grafana:
    image: grafana/grafana:latest
    container_name: grafana
    restart: unless-stopped
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=${GRAFANA_ADMIN_PASSWORD}
      - GF_USERS_ALLOW_SIGN_UP=false
    volumes:
      - grafana_data:/var/lib/grafana
      - ./grafana/provisioning:/etc/grafana/provisioning:ro
      - ./grafana/dashboards:/var/lib/grafana/dashboards:ro
    networks:
      - iot-network

  nginx:
    image: nginx:alpine
    container_name: nginx-proxy
    restart: unless-stopped
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx/nginx.conf:/etc/nginx/nginx.conf:ro
      - ./nginx/ssl:/etc/nginx/ssl:ro
      - nginx_logs:/var/log/nginx
    depends_on:
      - iot-emulator-web
      - grafana
    networks:
      - iot-network

volumes:
  device_data:
  mosquitto_data:
  mosquitto_logs:
  redis_data:
  prometheus_data:
  grafana_data:
  nginx_logs:

networks:
  iot-network:
    driver: bridge
    ipam:
      config:
        - subnet: 172.20.0.0/16
```

**Production Dockerfile (Multi-stage):**
```dockerfile
# docker/Dockerfile.core
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
WORKDIR /opt
RUN git clone https://github.com/Microsoft/vcpkg.git \
    && cd vcpkg \
    && ./bootstrap-vcpkg.sh \
    && ./vcpkg integrate install

# Copy source code
WORKDIR /app
COPY iot-emulator/ ./iot-emulator/
COPY CMakeLists.txt ./

# Build application
RUN cmake -S . -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTS=OFF \
    -DBUILD_BENCHMARKS=OFF \
    && cmake --build build --parallel $(nproc)

# Production stage
FROM ubuntu:22.04 AS production

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd -r iotuser && useradd -r -g iotuser iotuser

# Copy built application
COPY --from=builder /app/build/core/rest_server /usr/local/bin/
COPY --from=builder /app/build/cli/iot_emulator_cli /usr/local/bin/
COPY --from=builder /app/iot-emulator/config/ /app/config/

# Create necessary directories
RUN mkdir -p /app/logs /app/data \
    && chown -R iotuser:iotuser /app

# Switch to non-root user
USER iotuser

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8081/health || exit 1

# Expose ports
EXPOSE 8081 4840

# Set working directory
WORKDIR /app

# Start application
CMD ["rest_server", "--config", "/app/config/server.yaml"]
```

### 1.2 Kubernetes Deployment

**Namespace und RBAC:**
```yaml
# k8s/namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: iot-emulator
  labels:
    name: iot-emulator
    purpose: device-simulation

---
apiVersion: v1
kind: ServiceAccount
metadata:
  name: iot-emulator-sa
  namespace: iot-emulator

---
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  namespace: iot-emulator
  name: iot-emulator-role
rules:
- apiGroups: [""]
  resources: ["configmaps", "secrets", "services", "endpoints"]
  verbs: ["get", "list", "watch"]
- apiGroups: ["apps"]
  resources: ["deployments", "replicasets"]
  verbs: ["get", "list", "watch"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: iot-emulator-rolebinding
  namespace: iot-emulator
subjects:
- kind: ServiceAccount
  name: iot-emulator-sa
  namespace: iot-emulator
roleRef:
  kind: Role
  name: iot-emulator-role
  apiGroup: rbac.authorization.k8s.io
```

**ConfigMap und Secrets:**
```yaml
# k8s/configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: iot-emulator-config
  namespace: iot-emulator
data:
  server.yaml: |
    server:
      host: "0.0.0.0"
      port: 8081
      threads: 4
      
    logging:
      level: "INFO"
      file: "/app/logs/iot-emulator.log"
      max_size: "100MB"
      max_files: 5
      
    mqtt:
      broker_url: "mqtt://mosquitto-service:1883"
      client_id: "iot-emulator-core"
      keep_alive: 60
      clean_session: true
      
    opcua:
      port: 4840
      endpoint: "opc.tcp://0.0.0.0:4840"
      security_mode: "None"
      
    devices:
      max_devices: 1000
      data_retention_days: 30
      scenario_timeout: 300

  devices.yaml: |
    devices:
      - name: "temperature_sensor_1"
        type: "TemperatureSensor"
        config:
          mqtt_topic: "sensors/temperature/1"
          update_interval: 10
          min_value: -20.0
          max_value: 50.0
        scenarios:
          - name: "normal_operation"
            duration: 3600
            parameters:
              temperature_range: [18, 25]
          - name: "high_temperature_alert"
            duration: 300
            parameters:
              temperature_range: [45, 50]

---
apiVersion: v1
kind: Secret
metadata:
  name: iot-emulator-secrets
  namespace: iot-emulator
type: Opaque
data:
  redis_password: {{ .Values.redis.password | b64enc }}
  mqtt_username: {{ .Values.mqtt.username | b64enc }}
  mqtt_password: {{ .Values.mqtt.password | b64enc }}
  grafana_admin_password: {{ .Values.grafana.adminPassword | b64enc }}
```

**Core Application Deployment:**
```yaml
# k8s/deployment-core.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: iot-emulator-core
  namespace: iot-emulator
  labels:
    app: iot-emulator-core
    component: backend
spec:
  replicas: 2
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxUnavailable: 1
      maxSurge: 1
  selector:
    matchLabels:
      app: iot-emulator-core
  template:
    metadata:
      labels:
        app: iot-emulator-core
        component: backend
      annotations:
        prometheus.io/scrape: "true"
        prometheus.io/port: "8081"
        prometheus.io/path: "/metrics"
    spec:
      serviceAccountName: iot-emulator-sa
      securityContext:
        runAsNonRoot: true
        runAsUser: 1000
        fsGroup: 1000
      containers:
      - name: iot-emulator-core
        image: iot-emulator/core:v1.0.0
        imagePullPolicy: IfNotPresent
        ports:
        - containerPort: 8081
          name: http
          protocol: TCP
        - containerPort: 4840
          name: opcua
          protocol: TCP
        env:
        - name: LOG_LEVEL
          value: "INFO"
        - name: MQTT_BROKER_URL
          value: "mqtt://mosquitto-service:1883"
        - name: REDIS_URL
          value: "redis://redis-service:6379/0"
        - name: REDIS_PASSWORD
          valueFrom:
            secretKeyRef:
              name: iot-emulator-secrets
              key: redis_password
        volumeMounts:
        - name: config-volume
          mountPath: /app/config
          readOnly: true
        - name: logs-volume
          mountPath: /app/logs
        - name: data-volume
          mountPath: /app/data
        resources:
          requests:
            memory: "512Mi"
            cpu: "250m"
          limits:
            memory: "2Gi"
            cpu: "1000m"
        livenessProbe:
          httpGet:
            path: /health
            port: http
          initialDelaySeconds: 30
          periodSeconds: 30
          timeoutSeconds: 10
          failureThreshold: 3
        readinessProbe:
          httpGet:
            path: /ready
            port: http
          initialDelaySeconds: 5
          periodSeconds: 10
          timeoutSeconds: 5
          failureThreshold: 3
        startupProbe:
          httpGet:
            path: /health
            port: http
          initialDelaySeconds: 10
          periodSeconds: 10
          timeoutSeconds: 5
          failureThreshold: 30
      volumes:
      - name: config-volume
        configMap:
          name: iot-emulator-config
      - name: logs-volume
        emptyDir: {}
      - name: data-volume
        persistentVolumeClaim:
          claimName: iot-emulator-data-pvc
      tolerations:
      - key: "node.kubernetes.io/not-ready"
        operator: "Exists"
        effect: "NoExecute"
        tolerationSeconds: 300
      - key: "node.kubernetes.io/unreachable"
        operator: "Exists"
        effect: "NoExecute"
        tolerationSeconds: 300

---
apiVersion: v1
kind: Service
metadata:
  name: iot-emulator-core-service
  namespace: iot-emulator
  labels:
    app: iot-emulator-core
spec:
  type: ClusterIP
  ports:
  - port: 8081
    targetPort: http
    protocol: TCP
    name: http
  - port: 4840
    targetPort: opcua
    protocol: TCP
    name: opcua
  selector:
    app: iot-emulator-core

---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: iot-emulator-data-pvc
  namespace: iot-emulator
spec:
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 10Gi
  storageClassName: fast-ssd
```

**Horizontal Pod Autoscaler:**
```yaml
# k8s/hpa.yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: iot-emulator-core-hpa
  namespace: iot-emulator
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: iot-emulator-core
  minReplicas: 2
  maxReplicas: 10
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 80
  - type: Pods
    pods:
      metric:
        name: active_devices_count
      target:
        type: AverageValue
        averageValue: "500"
  behavior:
    scaleUp:
      stabilizationWindowSeconds: 300
      policies:
      - type: Percent
        value: 100
        periodSeconds: 15
      - type: Pods
        value: 2
        periodSeconds: 60
      selectPolicy: Max
    scaleDown:
      stabilizationWindowSeconds: 300
      policies:
      - type: Percent
        value: 10
        periodSeconds: 60
      selectPolicy: Min
```

### 1.3 Helm Chart Deployment

**Chart.yaml:**
```yaml
# helm/iot-emulator/Chart.yaml
apiVersion: v2
name: iot-emulator
description: IoT Device Emulator - Comprehensive device simulation platform
type: application
version: 1.0.0
appVersion: "1.0.0"
keywords:
  - iot
  - simulation
  - mqtt
  - opcua
  - devices
home: https://github.com/simeonar/DeviceEmulator
sources:
  - https://github.com/simeonar/DeviceEmulator
maintainers:
  - name: DevOps Team
    email: devops@company.com
dependencies:
  - name: redis
    version: "17.3.7"
    repository: "https://charts.bitnami.com/bitnami"
    condition: redis.enabled
  - name: postgresql
    version: "11.9.13"
    repository: "https://charts.bitnami.com/bitnami"
    condition: postgresql.enabled
  - name: prometheus
    version: "15.12.0"
    repository: "https://prometheus-community.github.io/helm-charts"
    condition: monitoring.prometheus.enabled
```

**Values.yaml:**
```yaml
# helm/iot-emulator/values.yaml
# Default values for iot-emulator
replicaCount: 2

image:
  core:
    repository: iot-emulator/core
    tag: "v1.0.0"
    pullPolicy: IfNotPresent
  web:
    repository: iot-emulator/web
    tag: "v1.0.0"
    pullPolicy: IfNotPresent

nameOverride: ""
fullnameOverride: ""

serviceAccount:
  create: true
  annotations: {}
  name: ""

podAnnotations:
  prometheus.io/scrape: "true"
  prometheus.io/port: "8081"
  prometheus.io/path: "/metrics"

podSecurityContext:
  runAsNonRoot: true
  runAsUser: 1000
  fsGroup: 1000

securityContext:
  capabilities:
    drop:
    - ALL
  readOnlyRootFilesystem: false
  runAsNonRoot: true
  runAsUser: 1000

service:
  core:
    type: ClusterIP
    port: 8081
    targetPort: http
  web:
    type: ClusterIP
    port: 8080
    targetPort: http

ingress:
  enabled: true
  className: "nginx"
  annotations:
    cert-manager.io/cluster-issuer: "letsencrypt-prod"
    nginx.ingress.kubernetes.io/rate-limit: "100"
    nginx.ingress.kubernetes.io/rate-limit-window: "1m"
  hosts:
    - host: iot-emulator.company.com
      paths:
        - path: /
          pathType: Prefix
          service: web
        - path: /api
          pathType: Prefix
          service: core
  tls:
    - secretName: iot-emulator-tls
      hosts:
        - iot-emulator.company.com

resources:
  core:
    limits:
      cpu: 1000m
      memory: 2Gi
    requests:
      cpu: 250m
      memory: 512Mi
  web:
    limits:
      cpu: 500m
      memory: 1Gi
    requests:
      cpu: 100m
      memory: 256Mi

autoscaling:
  enabled: true
  minReplicas: 2
  maxReplicas: 10
  targetCPUUtilizationPercentage: 70
  targetMemoryUtilizationPercentage: 80

nodeSelector: {}

tolerations: []

affinity:
  podAntiAffinity:
    preferredDuringSchedulingIgnoredDuringExecution:
    - weight: 100
      podAffinityTerm:
        labelSelector:
          matchExpressions:
          - key: app.kubernetes.io/name
            operator: In
            values:
            - iot-emulator
        topologyKey: kubernetes.io/hostname

persistence:
  enabled: true
  storageClass: "fast-ssd"
  accessMode: ReadWriteOnce
  size: 10Gi

config:
  server:
    host: "0.0.0.0"
    port: 8081
    threads: 4
  
  logging:
    level: "INFO"
    file: "/app/logs/iot-emulator.log"
    max_size: "100MB"
    max_files: 5
  
  mqtt:
    broker_url: "mqtt://mosquitto:1883"
    username: ""
    password: ""
    client_id: "iot-emulator"
    keep_alive: 60
    clean_session: true
  
  devices:
    max_devices: 1000
    data_retention_days: 30
    scenario_timeout: 300

# External dependencies
redis:
  enabled: true
  auth:
    enabled: true
    password: "redis-secret-password"
  master:
    persistence:
      enabled: true
      size: 8Gi

postgresql:
  enabled: false
  
mosquitto:
  enabled: true
  persistence:
    enabled: true
    size: 1Gi
  config:
    password_file: /mosquitto/config/passwd
    
monitoring:
  prometheus:
    enabled: true
  grafana:
    enabled: true
    adminPassword: "admin-secret-password"
```

### 1.4 CI/CD Pipeline für Deployment

**GitHub Actions Deployment Pipeline:**
```yaml
# .github/workflows/deploy.yml
name: Deploy to Production

on:
  push:
    tags:
      - 'v*'
  workflow_dispatch:
    inputs:
      environment:
        description: 'Environment to deploy to'
        required: true
        default: 'staging'
        type: choice
        options:
        - staging
        - production

env:
  REGISTRY: ghcr.io
  IMAGE_NAME: ${{ github.repository }}

jobs:
  build-and-push:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      packages: write
    
    steps:
    - name: Checkout repository
      uses: actions/checkout@v3
    
    - name: Set up Docker Buildx
      uses: docker/setup-buildx-action@v2
    
    - name: Log in to Container Registry
      uses: docker/login-action@v2
      with:
        registry: ${{ env.REGISTRY }}
        username: ${{ github.actor }}
        password: ${{ secrets.GITHUB_TOKEN }}
    
    - name: Extract metadata (tags, labels)
      id: meta-core
      uses: docker/metadata-action@v4
      with:
        images: ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}/core
        tags: |
          type=ref,event=branch
          type=ref,event=tag
          type=sha,prefix={{branch}}-
    
    - name: Build and push Core image
      uses: docker/build-push-action@v4
      with:
        context: .
        file: ./docker/Dockerfile.core
        push: true
        tags: ${{ steps.meta-core.outputs.tags }}
        labels: ${{ steps.meta-core.outputs.labels }}
        cache-from: type=gha
        cache-to: type=gha,mode=max
        platforms: linux/amd64,linux/arm64
    
    - name: Extract metadata for Web image
      id: meta-web
      uses: docker/metadata-action@v4
      with:
        images: ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}/web
        tags: |
          type=ref,event=branch
          type=ref,event=tag
          type=sha,prefix={{branch}}-
    
    - name: Build and push Web image
      uses: docker/build-push-action@v4
      with:
        context: .
        file: ./docker/Dockerfile.web
        push: true
        tags: ${{ steps.meta-web.outputs.tags }}
        labels: ${{ steps.meta-web.outputs.labels }}
        cache-from: type=gha
        cache-to: type=gha,mode=max
        platforms: linux/amd64,linux/arm64

  deploy-staging:
    needs: build-and-push
    runs-on: ubuntu-latest
    if: github.ref == 'refs/heads/main' || github.event.inputs.environment == 'staging'
    environment: staging
    
    steps:
    - name: Checkout repository
      uses: actions/checkout@v3
    
    - name: Setup Helm
      uses: azure/setup-helm@v3
      with:
        version: '3.10.0'
    
    - name: Setup kubectl
      uses: azure/setup-kubectl@v3
      with:
        version: '1.25.0'
    
    - name: Configure kubectl
      run: |
        echo "${{ secrets.KUBECONFIG_STAGING }}" | base64 -d > kubeconfig
        export KUBECONFIG=kubeconfig
        kubectl config current-context
    
    - name: Deploy to Staging
      run: |
        export KUBECONFIG=kubeconfig
        helm upgrade --install iot-emulator-staging ./helm/iot-emulator \
          --namespace iot-emulator-staging \
          --create-namespace \
          --values ./helm/iot-emulator/values-staging.yaml \
          --set image.core.tag=${{ github.sha }} \
          --set image.web.tag=${{ github.sha }} \
          --wait --timeout=600s
    
    - name: Run Health Checks
      run: |
        export KUBECONFIG=kubeconfig
        kubectl wait --for=condition=ready pod \
          -l app.kubernetes.io/name=iot-emulator \
          -n iot-emulator-staging \
          --timeout=300s
        
        # Run smoke tests
        kubectl run test-pod --rm -i --restart=Never \
          --image=curlimages/curl \
          --namespace=iot-emulator-staging \
          -- curl -f http://iot-emulator-core-service:8081/health

  deploy-production:
    needs: build-and-push
    runs-on: ubuntu-latest
    if: startsWith(github.ref, 'refs/tags/v') || github.event.inputs.environment == 'production'
    environment: production
    
    steps:
    - name: Checkout repository
      uses: actions/checkout@v3
    
    - name: Setup Helm
      uses: azure/setup-helm@v3
      with:
        version: '3.10.0'
    
    - name: Setup kubectl
      uses: azure/setup-kubectl@v3
      with:
        version: '1.25.0'
    
    - name: Configure kubectl
      run: |
        echo "${{ secrets.KUBECONFIG_PRODUCTION }}" | base64 -d > kubeconfig
        export KUBECONFIG=kubeconfig
        kubectl config current-context
    
    - name: Create Backup
      run: |
        export KUBECONFIG=kubeconfig
        
        # Backup current deployment
        kubectl get deployment iot-emulator-core \
          -n iot-emulator \
          -o yaml > backup-deployment-$(date +%Y%m%d-%H%M%S).yaml
        
        # Backup ConfigMaps
        kubectl get configmap iot-emulator-config \
          -n iot-emulator \
          -o yaml > backup-config-$(date +%Y%m%d-%H%M%S).yaml
    
    - name: Deploy to Production
      run: |
        export KUBECONFIG=kubeconfig
        
        # Blue-Green deployment strategy
        helm upgrade --install iot-emulator ./helm/iot-emulator \
          --namespace iot-emulator \
          --values ./helm/iot-emulator/values-production.yaml \
          --set image.core.tag=${{ github.ref_name }} \
          --set image.web.tag=${{ github.ref_name }} \
          --wait --timeout=900s
    
    - name: Production Health Checks
      run: |
        export KUBECONFIG=kubeconfig
        
        # Wait for pods to be ready
        kubectl wait --for=condition=ready pod \
          -l app.kubernetes.io/name=iot-emulator \
          -n iot-emulator \
          --timeout=600s
        
        # Extended health checks
        kubectl run prod-health-check --rm -i --restart=Never \
          --image=curlimages/curl \
          --namespace=iot-emulator \
          -- sh -c "
            curl -f http://iot-emulator-core-service:8081/health &&
            curl -f http://iot-emulator-core-service:8081/metrics &&
            curl -f http://iot-emulator-web-service:8080/health
          "
    
    - name: Update DNS and Traffic Routing
      run: |
        # Update external DNS to point to new deployment
        # This would typically involve updating external load balancer configuration
        echo "Traffic routing updated to new deployment"
    
    - name: Notify Deployment Success
      if: success()
      uses: 8398a7/action-slack@v3
      with:
        status: success
        channel: '#deployments'
        text: |
          ✅ Production deployment successful!
          Version: ${{ github.ref_name }}
          Environment: Production
          Commit: ${{ github.sha }}
      env:
        SLACK_WEBHOOK_URL: ${{ secrets.SLACK_WEBHOOK_URL }}
    
    - name: Notify Deployment Failure
      if: failure()
      uses: 8398a7/action-slack@v3
      with:
        status: failure
        channel: '#deployments'
        text: |
          ❌ Production deployment failed!
          Version: ${{ github.ref_name }}
          Environment: Production
          Commit: ${{ github.sha }}
          Check the logs: ${{ github.server_url }}/${{ github.repository }}/actions/runs/${{ github.run_id }}
      env:
        SLACK_WEBHOOK_URL: ${{ secrets.SLACK_WEBHOOK_URL }}

  rollback:
    runs-on: ubuntu-latest
    if: failure()
    environment: production
    needs: [deploy-production]
    
    steps:
    - name: Setup kubectl
      uses: azure/setup-kubectl@v3
      with:
        version: '1.25.0'
    
    - name: Configure kubectl
      run: |
        echo "${{ secrets.KUBECONFIG_PRODUCTION }}" | base64 -d > kubeconfig
        export KUBECONFIG=kubeconfig
    
    - name: Rollback Deployment
      run: |
        export KUBECONFIG=kubeconfig
        helm rollback iot-emulator -n iot-emulator
        
        # Wait for rollback to complete
        kubectl rollout status deployment/iot-emulator-core -n iot-emulator
```

## 2. Umgebungs-Management

### 2.1 Environment Configuration

**Environment-spezifische Konfigurationen:**

**Development Environment:**
```yaml
# config/environments/development.yaml
environment: development

server:
  host: "localhost"
  port: 8081
  debug: true
  hot_reload: true

logging:
  level: "DEBUG"
  console_output: true
  file_output: false

database:
  type: "sqlite"
  path: "./dev_data.db"
  
mqtt:
  broker_url: "mqtt://localhost:1883"
  client_id: "iot-emulator-dev"

security:
  authentication_required: false
  cors_enabled: true
  cors_origins: ["http://localhost:3000", "http://localhost:8080"]

monitoring:
  metrics_enabled: false
  tracing_enabled: false

performance:
  max_devices: 100
  max_concurrent_requests: 50
```

**Staging Environment:**
```yaml
# config/environments/staging.yaml
environment: staging

server:
  host: "0.0.0.0"
  port: 8081
  debug: false
  
logging:
  level: "INFO"
  file: "/app/logs/iot-emulator.log"
  max_size: "50MB"
  max_files: 3

database:
  type: "postgresql"
  host: "postgres-staging"
  port: 5432
  database: "iot_emulator_staging"
  ssl_mode: "require"

mqtt:
  broker_url: "mqtt://mosquitto-staging:1883"
  client_id: "iot-emulator-staging"
  username: "${MQTT_USERNAME}"
  password: "${MQTT_PASSWORD}"

security:
  authentication_required: true
  jwt_secret: "${JWT_SECRET}"
  cors_enabled: true
  cors_origins: ["https://staging.iot-emulator.company.com"]

monitoring:
  metrics_enabled: true
  metrics_endpoint: "/metrics"
  tracing_enabled: true
  jaeger_endpoint: "http://jaeger:14268/api/traces"

performance:
  max_devices: 500
  max_concurrent_requests: 200
  
backup:
  enabled: true
  interval: "24h"
  retention: "7d"
```

**Production Environment:**
```yaml
# config/environments/production.yaml
environment: production

server:
  host: "0.0.0.0"
  port: 8081
  debug: false
  graceful_shutdown_timeout: 30s

logging:
  level: "WARN"
  file: "/app/logs/iot-emulator.log"
  max_size: "100MB"
  max_files: 10
  structured: true
  format: "json"

database:
  type: "postgresql"
  host: "postgres-prod.company.internal"
  port: 5432
  database: "iot_emulator_prod"
  ssl_mode: "require"
  connection_pool_size: 20
  max_idle_connections: 5

mqtt:
  broker_url: "mqtts://mqtt-prod.company.internal:8883"
  client_id: "iot-emulator-prod"
  username: "${MQTT_USERNAME}"
  password: "${MQTT_PASSWORD}"
  tls_enabled: true
  tls_cert_file: "/app/certs/mqtt-client.crt"
  tls_key_file: "/app/certs/mqtt-client.key"
  tls_ca_file: "/app/certs/ca.crt"

security:
  authentication_required: true
  jwt_secret: "${JWT_SECRET}"
  cors_enabled: false
  rate_limiting:
    enabled: true
    requests_per_minute: 1000
    burst_size: 100

monitoring:
  metrics_enabled: true
  metrics_endpoint: "/metrics"
  tracing_enabled: true
  jaeger_endpoint: "https://jaeger-prod.company.internal:14268/api/traces"
  health_check_endpoint: "/health"
  readiness_check_endpoint: "/ready"

performance:
  max_devices: 10000
  max_concurrent_requests: 1000
  device_worker_threads: 8
  
backup:
  enabled: true
  interval: "6h"
  retention: "30d"
  encryption_enabled: true
  
alerts:
  enabled: true
  smtp_server: "smtp.company.com:587"
  alert_email: "ops-team@company.com"
  webhook_url: "https://alerts.company.internal/webhook"
```

### 2.2 Secret Management

**Kubernetes Secrets Management:**
```yaml
# k8s/secrets.yaml
apiVersion: v1
kind: Secret
metadata:
  name: iot-emulator-secrets
  namespace: iot-emulator
type: Opaque
data:
  # Database credentials
  DATABASE_URL: {{ .Values.database.url | b64enc }}
  DATABASE_PASSWORD: {{ .Values.database.password | b64enc }}
  
  # MQTT credentials
  MQTT_USERNAME: {{ .Values.mqtt.username | b64enc }}
  MQTT_PASSWORD: {{ .Values.mqtt.password | b64enc }}
  
  # JWT secrets
  JWT_SECRET: {{ .Values.security.jwtSecret | b64enc }}
  
  # External service API keys
  GRAFANA_API_KEY: {{ .Values.monitoring.grafana.apiKey | b64enc }}
  SLACK_WEBHOOK_URL: {{ .Values.alerts.slack.webhookUrl | b64enc }}

---
apiVersion: v1
kind: Secret
metadata:
  name: iot-emulator-tls
  namespace: iot-emulator
type: kubernetes.io/tls
data:
  tls.crt: {{ .Values.tls.certificate | b64enc }}
  tls.key: {{ .Values.tls.privateKey | b64enc }}

---
# External Secrets Operator configuration
apiVersion: external-secrets.io/v1beta1
kind: SecretStore
metadata:
  name: vault-backend
  namespace: iot-emulator
spec:
  provider:
    vault:
      server: "https://vault.company.internal"
      path: "secret"
      version: "v2"
      auth:
        kubernetes:
          mountPath: "kubernetes"
          role: "iot-emulator"
          serviceAccountRef:
            name: "iot-emulator-sa"

---
apiVersion: external-secrets.io/v1beta1
kind: ExternalSecret
metadata:
  name: iot-emulator-vault-secrets
  namespace: iot-emulator
spec:
  refreshInterval: 1h
  secretStoreRef:
    name: vault-backend
    kind: SecretStore
  target:
    name: iot-emulator-vault-secrets
    creationPolicy: Owner
  data:
  - secretKey: database-password
    remoteRef:
      key: iot-emulator/database
      property: password
  - secretKey: mqtt-password
    remoteRef:
      key: iot-emulator/mqtt
      property: password
  - secretKey: jwt-secret
    remoteRef:
      key: iot-emulator/auth
      property: jwt-secret
```

**Vault Configuration Script:**
```bash
#!/bin/bash
# scripts/setup-vault-secrets.sh

set -e

VAULT_ADDR="${VAULT_ADDR:-https://vault.company.internal}"
VAULT_TOKEN="${VAULT_TOKEN}"

if [[ -z "$VAULT_TOKEN" ]]; then
    echo "Error: VAULT_TOKEN environment variable is required"
    exit 1
fi

echo "Setting up Vault secrets for IoT Emulator..."

# Enable Kubernetes auth if not already enabled
vault auth enable -path=kubernetes kubernetes || true

# Configure Kubernetes auth
vault write auth/kubernetes/config \
    token_reviewer_jwt="$(cat /var/run/secrets/kubernetes.io/serviceaccount/token)" \
    kubernetes_host="https://kubernetes.default.svc:443" \
    kubernetes_ca_cert=@/var/run/secrets/kubernetes.io/serviceaccount/ca.crt

# Create policy for IoT Emulator
vault policy write iot-emulator-policy - <<EOF
path "secret/data/iot-emulator/*" {
  capabilities = ["read"]
}
path "secret/metadata/iot-emulator/*" {
  capabilities = ["read"]
}
EOF

# Create Kubernetes role
vault write auth/kubernetes/role/iot-emulator \
    bound_service_account_names=iot-emulator-sa \
    bound_service_account_namespaces=iot-emulator \
    policies=iot-emulator-policy \
    ttl=24h

# Store secrets
echo "Storing database secrets..."
vault kv put secret/iot-emulator/database \
    host="postgres-prod.company.internal" \
    port="5432" \
    database="iot_emulator_prod" \
    username="iot_emulator_user" \
    password="$(openssl rand -base64 32)"

echo "Storing MQTT secrets..."
vault kv put secret/iot-emulator/mqtt \
    broker_url="mqtts://mqtt-prod.company.internal:8883" \
    username="iot_emulator_client" \
    password="$(openssl rand -base64 32)"

echo "Storing authentication secrets..."
vault kv put secret/iot-emulator/auth \
    jwt-secret="$(openssl rand -base64 64)"

echo "Storing monitoring secrets..."
vault kv put secret/iot-emulator/monitoring \
    grafana-api-key="$(openssl rand -base64 32)" \
    slack-webhook-url="${SLACK_WEBHOOK_URL}"

echo "Vault secrets setup completed successfully!"
```

## 3. Load Balancing und Hochverfügbarkeit

### 3.1 Nginx Load Balancer Configuration

```nginx
# nginx/nginx.conf
user nginx;
worker_processes auto;
error_log /var/log/nginx/error.log warn;
pid /var/run/nginx.pid;

events {
    worker_connections 1024;
    use epoll;
    multi_accept on;
}

http {
    include /etc/nginx/mime.types;
    default_type application/octet-stream;
    
    # Logging format
    log_format main '$remote_addr - $remote_user [$time_local] "$request" '
                    '$status $body_bytes_sent "$http_referer" '
                    '"$http_user_agent" "$http_x_forwarded_for" '
                    'rt=$request_time uct="$upstream_connect_time" '
                    'uht="$upstream_header_time" urt="$upstream_response_time"';
    
    access_log /var/log/nginx/access.log main;
    
    # Performance optimizations
    sendfile on;
    tcp_nopush on;
    tcp_nodelay on;
    keepalive_timeout 65;
    types_hash_max_size 2048;
    client_max_body_size 100M;
    
    # Gzip compression
    gzip on;
    gzip_vary on;
    gzip_min_length 1024;
    gzip_types
        application/atom+xml
        application/javascript
        application/json
        application/ld+json
        application/manifest+json
        application/rss+xml
        application/vnd.geo+json
        application/vnd.ms-fontobject
        application/x-font-ttf
        application/x-web-app-manifest+json
        application/xhtml+xml
        application/xml
        font/opentype
        image/bmp
        image/svg+xml
        image/x-icon
        text/cache-manifest
        text/css
        text/plain
        text/vcard
        text/vnd.rim.location.xloc
        text/vtt
        text/x-component
        text/x-cross-domain-policy;
    
    # Rate limiting
    limit_req_zone $binary_remote_addr zone=api:10m rate=10r/s;
    limit_req_zone $binary_remote_addr zone=web:10m rate=50r/s;
    
    # Upstream definitions
    upstream iot_emulator_core {
        least_conn;
        server iot-emulator-core-1:8081 weight=1 max_fails=3 fail_timeout=30s;
        server iot-emulator-core-2:8081 weight=1 max_fails=3 fail_timeout=30s;
        server iot-emulator-core-3:8081 backup;
        
        # Health checks
        keepalive 32;
        keepalive_requests 100;
        keepalive_timeout 60s;
    }
    
    upstream iot_emulator_web {
        least_conn;
        server iot-emulator-web-1:8080 weight=1 max_fails=3 fail_timeout=30s;
        server iot-emulator-web-2:8080 weight=1 max_fails=3 fail_timeout=30s;
        
        keepalive 32;
        keepalive_requests 100;
        keepalive_timeout 60s;
    }
    
    # SSL configuration
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384;
    ssl_prefer_server_ciphers off;
    ssl_session_cache shared:SSL:10m;
    ssl_session_timeout 10m;
    
    # Security headers
    add_header X-Frame-Options DENY always;
    add_header X-Content-Type-Options nosniff always;
    add_header X-XSS-Protection "1; mode=block" always;
    add_header Referrer-Policy "strict-origin-when-cross-origin" always;
    add_header Content-Security-Policy "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; img-src 'self' data:; font-src 'self';" always;
    
    # Main server configuration
    server {
        listen 80;
        server_name iot-emulator.company.com;
        
        # Redirect HTTP to HTTPS
        return 301 https://$server_name$request_uri;
    }
    
    server {
        listen 443 ssl http2;
        server_name iot-emulator.company.com;
        
        ssl_certificate /etc/nginx/ssl/iot-emulator.crt;
        ssl_certificate_key /etc/nginx/ssl/iot-emulator.key;
        
        # API endpoints
        location /api/ {
            limit_req zone=api burst=20 nodelay;
            
            proxy_pass http://iot_emulator_core;
            proxy_http_version 1.1;
            proxy_set_header Upgrade $http_upgrade;
            proxy_set_header Connection 'upgrade';
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
            proxy_cache_bypass $http_upgrade;
            
            # Timeouts
            proxy_connect_timeout 5s;
            proxy_send_timeout 60s;
            proxy_read_timeout 60s;
            
            # Buffer settings
            proxy_buffering on;
            proxy_buffer_size 4k;
            proxy_buffers 8 4k;
            
            # Health check
            proxy_next_upstream error timeout invalid_header http_500 http_502 http_503 http_504;
        }
        
        # WebSocket support for real-time updates
        location /ws/ {
            proxy_pass http://iot_emulator_core;
            proxy_http_version 1.1;
            proxy_set_header Upgrade $http_upgrade;
            proxy_set_header Connection "upgrade";
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
            
            # WebSocket specific timeouts
            proxy_read_timeout 86400s;
            proxy_send_timeout 86400s;
        }
        
        # Static web interface
        location / {
            limit_req zone=web burst=100 nodelay;
            
            proxy_pass http://iot_emulator_web;
            proxy_http_version 1.1;
            proxy_set_header Upgrade $http_upgrade;
            proxy_set_header Connection 'upgrade';
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
            proxy_cache_bypass $http_upgrade;
            
            # Caching for static assets
            location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg)$ {
                expires 1y;
                add_header Cache-Control "public, immutable";
            }
        }
        
        # Health check endpoint
        location /nginx-health {
            access_log off;
            return 200 "healthy\n";
            add_header Content-Type text/plain;
        }
        
        # Metrics endpoint for monitoring
        location /nginx-metrics {
            access_log off;
            allow 10.0.0.0/8;
            allow 172.16.0.0/12;
            allow 192.168.0.0/16;
            deny all;
            
            stub_status on;
        }
    }
    
    # Monitoring server for Grafana
    server {
        listen 443 ssl http2;
        server_name monitoring.iot-emulator.company.com;
        
        ssl_certificate /etc/nginx/ssl/monitoring.crt;
        ssl_certificate_key /etc/nginx/ssl/monitoring.key;
        
        location / {
            proxy_pass http://grafana:3000;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
        }
    }
}

# Stream configuration for MQTT load balancing
stream {
    upstream mqtt_brokers {
        least_conn;
        server mosquitto-1:1883 weight=1 max_fails=3 fail_timeout=30s;
        server mosquitto-2:1883 weight=1 max_fails=3 fail_timeout=30s;
        server mosquitto-3:1883 backup;
    }
    
    server {
        listen 1883;
        proxy_pass mqtt_brokers;
        proxy_timeout 1s;
        proxy_responses 1;
        proxy_bind $remote_addr transparent;
    }
}
```

### 3.2 HAProxy Configuration

```haproxy
# haproxy/haproxy.cfg
global
    log stdout local0 info
    chroot /var/lib/haproxy
    stats socket /run/haproxy/admin.sock mode 660 level admin
    stats timeout 30s
    user haproxy
    group haproxy
    daemon

    # SSL Configuration
    ssl-default-bind-ciphers ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384
    ssl-default-bind-options ssl-min-ver TLSv1.2 no-tls-tickets

defaults
    mode http
    log global
    option httplog
    option dontlognull
    option log-health-checks
    option forwardfor
    option http-server-close
    timeout connect 5000
    timeout client 50000
    timeout server 50000
    errorfile 400 /etc/haproxy/errors/400.http
    errorfile 403 /etc/haproxy/errors/403.http
    errorfile 408 /etc/haproxy/errors/408.http
    errorfile 500 /etc/haproxy/errors/500.http
    errorfile 502 /etc/haproxy/errors/502.http
    errorfile 503 /etc/haproxy/errors/503.http
    errorfile 504 /etc/haproxy/errors/504.http

# Statistics interface
frontend stats
    bind *:8404
    stats enable
    stats uri /stats
    stats refresh 30s
    stats admin if LOCALHOST

# Main frontend
frontend iot_emulator_frontend
    bind *:80
    bind *:443 ssl crt /etc/haproxy/ssl/iot-emulator.pem
    
    # Redirect HTTP to HTTPS
    redirect scheme https code 301 if !{ ssl_fc }
    
    # Security headers
    http-response set-header X-Frame-Options DENY
    http-response set-header X-Content-Type-Options nosniff
    http-response set-header X-XSS-Protection "1; mode=block"
    
    # Rate limiting
    stick-table type ip size 100k expire 30s store http_req_rate(10s)
    http-request track-sc0 src
    http-request reject if { sc_http_req_rate(0) gt 20 }
    
    # ACLs for routing
    acl is_api path_beg /api/
    acl is_websocket hdr(Upgrade) -i websocket
    acl is_health path /health
    acl is_metrics path /metrics
    
    # Backend selection
    use_backend iot_emulator_core if is_api
    use_backend iot_emulator_core if is_websocket
    use_backend iot_emulator_core if is_health
    use_backend iot_emulator_core if is_metrics
    default_backend iot_emulator_web

# Core backend
backend iot_emulator_core
    balance roundrobin
    option httpchk GET /health
    http-check expect status 200
    
    server core1 iot-emulator-core-1:8081 check inter 10s fall 3 rise 2 weight 100
    server core2 iot-emulator-core-2:8081 check inter 10s fall 3 rise 2 weight 100
    server core3 iot-emulator-core-3:8081 check inter 10s fall 3 rise 2 weight 50 backup

# Web frontend backend
backend iot_emulator_web
    balance roundrobin
    option httpchk GET /health
    http-check expect status 200
    
    server web1 iot-emulator-web-1:8080 check inter 10s fall 3 rise 2 weight 100
    server web2 iot-emulator-web-2:8080 check inter 10s fall 3 rise 2 weight 100

# MQTT Load Balancing (TCP mode)
frontend mqtt_frontend
    mode tcp
    bind *:1883
    default_backend mqtt_brokers

backend mqtt_brokers
    mode tcp
    balance leastconn
    option tcp-check
    
    server mqtt1 mosquitto-1:1883 check inter 10s fall 3 rise 2
    server mqtt2 mosquitto-2:1883 check inter 10s fall 3 rise 2
    server mqtt3 mosquitto-3:1883 check inter 10s fall 3 rise 2 backup
```

---

**Nächste Review:** 01.09.2025  
**Verantwortlich:** DevOps Engineer  
**Genehmigt von:** Infrastructure Team Lead, Security Officer  
