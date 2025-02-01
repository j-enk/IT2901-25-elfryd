import React from "react";
import { Box, Button, CircularProgress } from "@mui/material";
import { useSelector } from "react-redux";
import { selectSelectedLockBox } from "../../features/lockbox/lockboxSelector";
import { green } from "@mui/material/colors";
import { Lock, LockOpen } from "@mui/icons-material/";

interface UnlockButtonProps {
  handleShow: () => void;
  openLockBox: (lockBoxID: number) => void;
  openLockBoxLoading: boolean;
  openLockBoxSuccess: boolean;
}

const UnlockButton: React.FC<UnlockButtonProps> = (
  props: UnlockButtonProps
) => {
  const { handleShow, openLockBox, openLockBoxLoading, openLockBoxSuccess } =
    props;
  const activeLockBox = useSelector(selectSelectedLockBox);

  const buttonSx = {
    backgroundColor: "white",
    color: "#000",
    "&:hover": {
      color: "#fff",
    },
    ...(openLockBoxSuccess && {
      bgcolor: "#4caf50 !important",
      "&:hover": {
        bgcolor: green[700],
      },
    }),
  };

  const handleButtonClick = () => {
    openLockBox(activeLockBox.lockBoxID);
  };

  return (
    <Box sx={{ m: 1, position: "relative", mt: 4 }}>
      <Button
        variant="contained"
        sx={buttonSx}
        disabled={openLockBoxLoading}
        onClick={handleButtonClick}
        endIcon={openLockBoxSuccess ? <LockOpen /> : <Lock />}
      >
        {openLockBoxSuccess ? "Lås opp igjen" : "Lås opp"}
      </Button>
      {openLockBoxLoading && (
        <CircularProgress
          size={24}
          sx={{
            color: green[500],
            position: "absolute",
            top: "50%",
            left: "50%",
            marginTop: "-12px",
            marginLeft: "-12px",
          }}
        />
      )}
    </Box>
  );
};

export default UnlockButton;
