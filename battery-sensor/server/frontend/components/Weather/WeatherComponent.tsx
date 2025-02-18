import React, { useState, useEffect } from 'react';

interface ForecastData {
  time: string;
  data: {
    instant: {
      details: {
        air_temperature: number;
        wind_speed: number;
      };
    };
    next_1_hours?: {
      summary: {
        symbol_code: string;
      };
    };
  };
}

const WeatherForecast: React.FC = () => {
  const [forecast, setForecast] = useState<ForecastData | null>(null);
  const [weatherIcon, setWeatherIcon] = useState<string>('');

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch(
          'https://api.met.no/weatherapi/locationforecast/2.0/compact?lat=63.4305&lon=10.3951'
        );
        const data = await response.json();
        setForecast(data.properties.timeseries[0]);
      } catch (error) {
        console.error(error);
      }
    };

    fetchData();
  }, []);

  useEffect(() => {
    const updateWeatherIcon = () => {
      if (forecast && forecast.data.next_1_hours) {
        const { data: { next_1_hours: { summary: { symbol_code } } } } = forecast;
        const iconPath = `/weather/svg/${symbol_code}.svg`;
        setWeatherIcon(iconPath);
      }
    };

    updateWeatherIcon();
  }, [forecast]);

  if (!forecast) {
    return <div>Loading...</div>;
  }

  const { data: { instant: { details: { air_temperature, wind_speed } } } } = forecast;

  return (
    <div className='flex'>
      <p className="ml-5 mr-1" style={{ marginTop: '13px', fontSize: '18px' }}>
        {Math.floor(air_temperature)}°C
      </p>
      {weatherIcon && (
        <div className="ml-1 mr-2">
          <img src={weatherIcon} alt="weather icon" width="50" height="50" />
        </div>
      )}
      <p className="ml-3 mr-3" style={{ marginTop: '13px', fontSize: '18px' }}>{wind_speed}  m/s</p>
    </div>
  );
};

export default WeatherForecast;
