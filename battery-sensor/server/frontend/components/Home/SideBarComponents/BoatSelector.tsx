import React, { useEffect } from "react";
import {
  Box,
  FormControl,
  InputLabel,
  MenuItem,
  Select,
  SelectChangeEvent,
  Typography,
  Divider
} from "@mui/material";
import { useSelector } from "react-redux";
import { selectSelectedBoat } from "../../../features/boat/boatSelector";
import { Dispatch } from "@reduxjs/toolkit";
import { setSelectedBoat } from "../../../features/boat/boatSlice";
import { useAppDispatch } from "../../../features/hooks";
import { resetBookingState } from "../../../features/booking/bookingSlice";
import useBoats from "../../../hooks/boats/useBoats";
import { BoatType } from "../../../typings/boatType";
import Loading from "../../loading/Loading";

const actionDispatch = (dispatch: Dispatch<any>) => ({
  setSelected: (boat: BoatType) => dispatch(setSelectedBoat(boat)),
  resetBookingStatus: () => dispatch(resetBookingState()),
});

const BoatSelector = () => {
  const selectedBoat = useSelector(selectSelectedBoat);
  const { data, isLoading } = useBoats("", 1, 999);
  const { setSelected, resetBookingStatus } = actionDispatch(useAppDispatch());

  useEffect(() => {
    if (data && data.items.length > 0 && selectedBoat.boatID === 0) {
      setSelected(data.items[0]);
    }
  }, [data]);

  if (isLoading || !data)
    return (
      <Box p={1} display="flex" justifyContent="center">
        <Loading />
      </Box>
    );

  const handleChange = (event: SelectChangeEvent<number>) => {
    const boat = data.items.find((boat) => boat.boatID === event.target.value);
    resetBookingStatus();
    setSelected(boat!);
  };
  const boats = data.items;

  if (boats.length <= 1) return <></>;

  return (
    <>
      <Box className="p-2">
        <Typography variant="h6" className="mb-4">
          Velg båt
        </Typography>
        <FormControl fullWidth className="mx-1 my-2">
          <InputLabel id="label_boat_selector">Båt</InputLabel>
          <Select
            labelId="label_boat_selector"
            id="boat_selector"
            value={selectedBoat.boatID}
            label="Båt"
            onChange={handleChange}
          >
            {boats.map((boat) => (
              <MenuItem key={boat.boatID} value={boat.boatID}>
                {boat.name}
              </MenuItem>
            ))}
          </Select>
        </FormControl>
      </Box>
      <Divider />
    </>
  );
};

export default BoatSelector;
