import React, { useEffect } from "react";
import { DialogContentText } from "@mui/material";
import UnlockButton from "../Buttons/UnlockButton";
import useUserHasActiveBooking from "../../hooks/bookings/useUserHasActiveBooking";
import openLockBoxMutation from "../../hooks/lockbox/openLockBoxMutation";
import GenericModal from "../modal/GenericModal";

interface UnlockComponentProps {
  handleShow: () => void;
  open: boolean;
}
const UnlockComponent: React.FC<UnlockComponentProps> = ({
  handleShow,
  open,
}) => {
  const { data: currentHasBooking, isSuccess: activeBookingSuccess } =
    useUserHasActiveBooking();

  const {
    mutate: openLockBox,
    isLoading: openLockBoxLoading,
    isSuccess: openLockBoxSuccess,
    isError: openLockBoxError,
    reset: resetOpenLockBox,
  } = openLockBoxMutation();

  useEffect(() => {
    if (
      activeBookingSuccess &&
      currentHasBooking &&
      currentHasBooking.status == "P"
    ) {
      handleShow();
    }
  }, [activeBookingSuccess, currentHasBooking]);

  useEffect(() => {
    resetOpenLockBox();
  }, [open]);

  return (
    <GenericModal
      open={open}
      handleShow={handleShow}
      title="Lås opp nøkkelskapet!"
    >
      <UnlockButton
        handleShow={handleShow}
        openLockBox={openLockBox}
        openLockBoxLoading={openLockBoxLoading}
        openLockBoxSuccess={openLockBoxSuccess}
      />
      {openLockBoxSuccess && (
        <DialogContentText mt="16px">
          Funket det ikke? Prøv igjen om noen sekunder! Det kan også ta litt tid
          før skapet låses opp.
        </DialogContentText>
      )}
      {openLockBoxError && (
        <DialogContentText className="text-red-500">
          Noe gikk galt. Prøv igjen senere.
        </DialogContentText>
      )}
    </GenericModal>
  );
};

export default UnlockComponent;
