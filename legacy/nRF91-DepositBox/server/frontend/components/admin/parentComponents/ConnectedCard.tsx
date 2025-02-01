import { getLockBox, pingQuery } from "../../../hooks/chip/pingLockBox";
import { queryClient } from "../../../pages/_app";
import { useEffect, useState } from "react";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";
import CustomSnackBar from "../../snackbar/CustomSnackbar";
import LockBoxStatusCard from "./LockBoxStatusCard";

const isConnectionOutDated = (date: string) => {
  const formattedDate = new Date(date);
  const now = new Date();

  const timeDiff = Math.abs(formattedDate.getTime() - now.getTime());
  const minutesDiff = Math.floor(timeDiff / (1000 * 60));

  return minutesDiff > 1;
};

const ConnectedCard = () => {
  const { data, isError } = pingQuery("1");
  const { data: lockBoxData, isError: lockBoxIsError } = getLockBox("1");

  const [active, setActive] = useState(false);
  const [formattedDate, setFormattedDate] = useState("");

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (lockBoxData) {
      setActive(!isConnectionOutDated(lockBoxData.lastResponse));
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
    if (lockBoxIsError || isError) {
      openSnackbar("Kunne ikke hente skapstatus", "error");
    }
  }, [lockBoxIsError]);

  return (
    <>
      <LockBoxStatusCard
        title="Nøkkelboks"
        subTitle={`Siste signal fra skapet: ${formattedDate}`}
        isActive={active}
        chipLabel={active ? "Tilkoblet" : "Ikke tilkoblet"}
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
