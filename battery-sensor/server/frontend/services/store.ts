import {
  Action,
  configureStore,
  ThunkAction,
  ThunkDispatch,
} from "@reduxjs/toolkit";
import BoatReducer from "../features/boat/boatSlice";
import BookingReducer from "../features/booking/bookingSlice";
import BookingPeriodReducer from "../features/bookingPeriod/bookingPeriodSlice";
import LockBoxReducer from "../features/lockbox/lockboxSlice";
import UserReducer from "../features/user/userSlice";
import AuthReducer from "../features/auth/authSlice";

export const store = configureStore({
  reducer: {
    boat: BoatReducer,
    booking: BookingReducer,
    bookingPeriod: BookingPeriodReducer,
    lockbox: LockBoxReducer,
    user: UserReducer,
    auth: AuthReducer,
  },
});

export type AppDispatch = typeof store.dispatch;
export type RootState = ReturnType<typeof store.getState>;
export type ThunkAppDispatch = ThunkDispatch<RootState, void, Action>;
export type AppThunk<ReturnType = void> = ThunkAction<
  ReturnType,
  RootState,
  unknown,
  Action<string>
>;