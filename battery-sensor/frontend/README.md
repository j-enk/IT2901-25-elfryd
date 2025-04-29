<div id="top"></div>

<div align="center">
  <a href="https://www.kystlaget-trh.no/">
    <img src="../docs/images/j-enk.png" alt="Logo" width="120" height="120">
  </a>

  <br />
  <h1 align="center">Elfryd Frontend</h1>

  <p align="center">
    A user-friendly dashboard for visualizing real-time and historical data from the Elfryd boat monitoring system.
  </p>
</div>

<hr />

<div align="center">
  <a href="https://vuejs.org/"><img src="https://tinyurl.com/22c28lf3" alt="Vue.js"></a>
  <a href="https://nuxtjs.org/"><img src="https://tinyurl.com/25w36sc9" alt="Nuxt.js"></a>
  <a href="https://tailwindcss.com/"><img src="https://tinyurl.com/bdbdssd4" alt="Tailwind CSS"></a>
  <a href="https://daisyui.com/"><img src="https://tinyurl.com/u9f94j75" alt="DaisyUI"></a>
</div>

<br />

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#system-overview">System Overview</a></li>
    <li><a href="#project-components">Project Components</a></li>
    <li><a href="#data-flow">Data Flow</a></li>
    <li><a href="#key-features">Key Features</a></li>
    <li><a href="#getting-started">Getting Started</a></li>
    <li><a href="#project-status">Project Status</a></li>
    <li><a href="#development-team">Development Team</a></li>
  </ol>
</details>

## System Overview

The Elfryd Frontend is a Nuxt.js-based web application that provides a dashboard for visualizing data collected from the Elfryd boat. It integrates with the backend API to display real-time and historical data, allowing users to monitor battery levels, environmental conditions, and more.

## Project Components

### 1. Vue.js Components
- **Reusable Components**: Includes `BatteryStatus.vue` for displaying battery information and `chart.vue` for rendering data visualizations.
- **Pages**: Individual pages like `sensor.vue` and `batsensor.vue` provide specific functionalities.

### 2. Composables
- **Data Fetching**: Reusable logic for interacting with the backend API, such as `useElfrydBatteryData.ts` and `useElfrydGyroData.ts`.

### 3. Styling
- **Tailwind CSS**: For utility-first styling.
- **DaisyUI**: For pre-designed UI components.

## Data Flow

1. **Data Collection**: Sensors on the boat collect data.
2. **Backend Integration**: The frontend fetches data from the backend API.
3. **Visualization**: Data is displayed in real-time charts and summaries.

## Key Features

- **Real-Time Data Visualization**: Displays live updates from the backend.
- **Historical Data**: Allows users to view trends over time.
- **Responsive Design**: Optimized for both desktop and mobile devices.
- **Configurable Parameters**: Users can adjust monitoring settings directly from the dashboard.

## Getting Started

### Prerequisites

- Node.js and npm installed on your system.

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

## Project Status

- ✅ Real-Time Data Visualization
- ✅ Historical Data Display
- 🔄 Advanced Configuration Options - In Progress
- 🔄 Testing and Documentation - In Progress

## Development Team

This project is developed as part of IT2901 Information Technology Project II at NTNU. Our team is focused on creating a reliable IoT monitoring solution for the Elfryd boat:

- Martin Vågseter Jakobsen 
- Magnus Hansen Åsheim
- Leif Eggenfellner
- David Salguero Spilde
- Jonas Elvedal Hole
- Sigve Fehn Kulsrud

<p align="right">(<a href="#top">back to top</a>)</p>
