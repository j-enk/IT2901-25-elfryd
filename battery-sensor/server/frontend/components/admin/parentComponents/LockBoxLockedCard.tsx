import { getLockBox } from "../../../hooks/chip/pingLockBox";
import { queryClient } from "../../../pages/_app";
import { useEffect, useState } from "react";
import CustomSnackBar from "../../snackbar/CustomSnackbar";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";
import LockBoxStatusCard from "./LockBoxStatusCard";

const ConnectedCard = () => {
  const { data: lockBoxData, isError: lockBoxIsError } = getLockBox("1");

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  const [open, setOpen] = useState(false);
  const [formattedDate, setFormattedDate] = useState("");

  useEffect(() => {
    if (lockBoxData) {
      setOpen(lockBoxData.isOpen);
      const date = new Date(lockBoxData.lastResponse);
      setFormattedDate(
        new Intl.DateTimeFormat("nb-No", {
          year: "numeric",
          month: "long",
          day: "numeric",
          hour: "numeric",
          minute: "numeric",
          second: "numeric",
          hour12: false,
        }).format(date)
      );
    }
  }, [lockBoxData]);

  const refreshStatus = () => {
    queryClient.invalidateQueries("pingLockBoxQuery");
    queryClient.invalidateQueries("getLockBoxQuery");
  };

  useEffect(() => {
    if (lockBoxIsError) {
      openSnackbar("Kunne ikke hente skapstatus", "error");
    }
  }, [lockBoxIsError]);

  return (
    <>
      <LockBoxStatusCard
        title="Dørstatus"
        subTitle={`Siste signal fra skapet: ${formattedDate}`}
        isActive={open}
        chipLabel={open ? "Åpent" : "Låst"}
        refreshFunction={refreshStatus}
      />
      <CustomSnackBar
        open={snackbarOpen}
        message={snackbarMessage}
        severity={snackbarSeverity}
        handleClose={closeSnackbar}
      />
    </>
  );
};

export default ConnectedCard;
