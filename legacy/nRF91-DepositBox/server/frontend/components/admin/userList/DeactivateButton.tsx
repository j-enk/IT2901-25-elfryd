import { Button, Portal } from "@mui/material";
import { useEffect } from "react";
import { useSelector } from "react-redux";
import { selectActiveUser } from "../../../features/user/userSelector";
import editUser from "../../../hooks/users/editUser";
import { EditUserFormType } from "../../../typings/formTypes";
import { UserType } from "../../../typings/userType";
import CustomSnackBar from "../../snackbar/CustomSnackbar";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";

interface DeactivateButtonProps {
  user: UserType;
}

const DeactivateButton = (props: DeactivateButtonProps) => {
  const profile = useSelector(selectActiveUser);
  const { mutate, isSuccess, isError } = editUser();
  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (isError) {
      openSnackbar(
        "Noe gikk galt, kunne ikke endre status til bruker",
        "error"
      );
    }
  }, [isError]);

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Statusen til brukeren ble endret!", "success");
    }
  }, [isSuccess]);

  const handleDeactivate = () => {
    const userForm: EditUserFormType = {
      Name: props.user.name,
      Email: props.user.email,
      Phone: props.user.phone,
      Role: props.user.roles[0],
      id: props.user.id,
      isActive: !props.user.isActive,
      bookingIDs: [],
    };
    mutate(userForm);
  };

  if (profile.id == props.user.id) {
    return <></>;
  }

  return (
    <>
      <Button
        onClick={() => handleDeactivate()}
        color="warning"
        variant="outlined"
      >
        {props.user.isActive && <p>Deaktiver bruker</p>}
        {!props.user.isActive && <p>Aktiver bruker</p>}
      </Button>

      <Portal>
        <CustomSnackBar
          severity={snackbarSeverity}
          message={snackbarMessage}
          open={snackbarOpen}
          handleClose={closeSnackbar}
        />
      </Portal>
    </>
  );
};

export default DeactivateButton;
