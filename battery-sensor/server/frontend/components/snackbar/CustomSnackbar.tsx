import { Alert, Portal, Snackbar } from "@mui/material";

type CustomSnackBarProps = {
  severity: "success" | "error" | "info" | "warning";
  open: boolean;
  handleClose: () => void;
  message: string;
};

const CustomSnackBar = (props: CustomSnackBarProps) => {
  const { severity, open, handleClose, message } = props;

  return (
    <Portal>
      <Snackbar open={open} autoHideDuration={6000} onClose={handleClose}>
        <Alert onClose={handleClose} severity={severity} sx={{ width: "100%" }}>
          {message}
        </Alert>
      </Snackbar>
    </Portal>
  );
};

export default CustomSnackBar;
