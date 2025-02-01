import { RootState } from "../../services/store";

export const selectSelectedBoat = (state: RootState) => state.boat.selectedBoat;