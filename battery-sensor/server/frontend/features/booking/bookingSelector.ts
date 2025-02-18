import { RootState } from "../../services/store";

export const selectBookingStatus = (state: RootState) => state.booking.status;
export const selectBookingError = (state: RootState) => state.booking.error;
export const selectFilterOnUser = (state: RootState) =>
  state.booking.filterOnUser;
export const selectActiveBooking = (state: RootState) =>
  state.booking.activeBooking;
