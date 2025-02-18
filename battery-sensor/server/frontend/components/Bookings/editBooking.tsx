import React, { Dispatch, useEffect, useState } from "react";
import { useSelector } from "react-redux";
import { useAppDispatch } from "../../features/hooks";
import { selectSelectedBoat } from "../../features/boat/boatSelector";
import { selectActiveBooking } from "../../features/booking/bookingSelector";
import { Box, Button, Input, Typography } from "@mui/material";
import useUpdateBooking from "../../hooks/bookings/editBookingMutation";
import { ErrorReturnType } from "./CreateBooking";
import CustomSnackBar from "../snackbar/CustomSnackbar";
import { resetBookingState } from "../../features/booking/bookingSlice";
import useCustomSnackbar from "../snackbar/useCustomSnackbar";

interface EditBookingProps {
  handleShow: () => void;
  handleEdit: () => void;
  startTime: string;
  endTime: string;
}

const actionDispatch = (dispatch: Dispatch<any>) => ({
  resetState: () => dispatch(resetBookingState()),
});

const EditBooking: React.FC<EditBookingProps> = ({
  handleEdit,
  handleShow,
  startTime,
  endTime,
}) => {
  const [editingEvent, setNewEvent] = useState({
    title: "",
    start: "",
    end: "",
  });
  const activeBooking = useSelector(selectActiveBooking);
  const boat = useSelector(selectSelectedBoat);
  const { resetState } = actionDispatch(useAppDispatch());
  const { mutate, isSuccess, isError, error } = useUpdateBooking();

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (isError && error) {
      openSnackbar((error as ErrorReturnType).response.data, "error");
    }
  }, [isError]);

  if (editingEvent.start == "" && editingEvent.end == "") {
    editingEvent.start = startTime;
    editingEvent.end = endTime;
  }

  const handleEditEvent = async () => {
    const b_ID = boat.boatID === 0 ? 1 : boat.boatID;
    const tempBooking = {
      startTime: editingEvent.start,
      endTime: editingEvent.end,
      status: "P",
      comment: activeBooking!.comment,
      userID: activeBooking!.bookingOwnerId,
      boatID: b_ID,
      isOfficial: activeBooking!.isOfficial,
    };

    mutate(tempBooking);
  };

  useEffect(() => {
    if (isSuccess) {
      resetState();
      handleEdit();
      handleShow();
    }
  }, [isSuccess]);

  return (
    <Box display="flex" flexDirection="column">
      <Box>
        <Typography> Rediger din booking </Typography>
        <Input
          className="input w-full max-w-xs mb-5"
          type="datetime-local"
          value={editingEvent.start}
          onChange={(e) =>
            setNewEvent({ ...editingEvent, start: e.target.value })
          }
        />
        <Input
          className="input w-full max-w-xs mb-5"
          type="datetime-local"
          value={editingEvent.end}
          onChange={(e) =>
            setNewEvent({ ...editingEvent, end: e.target.value })
          }
        />
      </Box>
      <Box display="flex" flexDirection="row" gap="12px" paddingBottom="20px">
        <Button variant="outlined" color="success" onClick={handleEditEvent}>
          Lagre booking
        </Button>
        <Button variant="outlined" onClick={handleEdit}>
          Gå tilbake
        </Button>
      </Box>
      <CustomSnackBar
        severity={snackbarSeverity}
        message={snackbarMessage}
        open={snackbarOpen}
        handleClose={closeSnackbar}
      />
    </Box>
  );
};

export default EditBooking;
