import { Box, Typography } from "@mui/material";
import AdminUnlockComponent from "./AdminUnlockComponent";

const OpenKeyAdministration = () => {
  return (
    <Box width="100%" paddingBottom="24px">
      <Typography
        variant="h1"
        textAlign="center"
        fontSize="48px"
        marginY="18px"
      >
        Åpne skap
      </Typography>

      <AdminUnlockComponent />
    </Box>
  );
};

export default OpenKeyAdministration;
