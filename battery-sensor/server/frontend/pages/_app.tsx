import "@fullcalendar/common/main.css";
import "@fullcalendar/daygrid/main.css";
import "@fullcalendar/timegrid/main.css";

import "../styles/globals.css";
import type { AppProps } from "next/app";
import { Provider } from "react-redux";
import { store } from "../services/store";
import { CookiesProvider } from "react-cookie";
import { ThemeProvider } from "@mui/material";
import theme from "../theme";
import { QueryClient, QueryClientProvider } from "react-query";

export const queryClient = new QueryClient();

function MyApp({ Component, pageProps }: AppProps) {
  return (
    <Provider store={store}>
      <ThemeProvider theme={theme}>
        <CookiesProvider>
        <QueryClientProvider client={queryClient}>
          <Component {...pageProps} />
          </QueryClientProvider>
        </CookiesProvider>
      </ThemeProvider>
    </Provider>
  );
}

export default MyApp;
