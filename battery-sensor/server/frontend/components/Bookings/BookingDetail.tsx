import React, { Dispatch, FC, useEffect, useState } from "react";
import {
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Typography,
} from "@mui/material";
import { useSelector } from "react-redux";
import { selectActiveBooking } from "../../features/booking/bookingSelector";
import { resetBookingState } from "../../features/booking/bookingSlice";
import { selectActiveUser } from "../../features/user/userSelector";
import EditBooking from "./editBooking";
import useCancelBookingMutation from "../../hooks/bookings/cancelBookingMutation";
import CustomSnackBar from "../snackbar/CustomSnackbar";
import { ErrorReturnType } from "./CreateBooking";
import useCustomSnackbar from "../snackbar/useCustomSnackbar";

interface BookingDetailProps {
  open: boolean;
  handleShow: () => void;
}

const BookingDetail: FC<BookingDetailProps> = ({ open, handleShow }) => {
  const activeBooking = useSelector(selectActiveBooking);
  const activeUser = useSelector(selectActiveUser);
  const [edit, setEdit] = useState(false);
  const isAdmin: boolean = activeUser.roles.includes("Admin");
  const [adminTriedToDelete, setAdminTriedToDelete] = useState<boolean>(false);
  const {
    mutate: cancelBooking,
    isSuccess,
    isError,
    error,
  } = useCancelBookingMutation();

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

  function parseDate(unformattedDate: string) {
    const date = new Date(unformattedDate);
    return date.toLocaleString("nb-NO");
  }

  function parseStatus(unformattedStatus: string) {
    switch (unformattedStatus) {
      case "D":
        return "Ferdig";
      case "P":
        return "Ikke startet";
      case "O":
        return "Startet";
      default:
        break;
    }
  }

  function handleClick() {
    if (
      activeUser.id !== activeBooking!.bookingOwnerId &&
      !adminTriedToDelete
    ) {
      setAdminTriedToDelete(true);
    } else {
      setAdminTriedToDelete(false);
      cancelBooking();
    }
  }

  useEffect(() => {
    if (isSuccess) {
      resetBookingState();
      handleShow();
    }
  }, [isSuccess]);

  function handleClose() {
    handleShow();
    setAdminTriedToDelete(false);
    setEdit(false);
  }

  function handleEdit() {
    setEdit(!edit);
  }

  return (
    <Dialog open={open} onClose={handleClose} fullWidth={true}>
      <DialogTitle>Booking #{activeBooking?.bookingID}</DialogTitle>
      <DialogContent>
        {!edit && (
          <>
            <Box className="flex-row flex content-center w-full justify-center">
              <Box className="w-6/12">
                <Typography variant="h6" className="mb-5">
                  Informasjon:
                </Typography>
                <Typography>
                  <b>Start: </b>
                  {parseDate(activeBooking!.startTime)}
                </Typography>
                <Typography>
                  <b>Slutt: </b>
                  {parseDate(activeBooking!.endTime)}
                </Typography>
                <Typography>
                  <b>Ferdig ladet: </b>
                  {parseDate(activeBooking!.chargingDone)}
                </Typography>
                <Typography>
                  <b>Status: </b>
                  {parseStatus(activeBooking!.status)}
                </Typography>
                <Typography>
                  <b>Båt: </b>
                  {activeBooking!.bookedBoat}
                </Typography>
                {activeBooking!.comment !== "" ? (
                  <Typography>
                    <b>Kommentar: </b>
                    {activeBooking!.comment}
                  </Typography>
                ) : null}
              </Box>
              <Box className="w-6/12">
                <Typography variant="h6" className="mb-5">
                  Kontaktinformasjon:
                </Typography>
                <Typography>
                  <b>Booket av: </b>
                  {activeBooking!.bookingOwner}
                </Typography>
                <Typography>
                  <b>Tlf:</b>{" "}
                  <a href={`tel:${activeBooking!.bookingOwnerPhone}`}>
                    {activeBooking!.bookingOwnerPhone}
                  </a>
                </Typography>
                <Typography>
                  <b>E-post:</b>{" "}
                  <a href={`mailto:${activeBooking!.bookingOwnerEmail}`}>
                    {activeBooking!.bookingOwnerEmail}
                  </a>
                </Typography>
              </Box>
            </Box>
            <Box className="flex justify-end m-2">
              {adminTriedToDelete && (
                <Box className="float-right text-sm self-center text-red-700 pl-2 text-center">
                  Er du sikker på at du vil kansellere{" "}
                  {activeBooking?.bookingOwner} sin booking?
                </Box>
              )}
              {(activeUser.id === activeBooking!.bookingOwnerId && !edit) ||
                isAdmin ? (
                <DialogActions className="float-right">
                  <Button
                    variant="contained"
                    sx={{
                      color: "white"
                    }}
                    style={{ backgroundColor: "#C42F30" }}
                    onClick={handleClick}
                  >
                    {adminTriedToDelete
                      ? "Ja, jeg er sikker"
                      : "Kanseller booking"}
                  </Button>
                  <Button onClick={handleEdit}>Rediger booking</Button>
                </DialogActions>
              ) : null}
            </Box>
          </>
        )}
        <Box className="flex items-center justify-center">
          {edit && (
            <EditBooking
              handleEdit={handleEdit}
              handleShow={handleShow}
              startTime={activeBooking!.startTime}
              endTime={activeBooking!.endTime}
            />
          )}
        </Box>
      </DialogContent>
      <CustomSnackBar
        severity={snackbarSeverity}
        message={snackbarMessage}
        open={snackbarOpen}
        handleClose={closeSnackbar}
      />
    </Dialog>
  );
};

export default BookingDetail;
