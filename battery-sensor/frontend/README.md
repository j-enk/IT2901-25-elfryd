# Elfryd Sensor Visualization Frontend

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#system-overview">System Overview</a></li>
    <li><a href="#project-components">Project Components</a></li>
    <li><a href="#data-flow">Data Flow</a></li>
    <li><a href="#key-features">Key Features</a></li>
    <li><a href="#getting-started">Getting Started</a></li>
    <li><a href="#api-integration">API Integration</a></li>
    <li><a href="#support-and-contribution">Support and Contribution</a></li>
  </ol>
</details>

## System Overview

The Elfryd Frontend is a Nuxt.js-based web application that provides a dashboard for visualizing data collected from the Elfryd boat. It integrates with the backend API to display real-time and historical data, allowing users to monitor battery levels, environmental conditions, and more.

## Project Components

### 1. Vue.js Components
- **Section Components**: 
  - `BatterySection.vue` - Manages battery information display
  - `GyroSection.vue` - Handles gyroscope data visualization
  - `TempSection.vue` - Shows temperature readings
  - `ConfigSection.vue` - Provides configuration options
- **Chart Components**: 
  - `BatteryLineChart.vue` - Renders battery data visualizations
  - `TempLineChart.vue` - Displays temperature trends
  - `GyroLineChart.vue` - Shows gyroscope data
- **UI Components**:
  - `TabSwitcher.vue` - Navigation between different data sections
  - `forms/ConfigForm.vue` - Form for configuration settings

### 2. Composables
- **Data Fetching**: 
  - `useElfrydBattery.ts` - Handles battery data API calls
  - `useElfrydTemp.ts` - Manages temperature data retrieval
  - `useElfrydGyro.ts` - Fetches gyroscope data
  - `useElfrydConfig.ts` - Handles configuration API interactions

### 3. Pages
- **Single Page Application**:
  - `index.vue` - The main dashboard that integrates all components

### 4. Styling
- **Tailwind CSS**: For utility-first styling
- **DaisyUI**: For pre-designed UI components

## Data Flow

1. **Data Collection**: Sensors on the boat collect data
2. **Backend Integration**: The frontend fetches data from the [backend API](../backend/README.md#api-documentation)
3. **Visualization**: Data is displayed in real-time charts and summaries

## Key Features

- **Real-Time Data Visualization**: Displays live updates from the backend
- **Historical Data**: Allows users to view trends over time
- **Responsive Design**: Optimized for both desktop and mobile devices
- **Configurable Parameters**: Users can adjust monitoring settings directly from the dashboard

## Getting Started

### Prerequisites

- Node.js and npm installed on your system

### Installation

1. Clone the repository:
   ```sh
   git clone https://github.com/j-enk/IT2901-25-elfryd.git
   ```
2. Navigate to the frontend directory:
   ```sh
   cd IT2901-25-elfryd/battery-sensor/frontend/sensor_frontend
   ```
3. Install dependencies:
   ```sh
   npm install
   ```

### Running the Application

- Start the development server:
  ```sh
  npm run dev
  ```
- Build the application for production:
  ```sh
  npm run build
  ```
- Preview the production build locally:
  ```sh
  npm run preview
  ```

## API Integration

This frontend application integrates with the Elfryd Battery Sensor API. For detailed information about the available endpoints and how to use them, please refer to the following resources:

- [Battery Data Endpoints](../backend/README.md#battery-endpoints)
- [Temperature Data Endpoints](../backend/README.md#temperature-endpoints)
- [Gyroscope Data Endpoints](../backend/README.md#gyroscope-endpoints)
- [Configuration Endpoints](../backend/README.md#configuration-endpoints)

The API provides standardized interfaces for retrieving sensor data and sending configuration commands to the Elfryd system.

## Support and Contribution

For questions or issues, please contact the contributors of the frontend system:

- Leif Eggenfeller (main developer)
- Sigve Fehn Kulsrud (contributor)
- Magnus Hansen Åsheim (contributor)
- Martin Vågseter Jakobsen (contributor)