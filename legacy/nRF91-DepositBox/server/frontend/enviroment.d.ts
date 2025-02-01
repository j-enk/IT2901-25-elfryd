declare global {
  namespace NodeJS {
    interface ProcessEnv {
      NODE_ENV: "development" | "production";
      API_URL_DEV: string;
      API_URL_PROD: string;
      COOKIE_DOMAIN_DEV: string;
      COOKIE_DOMAIN_PROD: string;
    }
  }
}

// If this file has no import/export statements (i.e. is a script)
// convert it into a module by adding an empty export statement.
export {};
