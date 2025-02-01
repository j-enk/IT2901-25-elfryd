import { createAsyncThunk, createSlice, PayloadAction } from "@reduxjs/toolkit";
import axiosService from "../../services/axiosService";
import { LockBoxFilterType } from "../../typings/filterType";
import { LockBoxType } from "../../typings/lockBoxType";

type LockBoxState = {
  lockboxes: LockBoxType[];
  activeLockBox: LockBoxType;
  status: string;
  error: string | null;
};

const initialState: LockBoxState = {
  lockboxes: [],
  activeLockBox: {
    lockBoxID: 0,
    topic: "",
    isOpen: false,
    lastResponse: "",
    boatNames: []
  },
  status: "idle",
  error: null,
};

export const fetchLockBoxes = createAsyncThunk(
  "lockbox/fetchLockBoxes",
  async () => {
    const axe = axiosService.get(`LockBoxes/`);
    const response = await axe.then((res) => res.data);
    return response;
  }
);

export const postLockBox = createAsyncThunk(
  "lockbox/postLockBox",
  async (data: LockBoxType) => {
    const axe = axiosService.post("LockBoxes/", data);
    const response = await axe.then((res) => res.data);
    return response;
  }
);

export const LockBoxSlice = createSlice({
  name: "lockbox",
  initialState,
  reducers: {
    setSelectedLockBox: (state, action: PayloadAction<number>) => {
      let tempObject = state.lockboxes.find(function (object) {
        return object.lockBoxID == action.payload;
      });
      if (tempObject) {
        state.activeLockBox = tempObject;
      }
    },
  },
  extraReducers(builder) {
    builder
      .addCase(fetchLockBoxes.pending, (state) => {
        state.status = "loading";
      })
      .addCase(fetchLockBoxes.fulfilled, (state, action) => {
        state.status = "succeeded";
        if (action.payload) {
          state.lockboxes = action.payload;
        }
      })
      .addCase(fetchLockBoxes.rejected, (state, action) => {
        state.status = "failed";
        if (action.error.message) {
          state.error = action.error.message;
        }
      })
      .addCase(postLockBox.pending, (state) => {
        state.status = "loading";
      })
      .addCase(postLockBox.fulfilled, (state, action) => {
        if (action.payload) {
          state.lockboxes.push(action.payload);
        }
        state.status = "succeeded";
      })
      .addCase(postLockBox.rejected, (state, action) => {
        if (action.error.message) {
          state.error = action.error.message;
        }
        state.status = "failed";
      });
  },
});

export const { setSelectedLockBox } = LockBoxSlice.actions;
export default LockBoxSlice.reducer;
