import { Box, Typography } from "@mui/material";
import ConnectedCard from "./ConnectedCard";
import KeyStatusCard from "./KeyStatusCard";
import LockBoxLockedCard from "./LockBoxLockedCard";
import AdminUnlockComponent from "./AdminUnlockComponent";

const StatusAdministration = () => {
  return (
    <Box width="100%" paddingBottom="24px">
      <Typography
        variant="h1"
        textAlign="center"
        fontSize="48px"
        marginY="18px"
      >
        Status
      </Typography>
      <Box
        width={["90vw", "60vw", "40vw", "35vw", "30vw", "25vw"]}
        marginX="auto"
      >
        <Box
          display="flex"
          flexDirection="column"
          justifyContent="center"
          gap={1}
          width="100%"
        >
          <ConnectedCard />
          <KeyStatusCard />
          <LockBoxLockedCard />
        </Box>
      </Box>
    </Box>
  );
};

export default StatusAdministration;
