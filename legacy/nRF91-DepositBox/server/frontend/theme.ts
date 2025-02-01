import { createTheme } from "@mui/material";

const theme = createTheme({
  breakpoints: {
    values: {
      xs: 0,
      sm: 600,
      md: 900,
      lg: 1200,
      xl: 1536,
      xxl: 1920,
    },
  },
  typography: {
    fontFamily: ["Quicksand", "sans-serif"].join(","),
  },
  palette: {
    primary: {
      main: "#000",
    },
  },
});

declare module "@mui/material/styles" {
  interface BreakpointOverrides {
    xxl: true;
  }
}

export default theme;
