import { RootState } from "../../services/store";

export const selectLockboxes = (state: RootState) => state.lockbox.lockboxes;
export const selectSelectedLockBox = (state: RootState) =>
  state.lockbox.activeLockBox;
export const selectLockStatus = (state: RootState) => state.lockbox.status;
export const selectLockError = (state: RootState) => state.lockbox.error;
