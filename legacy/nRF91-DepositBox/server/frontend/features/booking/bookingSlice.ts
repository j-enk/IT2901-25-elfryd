import { createAsyncThunk, createSlice, PayloadAction } from "@reduxjs/toolkit";
import axiosService from "../../services/axiosService";
import { BookingReadableType, BookingVMType } from "../../typings/bookingType";

type BookingState = {
  status: string;
  error: string | null;
  filterOnUser: boolean;
  activeBooking: BookingReadableType | null;
};

const initialState: BookingState = {
  status: "idle",
  error: null,
  filterOnUser: false,
  activeBooking: {
    bookingID: 0,
    startTime: "",
    endTime: "",
    chargingDone: "",
    keyTakenTime: "",
    keyReturnedTime: "",
    status: "",
    comment: "",
    bookingOwnerId: "",
    bookingOwner: "",
    bookingOwnerPhone: "",
    bookingOwnerEmail: "",
    bookedBoat: "",
    isOfficial: false,
  },
};

export const fetchBooking = createAsyncThunk(
  "Bookings/fetchBooking",
  async (bookingID: number) => {
    const axe = axiosService.get(`Bookings/${bookingID}`);
    const response = await axe.then((res) => res.data);
    return response;
  }
);

export const cancelBooking = createAsyncThunk(
  "Bookings/cancelBooking",
  async (data: { booking: BookingVMType; id: number }) => {
    const axe = axiosService.put(`Bookings/${data.id}`, data.booking);
    const response = await axe.then((res) => res.data);
    return response;
  }
);

export const BookingSlice = createSlice({
  name: "booking",
  initialState,
  reducers: {
    resetBookingState: (state) => {
      state.status = "idle";
      state.error = null;
    },
    resetError: (state) => {
      state.error = null;
    },
    setFilterOnUser: (state, action: PayloadAction<boolean>) => {
      state.filterOnUser = action.payload;
    },
  },
  extraReducers(builder) {
    builder
      .addCase(fetchBooking.pending, (state) => {
        state.status = "loading";
      })
      .addCase(fetchBooking.fulfilled, (state, action) => {
        if (action.payload) {
          state.activeBooking = action.payload;
        }
        state.status = "succeeded";
      })
      .addCase(fetchBooking.rejected, (state, action) => {
        if (action.error.message) {
          state.error = action.error.message;
        }
        state.status = "failed";
      });
  },
});

export const { resetBookingState, resetError, setFilterOnUser } =
  BookingSlice.actions;

export default BookingSlice.reducer;
