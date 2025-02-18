import {
  Box,
  Button,
  InputLabel,
  MenuItem,
  Select,
  TextField,
} from "@mui/material";
import { useEffect } from "react";
import { useForm } from "react-hook-form";
import useNewUserMutation from "../../../../hooks/users/newUserMutatation";
import { NewUserFormType } from "../../../../typings/formTypes";
import CustomSnackBar from "../../../snackbar/CustomSnackbar";
import useCustomSnackbar from "../../../snackbar/useCustomSnackbar";

const NewUserForm = () => {
  const { register, handleSubmit, getValues, setValue, reset } =
    useForm<NewUserFormType>();
  const { mutate, isSuccess, isError } = useNewUserMutation();

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  const onSubmit = (data: NewUserFormType) => {
    data.isActive = true;
    mutate(data);
    reset();
  };

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Brukeren ble lagt til", "success");
    }
  }, [isSuccess]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke legge til ny bruker", "error");
    }
  }, [isError]);

  return (
    <Box width="60%" marginX="auto">
      <form onSubmit={handleSubmit(onSubmit)}>
        <Box
          display="flex"
          flexDirection="column"
          gap="8px"
          marginX="auto"
          className="lg:w-8/12 xl:w-6/12"
        >
          <TextField label="Fullt navn" {...register("Name")} required />
          <TextField
            label="Telefon"
            {...register("Phone")}
            required
            type="tel"
          />
          <TextField
            label="Epost"
            {...register("Email")}
            required
            type="email"
          />
          <InputLabel id="selectRoleLabel">Rolle</InputLabel>
          <Select
            {...register("Role")}
            labelId="selectRoleLabel"
            id="selectRole"
            label="Rolle"
            required
            defaultValue="Member"
            value={getValues("Role")}
            onChange={(event, _) => setValue("Role", event.target.value)}
          >
            <MenuItem value={"Member"}>Medlem</MenuItem>
            <MenuItem value={"Admin"}>Administrator</MenuItem>
          </Select>

          <Button
            className="max-w-xs self-center"
            type="submit"
            variant="outlined"
          >
            Legg til bruker
          </Button>
        </Box>
      </form>
      <CustomSnackBar
        severity={snackbarSeverity}
        message={snackbarMessage}
        open={snackbarOpen}
        handleClose={closeSnackbar}
      />
    </Box>
  );
};

export default NewUserForm;
