import { queryClient } from "../../../pages/_app";
import { useEffect, useState } from "react";
import {
  generateKeyReport,
  latestKeyReport,
} from "../../../hooks/chip/keyReport";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";
import CustomSnackBar from "../../snackbar/CustomSnackbar";
import LockBoxStatusCard from "./LockBoxStatusCard";

const KeyStatusCard = () => {
  const { data, isError } = latestKeyReport("1");
  const {
    mutate: generateKeyReportMutation,
    isError: generateKeyReportIsError,
  } = generateKeyReport();

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
    if (data) {
      setActive(data.status == 1);

      const date = new Date(data.timeStamp);

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
  }, [data]);

  useEffect(() => {
    generateKeyReportMutation("1");
  }, []);

  const refreshStatus = () => {
    generateKeyReportMutation("1");
    queryClient.invalidateQueries("latestKeyReportQuery");
  };

  useEffect(() => {
    if (generateKeyReportIsError) {
      openSnackbar("Kunne ikke spørre etter nøkkelstatus", "error");
    }
  }, [generateKeyReportIsError]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke hente nøkkelstatus", "error");
    }
  }, [isError]);

  return (
    <>
      <LockBoxStatusCard
        title="Nøkkelstatus"
        subTitle={`Siste nøkkelsignal: ${formattedDate}`}
        isActive={active}
        chipLabel={active ? "I skapet" : "Ikke i skapet"}
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

export default KeyStatusCard;
