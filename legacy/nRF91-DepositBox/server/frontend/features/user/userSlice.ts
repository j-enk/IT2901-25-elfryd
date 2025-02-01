import { createAsyncThunk, createSlice } from "@reduxjs/toolkit";
import axiosService from "../../services/axiosService";
import { RootState } from "../../services/store";
import { UserType } from "../../typings/userType";
import { selectAccessToken } from "../auth/authSelector";

type UserState = {
  activeUser: UserType;
  isLoggedIn: boolean;
  status: string;
  error: string | null;
};

const initialState: UserState = {
  activeUser: {
    id: "",
    name: "",
    phone: "",
    email: "",
    roles: [],
    isActive: true,
  },
  isLoggedIn: false,
  status: "idle",
  error: null,
};

export const deleteUser = createAsyncThunk(
  "Users/fetchUsers",
  async (userID: string) => {
    const axe = axiosService.delete(`Bookings/${userID}`);
    const response = await axe.then((res) => res.data);
    return response;
  }
);

export const getProfile = createAsyncThunk<
  UserType,
  void,
  { state: RootState }
>("Users/getProfile", async () => {
  const axe = axiosService.get(`User/Profile`);
  const response = await axe.then((res) => res.data);
  return response;
});

export const UserSlice = createSlice({
  name: "user",
  initialState,
  reducers: {},
  extraReducers(builder) {
    builder
      .addCase(getProfile.pending, (state) => {
        state.status = "loading";
      })
      .addCase(getProfile.fulfilled, (state, action) => {
        if (action.payload) {
          state.status = "succeeded";
          state.activeUser = action.payload;
        }
      })
      .addCase(getProfile.rejected, (state, action) => {
        state.status = "failed";
        if (action.error.message) {
          state.error = action.error.message;
        }
      })
      .addCase(deleteUser.pending, (state) => {
        state.status = "loading";
      });
  },
});

export default UserSlice.reducer;
