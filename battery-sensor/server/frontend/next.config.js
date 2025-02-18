/** @type {import('next').NextConfig} */
const withTM = require("next-transpile-modules")([
  "@fullcalendar/common",
  "@babel/preset-react",
  "@fullcalendar/common",
  "@fullcalendar/daygrid",
  "@fullcalendar/interaction",
  "@fullcalendar/react",
  "@fullcalendar/timegrid",
]);

module.exports = withTM({
  env: {
    API_URL_DEV: "https://localhost:5001/api/",
    API_URL_PROD: "https://depositbox.api.narverk.no/api/",

    COOKIE_DOMAIN_DEV: "localhost",
    COOKIE_DOMAIN_PROD: ".narverk.no",
  },
});
