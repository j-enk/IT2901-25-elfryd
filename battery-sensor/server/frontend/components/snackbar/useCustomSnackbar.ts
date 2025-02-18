import { useState } from "react";

type snackbarSeverityType = "success" | "error" | "info" | "warning";

type UseCustomSnackBarReturnType = {
  snackbarOpen: boolean;
  snackbarMessage: string;
  snackbarSeverity: snackbarSeverityType;
  openSnackbar: (message: string, severity: snackbarSeverityType) => void;
  closeSnackbar: () => void;
};

const useCustomSnackbar = (): UseCustomSnackBarReturnType => {
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState("");
  const [snackbarSeverity, setSnackbarSeverity] = useState<
    "success" | "error" | "info" | "warning"
  >("success");

  const openSnackbar = (
    message: string,
    severity: "success" | "error" | "info" | "warning"
  ) => {
    setSnackbarMessage(message);
    setSnackbarSeverity(severity);
    setSnackbarOpen(true);
  };

  const closeSnackbar = () => {
    setSnackbarOpen(false);
  };

  return {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } as UseCustomSnackBarReturnType;
};

export default useCustomSnackbar;
