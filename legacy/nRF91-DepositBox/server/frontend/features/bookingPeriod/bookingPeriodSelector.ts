import { RootState } from "../../services/store";

export const selectThisPeriod = (state: RootState) => state.bookingPeriod.thisPeriod;
export const selectBookingPeriods = (state: RootState) => state.bookingPeriod.bookingPeriods;
export const selectBookingPeriodStatus = (state: RootState) => state.bookingPeriod.status;
export const selectBookingPeriodError = (state: RootState) => state.bookingPeriod.error;