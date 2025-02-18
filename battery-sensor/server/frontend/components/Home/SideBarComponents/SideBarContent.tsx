import { Box, Divider } from "@mui/material";
import BoatSelector from "./BoatSelector";
import ViewSelector from "./ViewSelector";
import PageSelector from "./PageSelector";
import LogoutButton from "./LogoutButton";

const SideBarContent = () => {
  return (
    <Box height="100vh">
      <BoatSelector />
      <PageSelector />
      <Divider />
      <ViewSelector />
      <Divider />
      <LogoutButton />
    </Box>
  );
};

export default SideBarContent;
