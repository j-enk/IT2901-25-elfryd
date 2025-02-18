import { Box, Button, MenuItem, Select, TextField } from "@mui/material";
import { Dispatch, SetStateAction, useEffect } from "react";
import { useForm } from "react-hook-form";
import editUser from "../../../../hooks/users/editUser";
import { EditUserFormType } from "../../../../typings/formTypes";
import { UserType } from "../../../../typings/userType";
import useCustomSnackbar from "../../../snackbar/useCustomSnackbar";
import CustomSnackBar from "../../../snackbar/CustomSnackbar";

export interface UserListItemProps {
  user: UserType;
  edit: boolean;
  setEdit: Dispatch<SetStateAction<boolean>>;
}

const EditUserForm = (props: UserListItemProps) => {
  const { user, edit, setEdit } = props;
  const { id, name, phone, email, roles, isActive } = user;
  const { register, handleSubmit, getValues, setValue } =
    useForm<EditUserFormType>();
  const { mutate, isSuccess, isError } = editUser();

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  const onSubmit = (data: EditUserFormType) => {
    data.id = id;
    data.isActive = isActive;
    if (data.Phone == undefined) {
      data.Phone = phone;
    }
    if (data.Email == undefined) {
      data.Email = email;
    }
    if (data.Name == undefined) {
      data.Name = name;
    }
    if (data.Role == undefined) {
      data.Role = roles[0];
    }
    mutate(data);
  };

  const handleBack = () => {
    setEdit(false);
    closeSnackbar();
  };

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Brukeren ble lagret", "success");
    }
  }, [isSuccess]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke redigere bruker", "error");
    }
  }, [isError]);

  return (
    <>
      <form onSubmit={handleSubmit(onSubmit)}>
        <Box
          display="flex"
          flexDirection="column"
          gap="12px"
          justifyContent="space-around"
        >
          <TextField
            label={"Navn"}
            defaultValue={name}
            focused
            disabled={!edit}
            {...register("Name")}
          ></TextField>
          <TextField
            label={"Telefonnummer"}
            defaultValue={phone}
            focused
            disabled={!edit}
            {...register("Phone")}
          ></TextField>
          <TextField
            label={"Epost"}
            defaultValue={email}
            focused
            disabled={!edit}
            {...register("Email")}
          ></TextField>
          <Select
            {...register("Role")}
            label={"Rolle"}
            defaultValue={roles[0]}
            value={getValues("Role")}
            onChange={(event, _) => setValue("Role", event.target.value)}
            disabled={!edit}
          >
            <MenuItem value={"Member"}>Medlem</MenuItem>
            <MenuItem value={"Admin"}>Administrator</MenuItem>
          </Select>

          {edit && (
            <Button type="submit" variant="outlined" color="success">
              Lagre endring
            </Button>
          )}
          {edit && (
            <Button
              onClick={() => handleBack()}
              variant="outlined"
              color="info"
            >
              Gå tilbake
            </Button>
          )}
        </Box>
      </form>
      <CustomSnackBar
        open={snackbarOpen}
        message={snackbarMessage}
        severity={snackbarSeverity}
        handleClose={closeSnackbar}
      />
    </>
  );
};

export default EditUserForm;
