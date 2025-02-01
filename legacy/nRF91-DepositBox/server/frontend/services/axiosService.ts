import axios from "axios";
import { logIn, refreshToken } from "../features/auth/authSlice";
import { store } from "./store";

type CookiesType = {
  [key: string]: string;
};

export const getCookies = (): CookiesType => {
  return document.cookie.split(";").reduce((res, c) => {
    const [key, val] = c.trim().split("=").map(decodeURIComponent);
    try {
      return Object.assign(res, { [key]: JSON.parse(val) });
    } catch (e) {
      return Object.assign(res, { [key]: val });
    }
  }, {});
};

export const setNewAccessToken = (accessToken: string) => {
  document.cookie = `token=${accessToken}`;
};

let baseURL =
  process.env.NODE_ENV === "production"
    ? process.env.API_URL_PROD
    : process.env.API_URL_DEV;
const axiosService = axios.create({
  baseURL: baseURL,
  withCredentials: true,
  headers: {
    "Content-Type": "application/json",
    "Access-Control-Allow-Origin": "*",
    "Access-Control-Allow-Credentials": true,
    accept: "application/json",
    "Access-Control-Expose-Headers": "Set-Cookie"
  },
});

axiosService.interceptors.request.use(
  (config) => {
    let access_token = store.getState().auth.accessToken;
    // Intercepts each request and sets the access token as bearer token in order to auth to the API
    if (!access_token) {
      const cookies = getCookies();
      access_token = cookies.token;
      store.dispatch(logIn(access_token));
    }

    config.headers = {
      Authorization: `Bearer ${access_token}`,
    };
    return config;
  },
  (error) => Promise.reject(error)
);

axiosService.interceptors.response.use(
  (response) => {
    return response;
  },
  async (error) => {
    /**
     * Intercepts http responses. If they have status code 401, a call to api/Auth/refresh-token is made
     * in order to get a new refresh token. Then the rejected request is re ran.
     * 
     * This ensures a smooth experience for the user.
     */
    const originalRequest = error.config;
    if (error.response.status === 401 && !originalRequest._retry) {
      originalRequest._retry = true;
      await store.dispatch(refreshToken());
      const access_token = store.getState().auth.accessToken;
      axios.defaults.headers.common["Authorization"] = "Bearer " + access_token;
      return axiosService(originalRequest);
    }
    return Promise.reject(error);
  }
);

export default axiosService;
