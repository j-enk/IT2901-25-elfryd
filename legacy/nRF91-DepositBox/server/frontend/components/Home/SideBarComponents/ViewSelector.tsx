import React from "react";
import {
  Box,
  FormControlLabel,
  Radio,
  RadioGroup,
  Typography,
} from "@mui/material";
import { useSelector } from "react-redux";
import { selectFilterOnUser } from "../../../features/booking/bookingSelector";
import { Dispatch } from "@reduxjs/toolkit";
import {
  resetBookingState,
  setFilterOnUser,
} from "../../../features/booking/bookingSlice";
import { useAppDispatch } from "../../../features/hooks";
import { useRouter } from "next/router";

const actionDispatch = (dispatch: Dispatch<any>) => ({
  setFilter: (filter: boolean) => dispatch(setFilterOnUser(filter)),
  resetState: () => dispatch(resetBookingState()),
});

const ViewSelector: React.FC = () => {
  const { setFilter, resetState } = actionDispatch(useAppDispatch());
  const isTrue = useSelector(selectFilterOnUser);
  const router = useRouter();
  const isOnAdminPage = router.pathname.split("/")[1] === "admin";

  const handleChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    switch (event.target.value) {
      case "true":
        setFilter(true);
        break;
      case "false":
        setFilter(false);
        break;
      default:
        break;
    }
    resetState();
  };
  if (isOnAdminPage) return <></>;

  return (
    <Box className="p-2">
      <Typography variant="h6" className="mb-4">
        Filtrer kalender
      </Typography>
      <RadioGroup onChange={handleChange} value={isTrue}>
        <FormControlLabel value={false} control={<Radio />} label="Vis alle" />
        <FormControlLabel
          value={true}
          control={<Radio />}
          label="Vis kun mine "
        />
      </RadioGroup>
    </Box>
  );
};

export default ViewSelector;
