import React, { useState } from "react";
import { styled } from "@mui/material/styles";
import { Box, Drawer, CssBaseline, Divider, IconButton } from "@mui/material";
import MenuOpenIcon from "@mui/icons-material/MenuOpen";
import BoatSelector from "./SideBarComponents/BoatSelector";
import ViewSelector from "./SideBarComponents/ViewSelector";
import MenuIcon from "@mui/icons-material/Menu";
import SideBarContent from "./SideBarComponents/SideBarContent";

const drawerWidth = 240;

const DrawerHeader = styled("div")(({ theme }) => ({
  display: "flex",
  alignItems: "center",
  padding: theme.spacing(0, 1),
  // necessary for content to be below app bar
  ...theme.mixins.toolbar,
  justifyContent: "flex-end",
}));

const SideBar  = () => {
  const [open, setOpen] = useState(false);
  const handleShow = () => setOpen(!open);

  return (
    <>
      <IconButton
        size="large"
        edge="start"
        color="inherit"
        aria-label="Åpne meny"
        sx={{ mr: 2 }}
        onClick={handleShow}
      >
        <MenuIcon />
      </IconButton>

      <Box sx={{ display: "flex" }}>
        <CssBaseline />
        <Drawer
          sx={{
            width: drawerWidth,
            flexShrink: 0,
            "& .MuiDrawer-paper": {
              width: drawerWidth,
              boxSizing: "border-box",
            },
          }}
          variant="persistent"
          anchor="left"
          open={open}
        >
          <DrawerHeader>
            <IconButton onClick={handleShow}>
              <MenuOpenIcon />
            </IconButton>
          </DrawerHeader>
          <Divider />
         <SideBarContent/> 
        </Drawer>
      </Box>
    </>
  );
};

export default SideBar;
