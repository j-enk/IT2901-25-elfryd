import { RootState } from "../../services/store";
export const selectActiveUser = (state: RootState) => state.user.activeUser;
export const selectUserStatus = (state: RootState) => state.user.status;
export const selectUserError = (state: RootState) => state.user.error;