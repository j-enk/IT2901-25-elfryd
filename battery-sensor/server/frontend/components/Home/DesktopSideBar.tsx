import { Box } from "@mui/material";
import SideBarContent from "./SideBarComponents/SideBarContent";

const DesktopSideBar = () => {
  return (
    <Box minWidth="240px" borderRight="1px solid black" px={2} py={2}>
      <SideBarContent />
    </Box>
  );
};

export default DesktopSideBar;
