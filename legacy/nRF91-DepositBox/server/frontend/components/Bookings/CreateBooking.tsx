import React, { useEffect, useState } from "react";
import { Box, Button, Input, Tab, Tabs, TextField } from "@mui/material";
import { useForm } from "react-hook-form";
import { NewBookingFormType } from "../../typings/formTypes";
import useNewBookingMutation from "../../hooks/bookings/newBookingMutation";
import { useSelector } from "react-redux";
import { selectActiveUser } from "../../features/user/userSelector";
import GenericModal from "../modal/GenericModal";
import CustomSnackBar from "../snackbar/CustomSnackbar";
import useCustomSnackbar from "../snackbar/useCustomSnackbar";

interface CreateBookingProps {
  open: boolean;
  handleShow: () => void;
  newEvent: { comment: string; start: string; end: string };
}

export type ErrorReturnType = {
  response: {
    data: string;
  };
};

const CreateBookingComp: React.FC<CreateBookingProps> = (
  props: CreateBookingProps
) => {
  const { open, handleShow, newEvent } = props;
  const { mutate, isError, isSuccess, error } = useNewBookingMutation();
  const user = useSelector(selectActiveUser);
  const [currentTab, setCurrentTab] = useState(0);

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

  const handleTabChange = (_: React.SyntheticEvent, newValue: number) => {
    setCurrentTab(newValue);
  };

  const { register, handleSubmit } = useForm<NewBookingFormType>({
    defaultValues: {
      from: newEvent.start,
      to: newEvent.end,
      isOfficial: false,
      comment: newEvent.comment,
    },
    values: {
      from: newEvent.start,
      to: newEvent.end,
      comment: newEvent.comment,
      isOfficial: false,
    },
  });

  const handleAddEvent = async (data: NewBookingFormType) => {
    mutate({
      ...data,
      isOfficial: currentTab === 1,
    });
  };

  useEffect(() => {
    if (isSuccess) {
      handleShow();
    }
  }, [isSuccess]);

  return (
    <>
      <GenericModal handleShow={handleShow} open={open} title="Book båten">
        <>
          {user.roles.includes("Admin") && (
            <Tabs
              value={currentTab}
              onChange={handleTabChange}
              centered
              sx={{ marginY: 2, width: "80%" }}
            >
              <Tab label="Book båt" />
              <Tab label="Vedlikehold" />
            </Tabs>
          )}
          <form onSubmit={handleSubmit(handleAddEvent)}>
            <Box display="flex" flexDirection="column" alignItems="center">
              <Input
                type="datetime-local"
                {...register("from")}
                required
                className="input w-full mb-5"
              />
              <Input
                type="datetime-local"
                {...register("to")}
                required
                className="input w-full mb-5"
              />

              {currentTab === 1 && (
                <>
                  <TextField
                    {...register("comment")}
                    className="input w-full mb-5 px-2"
                    label="Legg ved kommentar"
                    required
                  />
                </>
              )}

              <Box className="mt-4">
                <Button type="submit" className="btn ml-5">
                  Book
                </Button>
              </Box>
            </Box>
          </form>
        </>
      </GenericModal>
      <CustomSnackBar
        severity={snackbarSeverity}
        message={snackbarMessage}
        open={snackbarOpen}
        handleClose={closeSnackbar}
      />
    </>
  );
};

export default CreateBookingComp;
