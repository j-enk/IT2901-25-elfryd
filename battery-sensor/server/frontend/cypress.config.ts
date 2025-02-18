import { defineConfig } from "cypress";

export default defineConfig({
  e2e: {
    // We've imported your old cypress plugins here.
    // You may want to clean this up later by importing these.
    setupNodeEvents(on, config) {
      return require("./cypress/plugins/index.js")(on, config);
    },
    experimentalRunAllSpecs: true,
  },
  env: {
    refreshToken:
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiI4Nzc2NmQ2My0zNzc1LTQwODMtODhjMC1jMTM3MTBiMWI0ZDMiLCJleHAiOjE3MTI4MzQyNzcsImlzcyI6Imh0dHA6Ly9sb2NhbGhvc3Q6NTAwMSIsImF1ZCI6Imh0dHA6Ly9sb2NhbGhvc3Q6MzAwMCJ9.919938xUr_KH1RuNJze0NrXwOnAb-8vb4Ly7GWbmDWc",
    refreshTokenAdmin:
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJlNTdhYzQ5ZC1mNWU1LTQ1ZTQtODQxYi03ZmFjNzdkOGQ1ZTMiLCJleHAiOjE4NDA0NzU3MzcsImlzcyI6Imh0dHA6Ly9sb2NhbGhvc3Q6NTAwMSIsImF1ZCI6Imh0dHA6Ly9sb2NhbGhvc3Q6MzAwMCJ9.lKwN5KDm8HLaJ3aSt7dFGuuBC8yG-L0tngNS2swE8Uc",
    accessToken:
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6InRlc3RAdGVzdC5jb20iLCJuYW1lIjoiVGVzdCBUZXN0ZXNlbiIsInN1YiI6Ijg3NzY2ZDYzLTM3NzUtNDA4My04OGMwLWMxMzcxMGIxYjRkMyIsImlzQWN0aXZlIjoiVHJ1ZSIsInJvbGVzIjoiTWVtYmVyIiwiZXhwIjoxNjgxMjk5MTc3LCJpc3MiOiJodHRwOi8vbG9jYWxob3N0OjUwMDEiLCJhdWQiOiJodHRwOi8vbG9jYWxob3N0OjMwMDAifQ.QykI4VwP1WQxVQOxQvdSDyNTri0q-FHme0fklRDst24",
    accessTokenAdmin:
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6InRlc3RhZG1pbkB0ZXN0LmNvbSIsIm5hbWUiOiJUZXN0IEFkbWluIiwic3ViIjoiZTU3YWM0OWQtZjVlNS00NWU0LTg0MWItN2ZhYzc3ZDhkNWUzIiwiaXNBY3RpdmUiOiJUcnVlIiwicm9sZXMiOiJBZG1pbiIsImV4cCI6MTY4MjYyMzgzNywiaXNzIjoiaHR0cDovL2xvY2FsaG9zdDo1MDAxIiwiYXVkIjoiaHR0cDovL2xvY2FsaG9zdDozMDAwIn0.yeELU2ZJXRpuHujsUZaUrGpqmOGe50J8J_N-6lepln0",
  },

  component: {
    devServer: {
      framework: "next",
      bundler: "webpack",
    },
  },
});
