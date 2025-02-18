import { Box, Button, Modal, Typography } from "@mui/material";
import { useEffect, useState } from "react";
import deleteUserMutation from "../../../hooks/users/deleteUser";
import theme from "../../../theme";
import DeleteForeverOutlinedIcon from "@material-ui/icons/DeleteForeverOutlined";
import CustomSnackBar from "../../snackbar/CustomSnackbar";
import { useSelector } from "react-redux";
import { selectActiveUser } from "../../../features/user/userSelector";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";

interface deleteProps {
  id: string;
  trashcan: boolean;
}

const DeleteUserModal = (props: deleteProps) => {
  const profile = useSelector(selectActiveUser);
  const [open, setOpen] = useState(false);
  const { mutate, isError, isSuccess } = deleteUserMutation();
  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Brukeren ble slettet", "success");
    }
  }, [isSuccess]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Noe gikk galt, kunne ikke slette bruker", "error");
    }
  }, [isError]);

  const style = {
    position: "absolute" as "absolute",
    top: "50%",
    left: "50%",
    transform: "translate(-50%, -50%)",
    [theme.breakpoints.down("sm")]: {
      width: "80%",
    },
    [theme.breakpoints.up("md")]: {
      width: "50%",
    },
    [theme.breakpoints.up("lg")]: {
      width: "30%",
    },
    [theme.breakpoints.up("xl")]: {
      width: "25%",
    },

    bgcolor: "background.paper",
    border: "2px solid #000",
    borderRadius: "4px",
    boxShadow: 24,
    p: 4,
  };

  if (profile.id == props.id) {
    return <></>;
  }

  const handleOpen = () => setOpen(true);
  const handleClose = () => setOpen(false);

  return (
    <Box>
      <Button
        onClick={handleOpen}
        variant="outlined"
        color="error"
        sx={{ width: "100%" }}
      >
        {props.trashcan && <DeleteForeverOutlinedIcon />}
        {!props.trashcan && "Slett bruker"}
      </Button>
      <Modal open={open} onClose={handleClose}>
        <Box sx={style}>
          <Typography variant="h6" component="h2">
            Er du sikker på at du vil slette brukeren? OBS! Det å slette
            brukeren vil også slette alle tilhørende reservasjoner.
          </Typography>

          <Box display="flex" gap="8px" marginTop="16px">
            <Button onClick={handleClose} variant="outlined" color="info">
              Nei, ta meg tilbake
            </Button>
            <Button
              onClick={() => mutate(props.id)}
              variant="outlined"
              color="error"
            >
              Ja, jeg er sikker
            </Button>
          </Box>
        </Box>
      </Modal>
      <CustomSnackBar
        severity={snackbarSeverity}
        message={snackbarMessage}
        open={snackbarOpen}
        handleClose={closeSnackbar}
      />
    </Box>
  );
};

export default DeleteUserModal;
