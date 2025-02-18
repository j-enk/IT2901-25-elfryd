import { createSlice, PayloadAction } from "@reduxjs/toolkit";
import { BoatType } from "../../typings/boatType";

export type BoatState = {
  selectedBoat: BoatType;
};

const initialState: BoatState = {
  selectedBoat: {
    boatID: 0,
    name: "",
    chargingTime: 0,
    advanceBookingLimit: 0,
    maxBookingLimit: 0,
    lockBoxID: 0,
  },
};

export const BoatSlice = createSlice({
  name: "boat",
  initialState,
  reducers: {
    setSelectedBoat: (state, action: PayloadAction<BoatType>) => {
      state.selectedBoat = action.payload;
    },
  },
});

export const { setSelectedBoat } = BoatSlice.actions;

export default BoatSlice.reducer;
