# Projektcontrolling und Berichtswesen
## IoT Device Emulator

**Datum:** 25. August 2025  
**Version:** 1.0  
**Projektleiter:** [Name]  
**Status:** In Bearbeitung  

---

## 1. Controlling Framework

### 1.1 Key Performance Indicators (KPIs)

**Projekt-KPI Dashboard:**
```yaml
project_kpis:
  financial_metrics:
    - kpi: "Budget Variance"
      definition: "Geplantes Budget vs. tatsächliche Ausgaben"
      calculation: "(Actual Cost - Planned Cost) / Planned Cost * 100"
      target: "± 5%"
      current_value: "+2.3%"
      status: "Green"
      trend: "Stable"
      reporting_frequency: "Weekly"
    
    - kpi: "Cost Performance Index (CPI)"
      definition: "Earned Value / Actual Cost"
      calculation: "EV / AC"
      target: ">= 0.95"
      current_value: "0.98"
      status: "Green"
      trend: "Improving"
      reporting_frequency: "Weekly"
    
    - kpi: "Return on Investment (ROI)"
      definition: "Projected benefits vs. total project investment"
      calculation: "(Benefits - Investment) / Investment * 100"
      target: "> 150%"
      current_value: "185%"
      status: "Green"
      trend: "Stable"
      reporting_frequency: "Monthly"
  
  schedule_metrics:
    - kpi: "Schedule Performance Index (SPI)"
      definition: "Earned Value / Planned Value"
      calculation: "EV / PV"
      target: ">= 0.95"
      current_value: "0.92"
      status: "Amber"
      trend: "Declining"
      reporting_frequency: "Weekly"
    
    - kpi: "Milestone Adherence Rate"
      definition: "Anzahl der termingerecht erreichten Meilensteine"
      calculation: "On-time Milestones / Total Milestones * 100"
      target: ">= 90%"
      current_value: "85%"
      status: "Amber"
      trend: "Stable"
      reporting_frequency: "Weekly"
    
    - kpi: "Sprint Velocity"
      definition: "Durchschnittliche Story Points pro Sprint"
      calculation: "Average(Story Points Completed per Sprint)"
      target: "30-35 points"
      current_value: "32 points"
      status: "Green"
      trend: "Stable"
      reporting_frequency: "Bi-weekly"
  
  quality_metrics:
    - kpi: "Code Quality Score"
      definition: "SonarQube Overall Rating"
      calculation: "Automated analysis by SonarQube"
      target: "A-Rating"
      current_value: "A-Rating"
      status: "Green"
      trend: "Stable"
      reporting_frequency: "Daily"
    
    - kpi: "Test Coverage"
      definition: "Anteil der durch Tests abgedeckten Code-Zeilen"
      calculation: "Covered Lines / Total Lines * 100"
      target: ">= 90%"
      current_value: "93%"
      status: "Green"
      trend: "Improving"
      reporting_frequency: "Daily"
    
    - kpi: "Defect Density"
      definition: "Anzahl Defekte pro 1000 Lines of Code"
      calculation: "Defects / (Lines of Code / 1000)"
      target: "< 2.0"
      current_value: "1.3"
      status: "Green"
      trend: "Improving"
      reporting_frequency: "Weekly"
    
    - kpi: "Customer Satisfaction Score"
      definition: "Durchschnittliche Bewertung der Stakeholder"
      calculation: "Average(Stakeholder Ratings)"
      target: ">= 4.0/5.0"
      current_value: "4.2/5.0"
      status: "Green"
      trend: "Stable"
      reporting_frequency: "Monthly"
  
  productivity_metrics:
    - kpi: "Team Velocity Trend"
      definition: "Trend der Entwicklungsgeschwindigkeit"
      calculation: "Linear regression of sprint velocities"
      target: "Positive or stable"
      current_value: "+2.5% per sprint"
      status: "Green"
      trend: "Improving"
      reporting_frequency: "Monthly"
    
    - kpi: "Lead Time"
      definition: "Zeit von Requirements bis zur Produktivsetzung"
      calculation: "Average(Deployment Date - Requirement Date)"
      target: "< 4 weeks"
      current_value: "3.2 weeks"
      status: "Green"
      trend: "Improving"
      reporting_frequency: "Weekly"
    
    - kpi: "Code Review Efficiency"
      definition: "Durchschnittliche Zeit für Code Review"
      calculation: "Average(Review Completion Time)"
      target: "< 24 hours"
      current_value: "18 hours"
      status: "Green"
      trend: "Stable"
      reporting_frequency: "Weekly"
```

### 1.2 Earned Value Management (EVM)

**EVM Tracking System:**
```yaml
earned_value_management:
  project_baseline:
    total_budget: 165000  # EUR
    project_duration: 14  # weeks
    total_story_points: 420
    
    baseline_schedule:
      week_1: { planned_value: 5000, planned_work: 15 }
      week_2: { planned_value: 12000, planned_work: 35 }
      week_3: { planned_value: 20000, planned_work: 60 }
      week_4: { planned_value: 30000, planned_work: 90 }
      week_5: { planned_value: 42000, planned_work: 125 }
      week_6: { planned_value: 55000, planned_work: 165 }
      week_7: { planned_value: 70000, planned_work: 210 }
      week_8: { planned_value: 85000, planned_work: 255 }
      week_9: { planned_value: 100000, planned_work: 300 }
      week_10: { planned_value: 115000, planned_work: 345 }
      week_11: { planned_value: 130000, planned_work: 385 }
      week_12: { planned_value: 145000, planned_work: 405 }
      week_13: { planned_value: 160000, planned_work: 415 }
      week_14: { planned_value: 165000, planned_work: 420 }
  
  current_status:
    reporting_date: "2025-08-25"
    project_week: 6
    
    actual_metrics:
      actual_cost: 58000           # EUR
      earned_value: 53000          # EUR (based on completed work)
      planned_value: 55000         # EUR (from baseline)
      work_completed: 160          # Story points
    
    calculated_metrics:
      cost_variance: -5000         # EV - AC = 53000 - 58000
      schedule_variance: -2000     # EV - PV = 53000 - 55000
      cost_performance_index: 0.91 # EV / AC = 53000 / 58000
      schedule_performance_index: 0.96 # EV / PV = 53000 / 55000
    
    forecasts:
      estimate_at_completion: 180769  # BAC / CPI = 165000 / 0.91
      estimate_to_complete: 122769    # EAC - AC = 180769 - 58000
      variance_at_completion: 15769   # EAC - BAC = 180769 - 165000
      estimated_completion_date: "2025-11-20"  # Based on SPI
    
    status_interpretation:
      cost_status: "Over budget by 9%"
      schedule_status: "Slightly behind schedule by 4%"
      overall_health: "Yellow - Needs attention"
      
      recommended_actions:
        - "Review resource allocation and productivity"
        - "Identify opportunities for scope optimization"
        - "Consider additional resources for critical path items"
        - "Implement cost control measures"
```

### 1.3 Financial Tracking

**Budget Tracking Detail:**
```yaml
budget_tracking:
  budget_categories:
    personnel_costs:
      budgeted: 120000  # EUR
      actual: 42000     # EUR (Week 6)
      committed: 15000  # EUR (approved but not yet spent)
      forecast: 128000  # EUR
      variance: 8000    # EUR over budget
      
      breakdown:
        - role: "Technical Lead"
          budget_monthly: 8000
          actual_ytd: 12000
          utilization: "100%"
        
        - role: "Senior Developer (2x)"
          budget_monthly: 12000
          actual_ytd: 18000
          utilization: "100%"
        
        - role: "DevOps Engineer"
          budget_monthly: 6000
          actual_ytd: 9000
          utilization: "100%"
        
        - role: "QA Engineer"
          budget_monthly: 5000
          actual_ytd: 3000
          utilization: "60% (delayed start)"
    
    infrastructure_costs:
      budgeted: 15000
      actual: 4200
      committed: 2000
      forecast: 14500
      variance: -500    # Under budget
      
      breakdown:
        - item: "Cloud Infrastructure (AWS)"
          budget_monthly: 1500
          actual_ytd: 2100
          trend: "Increasing with usage"
        
        - item: "Development Tools & Licenses"
          budget_monthly: 800
          actual_ytd: 1200
          trend: "One-time purchases front-loaded"
        
        - item: "External Services"
          budget_monthly: 400
          actual_ytd: 900
          trend: "Monitoring and security tools"
    
    external_services:
      budgeted: 20000
      actual: 8500
      committed: 5000
      forecast: 22000
      variance: 2000    # Over budget
      
      breakdown:
        - item: "Security Audit"
          budgeted: 8000
          actual: 0
          scheduled: "Week 10"
        
        - item: "Performance Testing"
          budgeted: 5000
          actual: 3500
          status: "Completed"
        
        - item: "Legal/Compliance Review"
          budgeted: 4000
          actual: 2000
          status: "In progress"
        
        - item: "Training & Certification"
          budgeted: 3000
          actual: 3000
          status: "Completed"
    
    contingency_reserve:
      budgeted: 10000   # 6% of total budget
      utilized: 3300    # Used for scope adjustments
      remaining: 6700
      utilization_rate: "33%"
      
      usage_log:
        - date: "2025-08-10"
          amount: 2000
          reason: "Additional testing infrastructure"
          approved_by: "Projektleiter"
        
        - date: "2025-08-20"
          amount: 1300
          reason: "Extended security review"
          approved_by: "Steering Committee"

  cash_flow_forecast:
    monthly_projections:
      "2025-09":
        planned_spend: 28000
        committed_spend: 25000
        risk_adjusted: 30000
      
      "2025-10":
        planned_spend: 35000
        committed_spend: 20000
        risk_adjusted: 38000
      
      "2025-11":
        planned_spend: 25000
        committed_spend: 15000
        risk_adjusted: 28000
    
    payment_schedule:
      - vendor: "Cloud Provider"
        amount: 1500
        frequency: "Monthly"
        next_payment: "2025-09-01"
      
      - vendor: "Security Consultant"
        amount: 8000
        frequency: "Milestone-based"
        next_payment: "2025-10-15"
      
      - vendor: "Tool Licenses"
        amount: 5000
        frequency: "Annual"
        next_payment: "2025-12-01"
```

## 2. Berichtswesen

### 2.1 Executive Dashboard

**C-Level Dashboard Konfiguration:**
```json
{
  "executive_dashboard": {
    "title": "IoT Device Emulator - Executive View",
    "update_frequency": "Weekly",
    "sections": [
      {
        "section": "Project Health Overview",
        "widgets": [
          {
            "type": "health_indicator",
            "title": "Overall Project Health",
            "current_status": "Yellow",
            "trend": "Stable",
            "risk_factors": [
              "Budget variance trending upward",
              "Schedule pressure on critical milestones"
            ]
          },
          {
            "type": "progress_gauge",
            "title": "Project Completion",
            "percentage": 45,
            "target_date": "2025-10-30",
            "confidence_level": "Medium"
          }
        ]
      },
      {
        "section": "Financial Performance",
        "widgets": [
          {
            "type": "budget_chart",
            "title": "Budget vs Actual",
            "budget": 165000,
            "actual": 58000,
            "forecast": 180769,
            "variance_percentage": 9.5
          },
          {
            "type": "roi_indicator",
            "title": "Expected ROI",
            "current_projection": "185%",
            "target": "150%",
            "confidence": "High"
          }
        ]
      },
      {
        "section": "Key Milestones",
        "widgets": [
          {
            "type": "milestone_timeline",
            "milestones": [
              {
                "name": "Core Architecture",
                "planned": "2025-09-15",
                "status": "On Track",
                "completion": 75
              },
              {
                "name": "Device Types Implementation",
                "planned": "2025-10-01",
                "status": "At Risk",
                "completion": 0
              },
              {
                "name": "Production Deployment",
                "planned": "2025-10-30",
                "status": "Planning",
                "completion": 10
              }
            ]
          }
        ]
      },
      {
        "section": "Risk & Issues",
        "widgets": [
          {
            "type": "risk_summary",
            "high_risks": 1,
            "medium_risks": 3,
            "low_risks": 2,
            "trend": "Stable"
          },
          {
            "type": "top_issues",
            "issues": [
              {
                "title": "Resource allocation optimization needed",
                "priority": "High",
                "owner": "Projektleiter"
              },
              {
                "title": "Third-party integration complexity",
                "priority": "Medium",
                "owner": "Technical Lead"
              }
            ]
          }
        ]
      },
      {
        "section": "Quality Metrics",
        "widgets": [
          {
            "type": "quality_scorecard",
            "code_quality": "A",
            "test_coverage": 93,
            "customer_satisfaction": 4.2,
            "defect_rate": 1.3
          }
        ]
      }
    ]
  }
}
```

### 2.2 Status Reports

**Wöchentlicher Statusbericht Template:**
```markdown
# Wöchentlicher Projektstatus
**Projekt:** IoT Device Emulator  
**Berichtswoche:** KW 34 (21.-25.08.2025)  
**Projektleiter:** [Name]  

## Executive Summary
**Gesamtstatus:** 🟡 Gelb  
**Fortschritt:** 45% (Plan: 50%, Abweichung: -5%)  
**Budget:** 35% verbraucht (Plan: 33%, Abweichung: +2%)  
**Qualität:** 🟢 Grün  

### Woche im Überblick
- ✅ **Erreicht:** MQTT Protocol Implementation abgeschlossen
- ⚠️ **Verzögerung:** REST API Framework komplexer als erwartet
- 🎯 **Nächste Woche:** Web Interface Development beginnt
- ⚡ **Eskalation:** Ressourcen-Engpass bei Frontend-Entwicklung

## Fortschritt nach Arbeitspaketen

### 🟢 Abgeschlossen
**AP 2.1: MQTT Protocol Integration**
- Status: ✅ Abgeschlossen (25.08.2025)
- Qualität: Unit Tests 95% Coverage
- Aufwand: 32h (Plan: 30h, +6%)

**AP 2.2: Device Manager Core**
- Status: ✅ Abgeschlossen (23.08.2025)  
- Qualität: Code Review bestanden
- Aufwand: 28h (Plan: 25h, +12%)

### 🟡 In Bearbeitung
**AP 2.3: REST API Framework**
- Status: 🟡 70% abgeschlossen (Soll: 100%)
- Herausforderung: OpenAPI Schema Komplexität
- Maßnahme: Zusätzlicher Senior Developer ab Montag
- Neue Schätzung: Abschluss 02.09.2025 (Verzögerung: 3 Tage)

### 🔴 Kritischer Pfad
**AP 3.1: Web Interface Development**
- Status: 🔴 Nicht gestartet (Soll: gestartet)
- Abhängigkeit: REST API Framework
- Risiko: Milestone M1 gefährdet
- Maßnahme: Parallel-Entwicklung mit Mock-APIs

## Budget & Ressourcen

### Budgetverbrauch
| Kategorie | Budget | Verbraucht | Verbleibend | Trend |
|-----------|--------|------------|-------------|-------|
| Personal | €120.000 | €42.000 (35%) | €78.000 | 🟡 |
| Infrastruktur | €15.000 | €4.200 (28%) | €10.800 | 🟢 |
| Externe Services | €20.000 | €8.500 (43%) | €11.500 | 🟡 |
| Gesamt | €165.000 | €58.000 (35%) | €107.000 | 🟡 |

### Ressourcenauslastung
- **Technical Lead:** 100% (40h/Woche)
- **Senior Developer #1:** 100% (40h/Woche)  
- **Senior Developer #2:** 100% (40h/Woche)
- **DevOps Engineer:** 80% (32h/Woche)
- **QA Engineer:** 60% (24h/Woche) - Start verzögert

## Qualitätskennzahlen

### Code Quality (SonarQube)
- **Overall Rating:** A
- **Coverage:** 93% (↑ +2% zur Vorwoche)
- **Duplications:** 1.2% (Ziel: <3%)
- **Technical Debt:** 4.5h (Ziel: <10h)
- **Critical Issues:** 0 (Ziel: 0)

### Testing Metrics
- **Unit Tests:** 147 Tests, 100% Pass Rate
- **Integration Tests:** 23 Tests, 100% Pass Rate
- **Performance Tests:** Baseline etabliert
- **Security Scans:** Keine kritischen Findings

## Risiken & Issues

### 🔴 Neue Kritische Risiken
**R006: Frontend-Entwickler Verfügbarkeit**
- Beschreibung: Geplanter Frontend-Developer nicht verfügbar
- Wahrscheinlichkeit: Hoch
- Impact: Hoch  
- Maßnahme: Externe Freelancer-Suche initiiert
- Owner: HR Manager
- Target: 02.09.2025

### 🟡 Aktive Risiken
**R001: Third-party Dependencies**
- Status: Unter Beobachtung
- Letzte Aktion: Dependency-Pinning implementiert
- Nächste Review: 05.09.2025

**R002: Performance unter Last**
- Status: Testing in Vorbereitung
- Letzte Aktion: Load Testing Environment setup
- Nächste Review: 10.09.2025

### Issues (Top 5)
1. **ISS-042:** REST API Schema Validation Performance ⚡ Hoch
2. **ISS-038:** Docker Build Zeit Optimization 🟡 Mittel  
3. **ISS-035:** Documentation Auto-Generation Setup 🟡 Mittel
4. **ISS-033:** CI/CD Pipeline Flakiness 🔴 Niedrig
5. **ISS-029:** Code Review Tool Integration 🟢 Niedrig

## Meilensteine & Termine

### Nächste Meilensteine
**M1: Core Architecture Implementation (15.09.2025)**
- Status: 🟡 75% abgeschlossen
- Risiko: 3 Tage Verzögerung möglich
- Maßnahme: Weekend Work geplant falls nötig

**M2: Device Types Implementation (01.10.2025)**  
- Status: 🟢 Vorbereitung läuft
- Abhängigkeit: M1 Abschluss
- Risiko: Gering

### Kommende Termine
- **29.08:** Change Advisory Board Meeting
- **02.09:** Security Review Kickoff
- **05.09:** Stakeholder Demo (M1 Preview)
- **15.09:** Milestone M1 Review & Approval

## Entscheidungen & Actions

### Diese Woche getroffen
- **DEC-004:** Zusätzlicher Senior Developer für REST API (Budget: +€5.000)
- **DEC-005:** Parallel Web Interface Development mit Mock APIs

### Action Items
| Action | Owner | Due Date | Status |
|--------|-------|----------|--------|
| Frontend Freelancer Requirements definieren | HR Manager | 28.08 | 🟡 |
| Mock API Specification erstellen | Technical Lead | 29.08 | 🔄 |
| Load Testing Environment finalisieren | DevOps | 30.08 | 🔄 |
| Security Review Scope Meeting | Security Officer | 02.09 | 📅 |

## Nächste Woche Fokus
1. 🎯 REST API Framework finalisieren
2. 🚀 Web Interface Development starten  
3. 📋 Frontend Freelancer onboarding
4. 🔒 Security Review Kickoff
5. 📊 Load Testing Baseline etablieren

## Stakeholder Communications
- **Steering Committee:** Monatliches Review 05.09.2025
- **Customer:** Demo Session 05.09.2025 (optional)
- **Development Team:** Daily Standups, Sprint Review 30.08.2025

---
**Prepared by:** Projektleiter  
**Distribution:** Steering Committee, PMO, Development Team  
**Next Report:** 01.09.2025
```

### 2.3 Management Reports

**Monatlicher Management Report:**
```yaml
monthly_management_report:
  reporting_period: "August 2025"
  report_date: "2025-08-31"
  
  executive_summary:
    overall_status: "Yellow"
    completion_percentage: 50
    budget_consumed_percentage: 38
    schedule_variance_days: -3
    key_achievements:
      - "Core device simulation engine completed"
      - "MQTT protocol integration successful"
      - "CI/CD pipeline operational"
    
    key_concerns:
      - "Resource constraints in frontend development"
      - "Slight schedule pressure building"
      - "Third-party dependency management complexity"
    
    upcoming_critical_decisions:
      - "Additional resource allocation for Q4"
      - "Scope prioritization for MVP release"
      - "Production infrastructure vendor selection"
  
  financial_performance:
    budget_summary:
      total_budget: 165000
      spent_to_date: 62500
      committed_costs: 18000
      forecast_completion: 178000
      variance_amount: 13000
      variance_percentage: 7.9
    
    cost_drivers:
      personnel:
        budgeted: 120000
        actual: 45000
        variance: 3000
        explanation: "Additional senior developer hired earlier than planned"
      
      infrastructure:
        budgeted: 15000
        actual: 4800
        variance: -200
        explanation: "Efficient cloud resource utilization"
      
      external_services:
        budgeted: 20000
        actual: 9200
        variance: 1200
        explanation: "Extended security consulting scope"
    
    cash_flow_projection:
      september: 32000
      october: 38000
      november: 24000
      december: 15000
  
  schedule_performance:
    milestone_status:
      completed_on_time: 2
      completed_late: 0
      in_progress: 1
      upcoming: 3
    
    critical_path_analysis:
      current_slack: -3
      bottleneck_activities:
        - "REST API development completion"
        - "Frontend developer availability"
        - "Integration testing coordination"
    
    resource_utilization:
      development_team: 95
      qa_team: 70
      devops_team: 85
      management_overhead: 15
  
  quality_assessment:
    code_quality_trends:
      technical_debt_hours: 4.2
      code_coverage_percentage: 93
      sonarqube_rating: "A"
      critical_issues_count: 0
    
    testing_progress:
      unit_tests_count: 147
      integration_tests_count: 23
      e2e_tests_count: 8
      automated_test_coverage: 88
    
    customer_feedback:
      satisfaction_score: 4.2
      feature_requests_count: 7
      critical_feedback_items: 1
  
  risk_management:
    risk_summary:
      high_risks: 1
      medium_risks: 4
      low_risks: 3
      closed_risks: 2
    
    top_risks:
      - id: "R006"
        title: "Frontend developer availability"
        probability: "High"
        impact: "High"
        mitigation_status: "In Progress"
      
      - id: "R001"
        title: "Third-party dependency vulnerabilities"
        probability: "Medium"
        impact: "High"
        mitigation_status: "Controlled"
    
    issue_summary:
      critical_issues: 1
      high_priority_issues: 3
      medium_priority_issues: 8
      resolved_this_month: 12
  
  team_performance:
    velocity_metrics:
      current_sprint_velocity: 32
      average_velocity: 30
      velocity_trend: "Stable"
    
    productivity_indicators:
      commit_frequency: 4.2  # per day per developer
      code_review_turnaround: 18  # hours average
      deployment_frequency: 3  # per week
    
    team_satisfaction:
      overall_score: 4.1
      retention_risk: "Low"
      training_needs: ["Frontend frameworks", "Security best practices"]
  
  stakeholder_engagement:
    communication_frequency:
      steering_committee: "Monthly"
      product_owner: "Weekly"
      development_team: "Daily"
      customers: "Bi-weekly"
    
    feedback_incorporation:
      requirements_changes: 3
      scope_adjustments: 1
      priority_changes: 2
    
    stakeholder_satisfaction:
      business_stakeholders: 4.0
      technical_stakeholders: 4.3
      end_users: 4.1
  
  recommendations:
    immediate_actions:
      - "Secure additional frontend development resources"
      - "Accelerate integration testing preparation"
      - "Review scope priorities for MVP definition"
    
    strategic_considerations:
      - "Consider phased delivery approach for better risk management"
      - "Evaluate additional automation opportunities"
      - "Plan for post-launch support team transition"
    
    resource_needs:
      additional_budget_request: 15000
      justification: "Resource augmentation and scope extension"
      approval_required_by: "2025-09-15"
  
  next_month_focus:
    key_deliverables:
      - "Complete core architecture milestone"
      - "Begin device types implementation"
      - "Establish production infrastructure"
    
    critical_success_factors:
      - "Maintain development velocity"
      - "Resolve frontend resource constraints"
      - "Complete security review successfully"
```

---

**Nächste Review:** 30.08.2025  
**Verantwortlich:** Projektleiter  
**Verteiler:** Steering Committee, PMO, Stakeholder  
