import { Box, Button } from "@mui/material";
import { Dispatch } from "react";
import { logOut } from "../../../features/auth/authSlice";
import { useAppDispatch } from "../../../features/hooks";
import LogoutIcon from "@mui/icons-material/Logout";
import { useCookies } from "react-cookie";

const actionDispatch = (dispatch: Dispatch<any>) => ({
  logout: () => dispatch(logOut()),
});

const LogoutButton = () => {
  const { logout } = actionDispatch(useAppDispatch());

  const [cookie, setCookie, removeCookie] = useCookies();

  const cookieOptions = {
    path: "/",
    domain:
      process.env.NODE_ENV === "production"
        ? process.env.COOKIE_DOMAIN_PROD
        : process.env.COOKIE_DOMAIN_DEV,
  };

  const handleLogOut = () => {
    logout();
    removeCookie("token", cookieOptions);
    removeCookie("refresh_token", cookieOptions);
    window.location.replace(`${window.location.origin}`);
  };
  return (
    <Box p={1} my={2}>
      <Box width="75%" mx="auto">
        <Button
          startIcon={<LogoutIcon />}
          onClick={handleLogOut}
          variant="outlined"
          color="error"
          fullWidth
        >
          Logg ut
        </Button>
      </Box>
    </Box>
  );
};

export default LogoutButton;
