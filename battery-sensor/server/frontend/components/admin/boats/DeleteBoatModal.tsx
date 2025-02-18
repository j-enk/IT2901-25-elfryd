import { Box, Button, Modal, Portal, Typography } from "@mui/material";
import { useEffect, useState } from "react";
import theme from "../../../theme";
import DeleteForeverOutlinedIcon from "@material-ui/icons/DeleteForeverOutlined";
import CustomSnackBar from "../../snackbar/CustomSnackbar";
import deleteBoatMutation from "../../../hooks/boats/deleteBoatMutation";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";

interface DeteBoatModalProps {
  boatID: number;
  trashcan: boolean;
}

export const DeleteBoatModal = (props: DeteBoatModalProps) => {
  const { boatID, trashcan } = props;
  const [open, setOpen] = useState(false);
  const handleOpen = () => setOpen(true);
  const handleClose = () => setOpen(false);
  const { mutate, isError, isSuccess } = deleteBoatMutation();

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Båten ble slettet", "success");
    }
  }, [isSuccess]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Noe gikk galt, kunne ikke slette båten", "error");
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

  return (
    <Box>
      <Button
        onClick={handleOpen}
        variant="outlined"
        color="error"
        sx={{ width: "100%" }}
      >
        {trashcan && <DeleteForeverOutlinedIcon />}
        {!trashcan && "Slett båt"}
      </Button>
      <Modal open={open} onClose={handleClose}>
        <Box sx={style}>
          <Typography variant="h6" component="h2">
            Er du sikker på at du vil slette båten? OBS! Det å slette en båt kan
            ha uforutsette konsekvenser!
          </Typography>

          <Box display="flex" gap="8px" marginTop="16px">
            <Button onClick={handleClose} variant="outlined" color="info">
              Nei, ta meg tilbake
            </Button>
            <Button
              onClick={() => mutate(boatID)}
              variant="outlined"
              color="error"
            >
              Ja, jeg er sikker
            </Button>
          </Box>
        </Box>
      </Modal>
      <Portal>
        <CustomSnackBar
          severity={snackbarSeverity}
          message={snackbarMessage}
          open={snackbarOpen}
          handleClose={closeSnackbar}
        />
      </Portal>
    </Box>
  );
};
