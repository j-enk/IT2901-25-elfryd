import { createAsyncThunk, createSlice } from "@reduxjs/toolkit";
import axiosService, {
  getCookies,
  setNewAccessToken,
} from "../../services/axiosService";

type AuthState = {
  accessToken: string;
  isLoggedIn: boolean;
  status: string;
  error: string | null;
};

const initialState: AuthState = {
  accessToken: "",
  isLoggedIn: false,
  status: "",
  error: null,
};

export const refreshToken = createAsyncThunk("auth/refresh-token", async () => {
  const response = await axiosService.post(
    `Auth/refresh-token`,
    {},
    {
      withCredentials: true,
    }
  );
  if (response.status === 200) {
    const cookies = getCookies();
    if (cookies.token) {
      setNewAccessToken(cookies.token);
      return cookies.token;
    }
  }

  throw new Error("Refresh token failed");
});

const cookieDomain = process.env.NODE_ENV === "production"
  ? process.env.COOKIE_DOMAIN_PROD
  : process.env.COOKIE_DOMAIN_DEV

export const AuthSlice = createSlice({
  name: "auth",
  initialState,
  reducers: {
    logOut: (state) => {
      state.accessToken = "";
      state.isLoggedIn = false;
    },
    logIn: (state, action) => {
      state.accessToken = action.payload;
      state.isLoggedIn = true;
    },
  },
  extraReducers(builder) {
    builder
      .addCase(refreshToken.pending, (state) => {
        state.status = "loading";
      })
      .addCase(refreshToken.fulfilled, (state, action) => {
        if (action.payload) {
          state.status = "succeeded";
          state.accessToken = action.payload;
        }
      })
      .addCase(refreshToken.rejected, (state, action) => {
        state.status = "failed";
        if (action.error.message) {
          /**
           * If refresh-token request is rejected, then the refresh token is invalid.
           * The user is therefore logged out, and cookies are removed.
           */
          state.error = action.error.message;
          state.isLoggedIn = false;
          state.accessToken = "";
          document.cookie = `token=; path=/; domain=${cookieDomain};`;
          document.cookie = `refresh_token=; path=/; domain=${cookieDomain};`;
        }
      });
  },
});

export const { logOut, logIn } = AuthSlice.actions;
export default AuthSlice.reducer;
