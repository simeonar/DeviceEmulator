# Code-Qualitätsstandards
## IoT Device Emulator

**Datum:** 25. August 2025  
**Version:** 1.0  
**Lead Developer:** [Name]  
**Status:** In Bearbeitung  

---

## 1. Code-Stil und Formatierung

### 1.1 C++ Coding Standards

**Header Guard Convention:**
```cpp
// File: iot-emulator/core/device_manager.hpp
#ifndef IOT_EMULATOR_CORE_DEVICE_MANAGER_HPP
#define IOT_EMULATOR_CORE_DEVICE_MANAGER_HPP

// Header content here...

#endif // IOT_EMULATOR_CORE_DEVICE_MANAGER_HPP
```

**Naming Conventions:**
```cpp
// Classes: PascalCase
class DeviceManager {
public:
    // Public methods: camelCase
    bool registerDevice(const std::string& name, std::shared_ptr<Device> device);
    
    // Public constants: UPPER_SNAKE_CASE
    static const int MAX_DEVICES = 1000;
    
private:
    // Private members: snake_case with trailing underscore
    std::unordered_map<std::string, std::shared_ptr<Device>> registered_devices_;
    std::mutex devices_mutex_;
    
    // Private methods: camelCase
    bool validateDeviceName(const std::string& name) const;
};

// Enums: PascalCase with UPPER_CASE values
enum class DeviceState {
    INACTIVE,
    ACTIVE,
    ERROR,
    MAINTENANCE
};

// Free functions: snake_case
std::string format_device_status(DeviceState state);

// Namespaces: snake_case
namespace iot_emulator {
namespace protocols {
    // Content
}
}

// Constants: UPPER_SNAKE_CASE
constexpr int DEFAULT_PORT = 1883;
constexpr double MAX_TEMPERATURE = 100.0;

// Type aliases: snake_case_t
using device_map_t = std::unordered_map<std::string, std::shared_ptr<Device>>;
```

**Class Structure Template:**
```cpp
// iot-emulator/core/device_base.hpp
#ifndef IOT_EMULATOR_CORE_DEVICE_BASE_HPP
#define IOT_EMULATOR_CORE_DEVICE_BASE_HPP

#include <memory>
#include <string>
#include <vector>

namespace iot_emulator {
namespace core {

/**
 * @brief Base class for all IoT device emulations
 * 
 * This abstract class provides the common interface that all device
 * implementations must follow. It handles basic device lifecycle
 * management and defines the contract for device behavior.
 * 
 * @note All device implementations must be thread-safe
 */
class DeviceBase {
public:
    /// Device configuration structure
    struct Config {
        std::string name;
        std::string type;
        std::unordered_map<std::string, std::any> parameters;
    };
    
    /// Device status information
    struct Status {
        DeviceState state;
        std::string last_error;
        std::chrono::system_clock::time_point last_update;
    };

    // Construction and destruction
    explicit DeviceBase(const std::string& name);
    virtual ~DeviceBase() = default;
    
    // Delete copy constructor and assignment
    DeviceBase(const DeviceBase&) = delete;
    DeviceBase& operator=(const DeviceBase&) = delete;
    
    // Allow move operations
    DeviceBase(DeviceBase&&) = default;
    DeviceBase& operator=(DeviceBase&&) = default;

    // Public interface - pure virtual methods
    virtual bool initialize(const Config& config) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual Status getStatus() const = 0;
    virtual std::vector<std::string> getAvailableScenarios() const = 0;
    virtual bool executeScenario(const std::string& scenario_name) = 0;

    // Public interface - concrete methods
    const std::string& getName() const noexcept { return name_; }
    bool isRunning() const noexcept;
    std::chrono::milliseconds getUptime() const noexcept;

protected:
    // Protected methods for derived classes
    void setState(DeviceState new_state);
    void setError(const std::string& error_message);
    void logInfo(const std::string& message) const;
    void logError(const std::string& message) const;

private:
    // Private member variables
    const std::string name_;
    mutable std::mutex status_mutex_;
    Status status_;
    std::chrono::system_clock::time_point start_time_;
    
    // Private helper methods
    void updateLastUpdateTime();
};

} // namespace core
} // namespace iot_emulator

#endif // IOT_EMULATOR_CORE_DEVICE_BASE_HPP
```

**Implementation File Template:**
```cpp
// iot-emulator/core/device_base.cpp
#include "device_base.hpp"
#include <iostream>
#include <sstream>

namespace iot_emulator {
namespace core {

DeviceBase::DeviceBase(const std::string& name)
    : name_(name)
    , status_{DeviceState::INACTIVE, "", std::chrono::system_clock::now()}
    , start_time_(std::chrono::system_clock::now()) {
    
    if (name_.empty()) {
        throw std::invalid_argument("Device name cannot be empty");
    }
    
    logInfo("Device created: " + name_);
}

bool DeviceBase::isRunning() const noexcept {
    std::lock_guard<std::mutex> lock(status_mutex_);
    return status_.state == DeviceState::ACTIVE;
}

std::chrono::milliseconds DeviceBase::getUptime() const noexcept {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
}

void DeviceBase::setState(DeviceState new_state) {
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_.state = new_state;
    updateLastUpdateTime();
    
    logInfo("State changed to: " + stateToString(new_state));
}

void DeviceBase::setError(const std::string& error_message) {
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_.state = DeviceState::ERROR;
    status_.last_error = error_message;
    updateLastUpdateTime();
    
    logError("Device error: " + error_message);
}

void DeviceBase::logInfo(const std::string& message) const {
    std::cout << "[INFO] [" << name_ << "] " << message << std::endl;
}

void DeviceBase::logError(const std::string& message) const {
    std::cerr << "[ERROR] [" << name_ << "] " << message << std::endl;
}

void DeviceBase::updateLastUpdateTime() {
    status_.last_update = std::chrono::system_clock::now();
}

} // namespace core
} // namespace iot_emulator
```

### 1.2 Python Coding Standards (PEP 8)

**Module Structure:**
```python
# iot-emulator/monitor/backend_bridge.py
"""Backend bridge for communicating with C++ core engine.

This module provides the interface between the Python Flask web application
and the C++ core engine. It handles HTTP communication, error handling,
and data serialization.

Example:
    >>> bridge = BackendBridge("http://localhost:8081")
    >>> devices = bridge.get_devices()
    >>> print(f"Found {len(devices)} devices")
"""

import json
import logging
import time
from typing import Dict, List, Optional, Any, Union
from dataclasses import dataclass
from pathlib import Path

import requests
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

# Constants
DEFAULT_TIMEOUT = 30
MAX_RETRIES = 3
BACKOFF_FACTOR = 0.3

# Configure logging
logger = logging.getLogger(__name__)


@dataclass
class DeviceInfo:
    """Information about a device."""
    name: str
    device_type: str
    status: str
    scenarios: List[str]
    last_update: Optional[str] = None


class BackendCommunicationError(Exception):
    """Raised when communication with backend fails."""
    pass


class DeviceNotFoundError(Exception):
    """Raised when a requested device is not found."""
    
    def __init__(self, device_name: str):
        self.device_name = device_name
        super().__init__(f"Device '{device_name}' not found")


class BackendBridge:
    """Bridge to communicate with the C++ backend."""
    
    def __init__(self, backend_url: str, timeout: int = DEFAULT_TIMEOUT):
        """Initialize the backend bridge.
        
        Args:
            backend_url: URL of the C++ backend server
            timeout: Request timeout in seconds
            
        Raises:
            ValueError: If backend_url is invalid
        """
        if not backend_url or not backend_url.startswith(('http://', 'https://')):
            raise ValueError("Invalid backend URL")
        
        self.backend_url = backend_url.rstrip('/')
        self.timeout = timeout
        self._session = self._create_session()
        
        logger.info(f"Backend bridge initialized for {self.backend_url}")
    
    def _create_session(self) -> requests.Session:
        """Create a requests session with retry strategy."""
        session = requests.Session()
        
        retry_strategy = Retry(
            total=MAX_RETRIES,
            backoff_factor=BACKOFF_FACTOR,
            status_forcelist=[429, 500, 502, 503, 504],
        )
        
        adapter = HTTPAdapter(max_retries=retry_strategy)
        session.mount("http://", adapter)
        session.mount("https://", adapter)
        
        return session
    
    def get_devices(self) -> List[DeviceInfo]:
        """Get list of all devices from backend.
        
        Returns:
            List of DeviceInfo objects
            
        Raises:
            BackendCommunicationError: If communication fails
        """
        try:
            response = self._make_request('GET', '/devices')
            devices_data = response.json()
            
            return [
                DeviceInfo(
                    name=device['name'],
                    device_type=device['type'],
                    status=device['status'],
                    scenarios=device.get('scenarios', []),
                    last_update=device.get('last_update')
                )
                for device in devices_data
            ]
            
        except requests.RequestException as e:
            logger.error(f"Failed to get devices: {e}")
            raise BackendCommunicationError(f"Failed to get devices: {e}")
    
    def start_device(self, device_name: str) -> bool:
        """Start a specific device.
        
        Args:
            device_name: Name of the device to start
            
        Returns:
            True if device was started successfully
            
        Raises:
            DeviceNotFoundError: If device doesn't exist
            BackendCommunicationError: If communication fails
        """
        try:
            response = self._make_request('POST', f'/devices/{device_name}/start')
            result = response.json()
            return result.get('success', False)
            
        except requests.HTTPError as e:
            if e.response.status_code == 404:
                raise DeviceNotFoundError(device_name)
            raise BackendCommunicationError(f"Failed to start device: {e}")
        except requests.RequestException as e:
            logger.error(f"Failed to start device {device_name}: {e}")
            raise BackendCommunicationError(f"Failed to start device: {e}")
    
    def _make_request(self, method: str, endpoint: str, **kwargs) -> requests.Response:
        """Make HTTP request to backend.
        
        Args:
            method: HTTP method (GET, POST, etc.)
            endpoint: API endpoint (should start with /)
            **kwargs: Additional arguments for requests
            
        Returns:
            Response object
            
        Raises:
            requests.RequestException: If request fails
        """
        url = f"{self.backend_url}{endpoint}"
        
        # Set default timeout
        kwargs.setdefault('timeout', self.timeout)
        
        logger.debug(f"Making {method} request to {url}")
        
        response = self._session.request(method, url, **kwargs)
        response.raise_for_status()
        
        return response
    
    def __enter__(self):
        """Context manager entry."""
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self._session.close()


# Utility functions
def format_device_status(status: str) -> str:
    """Format device status for display.
    
    Args:
        status: Raw status string
        
    Returns:
        Formatted status string
    """
    status_map = {
        'active': '🟢 Active',
        'inactive': '🔴 Inactive',
        'error': '❌ Error',
        'maintenance': '🔧 Maintenance'
    }
    return status_map.get(status.lower(), f"❓ {status.title()}")


def validate_device_name(name: str) -> bool:
    """Validate device name format.
    
    Args:
        name: Device name to validate
        
    Returns:
        True if name is valid
    """
    if not name or not isinstance(name, str):
        return False
    
    # Device names should be 1-50 characters, alphanumeric + underscore/dash
    if len(name) > 50:
        return False
    
    return name.replace('_', '').replace('-', '').isalnum()
```

### 1.3 Documentation Standards

**API Documentation Template:**
```cpp
/**
 * @brief Manages IoT device instances and their lifecycle
 * 
 * The DeviceManager class is responsible for registering, starting, stopping,
 * and monitoring IoT device emulations. It provides thread-safe operations
 * and maintains the state of all registered devices.
 * 
 * Thread Safety:
 * This class is thread-safe. Multiple threads can safely call its methods
 * concurrently. Internal synchronization is handled using std::mutex.
 * 
 * Example Usage:
 * @code
 * auto manager = std::make_unique<DeviceManager>();
 * auto temp_sensor = std::make_shared<TemperatureSensor>("temp1");
 * 
 * if (manager->registerDevice("sensor1", temp_sensor)) {
 *     manager->startDevice("sensor1");
 *     // Device is now running
 * }
 * @endcode
 * 
 * @note Device names must be unique within a DeviceManager instance
 * @warning Stopping a device while it's publishing data may cause data loss
 * 
 * @see Device
 * @see DeviceBase
 * @since v1.0
 * @author Development Team
 */
class DeviceManager {
public:
    /**
     * @brief Construct a new Device Manager
     * 
     * Creates an empty device manager with no registered devices.
     * The manager is immediately ready to accept device registrations.
     */
    DeviceManager();
    
    /**
     * @brief Register a new device with the manager
     * 
     * Adds a device to the manager's registry. The device must have a unique
     * name within this manager instance. Once registered, the device can be
     * controlled through the manager's interface.
     * 
     * @param name Unique identifier for the device (max 50 characters)
     * @param device Shared pointer to the device implementation
     * 
     * @return true if device was successfully registered
     * @return false if device name already exists or is invalid
     * 
     * @pre device must not be nullptr
     * @pre name must be non-empty and unique
     * @post If successful, device is added to registry in INACTIVE state
     * 
     * @throws std::invalid_argument if device is nullptr or name is empty
     * 
     * @note This operation is thread-safe
     * @note Device names are case-sensitive
     * 
     * Example:
     * @code
     * auto sensor = std::make_shared<TemperatureSensor>();
     * bool success = manager.registerDevice("temp_sensor_1", sensor);
     * if (!success) {
     *     std::cout << "Failed to register device" << std::endl;
     * }
     * @endcode
     */
    bool registerDevice(const std::string& name, std::shared_ptr<Device> device);
    
    /**
     * @brief Start a registered device
     * 
     * Transitions the specified device from INACTIVE to ACTIVE state.
     * Once active, the device will begin its emulation behavior according
     * to its configured scenarios.
     * 
     * @param name Name of the device to start
     * 
     * @return true if device was successfully started
     * @return false if device not found or already running
     * 
     * @pre Device must be registered and in INACTIVE state
     * @post If successful, device is in ACTIVE state
     * 
     * @throws DeviceNotFoundError if device name is not registered
     * @throws DeviceStateError if device is in ERROR state
     * 
     * Time Complexity: O(1) average case for device lookup
     * 
     * @see stopDevice()
     * @see getDeviceStatus()
     */
    bool startDevice(const std::string& name);
    
private:
    /// Mutex for thread-safe access to device registry
    mutable std::mutex devices_mutex_;
    
    /// Registry of all managed devices
    std::unordered_map<std::string, std::shared_ptr<Device>> devices_;
    
    /**
     * @brief Validate device name format
     * 
     * @param name Device name to validate
     * @return true if name is valid
     * 
     * @note Names must be 1-50 characters, alphanumeric plus underscore/dash
     */
    bool isValidDeviceName(const std::string& name) const;
};
```

## 2. Static Code Analysis

### 2.1 C++ Static Analysis (Clang-Tidy)

**Clang-Tidy Configuration:**
```yaml
# .clang-tidy
Checks: >
  bugprone-*,
  cert-*,
  clang-analyzer-*,
  concurrency-*,
  cppcoreguidelines-*,
  google-*,
  hicpp-*,
  misc-*,
  modernize-*,
  performance-*,
  portability-*,
  readability-*,
  -readability-magic-numbers,
  -cppcoreguidelines-avoid-magic-numbers,
  -google-readability-braces-around-statements,
  -hicpp-braces-around-statements,
  -readability-braces-around-statements

WarningsAsErrors: >
  bugprone-use-after-move,
  bugprone-dangling-handle,
  cert-err34-c,
  cert-err52-cpp,
  cert-err60-cpp,
  clang-analyzer-security.insecureAPI.UncheckedReturn,
  concurrency-mt-unsafe,
  cppcoreguidelines-pro-bounds-array-to-pointer-decay,
  misc-misplaced-const,
  performance-for-range-copy,
  performance-unnecessary-copy-initialization

CheckOptions:
  readability-identifier-naming.ClassCase: 'CamelCase'
  readability-identifier-naming.FunctionCase: 'camelCase'
  readability-identifier-naming.VariableCase: 'lower_case'
  readability-identifier-naming.PrivateMemberCase: 'lower_case'
  readability-identifier-naming.PrivateMemberSuffix: '_'
  readability-identifier-naming.ConstantCase: 'UPPER_CASE'
  readability-identifier-naming.EnumCase: 'CamelCase'
  readability-identifier-naming.EnumConstantCase: 'UPPER_CASE'
  readability-identifier-naming.NamespaceCase: 'lower_case'
  cppcoreguidelines-special-member-functions.AllowSoleDefaultDtor: true
  modernize-use-default-member-init.UseAssignment: true
  performance-unnecessary-value-param.AllowedTypes: 'std::shared_ptr;std::unique_ptr'
```

**Custom Clang-Tidy Checks:**
```python
# tools/clang_tidy_runner.py
import subprocess
import sys
import json
from pathlib import Path
from typing import List, Dict

class ClangTidyRunner:
    def __init__(self, build_dir: Path, source_dir: Path):
        self.build_dir = build_dir
        self.source_dir = source_dir
        self.checks_config = self._load_config()
    
    def run_analysis(self, files: List[Path] = None) -> Dict:
        """Run clang-tidy analysis on specified files or all C++ files."""
        if files is None:
            files = self._find_cpp_files()
        
        results = {
            'total_files': len(files),
            'files_with_issues': 0,
            'total_warnings': 0,
            'total_errors': 0,
            'issues_by_category': {},
            'files': {}
        }
        
        for file_path in files:
            file_results = self._analyze_file(file_path)
            results['files'][str(file_path)] = file_results
            
            if file_results['warnings'] > 0 or file_results['errors'] > 0:
                results['files_with_issues'] += 1
            
            results['total_warnings'] += file_results['warnings']
            results['total_errors'] += file_results['errors']
            
            # Categorize issues
            for issue in file_results['issues']:
                category = issue['check_name'].split('-')[0]
                if category not in results['issues_by_category']:
                    results['issues_by_category'][category] = 0
                results['issues_by_category'][category] += 1
        
        return results
    
    def _analyze_file(self, file_path: Path) -> Dict:
        """Analyze a single file with clang-tidy."""
        cmd = [
            'clang-tidy',
            str(file_path),
            f'--config-file={self.source_dir}/.clang-tidy',
            f'--export-fixes={self.build_dir}/clang-tidy-fixes.yaml',
            '--',
            f'-I{self.source_dir}/iot-emulator',
            '-std=c++17'
        ]
        
        try:
            result = subprocess.run(
                cmd, 
                capture_output=True, 
                text=True, 
                cwd=self.build_dir
            )
            
            return self._parse_clang_tidy_output(result.stdout, result.stderr)
            
        except subprocess.CalledProcessError as e:
            return {
                'warnings': 0,
                'errors': 1,
                'issues': [{
                    'level': 'error',
                    'message': f'Failed to analyze file: {e}',
                    'check_name': 'analysis-failure'
                }]
            }
    
    def _parse_clang_tidy_output(self, stdout: str, stderr: str) -> Dict:
        """Parse clang-tidy output and extract issues."""
        issues = []
        warnings = 0
        errors = 0
        
        lines = stdout.split('\n') + stderr.split('\n')
        
        for line in lines:
            if 'warning:' in line or 'error:' in line:
                issue = self._parse_issue_line(line)
                if issue:
                    issues.append(issue)
                    if issue['level'] == 'warning':
                        warnings += 1
                    else:
                        errors += 1
        
        return {
            'warnings': warnings,
            'errors': errors,
            'issues': issues
        }
    
    def _parse_issue_line(self, line: str) -> Dict:
        """Parse a single clang-tidy issue line."""
        # Example: file.cpp:123:45: warning: message [check-name]
        parts = line.split(': ')
        if len(parts) < 3:
            return None
        
        location_part = parts[0]
        level_part = parts[1]
        message_part = ': '.join(parts[2:])
        
        # Extract check name
        check_name = ''
        if '[' in message_part and ']' in message_part:
            start = message_part.rfind('[')
            end = message_part.rfind(']')
            check_name = message_part[start+1:end]
            message_part = message_part[:start].strip()
        
        return {
            'location': location_part,
            'level': level_part,
            'message': message_part,
            'check_name': check_name
        }
```

### 2.2 Python Static Analysis

**Flake8 Configuration:**
```ini
# .flake8
[flake8]
max-line-length = 88
max-complexity = 10
select = E,W,F,C
ignore = 
    E203,  # whitespace before ':'
    E501,  # line too long (handled by black)
    W503,  # line break before binary operator
exclude =
    .git,
    __pycache__,
    .venv,
    venv,
    build,
    dist,
    *.egg-info

# Per-file ignores
per-file-ignores =
    __init__.py: F401  # Allow unused imports in __init__.py
    tests/*.py: F401,F811  # Allow unused imports and redefinitions in tests

# Error codes and complexity
max-cognitive-complexity = 10
import-order-style = google
```

**MyPy Configuration:**
```ini
# mypy.ini
[mypy]
python_version = 3.9
warn_return_any = True
warn_unused_configs = True
disallow_untyped_defs = True
disallow_incomplete_defs = True
check_untyped_defs = True
disallow_untyped_decorators = True
no_implicit_optional = True
warn_redundant_casts = True
warn_unused_ignores = True
warn_no_return = True
warn_unreachable = True
strict_equality = True
show_error_codes = True

# Module-specific overrides
[mypy-requests.*]
ignore_missing_imports = True

[mypy-flask.*]
ignore_missing_imports = True

[mypy-pytest.*]
ignore_missing_imports = True
```

**Bandit Security Analysis:**
```yaml
# .bandit
tests: [B101, B102, B103, B104, B105, B106, B107, B108, B110, B112, 
        B201, B301, B302, B303, B304, B305, B306, B307, B308, B309, 
        B310, B311, B312, B313, B314, B315, B316, B317, B318, B319, 
        B320, B321, B322, B323, B324, B325, B401, B402, B403, B404, 
        B405, B406, B407, B408, B409, B410, B411, B412, B413, B501, 
        B502, B503, B504, B505, B506, B507, B601, B602, B603, B604, 
        B605, B606, B607, B608, B609, B610, B611, B701, B702, B703]

exclude_dirs: ['tests', 'venv', '.venv', 'build']

# Security issue levels
skips: ['B101']  # Skip assert_used (needed for debug code)

# Custom configurations
bandit_config:
  hardcoded_password_entropy: 3.0
  word_list_file: security/wordlist.txt
```

## 3. Code-Review-Prozess

### 3.1 Pull Request Template

```markdown
<!-- .github/pull_request_template.md -->
## Description
Brief description of the changes made in this PR.

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update
- [ ] Refactoring (no functional changes)
- [ ] Performance improvement
- [ ] Test enhancement

## Related Issues
Fixes #(issue number)
Related to #(issue number)

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] Manual testing completed
- [ ] Performance testing completed (if applicable)

### Test Environment
- OS: [e.g. Windows 10, Ubuntu 20.04]
- Compiler: [e.g. GCC 9.4, MSVC 2019]
- Python Version: [e.g. 3.9.7]

### Test Results
```
Paste test results here
```

## Code Quality Checklist
- [ ] Code follows the project's coding standards
- [ ] Self-review of code completed
- [ ] Code is properly commented (especially complex logic)
- [ ] Static analysis passes (clang-tidy, flake8, mypy)
- [ ] Security scan passes (bandit)
- [ ] Documentation updated (if applicable)
- [ ] CHANGELOG.md updated

## Performance Impact
- [ ] No performance impact
- [ ] Performance improved
- [ ] Performance degraded (explain below)

**Performance Details:**
[Explain any performance implications]

## Security Considerations
- [ ] No security implications
- [ ] Security review completed
- [ ] Security tests added

**Security Details:**
[Explain any security considerations]

## Breaking Changes
- [ ] No breaking changes
- [ ] Breaking changes (list below)

**Breaking Changes:**
- [List any breaking changes]

## Screenshots (if applicable)
[Add screenshots to help explain your changes]

## Additional Notes
[Any additional information that reviewers should know]

## Reviewer Checklist (for reviewers)
- [ ] Code is readable and maintainable
- [ ] Logic is sound and efficient
- [ ] Error handling is appropriate
- [ ] Tests provide adequate coverage
- [ ] Documentation is clear and accurate
- [ ] Security considerations addressed
- [ ] Performance implications understood
```

### 3.2 Code Review Guidelines

**Review Criteria Checklist:**

**Functionality:**
```yaml
✓ Code correctly implements the required functionality
✓ Edge cases are properly handled
✓ Error conditions are handled gracefully
✓ Input validation is comprehensive
✓ Output is correct and properly formatted
✓ Performance requirements are met
✓ Memory usage is reasonable
```

**Design & Architecture:**
```yaml
✓ Code follows SOLID principles
✓ Proper separation of concerns
✓ Appropriate use of design patterns
✓ Clean interfaces and abstractions
✓ Minimal coupling between components
✓ Code is extensible and maintainable
✓ Threading considerations addressed
```

**Code Quality:**
```yaml
✓ Follows project coding standards
✓ Meaningful variable and function names
✓ Appropriate comments and documentation
✓ No code duplication
✓ Consistent formatting
✓ No compiler warnings
✓ Static analysis passes
```

**Testing:**
```yaml
✓ Unit tests cover all new code
✓ Integration tests for new features
✓ Test edge cases and error conditions
✓ Performance tests for critical paths
✓ Tests are maintainable and clear
✓ Mock objects used appropriately
✓ Test coverage meets requirements (≥90%)
```

**Security:**
```yaml
✓ Input sanitization implemented
✓ No hardcoded secrets or passwords
✓ Proper authentication/authorization
✓ SQL injection protection
✓ XSS protection (web components)
✓ Buffer overflow protection
✓ Secure communication protocols
```

### 3.3 Automated Review Tools

**PR Analysis Script:**
```python
# tools/pr_analyzer.py
import subprocess
import json
import re
from pathlib import Path
from typing import Dict, List, Set

class PRAnalyzer:
    def __init__(self, base_branch: str = "main"):
        self.base_branch = base_branch
        self.current_branch = self._get_current_branch()
    
    def analyze_pr(self) -> Dict:
        """Perform comprehensive PR analysis."""
        return {
            'changed_files': self._get_changed_files(),
            'code_metrics': self._calculate_code_metrics(),
            'complexity_analysis': self._analyze_complexity(),
            'security_issues': self._run_security_scan(),
            'test_coverage': self._check_test_coverage(),
            'documentation_changes': self._check_documentation(),
            'performance_impact': self._analyze_performance_impact()
        }
    
    def _get_changed_files(self) -> Dict:
        """Get list of changed files with change statistics."""
        cmd = f"git diff --stat {self.base_branch}..{self.current_branch}"
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        
        files = []
        for line in result.stdout.split('\n'):
            if '|' in line:
                parts = line.split('|')
                filename = parts[0].strip()
                stats = parts[1].strip()
                
                # Extract additions and deletions
                additions = re.search(r'(\d+) insertion', stats)
                deletions = re.search(r'(\d+) deletion', stats)
                
                files.append({
                    'filename': filename,
                    'additions': int(additions.group(1)) if additions else 0,
                    'deletions': int(deletions.group(1)) if deletions else 0
                })
        
        return {
            'files': files,
            'total_files': len(files),
            'total_additions': sum(f['additions'] for f in files),
            'total_deletions': sum(f['deletions'] for f in files)
        }
    
    def _analyze_complexity(self) -> Dict:
        """Analyze code complexity changes."""
        changed_cpp_files = self._get_changed_cpp_files()
        
        complexity_issues = []
        for file_path in changed_cpp_files:
            # Run complexity analysis tool
            cmd = f"clang-tidy {file_path} --checks='-*,readability-function-cognitive-complexity'"
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
            
            if "complexity" in result.stdout.lower():
                complexity_issues.append({
                    'file': file_path,
                    'issues': self._parse_complexity_output(result.stdout)
                })
        
        return {
            'files_with_complexity_issues': len(complexity_issues),
            'issues': complexity_issues
        }
    
    def _check_test_coverage(self) -> Dict:
        """Check if new code has adequate test coverage."""
        # Get list of new/modified functions
        new_functions = self._get_new_functions()
        
        # Check if tests exist for these functions
        test_coverage = {}
        for func in new_functions:
            test_coverage[func] = self._has_test_for_function(func)
        
        coverage_percentage = sum(test_coverage.values()) / len(test_coverage) * 100 if test_coverage else 100
        
        return {
            'new_functions': len(new_functions),
            'functions_with_tests': sum(test_coverage.values()),
            'coverage_percentage': coverage_percentage,
            'untested_functions': [func for func, tested in test_coverage.items() if not tested]
        }
    
    def generate_review_summary(self, analysis: Dict) -> str:
        """Generate human-readable review summary."""
        summary = []
        
        # File changes summary
        changes = analysis['changed_files']
        summary.append(f"📊 **Changes Overview:**")
        summary.append(f"- {changes['total_files']} files modified")
        summary.append(f"- +{changes['total_additions']} / -{changes['total_deletions']} lines")
        summary.append("")
        
        # Code quality issues
        if analysis['security_issues']['issues']:
            summary.append("🔒 **Security Issues Found:**")
            for issue in analysis['security_issues']['issues'][:5]:  # Top 5
                summary.append(f"- {issue['severity']}: {issue['message']}")
            summary.append("")
        
        # Test coverage
        coverage = analysis['test_coverage']
        if coverage['coverage_percentage'] < 90:
            summary.append("🧪 **Test Coverage Warning:**")
            summary.append(f"- Coverage: {coverage['coverage_percentage']:.1f}% (target: 90%)")
            if coverage['untested_functions']:
                summary.append("- Untested functions:")
                for func in coverage['untested_functions'][:3]:
                    summary.append(f"  - {func}")
            summary.append("")
        
        # Complexity issues
        complexity = analysis['complexity_analysis']
        if complexity['files_with_complexity_issues'] > 0:
            summary.append("🧠 **Complexity Issues:**")
            summary.append(f"- {complexity['files_with_complexity_issues']} files have high complexity")
            summary.append("")
        
        # Overall recommendation
        if (coverage['coverage_percentage'] >= 90 and 
            not analysis['security_issues']['issues'] and 
            complexity['files_with_complexity_issues'] == 0):
            summary.append("✅ **Review Status:** Ready for merge")
        else:
            summary.append("⚠️ **Review Status:** Issues need to be addressed")
        
        return '\n'.join(summary)
```

## 4. Performance und Optimierung

### 4.1 Performance Guidelines

**C++ Performance Best Practices:**
```cpp
// 1. Prefer stack allocation over heap allocation
void goodExample() {
    std::array<int, 100> stack_array;  // Good: stack allocation
    // Use stack_array...
}

void badExample() {
    auto heap_array = new int[100];  // Bad: manual heap allocation
    // Use heap_array...
    delete[] heap_array;  // Easy to forget!
}

// 2. Use move semantics for expensive-to-copy objects
class Device {
    std::vector<std::string> scenarios_;
    
public:
    // Good: move constructor
    Device(std::vector<std::string>&& scenarios) 
        : scenarios_(std::move(scenarios)) {}
    
    // Good: return by move
    std::vector<std::string> getScenarios() && {
        return std::move(scenarios_);
    }
};

// 3. Prefer range-based for loops
void processDevices(const std::vector<Device>& devices) {
    // Good: range-based for loop
    for (const auto& device : devices) {
        device.process();
    }
    
    // Avoid: traditional for loop (more error-prone)
    for (size_t i = 0; i < devices.size(); ++i) {
        devices[i].process();
    }
}

// 4. Use const-correctness
class DeviceManager {
public:
    // Good: const method for read-only operations
    size_t getDeviceCount() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return devices_.size();
    }
    
    // Good: const reference parameters for read-only access
    bool hasDevice(const std::string& name) const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return devices_.find(name) != devices_.end();
    }
    
private:
    mutable std::mutex mutex_;  // mutable for const methods
    std::unordered_map<std::string, std::shared_ptr<Device>> devices_;
};

// 5. Optimize string operations
class DeviceLogger {
private:
    static constexpr std::string_view LOG_PREFIX = "[DEVICE] ";
    
public:
    // Good: use string_view for read-only string parameters
    void log(std::string_view message) {
        // Good: reserve space for concatenation
        std::string log_line;
        log_line.reserve(LOG_PREFIX.size() + message.size() + 20);
        
        log_line += LOG_PREFIX;
        log_line += message;
        log_line += " [";
        log_line += getCurrentTimestamp();
        log_line += "]";
        
        writeToLog(log_line);
    }
    
    // Good: use fmt library for efficient formatting
    template<typename... Args>
    void logf(fmt::format_string<Args...> format, Args&&... args) {
        auto message = fmt::format(format, std::forward<Args>(args)...);
        log(message);
    }
};

// 6. Memory pool for frequent allocations
class DevicePool {
    static constexpr size_t POOL_SIZE = 1000;
    
    std::array<Device, POOL_SIZE> device_pool_;
    std::bitset<POOL_SIZE> used_slots_;
    std::mutex pool_mutex_;
    
public:
    Device* allocateDevice() {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        
        for (size_t i = 0; i < POOL_SIZE; ++i) {
            if (!used_slots_[i]) {
                used_slots_[i] = true;
                return &device_pool_[i];
            }
        }
        return nullptr;  // Pool exhausted
    }
    
    void deallocateDevice(Device* device) {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        
        auto index = device - device_pool_.data();
        if (index >= 0 && index < POOL_SIZE) {
            used_slots_[index] = false;
            device->reset();  // Clean up device state
        }
    }
};
```

**Python Performance Optimization:**
```python
# 1. Use appropriate data structures
import collections
from typing import DefaultDict, Deque
import bisect

class EfficientDeviceManager:
    def __init__(self):
        # Good: Use deque for fast append/pop operations
        self.device_queue: Deque[str] = collections.deque()
        
        # Good: Use defaultdict to avoid key checks
        self.device_stats: DefaultDict[str, dict] = collections.defaultdict(dict)
        
        # Good: Use sets for membership testing
        self.active_devices: set[str] = set()
        
        # Good: Keep sorted list for binary search
        self.sorted_device_names: list[str] = []
    
    def add_device(self, device_name: str) -> None:
        """Add device with efficient data structure operations."""
        # O(1) operations
        self.device_queue.append(device_name)
        self.active_devices.add(device_name)
        
        # O(log n) operation for maintaining sorted order
        bisect.insort(self.sorted_device_names, device_name)
    
    def find_device(self, device_name: str) -> bool:
        """Efficient device lookup."""
        # O(1) operation instead of O(n) list search
        return device_name in self.active_devices
    
    def find_devices_by_prefix(self, prefix: str) -> list[str]:
        """Efficient prefix search using binary search."""
        # Find insertion point for prefix
        start_idx = bisect.bisect_left(self.sorted_device_names, prefix)
        
        # Collect all names with the prefix
        result = []
        for i in range(start_idx, len(self.sorted_device_names)):
            name = self.sorted_device_names[i]
            if name.startswith(prefix):
                result.append(name)
            else:
                break  # Names are sorted, so we can stop
        
        return result

# 2. Use generators for memory efficiency
def process_large_dataset(file_path: str):
    """Process large dataset without loading everything into memory."""
    
    def read_devices(file_path: str):
        """Generator that yields devices one at a time."""
        with open(file_path, 'r') as f:
            for line in f:
                if line.strip():
                    yield json.loads(line)
    
    # Good: Process items one at a time
    for device_data in read_devices(file_path):
        process_device(device_data)
        # Memory is freed after each iteration

# 3. Use functools.lru_cache for expensive computations
import functools
from typing import Optional

class DeviceCalculator:
    
    @functools.lru_cache(maxsize=1000)
    def calculate_complex_metric(self, device_type: str, scenario: str) -> float:
        """Expensive calculation that benefits from caching."""
        # Simulate expensive computation
        result = 0.0
        for i in range(1000000):
            result += math.sin(i) * hash(device_type + scenario)
        return result
    
    def get_device_efficiency(self, device_type: str, scenario: str) -> float:
        """Use cached calculation for better performance."""
        base_metric = self.calculate_complex_metric(device_type, scenario)
        return base_metric * 0.85

# 4. Use __slots__ for memory optimization
class OptimizedDevice:
    """Memory-optimized device class using __slots__."""
    
    __slots__ = ['name', 'device_type', 'status', 'last_update', '_scenarios']
    
    def __init__(self, name: str, device_type: str):
        self.name = name
        self.device_type = device_type
        self.status = 'inactive'
        self.last_update = None
        self._scenarios = []
    
    @property
    def scenarios(self) -> list[str]:
        return self._scenarios
    
    def add_scenario(self, scenario: str) -> None:
        self._scenarios.append(scenario)

# 5. Batch operations for database/API calls
import asyncio
import aiohttp
from typing import List

class EfficientAPIClient:
    def __init__(self, base_url: str):
        self.base_url = base_url
        self.session: Optional[aiohttp.ClientSession] = None
    
    async def __aenter__(self):
        self.session = aiohttp.ClientSession()
        return self
    
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        if self.session:
            await self.session.close()
    
    async def start_devices_batch(self, device_names: List[str]) -> List[bool]:
        """Start multiple devices concurrently instead of sequentially."""
        if not self.session:
            raise RuntimeError("Client not initialized")
        
        async def start_single_device(device_name: str) -> bool:
            try:
                async with self.session.post(f"{self.base_url}/devices/{device_name}/start") as response:
                    return response.status == 200
            except Exception:
                return False
        
        # Good: Concurrent execution instead of sequential
        tasks = [start_single_device(name) for name in device_names]
        results = await asyncio.gather(*tasks, return_exceptions=True)
        
        return [isinstance(result, bool) and result for result in results]

# Usage example
async def main():
    async with EfficientAPIClient("http://localhost:8081") as client:
        device_names = ["temp1", "temp2", "pressure1", "valve1"]
        results = await client.start_devices_batch(device_names)
        print(f"Started {sum(results)} out of {len(device_names)} devices")
```

### 4.2 Performance Monitoring

**Performance Benchmarking Framework:**
```cpp
// tools/benchmark.hpp
#include <chrono>
#include <functional>
#include <vector>
#include <string>
#include <iostream>

class Benchmark {
public:
    struct Result {
        std::string name;
        std::chrono::nanoseconds min_time;
        std::chrono::nanoseconds max_time;
        std::chrono::nanoseconds avg_time;
        size_t iterations;
    };
    
    static Result measureFunction(const std::string& name,
                                std::function<void()> func,
                                size_t iterations = 1000) {
        std::vector<std::chrono::nanoseconds> times;
        times.reserve(iterations);
        
        // Warm-up
        for (size_t i = 0; i < 10; ++i) {
            func();
        }
        
        // Actual measurements
        for (size_t i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            
            times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));
        }
        
        // Calculate statistics
        auto min_time = *std::min_element(times.begin(), times.end());
        auto max_time = *std::max_element(times.begin(), times.end());
        
        auto total_time = std::accumulate(times.begin(), times.end(), std::chrono::nanoseconds{0});
        auto avg_time = total_time / iterations;
        
        return Result{name, min_time, max_time, avg_time, iterations};
    }
    
    static void printResult(const Result& result) {
        std::cout << "Benchmark: " << result.name << "\n"
                  << "  Iterations: " << result.iterations << "\n"
                  << "  Min time: " << result.min_time.count() << " ns\n"
                  << "  Max time: " << result.max_time.count() << " ns\n"
                  << "  Avg time: " << result.avg_time.count() << " ns\n"
                  << "  Throughput: " << (1e9 / result.avg_time.count()) << " ops/sec\n\n";
    }
};

// Performance test example
void benchmarkDeviceOperations() {
    auto device_manager = std::make_unique<DeviceManager>();
    auto device = std::make_shared<TemperatureSensor>("temp1");
    
    // Benchmark device registration
    auto register_result = Benchmark::measureFunction(
        "Device Registration",
        [&]() {
            device_manager->registerDevice("temp_" + std::to_string(rand()), device);
        },
        1000
    );
    Benchmark::printResult(register_result);
    
    // Benchmark device status query
    device_manager->registerDevice("test_device", device);
    auto status_result = Benchmark::measureFunction(
        "Device Status Query",
        [&]() {
            device_manager->getDeviceStatus("test_device");
        },
        10000
    );
    Benchmark::printResult(status_result);
}
```

---

**Nächste Review:** 01.09.2025  
**Verantwortlich:** Lead Developer  
**Genehmigt von:** Architekt, QA Manager  
