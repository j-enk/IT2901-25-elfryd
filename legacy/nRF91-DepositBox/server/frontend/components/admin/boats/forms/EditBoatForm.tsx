import {
  Box,
  Button,
  InputLabel,
  MenuItem,
  Portal,
  Select,
  TextField,
} from "@mui/material";
import { Dispatch, SetStateAction, useEffect, useState } from "react";
import { useForm } from "react-hook-form";
import { EditBoatFormType } from "../../../../typings/formTypes";
import { BoatType } from "../../../../typings/boatType";
import { useSelector } from "react-redux";
import { selectLockboxes } from "../../../../features/lockbox/lockboxSelector";
import editBoatMutation from "../../../../hooks/boats/editBoatMutation";
import CustomSnackBar from "../../../snackbar/CustomSnackbar";
import useCustomSnackbar from "../../../snackbar/useCustomSnackbar";

export interface UserListItemProps {
  boat: BoatType;
  edit: boolean;
  setEdit: Dispatch<SetStateAction<boolean>>;
}

const EditBoatForm = (props: UserListItemProps) => {
  const { boat, edit, setEdit } = props;

  const { register, handleSubmit, getValues, setValue } =
    useForm<EditBoatFormType>();

  const { mutate, isSuccess, isError } = editBoatMutation();
  const lockboxes = useSelector(selectLockboxes);

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  const onSubmit = (data: EditBoatFormType) => {
    data.BoatID = boat.boatID;
    if (data.Name == undefined) data.Name = boat.name;
    if (data.ChargingTime == undefined) data.ChargingTime = boat.chargingTime;
    if (data.AdvanceBookingLimit == undefined)
      data.AdvanceBookingLimit = boat.advanceBookingLimit;
    if (data.MaxBookingLimit == undefined)
      data.MaxBookingLimit = boat.maxBookingLimit;
    if (data.LockBoxID == undefined) data.LockBoxID = boat.lockBoxID;

    mutate(data);
    setEdit(false);
  };

  const handleBack = () => {
    setEdit(false);
  };

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Endringene ble lagret", "success");
    }
  }, [isSuccess]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Noe gikk galt under redigering av båt", "error");
    }
  }, [isError]);

  return (
    <form onSubmit={handleSubmit(onSubmit)}>
      <Box
        display="flex"
        flexDirection="column"
        gap="12px"
        justifyContent="space-around"
      >
        <TextField
          label={"Navn"}
          defaultValue={boat.name}
          focused
          disabled={!edit}
          {...register("Name")}
        ></TextField>
        <TextField
          type="number"
          label={"Ladetid"}
          defaultValue={boat.chargingTime}
          focused
          disabled={!edit}
          {...register("ChargingTime")}
        ></TextField>
        <TextField
          type="number"
          label={"Forhåndsbooking"}
          defaultValue={boat.advanceBookingLimit}
          focused
          disabled={!edit}
          {...register("AdvanceBookingLimit")}
        ></TextField>
        <TextField
          type="number"
          label={"Maks bookinglengde"}
          defaultValue={boat.maxBookingLimit}
          focused
          disabled={!edit}
          {...register("MaxBookingLimit")}
        ></TextField>

        <InputLabel id="selectLockboxLabel">Nøkkelskap ID</InputLabel>
        <Select
          labelId="selectLockboxLabel"
          label={"Nøkkelskap ID"}
          {...register("LockBoxID")}
          defaultValue={boat.lockBoxID}
          value={getValues("LockBoxID")}
          onChange={(event, _) =>
            setValue("LockBoxID", Number(event.target.value))
          }
          disabled={!edit}
        >
          {lockboxes.map((lockbox) => (
            <MenuItem value={lockbox.lockBoxID}>{lockbox.lockBoxID}</MenuItem>
          ))}
        </Select>

        {edit && (
          <Button type="submit" variant="outlined" color="success">
            Lagre endring
          </Button>
        )}
        {edit && (
          <Button onClick={() => handleBack()} variant="outlined" color="info">
            Gå tilbake
          </Button>
        )}
      </Box>
      <Portal>
        <CustomSnackBar
          severity={snackbarSeverity}
          message={snackbarMessage}
          open={snackbarOpen}
          handleClose={closeSnackbar}
        />
      </Portal>
    </form>
  );
};

export default EditBoatForm;
